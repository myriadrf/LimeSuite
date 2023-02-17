#include <assert.h>
#include "FPGA_common.h"
#include "limesuite/LMS7002M.h"
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
#include "limesuite/SDRDevice.h"
#include "LitePCIe.h"
#include "Profiler.h"
#include "DataPacket.h"
#include "SamplesPacket.h"

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
    : TRXLooper(f, chip, moduleIndex)
{
    mMaxBufferSize = 256;
    mRxPacketsToBatch = 1;
    mTxPacketsToBatch = 1;
    mRxArgs.port = rxPort;
    mTxArgs.port = txPort;
}

TRXLooper_PCIE::~TRXLooper_PCIE() {}

void TRXLooper_PCIE::Setup(const lime::SDRDevice::StreamConfig &config)
{
    if (config.rxCount > 0 && !mRxArgs.port->IsOpen())
        throw std::runtime_error("Rx data port not open\n");
    if (config.txCount > 0 && !mTxArgs.port->IsOpen())
        throw std::runtime_error("Tx data port not open\n");

    float combinedSampleRate = std::max(config.txCount, config.rxCount)*config.hintSampleRate;
    int batchSize = 7; // should be good enough for most cases
    // for high data rates e.g 16bit ADC/DAC 2x2 MIMO @ 122.88Msps = ~1973 MB/s
    // need to batch as many packets as possible into transfer buffers
    if (combinedSampleRate != 0)
    {
        batchSize = combinedSampleRate/61.44e6;
        batchSize = std::min(batchSize, 4);
        batchSize = std::max(1, batchSize);
    }

    if (config.hintSampleRate)
        mRxPacketsToBatch = 2*config.hintSampleRate/30.72e6;
    else
        mRxPacketsToBatch = 4;
    mTxPacketsToBatch = 4;
    //printf("Batch size %i\n", batchSize);

    fpga->WriteRegister(0xFFFF, 1 << chipId);
    fpga->StopStreaming();
    uint16_t startAddress = 0x7FE1 + (chipId*5);
    // reset Tx received/dropped packets counters
    uint32_t addrs[] = {startAddress, startAddress, startAddress};
    uint32_t values[] = {0, 3, 0};
    fpga->WriteRegisters(addrs, values, 3);

    mConfig = config;
    if (config.rxCount > 0)
        RxSetup();
    if (config.txCount > 0)
        TxSetup();

    TRXLooper::Setup(config);
}

void TRXLooper_PCIE::Start()
{
    fpga->WriteRegister(0xFFFF, 1 << chipId);
    TRXLooper::Start();
}

int TRXLooper_PCIE::TxSetup()
{
    txLastTimestamp.store(0, std::memory_order_relaxed);
    const bool usePoll = mConfig.extraConfig ? mConfig.extraConfig->usePoll : true;
    const int chCount = std::max(mConfig.rxCount, mConfig.txCount);
    const int sampleSize = (mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 4 : 3); // sizeof IQ pair
    const int samplesInPkt = (mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 1020 : 1360) / chCount;
    const int packetSize = sizeof(TxDataPacket);

    LitePCIe::DMAInfo dma = mTxArgs.port->GetDMAInfo();

    if(mConfig.extraConfig && mConfig.extraConfig->txMaxPacketsInBatch != 0)
        mTxPacketsToBatch = mConfig.extraConfig->txMaxPacketsInBatch;

    mTxPacketsToBatch = clamp((int)mTxPacketsToBatch, 1, (int)(dma.bufferSize/packetSize));

    std::vector<uint8_t*> dmaBuffers(dma.bufferCount);
    for(uint32_t i=0; i<dmaBuffers.size();++i)
        dmaBuffers[i] = dma.txMemory+dma.bufferSize*i;

    mTxArgs.buffers = std::move(dmaBuffers);
    mTxArgs.bufferSize = dma.bufferSize;
    mTxArgs.packetSize = packetSize;
    mTxArgs.packetsToBatch = mTxPacketsToBatch;
    mTxArgs.samplesInPacket = samplesInPkt;

    float bufferTimeDuration = float(samplesInPkt*mTxPacketsToBatch) / mConfig.hintSampleRate;
    if(mCallback_logMessage)
    {
        char msg[256];
        sprintf(msg, "Stream%i samplesInTxPkt:%i maxTxPktInBatch:%i, batchSizeInTime:%gus\n",
            chipId, samplesInPkt, mTxPacketsToBatch, bufferTimeDuration*1e6);
        mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
    }

    char name[64];
    sprintf(name, "Tx%i_memPool", chipId);
    const int upperAllocationLimit = sizeof(complex32f_t) * mTxPacketsToBatch * samplesInPkt * chCount + SamplesPacketType::headerSize;
    mTx.memPool = new MemoryPool(1024, upperAllocationLimit, 4096, name);
    return 0;
}

void TRXLooper_PCIE::TransmitPacketsLoop()
{
    const bool mimo = std::max(mConfig.txCount, mConfig.rxCount) > 1;
    const bool compressed = mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I12;

    const int usedBufferSize = mTxArgs.packetsToBatch * mTxArgs.packetSize;
    assert(usedBufferSize <= mTxArgs.bufferSize);
    const int irqPeriod = 8;

    const int32_t samplesInPkt = mTxArgs.samplesInPacket;
    const int32_t bufferCount = mTxArgs.buffers.size();
    const uint32_t overflowLimit = bufferCount-2*irqPeriod;
    const int32_t bufferSize = mTxArgs.bufferSize;
    const std::vector<uint8_t*> &dmaBuffers = mTxArgs.buffers;

    SDRDevice::StreamStats &stats = mTx.stats;

    auto fifo = mTx.fifo;

    // Initialize DMA
    mTxArgs.port->TxDMAEnable(true);

    int64_t totalBytesSent = 0; //for data rate calculation
    int retryWrites = 0;
    int packetsSent = 0;
    int totalPacketSent = 0;
    uint32_t maxFIFOlevel = 0;
    int64_t lastTS = 0;

    struct PendingWrite {
        uint32_t id;
        uint8_t* data;
        int32_t offset;
        int32_t size;
    };
    std::queue<PendingWrite> pendingWrites;

    // std::vector<uint8_t*> dmaBuffers(dma.bufferCount);
    // for(uint32_t i=0; i<dmaBuffers.size();++i)
    //     dmaBuffers[i] = dmaMem+dma.bufferSize*i;

    uint32_t stagingBufferIndex = 0;
    int underrun = 0;
    float packetsIn = 0;

    SamplesPacketType* srcPkt = nullptr;

    TxBufferManager<SamplesPacketType> output(mimo, compressed, mTxArgs.samplesInPacket);

    uint8_t tempBuffer[32768];
    //output.Reset(dmaBuffers[0], usedBufferSize);
    output.Reset(tempBuffer, usedBufferSize);

    bool outputReady = false;

    AvgRmsCounter txTSAdvance;
    AvgRmsCounter transferSize;

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
        state = mTxArgs.port->GetTxDMAState();

        // process pending transactions
        while(!pendingWrites.empty() && !terminateTx.load(std::memory_order_relaxed))
        {
            PendingWrite &dataBlock = pendingWrites.front();
            if (dataBlock.id - state.hwIndex > 255)
            {
                totalBytesSent += dataBlock.size;
                pendingWrites.pop();
            }
            else
                break;
        }

        // get input data
        maxFIFOlevel = std::max(maxFIFOlevel, fifo->size());

        // collect and transform samples data to output buffer
        while (!outputReady && output.hasSpace() && !terminateTx.load(std::memory_order_relaxed))
        {
            if(!srcPkt)
            {
                if(fifo->pop(&srcPkt, true, 100))
                    packetsIn += (float)srcPkt->size()/samplesInPkt;
                else
                    break;
            }

            const bool doFlush = output.consume(srcPkt);
            stats.packets += output.packetCount();
            if(srcPkt->empty())
            {
                mTx.memPool->Free(srcPkt);
                srcPkt = nullptr;
            }
            if(doFlush)
            {
                outputReady = true;
                break;
            }
        }
        uint16_t pendingBuffers = stagingBufferIndex - state.hwIndex;
        bool canSend = pendingBuffers < overflowLimit;
        if(!canSend && (mConfig.extraConfig == nullptr || mConfig.extraConfig->usePoll))
        {
            mTxArgs.port->WaitTx();
            state = mTxArgs.port->GetTxDMAState();
            canSend = stagingBufferIndex - state.hwIndex < overflowLimit;
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

            TxDataPacket* pkt = reinterpret_cast<TxDataPacket*>(output.data());
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
                ++stats.underrun;
                // TODO: first packet in the buffer is already late, could just skip this
                // buffer transmission, but packets at the end of buffer might just still
                // make it in time.
                // outputReady = false;
                // output.Reset(dmaBuffers[stagingBufferIndex % DMA_BUFFER_COUNT], usedBufferSize);
                // continue;
            }

            // DMA memory is write only, so for now form buffer in local storage and copy to dma
            // otherwise trying to read from the buffer will trigger Bus errors
            memcpy(dmaBuffers[stagingBufferIndex % bufferCount], output.data(), output.size());
            int ret = mTxArgs.port->SetTxDMAState(state);
            if(ret)
            {
                //printf("Failed to submit dma write (%i) %s\n", errno, strerror(errno));
                ++stats.overrun;
                continue;
            }
            else
            {
                //printf("Sent sw: %li hw: %li, diff: %li\n", stagingBufferIndex, reader.hw_count, stagingBufferIndex-reader.hw_count);
                outputReady = false;
                pendingWrites.push(wrInfo);
                transferSize.Add(wrInfo.size);
                ++stagingBufferIndex;
                stagingBufferIndex &= 0xFFFF;
                packetsSent += output.packetCount();
                totalPacketSent += output.packetCount();
                //output.Reset(dmaBuffers[stagingBufferIndex % bufferCount], bufferSize);
                output.Reset(tempBuffer, usedBufferSize);
            }
        }

        t2 = perfClock::now();
        auto timePeriod = duration_cast<milliseconds>(t2 - t1).count();
        if (timePeriod >= statsPeriod_ms || terminateTx.load(std::memory_order_relaxed)) {
            t1 = t2;

            state = mTxArgs.port->GetTxDMAState();
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
                snprintf(msg, sizeof(msg)-1, "%s Tx: %3.3f MB/s | TS:%li pkt:%i avgBatch:%.0f retry:%i totalOut:%i(x%08X)-fpga(x%08X)=%i, shw:%u/%u(%+i) underrun:%i tsAdvance:%+.0f/%+.0f/%+.0f%s",
                    mRxArgs.port->GetPathName().c_str(),
                    dataRate / 1000000.0,
                    lastTS,
                    stats.packets,
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
}

void TRXLooper_PCIE::TxTeardown()
{
    mTxArgs.port->TxDMAEnable(false);
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
        sprintf(msg, "Tx Loop totals: packets sent: %li (0x%08X) , FPGA packet counter: %li (0x%08X), diff: %i, FPGA tx drops: %i\n",
            mTx.stats.packets, mTx.stats.packets, fpgaTxPktIngressCount, fpgaTxPktIngressCount, (mTx.stats.packets&0xFFFFFFFF)-fpgaTxPktIngressCount, fpgaTxPktDropCounter
        );
        mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
    }
}

int TRXLooper_PCIE::RxSetup()
{
    rxLastTimestamp.store(0, std::memory_order_relaxed);
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

    LitePCIe::DMAInfo dma = mRxArgs.port->GetDMAInfo();

    if(mConfig.extraConfig && mConfig.extraConfig->rxPacketsInBatch != 0)
        mRxPacketsToBatch = mConfig.extraConfig->rxPacketsInBatch;
    mRxPacketsToBatch = clamp((int)mRxPacketsToBatch, 1, (int)(dma.bufferSize/packetSize));

    int irqPeriod = 8;
    float bufferTimeDuration = 0;
    if (mConfig.hintSampleRate > 0)
    {
        bufferTimeDuration = float(samplesInPkt*mRxPacketsToBatch) / mConfig.hintSampleRate;
        irqPeriod = 80e-6 / bufferTimeDuration;
    }
    irqPeriod = clamp(irqPeriod, 1, 16);

    if(mCallback_logMessage)
    {
        char msg[256];
        sprintf(msg, "Stream%i usePoll:%i rxSamplesInPkt:%i rxPacketsInBatch:%i, DMA_ReadSize:%i, batchSizeInTime:%gus\n",
            chipId, usePoll ? 1 : 0, samplesInPkt, mRxPacketsToBatch, mRxPacketsToBatch*packetSize, bufferTimeDuration*1e6);
        mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
    }

    float expectedDataRateBps = 0;
    if (mConfig.hintSampleRate != 0)
        expectedDataRateBps = (mConfig.hintSampleRate/samplesInPkt) * (16 + samplesInPkt * sampleSize * chCount);

    std::vector<uint8_t*> dmaBuffers(dma.bufferCount);
    for(uint32_t i=0; i<dmaBuffers.size();++i)
        dmaBuffers[i] = dma.rxMemory+dma.bufferSize*i;

    mRxArgs.buffers = std::move(dmaBuffers);
    mRxArgs.bufferSize = dma.bufferSize;
    mRxArgs.packetSize = packetSize;
    mRxArgs.packetsToBatch = mRxPacketsToBatch;
    mRxArgs.samplesInPacket = samplesInPkt;

    char name[64];
    sprintf(name, "Rx%i_memPool", chipId);
    const int upperAllocationLimit = sizeof(complex32f_t) * mRxPacketsToBatch * samplesInPkt * chCount + SamplesPacketType::headerSize;
    mRx.memPool = new MemoryPool(1024, upperAllocationLimit, 4096, name);
    return 0;
}

static void PrintStats(const char* prefix, SDRDevice::StreamStats& stats, TRXLooper_PCIE::TransferArgs& args, SDRDevice::LogCallbackType logCallback, uint32_t sw, uint32_t hw)
{
    // const double dataRateMargin = expectedDataRateBps*0.02;
    // if( expectedDataRateBps!=0 && abs(expectedDataRateBps - dataRateBps) > dataRateMargin && mCallback_logMessage)
    // {
    //     char msg[256];
    //     sprintf(msg, "Rx%i Unexpected Rx data rate, should be: ~%g MB/s, got: %g MB/s, diff: %g", chipId, expectedDataRateBps/1e6, dataRateBps/1e6, (expectedDataRateBps - dataRateBps)/1e6);
    //     mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
    // }
    // if ( abs(streamDelay) > 2000 && mCallback_logMessage)
    // {
    //     char msg[256];
    //     sprintf(msg, "\n\tDiscrepancy between PC clock and Rx timestamps: %ius", streamDelay);
    //     mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
    // }
    char msg[512];
    snprintf(msg, sizeof(msg)-1, "%s %s: %3.3f MB/s | TS:%li pkts:%i overrun:%i loss:%i  dma:%u/%u(%u)",
        args.port->GetPathName().c_str(),
        prefix,
        stats.dataRate_Bps / 1e6,
        stats.timestamp,
        stats.packets,
        stats.overrun,
        stats.loss,
        sw,
        hw,
        hw-sw
    );
    if(showStats)
        printf("%s\n", msg);
    if(logCallback)
        logCallback(SDRDevice::LogLevel::DEBUG, msg);
}

/** @brief Function dedicated for receiving data samples from board
    @param stream a pointer to an active receiver stream
*/
void TRXLooper_PCIE::ReceivePacketsLoop()
{
    DataConversion conversion;
    conversion.srcFormat = mConfig.linkFormat;
    conversion.destFormat = mConfig.format;
    conversion.channelCount = std::max(mConfig.txCount, mConfig.rxCount);

    const int32_t bufferCount = mRxArgs.buffers.size();
    const int32_t readSize = mRxArgs.packetSize * mRxArgs.packetsToBatch;
    const int32_t packetSize = mRxArgs.packetSize;
    const int32_t samplesInPkt = mRxArgs.samplesInPacket;
    const std::vector<uint8_t*> &dmaBuffers = mRxArgs.buffers;
    SDRDevice::StreamStats &stats = mRx.stats;
    auto fifo = mRx.fifo;

    const int32_t outputPktSize = SamplesPacketType::headerSize
        + mRxArgs.packetsToBatch * samplesInPkt
        * (mConfig.format == SDRDevice::StreamConfig::F32 ? sizeof(complex32f_t) : sizeof(complex16_t));

    // thread ready for work, just wait for stream enable
    {
        std::unique_lock<std::mutex> lk(streamMutex);
        while (!mStreamEnabled && !terminateRx.load(std::memory_order_relaxed))
            streamActive.wait_for(lk, milliseconds(100));
        lk.unlock();
    }

    uint8_t irqPeriod = 4;
    // Anticipate the overflow 2 interrupts early, just in case of missing an interrupt
    // Avoid situations where CPU and device is at the same buffer index
    // CPU reading while device writing creates coherency issues.
    const uint16_t overrunLimit = std::max(bufferCount-2*irqPeriod, bufferCount/2);
    mRxArgs.port->RxDMAEnable(true, readSize, irqPeriod);
    mRxArgs.cnt = 0;
    mRxArgs.sw = 0;
    mRxArgs.hw = 0;

    auto t1 = perfClock::now();
    auto t2 = t1;

    int32_t Bps = 0;

    LitePCIe::DMAState dma;
    int64_t lastHwIndex = 0;
    int64_t expectedTS = 0;
    SamplesPacketType* outputPkt = nullptr;
    while (terminateRx.load(std::memory_order_relaxed) == false)
    {
        if (!outputPkt)
            outputPkt = SamplesPacketType::ConstructSamplesPacket(mRx.memPool->Allocate(outputPktSize), samplesInPkt * mRxArgs.packetsToBatch, sizeof(complex32f_t));
        dma = mRxArgs.port->GetRxDMAState();
        if(dma.hwIndex != lastHwIndex)
        {
            const int bytesTransferred = uint16_t(dma.hwIndex - lastHwIndex) * readSize;
            Bps += bytesTransferred;
            stats.bytesTransferred += bytesTransferred;
            lastHwIndex = dma.hwIndex;
        }

        // print stats
        t2 = perfClock::now();
        auto timePeriod = duration_cast<milliseconds>(t2 - t1).count();
        if (timePeriod >= statsPeriod_ms)
        {
            t1 = t2;
            double dataRateBps = 1000.0 * Bps / timePeriod;
            stats.dataRate_Bps = dataRateBps;
            rxDataRate_Bps.store(dataRateBps, std::memory_order_relaxed);
            PrintStats("Rx", stats, mRxArgs, mCallback_logMessage, dma.swIndex, dma.hwIndex);
            Bps = 0;
        }

        uint16_t buffersAvailable = dma.hwIndex - dma.swIndex;

        // process received data
        bool reportProblems = false;
        if(buffersAvailable >= overrunLimit) // data overflow
        {
            //printf("Overrun: sw:%i(%i) hw:%i diff:%i pkt:%i,  newSw:%i(%i)\n", dma.swIndex, dma.swIndex%bufferCount, dma.hwIndex, buffersAvailable, stats.packets, (dma.hwIndex - 1), (dma.hwIndex - 1)%bufferCount);
            // jump CPU to 1 buffer behind hardware index, to avoid device starting to write into buffer being read by CPU
            dma.swIndex = (dma.hwIndex - 1);
            ++stats.loss;
        }

        if (!buffersAvailable)
        {
            if(!mConfig.extraConfig || (mConfig.extraConfig && mConfig.extraConfig->usePoll))
                mRxArgs.port->WaitRx();
            continue;
        }

        uint8_t* buffer = dmaBuffers[dma.swIndex % bufferCount];
        const RxDataPacket *pkt = reinterpret_cast<const RxDataPacket*>(buffer);
        if(outputPkt)
            outputPkt->timestamp = pkt->counter;

        const int srcPktCount = mRxArgs.packetsToBatch;
        for (int i=0; i<srcPktCount; ++i)
        {
            pkt = reinterpret_cast<const RxDataPacket*>(&buffer[packetSize*i]);
            if (pkt->counter - expectedTS != 0)
            {
                //printf("Loss: pkt:%i exp: %li, got: %li, diff: %li\n", stats.packets+i, expectedTS, pkt->counter, pkt->counter-expectedTS);
                ++stats.loss;
            }
            if (pkt->txWasDropped())
                ++mTx.stats.underrun;

            const int payloadSize = packetSize - 16;
            const int samplesProduced = Deinterleave(conversion, pkt->data, payloadSize, outputPkt);
            expectedTS = pkt->counter + samplesProduced;
        }
        stats.packets += srcPktCount;
        stats.timestamp = expectedTS;
        rxLastTimestamp.store(expectedTS, std::memory_order_relaxed);

        if (outputPkt && fifo->push(outputPkt, false))
        {
            //maxFIFOlevel = std::max(maxFIFOlevel, (int)rxFIFO.size());
            outputPkt = nullptr;
        }
        else
        {
            ++stats.overrun;
            if(outputPkt)
                outputPkt->Reset();
        }

        ++dma.swIndex;
        mRxArgs.port->SetRxDMAState(dma);

        mRxArgs.sw = dma.swIndex;
        mRxArgs.hw = dma.hwIndex;
        ++mRxArgs.cnt;
        // one callback for the entire batch
        if(reportProblems && mConfig.statusCallback)
            mConfig.statusCallback(&stats, mConfig.userData);
    }

    if(mCallback_logMessage)
    {
        char msg[256];
        sprintf(msg, "Rx%i: packetsIn: %li\n", chipId, stats.packets);
        mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
    }
}

void TRXLooper_PCIE::RxTeardown()
{
    mRxArgs.port->RxDMAEnable(false, mRxArgs.bufferSize, 1);
    rxDataRate_Bps.store(0, std::memory_order_relaxed);
}

} // namespace lime
