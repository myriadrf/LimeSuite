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
//#define OLD_PCIE_CORE

static bool showStats = false;
static const int statsPeriod_ms = 1000; // at 122.88 MHz MIMO, fpga tx pkt counter overflows every 272ms

namespace lime {

static std::mutex gIOCTLmutex;

static int guarded_ioctl(int fd, unsigned long request, void* argp)
{
#ifdef OLD_PCIE_CORE
    return 0;
#endif
    std::unique_lock<std::mutex> lock(gIOCTLmutex);
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
    TRXLooper::Setup(config);

    float combinedSampleRate = std::max(config.txCount, config.rxCount)*config.hintSampleRate;
    int batchSize = 4; // should be good enough for most cases
    // for high data rates e.g 16bit ADC/DAC 2x2 MIMO @ 122.88Msps = ~1973 MB/s
    // need to batch as many packets as possible into transfer buffers
    if (combinedSampleRate != 0)
    {
        batchSize = combinedSampleRate/30.72e6;
        batchSize = std::min(batchSize, int(DMA_BUFFER_SIZE/sizeof(FPGA_DataPacket)));
        batchSize = std::max(1, batchSize);
    }
    mRxPacketsToBatch = mTxPacketsToBatch = batchSize;
}

void TRXLooper_PCIE::Start()
{
    fpga->WriteRegister(0xFFFF, 1 << chipId);
    fpga->StopStreaming();
    uint16_t startAddress = 0x7FE1 + (chipId*5);
    // reset Tx received/dropped packets counters
    fpga->WriteRegister(startAddress, 0);
    fpga->WriteRegister(startAddress, 3);
    fpga->WriteRegister(startAddress, 0);
    TRXLooper::Start();
}

void TRXLooper_PCIE::TransmitPacketsLoop()
{
    typedef PartialPacket<lime::complex32f_t> InPacketType;
    const bool mimo = std::max(mConfig.txCount, mConfig.rxCount) > 1;
    const bool compressed = mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I12;
    const int samplesInPkt = (mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 1020 : 1360) / std::max(mConfig.rxCount, mConfig.txCount);
    const int packetSize = sizeof(FPGA_DataPacket);

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

#ifdef OLD_PCIE_CORE
    uint8_t* dmaMem = (uint8_t*)aligned_alloc(4096, DMA_BUFFER_TOTAL_SIZE);
#else
    uint8_t* dmaMem = (uint8_t*)mmap(NULL, DMA_BUFFER_TOTAL_SIZE, PROT_WRITE, MAP_SHARED,
                       fd, mmap_dma_info.dma_tx_buf_offset);
#endif
    if (dmaMem == MAP_FAILED || dmaMem == nullptr) {
        throw std::runtime_error("TransmitLoop failed to allocate memory\n");
        return;
    }

    const int outDMA_BUFFER_SIZE = packetSize * mTxPacketsToBatch;
    assert(outDMA_BUFFER_SIZE <= DMA_BUFFER_SIZE);
    memset(dmaMem, 0, DMA_BUFFER_TOTAL_SIZE);
    const int irqPeriod = 8;
    const int maxDMAqueue = 255;

    // Initialize DMA
    litepcie_ioctl_dma_reader reader;
    reader.enable = 1;
    reader.irqFreq = irqPeriod;
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_READER, &reader);
    if(ret)
        throw std::runtime_error("TransmitLoop IOCTL failed to enable DMA reader");

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;

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

    int pendingWriteBufferIndex = 0;
    int stagingBufferIndex = 0;
    int writeTransactions = 0;
    int underrun = 0;
    float packetsIn = 0;

    StagingPacketType* srcPkt = nullptr;

    TxBufferManager<StagingPacketType> output(mimo, compressed);
    output.Reset(dmaBuffers[0], outDMA_BUFFER_SIZE);

    bool outputReady = false;

    mThreadsReady.fetch_add(1);

    while (terminateTx.load(std::memory_order_relaxed) == false) {
        // check if DMA transfers have completed
        ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_READER, &reader);
        if (ret)
            throw std::runtime_error("TransmitLoop IOCTL failed to get DMA reader counters");

        // process pending transactions
        while(!pendingWrites.empty() && !terminateTx.load(std::memory_order_relaxed))
        {
            PendingWrite &dataBlock = pendingWrites.front();
#ifdef OLD_PCIE_CORE
            FPGA_DataPacket* pkt = reinterpret_cast<FPGA_DataPacket*>(dataBlock.data + dataBlock.offset);
            lastTS = pkt->counter;
            int64_t rxNow = rxLastTimestamp.load();
            if (pkt->counter-rxNow < 0)
            {
                ++underrun;
                //printf("TxOUT: %li, rxNow: %li, diff: %li\n", pkt->counter, rxNow, pkt->counter-rxNow);
            }
            const int bytesWritten = rxPort->WriteRaw(dataBlock.data + dataBlock.offset, dataBlock.size - dataBlock.offset, 500);
            if(bytesWritten < 0)
            {
                ++retryWrites;
                break;
            }
            dataBlock.offset += bytesWritten;
            if(dataBlock.size - dataBlock.offset == 0)
            {
                pendingWrites.pop();
                ++pendingWriteBufferIndex;
            }
            totalBytesSent += bytesWritten;
#else
            if (reader.hw_count > dataBlock.id)
            {
                ++pendingWriteBufferIndex;
                totalBytesSent += dataBlock.size;
                pendingWrites.pop();
            }
            else
                break;
#endif
            //++writeTransactions;
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
                delete srcPkt;
                srcPkt = nullptr;
            }
            if(doFlush)
            {
                outputReady = true;
                break;
            }
        }

        ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_READER, &reader);
        if (ret)
            throw std::runtime_error("TransmitLoop IOCTL failed to get DMA reader counters");

        //printf("sw: %li hw: %li, diff: %li\n", stagingBufferIndex, reader.hw_count, stagingBufferIndex-reader.hw_count);
        bool canSend = stagingBufferIndex - reader.hw_count < maxDMAqueue;
        if(!canSend)
        {
            pollfd desc;
            desc.fd = fd;
            desc.events = POLLOUT;

            //printf("Poll\n");
            const int pollTimeout = 10;
            const int ret = poll(&desc, 1, pollTimeout);
            if (ret < 0)
                printf("TransmitLoop poll errno(%i) %s\n", errno, strerror(errno));
            if (ret == 0)
                printf("TransmitLoop poll timeout\n");
            else
                canSend = true;
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
#ifdef OLD_PCIE_CORE
            const int bytesWritten = rxPort->WriteRaw(dataBlock.data + dataBlock.offset, dataBlock.size - dataBlock.offset, 500);
            if(bytesWritten < 0)
            {
                ++retryWrites;
                continue;
            }
            dataBlock.offset += bytesWritten;
            if(dataBlock.size - dataBlock.offset != 0)
                pendingWrites.push(wrInfo); // not all data has been written, add to pending for later retry
            else
                ++pendingWriteBufferIndex;
            totalBytesSent += bytesWritten;
#else
            litepcie_ioctl_mmap_dma_update sub;
            sub.sw_count = stagingBufferIndex;
            sub.buffer_size = wrInfo.size;
            sub.genIRQ = (wrInfo.id % irqPeriod) == 0 && wrInfo.id > 0;
            assert(sub.buffer_size <= DMA_BUFFER_SIZE);
            assert(sub.buffer_size <= outDMA_BUFFER_SIZE);
            assert(sub.buffer_size > 0);

            int bytesWritten = 0;
            FPGA_DataPacket* pkt = reinterpret_cast<FPGA_DataPacket*>(output.data());
            lastTS = pkt->counter;
            int64_t rxNow = rxLastTimestamp.load();
            if (pkt->counter-rxNow <= 0)
            {
                ++underrun;
                //printf("TxOUT: %li, rxNow: %li, diff: %li\n", pkt->counter, rxNow, pkt->counter-rxNow);
            }
            msync(dmaBuffers[stagingBufferIndex % DMA_BUFFER_COUNT], sub.buffer_size, MS_SYNC);
            int ret = guarded_ioctl(fd, LITEPCIE_IOCTL_MMAP_DMA_READER_UPDATE, &sub);
            if(ret)
            {
                printf("Failed to submit dma write (%i) %s\n", errno, strerror(errno));
                ++retryWrites;
                continue;
            }
            else
            {
                //printf("Sent sw: %li hw: %li, diff: %li\n", stagingBufferIndex, reader.hw_count, stagingBufferIndex-reader.hw_count);
                outputReady = false;
                ++writeTransactions;
                pendingWrites.push(wrInfo);
            }
#endif
            ++stagingBufferIndex;

            // todo: check if can take next one
            packetsSent += output.packetCount();
            totalPacketSent += output.packetCount();
            output.Reset(dmaBuffers[stagingBufferIndex % DMA_BUFFER_COUNT], outDMA_BUFFER_SIZE);
        }

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= statsPeriod_ms || terminateTx.load(std::memory_order_relaxed)) {
            t1 = t2;

            ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_READER, &reader);
            double dataRate = 1000.0 * totalBytesSent / timePeriod;
            float avgBatchPktCount = float(packetsSent)/writeTransactions;

            const uint16_t addr = 0x7FE2 + (chipId*5);
            //fpga->WriteRegister(addr, 0);
            uint32_t addrs[] = {addr, addr+1u};
            uint32_t values[2];
            fpga->ReadRegisters(addrs, values, 2);
            uint32_t fpgaTxPktIngressCount = (values[0] << 16) | values[1];
            //blockTiming.avg = 0;//sqrt(blockProcessTime / blockTiming.blockCount);

            if(showStats || mCallback_logMessage)
            {
                char msg[512];
                int len = snprintf(msg, sizeof(msg)-1, "%s Tx: %.3f MB/s | FIFO:%i/%i/%i pktIn:%g pktOut:%i TS:%li avgBatch:%.1f retry:%i totalOut:%i(x%08X)-fpga(x%08X)=%i, shw:%li/%li underrun:%i",
                    rxPort->GetPathName().c_str(),
                    dataRate / 1000000.0,
                    txIn.size(),
                    maxFIFOlevel,
                    txIn.max_size(),
                    packetsIn,
                    packetsSent,
                    lastTS,
                    avgBatchPktCount,
                    retryWrites,
                    totalPacketSent,
                    totalPacketSent&0xFFFFFFFF,
                    fpgaTxPktIngressCount,
                    totalPacketSent - fpgaTxPktIngressCount,
                    reader.sw_count,
                    reader.hw_count,
                    underrun
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
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_READER, &reader);
    printf("DMA reader %li/%li, diff: %li\n", reader.sw_count, reader.hw_count, reader.sw_count - reader.hw_count);

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

    printf("Tx pending transactions: %lu\n", pendingWrites.size());

    uint16_t addr = 0x7FE1 + chipId*5;
    //fpga->WriteRegister(addr, 0);
    uint32_t addrs[] = {addr+1u, addr+2u, addr+3u, addr+4u};
    uint32_t values[4];
    fpga->ReadRegisters(addrs, values, 4);
    const uint32_t fpgaTxPktIngressCount = (values[0] << 16) | values[1];
    const uint32_t fpgaTxPktDropCounter = (values[2] << 16) | values[3];
    printf("Tx Loop totals: packets sent: %i (0x%08X) , FPGA packet counter: %i (0x%08X), diff: %i, FPGA tx drops: %i\n",
        totalPacketSent, totalPacketSent, fpgaTxPktIngressCount, fpgaTxPktIngressCount, (totalPacketSent&0xFFFFFFFF)-fpgaTxPktIngressCount, fpgaTxPktDropCounter
        );
#ifdef OLD_PCIE_CORE
    free(dmaMem);
#else
    munmap(dmaMem, mmap_dma_info.dma_tx_buf_size * mmap_dma_info.dma_tx_buf_count);
#endif
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
    //at this point FPGA has to be already configured to output samples
    const int samplesInPkt = (mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 1020 : 1360) / std::max(mConfig.rxCount, mConfig.txCount);
    const int packetSize = sizeof(FPGA_DataPacket);

    // Initialize DMA
    const int fd = rxPort->GetFd();
    int ret;

    litepcie_ioctl_mmap_dma_info mmap_dma_info;
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_MMAP_DMA_INFO, &mmap_dma_info);
    if(ret)
        throw std::runtime_error("TransmitLoop IOCTL failed to get DMA info\n");

#ifdef OLD_PCIE_CORE
    uint8_t* dmaMem = (uint8_t*)aligned_alloc(4096, DMA_BUFFER_TOTAL_SIZE);
#else
    uint8_t* dmaMem = (uint8_t*)mmap(NULL, DMA_BUFFER_TOTAL_SIZE, PROT_READ, MAP_SHARED,
                       fd, mmap_dma_info.dma_rx_buf_offset);
#endif
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
    const int stagingSamplesCapacity = samplesInPkt*readBlockSize/packetSize;
    assert(readBlockSize <= DMA_BUFFER_SIZE);

    litepcie_ioctl_dma_writer writer;
    writer.enable = 1;
    writer.write_size = readBlockSize;
    writer.irqFreq = 8;

    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_WRITER, &writer);
    if( ret < 0)
    {
        printf("Failed to start DMA writer\n");
        return;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;

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
    StagingPacketType* rxStaging = new StagingPacketType(stagingSamplesCapacity);

    uint64_t readIndex = 0;

    lime::complex32f_t tempAbuffer[1360];
    lime::complex32f_t tempBbuffer[1360];

    float expectedDataRateBps = 0;
    if (mConfig.hintSampleRate != 0)
    {
        // samples data
        expectedDataRateBps = mConfig.hintSampleRate * (compressed ? 3 : 4) * (mimo ? 2 : 1);
        // add packet headers overhead
        expectedDataRateBps = (expectedDataRateBps/sizeof(FPGA_DataPacket::data))*sizeof(FPGA_DataPacket);
    }

    mThreadsReady.fetch_add(1);
    int64_t realTS = 0;
    while (terminateRx.load(std::memory_order_relaxed) == false) {
        ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_WRITER, &writer);
        if( ret < 0)
            throw std::runtime_error("Failed to get DMA writer status\n");

        int buffersAvailable = writer.hw_count - readIndex;
        if(buffersAvailable)
        {
            totalBytesReceived += readBlockSize;
            bool reportProblems = false;

            if(buffersAvailable >= DMA_BUFFER_COUNT) // data overflow
            {
                readIndex = writer.hw_count-2; // reset read position right behind current hardware index
                ++rxOverrun;
                ++stats.overrun;
                reportProblems = true;
            }

            msync(dmaBuffers[readIndex % DMA_BUFFER_COUNT], readBlockSize, MS_SYNC);

            FPGA_DataPacket *srcPkt = reinterpret_cast<FPGA_DataPacket*>(dmaBuffers[readIndex % DMA_BUFFER_COUNT]);
            const int srcPktCount = readBlockSize / packetSize;
            totalPacketsReceived += srcPktCount;
            packetsRecv += srcPktCount;
            rxLastTimestamp.store(srcPkt->counter+samplesInPkt*srcPktCount, std::memory_order_relaxed);

            lime::complex32f_t *f32_dest[2] = {
                static_cast<lime::complex32f_t *>(tempAbuffer),
                static_cast<lime::complex32f_t *>(tempBbuffer)
            };

            for (int i=0; i<srcPktCount && !terminateRx.load(std::memory_order_relaxed); ++i) {
                if(!rxStaging)
                    rxStaging = new StagingPacketType(stagingSamplesCapacity);
                rxStaging->timestamp = srcPkt->counter;

                int64_t ts = 0;
                uint8_t* ptr = (uint8_t*)srcPkt;
                ptr+=2;
                for(int j=0; j<6; ++j)
                    ts |= ((int64_t)ptr[j]) << (j*8);
                realTS = ts;// | srcPkt->payloadSizeMSB;
                //printf("RealTS: %08X\n    TS: %08X\n", realTS, srcPkt->counter);
                
                const int64_t samplesDiff = srcPkt->counter - lastTS;
                if (samplesDiff != samplesInPkt) {
                    printf("Rx discontinuity: %li -> %li (diff: %li), bytesIn: %i, sw:%li hw:%li\n",
                        lastTS, srcPkt->counter, samplesDiff, readBlockSize, readIndex, writer.hw_count);
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
                const int samplesParsed = FPGA::FPGAPacketPayload2SamplesFloat(srcPkt->data, sizeof(FPGA_DataPacket::data), mimo, compressed, f32_dest);
                int samplesPushed = rxStaging->push(f32_dest, samplesParsed);

                if(rxStaging->isFull())
                {
                    if(rxOut.push(rxStaging, false))
                    {
                        const int fifoLevel = rxOut.size();
                        maxFIFOlevel = std::max(maxFIFOlevel, fifoLevel);
                        ++packetsOut;
                        rxStaging = new StagingPacketType(stagingSamplesCapacity);
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
                ++srcPkt;
            }
            ++readIndex;
            // one callback for the entire batch
            if(reportProblems && mConfig.statusCallback)
                mConfig.statusCallback(&stats, mConfig.userData);
        }
        else
        {
            pollfd desc;
            desc.fd = fd;
            desc.events = POLLIN;

            const int pollTimeout = 10;
            int ret = poll(&desc, 1, pollTimeout);
            if (ret < 0)
            {
                printf("ReceiveLoop poll errno(%i) %s\n", errno, strerror(errno));
                return;
            }
        }

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= statsPeriod_ms || terminateRx.load(std::memory_order_relaxed)) {
            t1 = t2;
            double dataRateBps = 1000.0 * totalBytesReceived / timePeriod;
            stats.dataRate_Bps = dataRateBps;

            const double dataRateMargin = expectedDataRateBps*0.02;
            if( expectedDataRateBps!=0 && abs(expectedDataRateBps - dataRateBps) > dataRateMargin )
                printf("Unexpected Rx data rate, should be: ~%g MB/s, got: %g MB/s, diff: %g\n", expectedDataRateBps/1e6, dataRateBps/1e6, (expectedDataRateBps - dataRateBps)/1e6);
            stats.txDataRate_Bps = txDataRate_Bps.load(std::memory_order_relaxed);
            if(showStats || mCallback_logMessage)
            {
                ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_WRITER, &writer);
                // sprintf(ctemp, "%02X %02X %02X %02X %02X %02X %02X %02X", probeBuffer[0],probeBuffer[1],probeBuffer[2],probeBuffer[3],probeBuffer[4],probeBuffer[5], probeBuffer[6],probeBuffer[7]);
                char msg[512];
                int len = snprintf(msg, sizeof(msg)-1, "%s Rx: %.3f MB/s | FIFO:%i/%i/%i pktIn:%i pktOut:%i TS:%li/%li=%li overrun:%i loss:%i, txDrops:%i, shw:%li/%li",
                    rxPort->GetPathName().c_str(),
                    dataRateBps / 1000000.0,
                    rxOut.size(),
                    maxFIFOlevel,
                    rxOut.max_size(),
                    packetsRecv,
                    packetsOut,
                    lastTS,
                    realTS,
                    realTS-lastTS,
                    rxOverrun,
                    rxDiscontinuity,
                    txDrops,
                    readIndex,//writer.sw_count,
                    writer.hw_count
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

    printf("Rx loop totals: packets recv: %i, txDrops: %i\n", totalPacketsReceived, totalTxDrops);
#ifdef OLD_PCIE_CORE
    free(dmaMem);
#else
    munmap(dmaMem, mmap_dma_info.dma_rx_buf_size * mmap_dma_info.dma_rx_buf_count);
#endif
}

} // namespace lime
