#include <assert.h>
#include "FPGA_common.h"
#include "LMS7002M.h"
#include <ciso646>
#include "Logger.h"
#include <complex>
#include <errno.h>
#include <string.h>
#include <queue>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <poll.h>

#include "TRXLooper_PCIE.h"
#include "LitePCIe.h"
#include "Profiler.h"


static bool showStats = false;
static const int statsPeriod_ms = 1000; // at 122.88 MHz MIMO, fpga tx pkt counter overflows every 272ms

typedef std::chrono::steady_clock perfClock;
using namespace std::chrono;

namespace lime {

class AvgRmsCounter
{
public:
    AvgRmsCounter() : counter(0), avgAccumulator(0), rmsAccumulator(0), min(1e16), max(1e-16) {};

    void Add(double value)
    {
        avgAccumulator += value;
        rmsAccumulator += value*value;
        ++counter;
        if(value < min)
            min = value;
        if(value > max)
            max = value;
    }
    void GetResult(double& avg, double& rms)
    {
        if(counter == 0)
            return;
        avg = avgAccumulator/(double)counter;
        rms = sqrt(rmsAccumulator/(double)counter);
        avgAccumulator = 0;
        rmsAccumulator = 0;
        counter = 0;
    }

    double Min()
    {
        auto temp = min;
        min = 1e16;
        return temp;
    }
    double Max()
    {
        auto temp = max;
        max = 1e-16;
        return temp;
    }
private:
    int32_t counter;
    double avgAccumulator;
    double rmsAccumulator;
    double min;
    double max;
};

static inline int64_t ts_to_us(int64_t fs, int64_t ts)
{
    int n, r;
    n = (ts / fs);
    r = (ts % fs);
    return (int64_t)n * 1000000 + (((int64_t)r * 1000000) / fs);
}

template<typename T>
inline static T clamp(T value, T low, T high)
{
    assert(low <= high);
    return value < low ? low : (value > high ? high : value);
}

TRXLooper_PCIE::TRXLooper_PCIE(LitePCIe *rxPort, LitePCIe *txPort, FPGA *f, LMS7002M *chip, uint8_t moduleIndex)
    : TRXLooper(f, chip, moduleIndex), rxPort(rxPort), txPort(txPort)
{
    mMaxBufferSize = DMA_BUFFER_SIZE;
    mRxPacketsToBatch = 1;
    mTxPacketsToBatch = 1;
}

TRXLooper_PCIE::~TRXLooper_PCIE() {}

void TRXLooper_PCIE::Setup(const lime::SDRDevice::StreamConfig &config)
{
    float combinedSampleRate = std::max(config.txCount, config.rxCount)*config.hintSampleRate;
    int batchSize = 7; // should be good enough for most cases
    // for high data rates e.g 16bit ADC/DAC 2x2 MIMO @ 122.88Msps = ~1973 MB/s
    // need to batch as many packets as possible into transfer buffers
    if (combinedSampleRate != 0)
    {
        batchSize = combinedSampleRate/61.44e6;
        batchSize = std::min(batchSize, 4);//int(DMA_BUFFER_SIZE/sizeof(FPGA_DataPacket)));
        batchSize = std::max(1, batchSize);
    }

    if (config.hintSampleRate)
        mRxPacketsToBatch = 2*config.hintSampleRate/30.72e6;
    else
        mRxPacketsToBatch = 4;
    mTxPacketsToBatch = 8;
    //printf("Batch size %i\n", batchSize);

    fpga->WriteRegister(0xFFFF, 1 << chipId);
    fpga->StopStreaming();
    uint16_t startAddress = 0x7FE1 + (chipId*5);
    // reset Tx received/dropped packets counters
    uint32_t addrs[] = {startAddress, startAddress, startAddress};
    uint32_t values[] = {0, 3, 0};
    fpga->WriteRegisters(addrs, values, 3);

    TRXLooper::Setup(config);
}

void TRXLooper_PCIE::Start()
{
    fpga->WriteRegister(0xFFFF, 1 << chipId);
    TRXLooper::Start();
}

void TRXLooper_PCIE::TransmitPacketsLoop()
{
    const bool mimo = std::max(mConfig.txCount, mConfig.rxCount) > 1;
    const bool compressed = mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I12;
    const int samplesInPkt = (mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 1020 : 1360) / std::max(mConfig.rxCount, mConfig.txCount);
    const int packetSize = sizeof(FPGA_DataPacket);

    if(!txPort->IsOpen())
        throw std::runtime_error("TransmitPacketsLoop tx data port not open\n");

    LitePCIe::DMAInfo dma = txPort->GetDMAInfo();
    uint8_t* dmaMem = dma.txMemory;

    if(mConfig.extraConfig && mConfig.extraConfig->txMaxPacketsInBatch != 0)
        mTxPacketsToBatch = mConfig.extraConfig->txMaxPacketsInBatch;

    mTxPacketsToBatch = clamp((int)mTxPacketsToBatch, 1, (int)(dma.bufferSize/packetSize));

    float bufferTimeDuration = float(samplesInPkt*mTxPacketsToBatch) / mConfig.hintSampleRate;
    if(mCallback_logMessage)
    {
        char msg[256];
        sprintf(msg, "Stream%i samplesInTxPkt:%i maxTxPktInBatch:%i, batchSizeInTime:%gus\n",
            chipId, samplesInPkt, mTxPacketsToBatch, bufferTimeDuration*1e6);
        mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
    }

    const int outDMA_BUFFER_SIZE = packetSize * mTxPacketsToBatch;
    assert(outDMA_BUFFER_SIZE <= dma.bufferSize);
    const int irqPeriod = 16;
    const int maxDMAqueue = dma.bufferCount-16;

    // Initialize DMA
    txPort->TxDMAEnable(true);

    int64_t totalBytesSent = 0; //for data rate calculation
    int retryWrites = 0;
    int packetsSent = 0;
    int totalPacketSent = 0;
    uint32_t maxFIFOlevel = 0;
    int64_t lastTS = 0;

    struct PendingWrite {
        int64_t id;
        uint8_t* data;
        int32_t offset;
        int32_t size;
    };
    std::queue<PendingWrite> pendingWrites;

    std::vector<uint8_t*> dmaBuffers(dma.bufferCount);
    for(int i=0; i<dmaBuffers.size();++i)
        dmaBuffers[i] = dmaMem+dma.bufferSize*i;

    int stagingBufferIndex = 0;
    int underrun = 0;
    float packetsIn = 0;

    // prefill packets pool
    for(int i=0; i<768; ++i)
        txPacketsPool.push(new StagingPacketType(samplesInPkt*mTxPacketsToBatch));

    StagingPacketType* srcPkt = nullptr;

    TxBufferManager<StagingPacketType> output(mimo, compressed);
    output.Reset(dmaBuffers[0], outDMA_BUFFER_SIZE);

    bool outputReady = false;

    AvgRmsCounter txTSAdvance;
    AvgRmsCounter transferSize;

    mThreadsReady.fetch_add(1);

    // thread ready for work, just wait for stream enable
    {
        std::unique_lock<std::mutex> lk(streamMutex);
        while (!mStreamEnabled && !terminateTx.load(std::memory_order_relaxed))
            streamActive.wait_for(lk, milliseconds(100));
        lk.unlock();
    }

    auto t1 = perfClock::now();
    auto t2 = t1;

    LitePCIe::DMAState state;
    state.swIndex = 0;
    while (terminateTx.load(std::memory_order_relaxed) == false) {
        // check if DMA transfers have completed
        state = txPort->GetTxDMAState();

        // process pending transactions
        while(!pendingWrites.empty() && !terminateTx.load(std::memory_order_relaxed))
        {
            PendingWrite &dataBlock = pendingWrites.front();
            if (state.hwIndex > dataBlock.id)
            {
                totalBytesSent += dataBlock.size;
                pendingWrites.pop();
            }
            else
                break;
        }

        // get input data
        maxFIFOlevel = std::max(maxFIFOlevel, txIn.size());

        // collect and transform samples data to output buffer
        while (!outputReady && output.hasSpace() && !terminateTx.load(std::memory_order_relaxed))
        {
            if(!srcPkt)
            {
                if(txIn.pop(&srcPkt, true, 100))
                    packetsIn += (float)srcPkt->size()/samplesInPkt;
                else
                    break;
            }

            const bool doFlush = output.consume(srcPkt);
            if(srcPkt->empty())
            {
                txPacketsPool.push(srcPkt, true, 100);
                srcPkt = nullptr;
            }
            if(doFlush)
            {
                outputReady = true;
                break;
            }
        }
        bool canSend = stagingBufferIndex - state.hwIndex < maxDMAqueue;
        if(!canSend && (mConfig.extraConfig == nullptr || mConfig.extraConfig->usePoll))
        {
            txPort->WaitTx();
            state = txPort->GetTxDMAState();
            canSend = stagingBufferIndex - state.hwIndex < maxDMAqueue;
        }
        // send output buffer if possible
        if(outputReady && canSend)
        {
            PendingWrite wrInfo;
            wrInfo.id = stagingBufferIndex;
            wrInfo.offset = 0;
            wrInfo.size = output.size();
            wrInfo.data = output.data();
            // write or schedule write
            state.swIndex = stagingBufferIndex;
            state.bufferSize = wrInfo.size;
            state.genIRQ = (wrInfo.id % irqPeriod) == 0;

            msync(dmaBuffers[stagingBufferIndex % dma.bufferCount], state.bufferSize, MS_SYNC);
            FPGA_DataPacket* pkt = reinterpret_cast<FPGA_DataPacket*>(output.data());
            lastTS = pkt->counter;
            int64_t rxNow = rxLastTimestamp.load(std::memory_order_relaxed);
            const int64_t txAdvance = pkt->counter-rxNow;
            if(mConfig.hintSampleRate)
            {
                int64_t timeAdvance = ts_to_us(mConfig.hintSampleRate, txAdvance);
                txTSAdvance.Add(timeAdvance);
            }
            else
                txTSAdvance.Add(txAdvance);
            if(txAdvance <= 0)
            {
                ++underrun;
                // TODO: first packet in the buffer is already late, could just skip this
                // buffer transmission, but packets at the end of buffer might just still
                // make it in time.
                // outputReady = false;
                // output.Reset(dmaBuffers[stagingBufferIndex % DMA_BUFFER_COUNT], outDMA_BUFFER_SIZE);
                // continue;
            }

            int ret = txPort->SetTxDMAState(state);
            if(ret)
            {
                //printf("Failed to submit dma write (%i) %s\n", errno, strerror(errno));
                ++retryWrites;
                continue;
            }
            else
            {
                //printf("Sent sw: %li hw: %li, diff: %li\n", stagingBufferIndex, reader.hw_count, stagingBufferIndex-reader.hw_count);
                outputReady = false;
                pendingWrites.push(wrInfo);
                transferSize.Add(wrInfo.size);
                ++stagingBufferIndex;
                packetsSent += output.packetCount();
                totalPacketSent += output.packetCount();
                output.Reset(dmaBuffers[stagingBufferIndex % dma.bufferCount], outDMA_BUFFER_SIZE);
            }
        }

        t2 = perfClock::now();
        auto timePeriod = duration_cast<milliseconds>(t2 - t1).count();
        if (timePeriod >= statsPeriod_ms || terminateTx.load(std::memory_order_relaxed)) {
            t1 = t2;

            state = txPort->GetTxDMAState();
            double dataRate = 1000.0 * totalBytesSent / timePeriod;
            double avgTransferSize = 0, rmsTransferSize = 0;
            transferSize.GetResult(avgTransferSize, rmsTransferSize);

            double avgTxAdvance = 0, rmsTxAdvance = 0;
            txTSAdvance.GetResult(avgTxAdvance, rmsTxAdvance);

            if(showStats || mCallback_logMessage)
            {
                fpga->WriteRegister(0xFFFF, 1 << chipId);
                const uint16_t addr = 0x7FE2 + (chipId*5);
                uint32_t addrs[] = {addr, addr+1u};
                uint32_t values[2];
                fpga->ReadRegisters(addrs, values, 2);
                uint32_t fpgaTxPktIngressCount = (values[0] << 16) | values[1];

                char msg[512];
                snprintf(msg, sizeof(msg)-1, "%s Tx: %3.3f MB/s | FIFO:%i/%i/%i TS:%li avgBatch:%.0f retry:%i totalOut:%i(x%08X)-fpga(x%08X)=%i, shw:%u/%u(%+i) underrun:%i tsAdvance:%+.0f/%+.0f/%+.0f%s",
                    rxPort->GetPathName().c_str(),
                    dataRate / 1000000.0,
                    txIn.size(),
                    maxFIFOlevel,
                    txIn.max_size(),
                    lastTS,
                    avgTransferSize,
                    retryWrites,
                    totalPacketSent,
                    totalPacketSent&0xFFFFFFFF,
                    fpgaTxPktIngressCount,
                    totalPacketSent - fpgaTxPktIngressCount,
                    state.swIndex,
                    state.hwIndex,
                    state.swIndex-state.hwIndex,
                    underrun,
                    txTSAdvance.Min(),
                    avgTxAdvance,
                    txTSAdvance.Max(),
                    (mConfig.hintSampleRate ? "us" : "")
                );
                if(showStats)
                    printf("%s\n", msg);
                if(mCallback_logMessage)
                    mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
            }
            // hardware counter should be less than software's
            //assert(reader.hw_count < reader.sw_count);
            // {
            //     printf("HW (%li) reader should be behind SW (%li), diff: %li\n", reader.hw_count, reader.sw_count, reader.hw_count-reader.sw_count);
            // }
            packetsIn = 0;
            packetsSent = 0;
            totalBytesSent = 0;
            txDataRate_Bps.store((uint32_t)dataRate, std::memory_order_relaxed);
            retryWrites = 0;
            maxFIFOlevel = 0;
        }
    }

    // give some time for the dma transfers to complete, than check the fpga packets counter
    std::this_thread::sleep_for(milliseconds(10));
    state = txPort->GetTxDMAState();
    //printf("DMA reader %li/%li, diff: %li\n", reader.sw_count, reader.hw_count, reader.sw_count - reader.hw_count);

    txPort->TxDMAEnable(false);
    txDataRate_Bps.store(0, std::memory_order_relaxed);

    uint16_t addr = 0x7FE1 + chipId*5;
    //fpga->WriteRegister(addr, 0);
    uint32_t addrs[] = {addr+1u, addr+2u, addr+3u, addr+4u};
    uint32_t values[4];
    fpga->ReadRegisters(addrs, values, 4);
    const uint32_t fpgaTxPktIngressCount = (values[0] << 16) | values[1];
    const uint32_t fpgaTxPktDropCounter = (values[2] << 16) | values[3];
    if(mCallback_logMessage)
    {
        char msg[256];
        sprintf(msg, "Tx Loop totals: packets sent: %i (0x%08X) , FPGA packet counter: %i (0x%08X), diff: %i, FPGA tx drops: %i\n",
            totalPacketSent, totalPacketSent, fpgaTxPktIngressCount, fpgaTxPktIngressCount, (totalPacketSent&0xFFFFFFFF)-fpgaTxPktIngressCount, fpgaTxPktDropCounter
        );
        mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
    }
}

#include <unistd.h>
/** @brief Function dedicated for receiving data samples from board
    @param stream a pointer to an active receiver stream
*/
void TRXLooper_PCIE::ReceivePacketsLoop()
{
    if(!rxPort->IsOpen())
        throw std::runtime_error("ReceivePacketsLoop rx data port not open\n");

    rxLastTimestamp.store(0, std::memory_order_relaxed);
    const bool mimo = std::max(mConfig.txCount, mConfig.rxCount) > 1;
    const bool compressed = mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I12;
    const bool usePoll = mConfig.extraConfig ? mConfig.extraConfig->usePoll : true;
    const int chCount = std::max(mConfig.rxCount, mConfig.txCount);
    const int sampleSize = (mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 4 : 3); // sizeof IQ pair
    const int maxSamplesInPkt = 1024 / chCount;

    int requestSamplesInPkt = 256;//maxSamplesInPkt;
    if(mConfig.extraConfig && mConfig.extraConfig->rxSamplesInPacket != 0)
        requestSamplesInPkt = mConfig.extraConfig->rxSamplesInPacket;


    int samplesInPkt = clamp(requestSamplesInPkt, 64, maxSamplesInPkt);
    int payloadSize = requestSamplesInPkt * sampleSize * chCount;

    // iqSamplesCount must be N*16, or N*8 depending on device BUS width
    const uint32_t iqSamplesCount = (payloadSize/(sampleSize*2)) & ~0xF; //magic number needed for fpga's FSMs
    payloadSize = iqSamplesCount * sampleSize * 2;
    samplesInPkt = payloadSize / (sampleSize * chCount);

    const uint32_t packetSize = 16 + payloadSize;

    // request fpga to provide Rx packets with desired payloadSize
    //// Two writes are needed
    fpga->WriteRegister(0xFFFF, 1 << chipId);
    uint32_t requestAddr[] = {0x0019, 0x000E};
    uint32_t requestData[] = {packetSize, iqSamplesCount};
    fpga->WriteRegisters(requestAddr, requestData, 2);

    LitePCIe::DMAInfo dma = rxPort->GetDMAInfo();

    if(mConfig.extraConfig && mConfig.extraConfig->rxPacketsInBatch != 0)
        mRxPacketsToBatch = mConfig.extraConfig->rxPacketsInBatch;
    mRxPacketsToBatch = clamp((int)mRxPacketsToBatch, 1, (int)(dma.bufferSize/packetSize));

    std::vector<uint8_t*> dmaBuffers(dma.bufferCount);
    for(int i=0; i<dmaBuffers.size();++i)
        dmaBuffers[i] = dma.rxMemory+dma.bufferSize*i;

    const int readBlockSize = packetSize * mRxPacketsToBatch;

    int totalTxDrops = 0;
    int totalPacketsReceived = 0;
    int64_t totalBytesReceived = 0; //for data rate calculation
    int64_t lastTS = -samplesInPkt;

    SDRDevice::StreamStats stats;
    int txDrops = 0;
    int rxDiscontinuity = 0;
    int rxOverrun = 0; // fifo is full, data is arriving faster than PC can process
    int maxFIFOlevel = 0;
    int packetsOut = 0;
    int packetsRecv = 0;

    // prefill packets pool
    for(int i=0; i<768; ++i)
        rxPacketsPool.push(new StagingPacketType(samplesInPkt*mRxPacketsToBatch));
    StagingPacketType* rxStaging = nullptr;
    rxPacketsPool.pop(&rxStaging, true, 1000);

    lime::complex32f_t tempAbuffer[1360];
    lime::complex32f_t tempBbuffer[1360];

    int irqPeriod = 8;
    float bufferTimeDuration = 0;
    if (mConfig.hintSampleRate > 0)
    {
        bufferTimeDuration = float(samplesInPkt*mRxPacketsToBatch) / mConfig.hintSampleRate;
        irqPeriod = 80e-6 / bufferTimeDuration;
    }
    irqPeriod = clamp(irqPeriod, 1, 16);
    //printf("Buffer duration: %g us, irq: %i\n", bufferTimeDuration*1e6, irqPeriod);

    if(mCallback_logMessage)
    {
        char msg[256];
        sprintf(msg, "Stream%i usePoll:%i rxSamplesInPkt:%i rxPacketsInBatch:%i, DMA_ReadSize:%i, batchSizeInTime:%gus\n",
            chipId, usePoll ? 1 : 0, samplesInPkt, mRxPacketsToBatch, readBlockSize, bufferTimeDuration*1e6);
        mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
    }

    float expectedDataRateBps = 0;
    if (mConfig.hintSampleRate != 0)
        expectedDataRateBps = (mConfig.hintSampleRate/samplesInPkt) * (16 + samplesInPkt * sampleSize * chCount);

    rxPort->RxDMAEnable(true, readBlockSize, irqPeriod);

    mThreadsReady.fetch_add(1);
    //at this point FPGA has to be already configured to output samples
    int64_t realTS = 0;
    double latency = 0;
    int streamDelay = 0;

    // thread ready for work, just wait for stream enable
    {
        std::unique_lock<std::mutex> lk(streamMutex);
        while (!mStreamEnabled && !terminateRx.load(std::memory_order_relaxed))
            streamActive.wait_for(lk, milliseconds(100));
        lk.unlock();
    }

    auto t1 = perfClock::now();
    auto t2 = t1;
    auto pushT1 = t1;

    int64_t lastDMAcounter = 0;

    AvgRmsCounter pktPushTimes_us;
    AvgRmsCounter timeDiscrepancy_us;

    const int bufferCount = dma.bufferCount;

    uint32_t readIndex = 0;
    LitePCIe::DMAState state;
    state.swIndex = 0;
    while (terminateRx.load(std::memory_order_relaxed) == false) {
        state = rxPort->GetRxDMAState();
        uint32_t hwIndex = state.hwIndex;
        if(hwIndex != lastDMAcounter)
        {
            totalBytesReceived += (hwIndex - lastDMAcounter) * readBlockSize;
            lastDMAcounter = hwIndex;
        }

        int buffersAvailable = hwIndex - readIndex;
        if(buffersAvailable)
        {
            bool reportProblems = false;
            if(buffersAvailable >= bufferCount) // data overflow
            {
                readIndex = (hwIndex - 1); // reset to 1 buffer behind latest data
                ++rxOverrun;
                ++stats.overrun;
                reportProblems = true;
            }

            msync(dmaBuffers[readIndex % bufferCount], readBlockSize, MS_SYNC);

            const uint8_t* inBuffer = dmaBuffers[readIndex % bufferCount];
            const FPGA_DataPacket *firstPkt = reinterpret_cast<const FPGA_DataPacket*>(inBuffer);
            const int srcPktCount = readBlockSize / packetSize;
            totalPacketsReceived += srcPktCount;
            packetsRecv += srcPktCount;
            rxLastTimestamp.store(firstPkt->counter+samplesInPkt*srcPktCount, std::memory_order_relaxed);

            lime::complex32f_t *f32_dest[2] = {
                static_cast<lime::complex32f_t *>(tempAbuffer),
                static_cast<lime::complex32f_t *>(tempBbuffer)
            };

            for (int i=0; i<srcPktCount && !terminateRx.load(std::memory_order_relaxed); ++i) {
                const FPGA_DataPacket *srcPkt = reinterpret_cast<const FPGA_DataPacket*>(&inBuffer[i*packetSize]);
                int64_t ts = 0;
                uint8_t* ptr = (uint8_t*)srcPkt;
                ptr+=2;
                for(int j=0; j<6; ++j)
                    ts |= ((int64_t)ptr[j]) << (j*8);
                realTS = ts;// | srcPkt->payloadSizeMSB;
                //printf("RealTS: %08X\n    TS: %08X\n", realTS, srcPkt->counter);

                const int64_t samplesDiff = srcPkt->counter - lastTS;
                if (samplesDiff != samplesInPkt) {
                    // printf("Rx discontinuity: %li -> %li (diff: %li), bytesIn: %i, sw:%li hw:%li, %s\n",
                    //     lastTS, srcPkt->counter, samplesDiff, readBlockSize, readIndex, writer.hw_count,
                    //     (i == 0 ? "@ buffer start" : "withing buffer")
                    //     );
                    ++rxDiscontinuity;
                    ++stats.loss;
                    stats.timestamp = srcPkt->counter;
                    reportProblems = true;
                }
                if (srcPkt->txWasDropped()) {
                    //printf("Rx packet %i, TS: %li, tx was Droped: %i\n", totalPacketsReceived, srcPkt->counter, 1);
                    ++totalTxDrops;
                    ++txDrops;
                    ++stats.late;
                    stats.timestamp = srcPkt->counter;
                    reportProblems = true;
                }
                lastTS = srcPkt->counter;

                // parse samples data
                // TODO: parse packet directly to rxStaging memory
                if(!rxStaging)
                {
                    if(rxPacketsPool.pop(&rxStaging, true, 100))
                        rxStaging->Reset();
                }
                if(!rxStaging)
                    continue;
                rxStaging->timestamp = srcPkt->counter;
                const int samplesParsed = FPGA::FPGAPacketPayload2SamplesFloat(srcPkt->data, payloadSize, mimo, compressed, f32_dest);
                int samplesPushed = rxStaging->push(f32_dest, samplesParsed);

                if(rxStaging->isFull())
                {
                    if(rxOut.push(rxStaging, false))
                    {
                        auto pushT2 = perfClock::now();
                        auto pushPeriod = duration_cast<microseconds>(pushT2 - pushT1).count();
                        pktPushTimes_us.Add(pushPeriod);
                        pushT1 = pushT2;
                        maxFIFOlevel = std::max(maxFIFOlevel, (int)rxOut.size());
                        ++packetsOut;
                        rxStaging = nullptr;
                        if (rxPacketsPool.pop(&rxStaging, true, 100))
                            rxStaging->Reset();
                    }
                    else
                    {
                        ++rxOverrun;
                        ++stats.overrun;
                        reportProblems = true;
                        rxStaging->Reset();
                    }
                    const int samplesRemaining = samplesParsed - samplesPushed;
                    if(samplesRemaining)
                    {
                        f32_dest[0] += samplesPushed;
                        f32_dest[1] += samplesPushed;
                        rxStaging->timestamp = srcPkt->counter + samplesPushed;
                        int samplesPushed = rxStaging->push(f32_dest, samplesRemaining);
                    }
                }
            }
            auto streamDuration = duration_cast<microseconds>(perfClock::now()-steamClockStart).count();
            if(mConfig.hintSampleRate)
            {
                int64_t rxTsDuration = ts_to_us(mConfig.hintSampleRate, lastTS+samplesInPkt);
                streamDelay = streamDuration-rxTsDuration;
                timeDiscrepancy_us.Add(streamDelay);
            }
            readIndex = (readIndex + 1);// % bufferCount;
            state.swIndex = readIndex;
            rxPort->SetRxDMAState(state);
            // one callback for the entire batch
            if(reportProblems && mConfig.statusCallback)
                mConfig.statusCallback(&stats, mConfig.userData);
        }
        else if(usePoll)
        {
            rxPort->WaitRx();
        }

        t2 = perfClock::now();
        auto timePeriod = duration_cast<milliseconds>(t2 - t1).count();
        if (timePeriod >= statsPeriod_ms || terminateRx.load(std::memory_order_relaxed)) {
            t1 = t2;
            double dataRateBps = 1000.0 * totalBytesReceived / timePeriod;
            stats.dataRate_Bps = dataRateBps;

            const double dataRateMargin = expectedDataRateBps*0.02;
            if( expectedDataRateBps!=0 && abs(expectedDataRateBps - dataRateBps) > dataRateMargin && mCallback_logMessage)
            {
                char msg[256];
                sprintf(msg, "Rx%i Unexpected Rx data rate, should be: ~%g MB/s, got: %g MB/s, diff: %g", chipId, expectedDataRateBps/1e6, dataRateBps/1e6, (expectedDataRateBps - dataRateBps)/1e6);
                mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
            }
            if ( abs(streamDelay) > 2000 && mCallback_logMessage)
            {
                char msg[256];
                sprintf(msg, "\n\tDiscrepancy between PC clock and Rx timestamps: %ius", streamDelay);
                mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
            }
            stats.txDataRate_Bps = txDataRate_Bps.load(std::memory_order_relaxed);
            if(showStats || mCallback_logMessage)
            {
                double pushPeriodAvg, pushPeriodRms;
                pktPushTimes_us.GetResult(pushPeriodAvg, pushPeriodRms);
                state = rxPort->GetRxDMAState();
                char msg[512];
                snprintf(msg, sizeof(msg)-1, "%s Rx: %3.3f MB/s | FIFO:%i/%i/%i TS:%li/%li=%li overrun:%i loss:%i, txDrops:%i, shw:%u/%u(%+i), l:%.0f/%.0f/%.0fus, pcTime-RxTS:%ius",
                    rxPort->GetPathName().c_str(),
                    dataRateBps / 1000000.0,
                    rxOut.size(),
                    maxFIFOlevel,
                    rxOut.max_size(),
                    lastTS,
                    realTS,
                    realTS-lastTS,
                    rxOverrun,
                    rxDiscontinuity,
                    txDrops,
                    readIndex,//writer.sw_count,
                    state.hwIndex,
                    int(state.hwIndex-readIndex),
                    pushPeriodAvg,
                    pushPeriodRms,
                    pktPushTimes_us.Max(),
                    streamDelay
                );
                if(showStats)
                    printf("%s\n", msg);
                if(mCallback_logMessage)
                    mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
            }
            packetsRecv = 0;
            packetsOut = 0;
            totalBytesReceived = 0;
            rxDiscontinuity = 0;
            txDrops = 0;
            rxOverrun = 0;
            maxFIFOlevel = 0;
            rxDataRate_Bps.store((uint32_t)dataRateBps, std::memory_order_relaxed);
        }
    }

    rxPort->RxDMAEnable(false, readBlockSize, irqPeriod);

    rxDataRate_Bps.store(0, std::memory_order_relaxed);

    if(mCallback_logMessage)
    {
        char msg[256];
        sprintf(msg, "Rx loop totals: packets recv: %i, txDrops: %i\n", totalPacketsReceived, totalTxDrops);
        mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
    }
}

} // namespace lime
