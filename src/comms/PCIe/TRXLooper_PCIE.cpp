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

#include "ConstellationDisplay.h"

#define ZEROED_MEM 0

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

static std::mutex gIOCTLmutex;

static int guarded_ioctl(int fd, unsigned long request, void* argp)
{
    //std::unique_lock<std::mutex> lock(gIOCTLmutex);
    return ioctl(fd, request, argp);
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
    mRxPacketsToBatch = 2*config.hintSampleRate/30.72e6;//4;//batchSize;
    mTxPacketsToBatch = 8;//mRxPacketsToBatch;
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

    if(!rxPort->IsOpen())
        throw std::runtime_error("ReceivePacketsLoop tx data port not open\n");

    const int fd = rxPort->GetFd();
    int ret;

    litepcie_ioctl_lock lockInfo;
    memset(&lockInfo, 0, sizeof(lockInfo));
    lockInfo.dma_reader_request = 1;
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_LOCK, &lockInfo);
    if(ret)
        throw std::runtime_error("IOCTL failed to lock DMA reader");

    litepcie_ioctl_mmap_dma_info mmap_dma_info;
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_MMAP_DMA_INFO, &mmap_dma_info);
    if(ret)
        throw std::runtime_error("TransmitLoop IOCTL failed to get DMA info\n");

    uint8_t* dmaMem = (uint8_t*)mmap(NULL, DMA_BUFFER_TOTAL_SIZE, PROT_WRITE, MAP_SHARED,
                       fd, mmap_dma_info.dma_tx_buf_offset);
    if (dmaMem == MAP_FAILED || dmaMem == nullptr)
        throw std::runtime_error("TransmitLoop failed to allocate memory\n");

    const int outDMA_BUFFER_SIZE = packetSize * mTxPacketsToBatch;
    assert(outDMA_BUFFER_SIZE <= DMA_BUFFER_SIZE);
    memset(dmaMem, 0, DMA_BUFFER_TOTAL_SIZE);
    const int irqPeriod = 16;
    const int maxDMAqueue = DMA_BUFFER_COUNT-16;

    // Initialize DMA
    litepcie_ioctl_dma_reader reader;
    reader.enable = 1;
    reader.irqFreq = irqPeriod;
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_READER, &reader);
    if(ret)
        throw std::runtime_error("TransmitLoop IOCTL failed to enable DMA reader");

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

    uint8_t* dmaBuffers[DMA_BUFFER_COUNT];
    for(int i=0; i<DMA_BUFFER_COUNT;++i)
        dmaBuffers[i] = dmaMem+DMA_BUFFER_SIZE*i;

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

    while (terminateTx.load(std::memory_order_relaxed) == false) {
        // check if DMA transfers have completed
        ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_READER, &reader);
        if (ret)
            throw std::runtime_error("TransmitLoop IOCTL failed to get DMA reader counters");

        // process pending transactions
        while(!pendingWrites.empty() && !terminateTx.load(std::memory_order_relaxed))
        {
            PendingWrite &dataBlock = pendingWrites.front();
            if (reader.hw_count > dataBlock.id)
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
        bool canSend = stagingBufferIndex - reader.hw_count < maxDMAqueue;
        if(!canSend)
        {
            // auto pt1 = perfClock::now();
            pollfd desc;
            desc.fd = fd;
            desc.events = POLLOUT;

            const int pollTimeout = 50;
            const int ret = poll(&desc, 1, pollTimeout);
            if (ret < 0)
                printf("TransmitLoop poll errno(%i) %s\n", errno, strerror(errno));

            int ret2 = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_READER, &reader);
            if (ret2)
                throw std::runtime_error("TransmitLoop IOCTL failed to get DMA reader counters");

            canSend = stagingBufferIndex - reader.hw_count < maxDMAqueue;

            // auto pt2 = perfClock::now();
            // auto pollDur = duration_cast<microseconds>(pt2 - pt1).count();
            //printf("Tx Poll %ius\n", pollDur);
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

            litepcie_ioctl_mmap_dma_update sub;
            sub.sw_count = stagingBufferIndex;
            sub.buffer_size = wrInfo.size;
            sub.genIRQ = (wrInfo.id % irqPeriod) == 0;
            assert(sub.buffer_size <= DMA_BUFFER_SIZE);
            assert(sub.buffer_size <= outDMA_BUFFER_SIZE);
            assert(sub.buffer_size > 0);

            msync(dmaBuffers[stagingBufferIndex % DMA_BUFFER_COUNT], sub.buffer_size, MS_SYNC);
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

            int ret = guarded_ioctl(fd, LITEPCIE_IOCTL_MMAP_DMA_READER_UPDATE, &sub);
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
                output.Reset(dmaBuffers[stagingBufferIndex % DMA_BUFFER_COUNT], outDMA_BUFFER_SIZE);
            }
        }

        t2 = perfClock::now();
        auto timePeriod = duration_cast<milliseconds>(t2 - t1).count();
        if (timePeriod >= statsPeriod_ms || terminateTx.load(std::memory_order_relaxed)) {
            t1 = t2;

            ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_READER, &reader);
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
                int len = snprintf(msg, sizeof(msg)-1, "%s Tx: %3.3f MB/s | FIFO:%i/%i/%i TS:%li avgBatch:%.0f retry:%i totalOut:%i(x%08X)-fpga(x%08X)=%i, shw:%li/%li(%+li) underrun:%i tsAdvance:%+.0f/%+.0f/%+.0f%s",
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
                    reader.sw_count,
                    reader.hw_count,
                    reader.sw_count-reader.hw_count,
                    underrun,
                    txTSAdvance.Min(),
                    avgTxAdvance,
                    txTSAdvance.Max(),
                    (mConfig.hintSampleRate ? "us" : "")
                );
                if(showStats)
                    printf("%s\n", msg);
                if(mCallback_logMessage)
                    mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg, len);
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
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_READER, &reader);
    //printf("DMA reader %li/%li, diff: %li\n", reader.sw_count, reader.hw_count, reader.sw_count - reader.hw_count);

    reader.enable = 0;
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_READER, &reader);
    if(ret)
        throw std::runtime_error("TransmitLoop IOCTL failed to stop DMA reader\n");

    lockInfo.dma_reader_request = 0;
    lockInfo.dma_reader_release = 1;
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_LOCK, &lockInfo);
    if (ret)
        throw std::runtime_error("TransmitLoop IOCTL failed to release DMA reader\n");
    txDataRate_Bps.store(0, std::memory_order_relaxed);

    //printf("Tx pending transactions: %lu\n", pendingWrites.size());

    uint16_t addr = 0x7FE1 + chipId*5;
    //fpga->WriteRegister(addr, 0);
    uint32_t addrs[] = {addr+1u, addr+2u, addr+3u, addr+4u};
    uint32_t values[4];
    fpga->ReadRegisters(addrs, values, 4);
    const uint32_t fpgaTxPktIngressCount = (values[0] << 16) | values[1];
    const uint32_t fpgaTxPktDropCounter = (values[2] << 16) | values[3];
    char msg[256];
    int len = sprintf(msg, "Tx Loop totals: packets sent: %i (0x%08X) , FPGA packet counter: %i (0x%08X), diff: %i, FPGA tx drops: %i\n",
        totalPacketSent, totalPacketSent, fpgaTxPktIngressCount, fpgaTxPktIngressCount, (totalPacketSent&0xFFFFFFFF)-fpgaTxPktIngressCount, fpgaTxPktDropCounter
        );
    if(mCallback_logMessage)
        mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg, len);
    munmap(dmaMem, mmap_dma_info.dma_tx_buf_size * mmap_dma_info.dma_tx_buf_count);
}
#include <unistd.h>
/** @brief Function dedicated for receiving data samples from board
    @param stream a pointer to an active receiver stream
*/
void TRXLooper_PCIE::ReceivePacketsLoop()
{
    const bool mimo = std::max(mConfig.txCount, mConfig.rxCount) > 1;
    const bool compressed = mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I12;

    rxLastTimestamp.store(0, std::memory_order_relaxed);
    const int chCount = std::max(mConfig.rxCount, mConfig.txCount);
    const int sampleSize = (mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 4 : 3); // sizeof IQ pair
    //const int maxSamplesInPkt = sizeof(FPGA_DataPacket::data) / chCount / sampleSize;
    const int maxSamplesInPkt = 1024 / chCount;

    int requestSamplesInPkt = 256;//maxSamplesInPkt;
    int samplesInPkt = std::min(requestSamplesInPkt, maxSamplesInPkt);
    assert(samplesInPkt > 0);

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

    mRxPacketsToBatch = std::min((uint32_t)mRxPacketsToBatch, DMA_BUFFER_SIZE/packetSize);
    mRxPacketsToBatch = std::max((int)mRxPacketsToBatch, 1);
    assert(mRxPacketsToBatch > 0);

    if(!rxPort->IsOpen())
        throw std::runtime_error("ReceivePacketsLoop rx data port not open\n");

    // Initialize DMA
    const int fd = rxPort->GetFd();
    int ret;

    litepcie_ioctl_mmap_dma_info mmap_dma_info;
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_MMAP_DMA_INFO, &mmap_dma_info);
    if(ret)
        throw std::runtime_error("TransmitLoop IOCTL failed to get DMA info\n");

    uint8_t* dmaMem = (uint8_t*)mmap(NULL, DMA_BUFFER_TOTAL_SIZE, PROT_READ, MAP_SHARED,
                       fd, mmap_dma_info.dma_rx_buf_offset);
    if (dmaMem == MAP_FAILED || dmaMem == nullptr) {
        throw std::runtime_error("ReceiverLoop memory allocation failed\n");
        return;
    }

    uint8_t* dmaBuffers[DMA_BUFFER_COUNT];
    for(int i=0; i<DMA_BUFFER_COUNT;++i)
        dmaBuffers[i] = dmaMem+DMA_BUFFER_SIZE*i;

    litepcie_ioctl_lock lockInfo;
    memset(&lockInfo, 0, sizeof(lockInfo));
    lockInfo.dma_writer_request = 1;
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_LOCK, &lockInfo);

    const int readBlockSize = packetSize * mRxPacketsToBatch;
    const int stagingSamplesCapacity = samplesInPkt*mRxPacketsToBatch;
    assert(readBlockSize <= DMA_BUFFER_SIZE);

    //printf("Stream, samples:%i pktSize:%i payload:%i smplSize:%i, batch:%i, DMAread:%i\n", samplesInPkt, packetSize, payloadSize, iqSamplesCount, mRxPacketsToBatch, readBlockSize);

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
        rxPacketsPool.push(new StagingPacketType(stagingSamplesCapacity));
    StagingPacketType* rxStaging = nullptr;
    rxPacketsPool.pop(&rxStaging, true, 1000);

    uint64_t readIndex = 0;

    lime::complex32f_t tempAbuffer[1360];
    lime::complex32f_t tempBbuffer[1360];

    float bufferTimeDuration = float(samplesInPkt*mRxPacketsToBatch) / mConfig.hintSampleRate;
    int irqPeriod = 80e-6 / bufferTimeDuration;
    irqPeriod = std::max(1, irqPeriod);
    irqPeriod = std::min(irqPeriod, 16);
    //printf("Buffer duration: %g us, irq: %i\n", bufferTimeDuration*1e6, irqPeriod);

    float expectedDataRateBps = 0;
    if (mConfig.hintSampleRate != 0)
    {
        // samples data
        expectedDataRateBps = (mConfig.hintSampleRate/samplesInPkt) * (16 + samplesInPkt * sampleSize * chCount);
    }

    litepcie_ioctl_dma_writer writer;
    writer.enable = 1;
    writer.write_size = readBlockSize; // bad size here can cause system instability/reboot!!!
    writer.irqFreq = irqPeriod;
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_WRITER, &writer);
    if( ret < 0)
    {
        printf("Failed to start DMA writer\n");
        return;
    }

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

    while (terminateRx.load(std::memory_order_relaxed) == false) {
        writer.sw_count = readIndex;
        ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_WRITER, &writer);
        if( ret < 0)
            throw std::runtime_error("Failed to get DMA writer status\n");
        if(writer.hw_count != lastDMAcounter)
        {
            totalBytesReceived += (writer.hw_count - lastDMAcounter) * readBlockSize;
            lastDMAcounter = writer.hw_count;
        }

        int buffersAvailable = writer.hw_count - readIndex;
        if(buffersAvailable)
        {
            bool reportProblems = false;
            if(buffersAvailable >= DMA_BUFFER_COUNT) // data overflow
            {
                readIndex = (writer.hw_count - 1); // reset to 1 buffer behind latest data
                ++rxOverrun;
                ++stats.overrun;
                reportProblems = true;
            }

            msync(dmaBuffers[readIndex % DMA_BUFFER_COUNT], readBlockSize, MS_SYNC);

            uint8_t* inBuffer = dmaBuffers[readIndex % DMA_BUFFER_COUNT];
            FPGA_DataPacket *firstPkt = reinterpret_cast<FPGA_DataPacket*>(inBuffer);
            const int srcPktCount = readBlockSize / packetSize;
            totalPacketsReceived += srcPktCount;
            packetsRecv += srcPktCount;
            rxLastTimestamp.store(firstPkt->counter+samplesInPkt*srcPktCount, std::memory_order_relaxed);

            lime::complex32f_t *f32_dest[2] = {
                static_cast<lime::complex32f_t *>(tempAbuffer),
                static_cast<lime::complex32f_t *>(tempBbuffer)
            };

            for (int i=0; i<srcPktCount && !terminateRx.load(std::memory_order_relaxed); ++i) {
                FPGA_DataPacket *srcPkt = reinterpret_cast<FPGA_DataPacket*>(&inBuffer[i*packetSize]);
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
            ++readIndex;
            litepcie_ioctl_mmap_dma_update sub;
            sub.sw_count = readIndex;
            int ret = guarded_ioctl(fd, LITEPCIE_IOCTL_MMAP_DMA_WRITER_UPDATE, &sub);
            // one callback for the entire batch
            if(reportProblems && mConfig.statusCallback)
                mConfig.statusCallback(&stats, mConfig.userData);
        }
        else
        {
            auto pt1 = perfClock::now();
            pollfd desc;
            desc.fd = fd;
            desc.events = POLLIN;

            struct timespec timeout_ts;
            timeout_ts.tv_sec = 0;
            timeout_ts.tv_nsec = 50e3;
            sigset_t origmask;
            int ret = ppoll(&desc, 1, &timeout_ts, &origmask);

            //int ret = poll(&desc, 1, pollTimeout);
            if (ret < 0)
            {
                printf("ReceiveLoop poll errno(%i) %s\n", errno, strerror(errno));
                return;
            }
            auto pt2 = perfClock::now();
            auto pollDur = duration_cast<microseconds>(pt2 - pt1).count();
            //printf("POLLIN %ius\n", pollDur);
            ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_WRITER, &writer);
            if( ret < 0)
                throw std::runtime_error("Failed to get DMA writer status\n");
        }

        t2 = perfClock::now();
        auto timePeriod = duration_cast<milliseconds>(t2 - t1).count();
        if (timePeriod >= statsPeriod_ms || terminateRx.load(std::memory_order_relaxed)) {
            t1 = t2;
            double dataRateBps = 1000.0 * totalBytesReceived / timePeriod;
            stats.dataRate_Bps = dataRateBps;

            const double dataRateMargin = expectedDataRateBps*0.02;
            if( expectedDataRateBps!=0 && abs(expectedDataRateBps - dataRateBps) > dataRateMargin )
                printf("Rx%i Unexpected Rx data rate, should be: ~%g MB/s, got: %g MB/s, diff: %g\n", chipId, expectedDataRateBps/1e6, dataRateBps/1e6, (expectedDataRateBps - dataRateBps)/1e6);
            if ( abs(streamDelay) > 2000 )
                printf("Discrepancy between PC clock and Rx timestamps: %ius\n", streamDelay);
            stats.txDataRate_Bps = txDataRate_Bps.load(std::memory_order_relaxed);
            if(showStats || mCallback_logMessage)
            {
                double pushPeriodAvg, pushPeriodRms;
                pktPushTimes_us.GetResult(pushPeriodAvg, pushPeriodRms);
                ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_WRITER, &writer);
                // sprintf(ctemp, "%02X %02X %02X %02X %02X %02X %02X %02X", probeBuffer[0],probeBuffer[1],probeBuffer[2],probeBuffer[3],probeBuffer[4],probeBuffer[5], probeBuffer[6],probeBuffer[7]);
                char msg[512];
                int len = snprintf(msg, sizeof(msg)-1, "%s Rx: %3.3f MB/s | FIFO:%i/%i/%i TS:%li/%li=%li overrun:%i loss:%i, txDrops:%i, shw:%li/%li(%+i), l:%.0f/%.0f/%.0fus, pcTime-RxTS:%ius",
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
                    writer.hw_count,
                    int(writer.hw_count-readIndex),
                    pushPeriodAvg,
                    pushPeriodRms,
                    pktPushTimes_us.Max(),
                    streamDelay
                );
                if(showStats)
                    printf("%s\n", msg);
                if(mCallback_logMessage)
                    mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg, len);
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
    writer.enable = 0;
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_WRITER, &writer);
    if( ret < 0)
    {
        printf("Failed to stop DMA writer\n");
        return;
    }

    lockInfo.dma_writer_request = 0;
    lockInfo.dma_writer_release = 1;
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_LOCK, &lockInfo);

    rxDataRate_Bps.store(0, std::memory_order_relaxed);

    char msg[256];
    int len = sprintf(msg, "Rx loop totals: packets recv: %i, txDrops: %i\n", totalPacketsReceived, totalTxDrops);

    if(mCallback_logMessage)
        mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg, len);
    munmap(dmaMem, mmap_dma_info.dma_rx_buf_size * mmap_dma_info.dma_rx_buf_count);
}

} // namespace lime
