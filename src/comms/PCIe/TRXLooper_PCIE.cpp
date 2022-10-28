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

static bool showStats = true;
static const int statsPeriod_ms = 1000; // at 122.88 MHz MIMO, fpga tx pkt counter overflows every 272ms

namespace lime {

static std::mutex gIOCTLmutex;

static int guarded_ioctl(int fd, unsigned long request, void* argp)
{
    std::unique_lock<std::mutex> lock(gIOCTLmutex);
    return ioctl(fd, request, argp);
}

TRXLooper_PCIE::TRXLooper_PCIE(LitePCIe *trxPort, FPGA *f, LMS7002M *chip, uint8_t moduleIndex)
    : TRXLooper(f, chip, moduleIndex), trxPort(trxPort)
{
    mMaxBufferSize = DMA_BUFFER_SIZE;
}

TRXLooper_PCIE::~TRXLooper_PCIE() {}

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

    const int fd = trxPort->GetFd();
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
    if (dmaMem == MAP_FAILED) {
        throw std::runtime_error("TransmitLoop failed mmap()\n");
        return;
    }

    const int outDMA_BUFFER_SIZE = 4096;//DMA_BUFFER_SIZE;
    memset(dmaMem, 0, DMA_BUFFER_SIZE*DMA_BUFFER_COUNT);
    const int packetSize = sizeof(FPGA_DataPacket);

    // Initialize DMA
    litepcie_ioctl_dma_reader reader;
    reader.hw_count = 0;
    reader.sw_count = 0;
    reader.enable = 1;
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
    double blockProcessTime = 0;
    struct {
        int blockCount;
        int min;
        int max;
        float avg;
    } blockTiming;
    blockTiming.min = 1e6;
    blockTiming.blockCount = 0;

    int64_t batchesSubmitted = 0;
    int64_t batchesConfirmed = 0;

    struct asyncTx {
        int64_t id;
        int32_t size;
        int16_t pktCount;
    };

    int64_t underrun = 0;

    std::queue<asyncTx> transactions;

    RawDataBlock srcBlock;

    const int irqPeriod = 8;
    const int maxDMAqueue = 255;//-1*irqPeriod;
    SamplesBlock inBlock;
    inBlock.ptr = nullptr;

    RawDataBlock outBlock;
    outBlock.ptr = nullptr;
    uint8_t* buffer = (uint8_t*)aligned_alloc(4096, outDMA_BUFFER_SIZE);
    mThreadsReady.fetch_add(1);

    while (terminateTx.load(std::memory_order_relaxed) == false) {
        if(!inBlock.ptr)
        {
            bool got = txIn.pop(&inBlock, true, 100);
            if(got)
            {
                assert(inBlock.size > 0);
                assert(inBlock.usage > 0);
                assert(inBlock.offset == 0);
                int blockProcessingTime = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - inBlock.genTime).count();
                blockTiming.max = std::max(blockTiming.max, blockProcessingTime);
                blockTiming.min = std::min(blockTiming.min, blockProcessingTime);
                blockProcessTime += blockProcessingTime*blockProcessingTime;
                ++blockTiming.blockCount;
            }
        }

        ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_READER, &reader);
        if (ret)
            throw std::runtime_error("TransmitLoop IOCTL failed to get DMA reader counters");

        const bool canDoBatch = reader.sw_count - reader.hw_count < maxDMAqueue;
        if(canDoBatch && inBlock.ptr)
        {
            //printf("Can do batch %li %li\n", reader.sw_count, reader.hw_count);
            bool flush = false;
            int freePktCount = (outDMA_BUFFER_SIZE)/sizeof(FPGA_DataPacket);
            //FPGA_DataPacket *outPkt = reinterpret_cast<FPGA_DataPacket*>(dmaPtr);
            int bytesToSend = 0;
            int packetsPrepared = 0;
            FPGA_DataPacket *outPkt = reinterpret_cast<FPGA_DataPacket*>(buffer);
            int partialFill = 0;
            while(freePktCount > 0 && terminateTx.load(std::memory_order_relaxed) == false)
            {
                if(!inBlock.ptr)
                {
                    bool got = txIn.pop(&inBlock, true, 20);
                    if(got)
                    {
                        assert(inBlock.size > 0);
                        assert(inBlock.usage > 0);
                        assert(inBlock.offset == 0);
                        int blockProcessingTime = std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::high_resolution_clock::now() - inBlock.genTime).count();
                        blockTiming.max = std::max(blockTiming.max, blockProcessingTime);
                        blockTiming.min = std::min(blockTiming.min, blockProcessingTime);
                        blockProcessTime += blockProcessingTime*blockProcessingTime;
                        ++blockTiming.blockCount;
                    }
                    else
                        break;
                }
                InPacketType* inPkt = reinterpret_cast<InPacketType*>((uint8_t*)inBlock.ptr+inBlock.offset);
                if(!partialFill)
                {
                    bytesToSend += 16; // Packet header size
                    outPkt->ClearHeader();
                    outPkt->counter = inPkt->timestamp + inPkt->start;
                    outPkt->ignoreTimestamp(!inPkt->useTimestamp);
                }

                const lime::complex32f_t *f32_src[2] = {
                    static_cast<lime::complex32f_t *>(&inPkt->chA[inPkt->start]),
                    static_cast<lime::complex32f_t *>(&inPkt->chB[inPkt->start])
                };

                const int srcSamplesAvailable = inPkt->end - inPkt->start;
                assert(srcSamplesAvailable > 0);
                const int samplesToTransfer = std::min(srcSamplesAvailable, samplesInPkt-partialFill);
                assert(samplesToTransfer > 0);
                int payloadSize = outPkt->GetPayloadSize();
                uint8_t* destMemory = outPkt->data + payloadSize;
                int additionalBytes = FPGA::Samples2FPGAPacketPayloadFloat(f32_src, samplesToTransfer, mimo, compressed, destMemory);
                payloadSize += additionalBytes;
                outPkt->SetPayloadSize(payloadSize);
                assert(payloadSize > 0 && payloadSize <= sizeof(FPGA_DataPacket::data));

                partialFill += samplesToTransfer;
                inPkt->start += samplesToTransfer;

                const bool srcDepleted = inPkt->start == inPkt->end;
                bool destReady = payloadSize == sizeof(FPGA_DataPacket::data);
                if(srcDepleted)
                {
                    if(inPkt->flush)
                    {
                        flush = true;
                        destReady = true;
                    }
                    inBlock.offset += sizeof(InPacketType);
                    if(inBlock.offset == inBlock.usage)
                    {
                        inBlock.offset = 0;
                        inBlock.usage = 0;
                        txInPool.Free(inBlock.ptr);
                        inBlock.ptr = nullptr;
                    }
                }
                if(destReady)
                {
                    partialFill = 0;
                    ++packetsPrepared;
                    bytesToSend += outPkt->GetPayloadSize();
                    lastTS = outPkt->counter;
                    ++outPkt;
                    --freePktCount;
                }
                if(flush)
                    break;
            }

            litepcie_ioctl_mmap_dma_update sub;
            sub.sw_count = batchesSubmitted;
            sub.buffer_size = bytesToSend;

            if(sub.buffer_size == 0)
            {
                printf("Empty DMA buffer\n");
                continue;
            }

            assert(sub.buffer_size <= DMA_BUFFER_SIZE);
            assert(sub.buffer_size <= outDMA_BUFFER_SIZE);
            assert(sub.buffer_size > 0);
            assert(packetsPrepared > 0);
            sub.genIRQ = (batchesSubmitted % irqPeriod) == 0 && batchesSubmitted > 0;

            outPkt = reinterpret_cast<FPGA_DataPacket*>(buffer);
            //printf("\n");
            for(int i=0; i<sub.buffer_size/sizeof(FPGA_DataPacket); ++i)
            {
                //printf(" %li", outPkt->counter);
                //outPkt->ignoreTimestamp(true);
                //outPkt->SetPayloadSize(4080);
                // outPkt[i].ignoreTimestamp(true);
                // outPkt[i].counter = 0;
                //outPkt[i].SetPayloadSize(4000);
                assert(outPkt[i].GetPayloadSize() > 0);
                assert(outPkt[i].GetPayloadSize() <= 4080);
                // if (outPkt[i].GetPayloadSize() > 4080)
                //     printf("bad payload %i\n", outPkt[i].GetPayloadSize());
                // if(outPkt[i].GetPayloadSize() != 0 || outPkt[i].GetPayloadSize() != 4080)
                //     outPkt[i].SetPayloadSize(0);
                // if(i>0)
                //     assert(outPkt[i].counter > outPkt[i-1].counter);
            }
            //memset(buffer, 0, outDMA_BUFFER_SIZE);
            const int bufIndex = batchesSubmitted % DMA_BUFFER_COUNT;
            uint8_t* dmaPtr = dmaMem + DMA_BUFFER_SIZE * bufIndex;
            memcpy(dmaPtr, buffer, sub.buffer_size);

            ret = guarded_ioctl(fd, LITEPCIE_IOCTL_MMAP_DMA_READER_UPDATE, &sub);
            if(ret)
            {
                printf("Failed to submit dma write\n");
                ++retryWrites;
            }
            else
            {
                transactions.push({batchesSubmitted, sub.buffer_size, packetsPrepared});
                //printf("batch sent %i sz: %i, genIRQ:%i,  sw:%li hw:%li, packetsPrepared:%i, totalSent: %i\n", batchesSubmitted, sub.buffer_size, sub.genIRQ, reader.sw_count, reader.hw_count, packetsPrepared, totalPacketSent);
                ++batchesSubmitted;
            }
            //int32_t bytesSent = trxPort->WriteRaw((uint8_t*)srcBlock.ptr, bufSize, 1000);
            maxFIFOlevel = std::max(maxFIFOlevel, txIn.size());
        }
        else
        {
            pollfd desc;
            desc.fd = fd;
            desc.events = POLLOUT;

            //printf("Poll\n");
            const int pollTimeout = 15;
            const int ret = poll(&desc, 1, pollTimeout);
            if (ret < 0)
                printf("TransmitLoop poll errno(%i) %s\n", errno, strerror(errno));
            if (ret == 0)
                printf("TransmitLoop poll timeout\n");
        }

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= statsPeriod_ms || terminateTx.load(std::memory_order_relaxed)) {
            t1 = t2;

            ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_READER, &reader);

            int writeTransactions = 0;
            while(!transactions.empty())
            {
                const asyncTx &sub = transactions.front();
                if(sub.id >= reader.hw_count)
                    break; // not yet processed by hardware

                ++writeTransactions;
                ++batchesConfirmed;
                totalBytesSent += sub.size;
                packetsSent += sub.pktCount;
                totalPacketSent += sub.pktCount;
                transactions.pop();
            }

            double dataRate = 1000.0 * totalBytesSent / timePeriod;
            float avgBatchPktCount = float(totalBytesSent/packetSize)/writeTransactions;

            const uint16_t addr = 0x7FE2 + (chipId*5);
            //fpga->WriteRegister(addr, 0);
            uint32_t addrs[] = {addr, addr+1u};
            uint32_t values[2];
            fpga->ReadRegisters(addrs, values, 2);
            uint32_t fpgaTxPktIngressCount = (values[0] << 16) | values[1];
            blockTiming.avg = sqrt(blockProcessTime / blockTiming.blockCount);

            if(showStats)
                printf("%s Tx: %.3f MB/s | FIFO:%i/%i/%i pktOut:%i TS:%li avgBatch:%.1f retry:%i totalOut:%i(x%08X)-fpga(x%08X)=%i bTime:%i/%.1f/%ius, shw:%li/%li, underrun:%li\n",
                    trxPort->GetPathName().c_str(),
                    dataRate / 1000000.0,
                    txIn.size(),
                    maxFIFOlevel,
                    txIn.max_size(),
                    packetsSent,
                    lastTS,
                    avgBatchPktCount,
                    retryWrites,
                    totalPacketSent,
                    totalPacketSent&0xFFFFFFFF,
                    fpgaTxPktIngressCount,
                    totalPacketSent - fpgaTxPktIngressCount,
                    blockTiming.min,
                    blockTiming.avg,
                    blockTiming.max,
                    reader.sw_count,
                    reader.hw_count,
                    underrun
                );
            // hardware counter should be less than software's
            //assert(reader.hw_count < reader.sw_count);
            // {
            //     printf("HW (%li) reader should be behind SW (%li), diff: %li\n", reader.hw_count, reader.sw_count, reader.hw_count-reader.sw_count);
            // }
            packetsSent = 0;
            totalBytesSent = 0;
            txDataRate_Bps.store((uint32_t)dataRate, std::memory_order_relaxed);
            retryWrites = 0;
            maxFIFOlevel = 0;
            blockProcessTime = 0;
            blockTiming.min = 1e6;
            blockTiming.max = 0;
            blockTiming.blockCount = 0;
        }
    }
    free(buffer);

    // give some time for the dma transfers to complete, than check the fpga packets counter
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_READER, &reader);
    printf("DMA reader %li/%li, diff: %li\n", reader.sw_count, reader.hw_count, reader.sw_count - reader.hw_count);
    while(!transactions.empty())
    {
        const asyncTx &sub = transactions.front();
        if(sub.id >= reader.hw_count)
            break; // not yet processed by hardware

        ++batchesConfirmed;
        totalBytesSent += sub.size;
        packetsSent += sub.pktCount;
        totalPacketSent += sub.pktCount;
        transactions.pop();
    }
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

    printf("Tx subs: %i\n", transactions.size());

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
    munmap(dmaMem, mmap_dma_info.dma_tx_buf_size * mmap_dma_info.dma_tx_buf_count);
}
#include <unistd.h>
/** @brief Function dedicated for receiving data samples from board
    @param stream a pointer to an active receiver stream
*/
void TRXLooper_PCIE::ReceivePacketsLoop()
{
    MemoryPool rxMemPool(256, 32768, 4096, "rxMemPool");
    typedef PartialPacket<lime::complex32f_t> PacketType;
    const bool mimo = std::max(mConfig.txCount, mConfig.rxCount) > 1;
    const bool compressed = mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I12;
    const int allocSize = sizeof(PacketType) * 8;

    SamplesBlock destBlock;
    destBlock.ptr = rxOutPool.Allocate(allocSize);
    destBlock.size = allocSize;

    rxLastTimestamp.store(0, std::memory_order_relaxed);
    //at this point FPGA has to be already configured to output samples
    const int samplesInPkt = (mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 1020 : 1360);
    const int64_t expectedTSdiff = samplesInPkt / std::max(mConfig.rxCount, mConfig.txCount);
    const int packetSize = sizeof(FPGA_DataPacket);
    const uint8_t maxPacketsToBatch = 1;//mMaxBufferSize/packetSize; // 4; // ideally to fill DMA buffer
    const int memBlockSize = maxPacketsToBatch * packetSize;

    // Initialize DMA
    const int fd = trxPort->GetFd();
    int ret;

    litepcie_ioctl_lock lockInfo;
    memset(&lockInfo, 0, sizeof(lockInfo));
    lockInfo.dma_writer_request = 1;
    ret = guarded_ioctl(fd, LITEPCIE_IOCTL_LOCK, &lockInfo);

    litepcie_ioctl_dma_writer writer;
    writer.enable = 1;
    writer.hw_count = 0;
    writer.sw_count = 0;
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
    int64_t lastTS = -expectedTSdiff;

    SDRDevice::StreamStats stats;
    int txDrops = 0;
    int rxDiscontinuity = 0;
    int rxOverrun = 0; // fifo is full, data is arriving faster than PC can process
    int maxFIFOlevel = 0;
    int packetsOut = 0;

    //uint8_t probeBuffer[8];

    int packetsRecv = 0;

    uint64_t batchCounter = 0;

    RawDataBlock srcBlock;
    srcBlock.ptr = (RawDataBlock*)rxMemPool.Allocate(memBlockSize);
    srcBlock.size = memBlockSize;
    mThreadsReady.fetch_add(1);
    while (terminateRx.load(std::memory_order_relaxed) == false) {
        if(!srcBlock.ptr)
        {
            assert(srcBlock.ptr);
            printf("ReceivePacketsLoop failed to acquire memory block\n");
            return;
        }
#if ZEROED_MEM
        memset(srcBlock.ptr, 0, srcBlock.size);
#endif
        assert(memBlockSize <= srcBlock.size);

        if(!destBlock.isValid())
        {
            throw std::runtime_error("Rx parsing did not get write memory block\n");
            return;
        }

        // pollfd desc;
        // desc.fd = fd;
        // desc.events = POLLIN;

        // const int pollTimeout = 100;
        // int ret = poll(&desc, 1, pollTimeout);
        // if (ret < 0)
        // {
        //     printf("ReceiveLoop poll errno(%i) %s\n", errno, strerror(errno));
        //     return;
        // }

        int bytesIn = trxPort->ReadRaw((uint8_t*)srcBlock.ptr, memBlockSize, 100);
        srcBlock.offset = 0;
        srcBlock.usage = bytesIn;

        if (bytesIn <= 0) {
            if(bytesIn < 0)
                printf("Rx read failed - bytesRecv: %i, errno(%i) %s\n", bytesIn, errno, strerror(errno));
            // else
            // {
            //     // we're reading data faster than it's produced
            //     // sleep for a bit to not waste CPU cycles
            //     //std::this_thread::sleep_for(std::chrono::microseconds(100));
            //     bytesIn = 0;
            // }
        }
        else {
            //ProfilerScope p(rxProfiler, "RxPackets", 1);
            ++batchCounter;
            totalBytesReceived += bytesIn;
            // TODO: handle partial reads, even though they should never happen
            if(bytesIn % packetSize != 0)
                printf("Rx partial read of packet\n");
            bool reportProblems = false;

            FPGA_DataPacket *srcPkt = reinterpret_cast<FPGA_DataPacket*>((uint8_t*)srcBlock.ptr+srcBlock.offset);
            const int srcPktCount = bytesIn / packetSize;
            rxLastTimestamp.store(srcPkt->counter+expectedTSdiff*srcPktCount, std::memory_order_relaxed);

            const int destFreePktCount = (destBlock.size - destBlock.usage) / sizeof(PacketType);
            const int packetsToParse = std::min(srcPktCount, destFreePktCount);
            assert(packetsToParse > 0);
            PacketType *destPkt = reinterpret_cast<PacketType*>((uint8_t*)destBlock.ptr+destBlock.usage);

            for (int i=0; i<packetsToParse; ++i) {
                ++packetsRecv;
                ++totalPacketsReceived;
                const int64_t samplesDiff = srcPkt->counter - lastTS;
                if (samplesDiff != expectedTSdiff) {
                    if(samplesDiff < 0)
                        printf("Rx discontinuity: %li -> %li (diff: %li), bytesIn: %i\n", lastTS, srcPkt->counter, samplesDiff, bytesIn);
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
                    stats.timestamp = lastTS;
                    reportProblems = true;
                }
                lastTS = srcPkt->counter;

                // parse samples data
                lime::complex32f_t *f32_dest[2] = {
                    static_cast<lime::complex32f_t *>(destPkt[i].chA),
                    static_cast<lime::complex32f_t *>(destPkt[i].chB)
                };
                destPkt[i].timestamp = srcPkt->counter;
                destPkt[i].start = 0;
                const int recv = FPGA::FPGAPacketPayload2SamplesFloat(srcPkt->data, sizeof(FPGA_DataPacket::data), mimo, compressed, f32_dest);
                destPkt[i].end = recv;
                destBlock.usage += sizeof(PacketType);
                srcBlock.offset += sizeof(FPGA_DataPacket);

                ++srcPkt;
            }
            assert(destBlock.usage <= destBlock.size);
            //assert(srcBlock.offset <= srcBlock.usage);

            destBlock.genTime = std::chrono::high_resolution_clock::now();
            if (rxOut.push(destBlock, false, 100))
            {
                destBlock.ptr = nullptr;
                const int fifoLevel = rxOut.size();
                maxFIFOlevel = std::max(maxFIFOlevel, fifoLevel);
                packetsOut += packetsToParse;
                destBlock.ptr = rxOutPool.Allocate(allocSize);
                destBlock.offset = 0;
                destBlock.usage = 0;
            }
            else
            {
                ++rxOverrun;
                ++stats.overrun;
                reportProblems = true;
                // reset buffer to reuse for next read
                destBlock.usage = 0;
            }

            // one callback for the entire batch
            if(reportProblems && mConfig.statusCallback)
                mConfig.statusCallback(&stats, mConfig.userData);
        }

        t2 = std::chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= statsPeriod_ms || terminateRx.load(std::memory_order_relaxed)) {
            t1 = t2;
            double dataRateBps = 1000.0 * totalBytesReceived / timePeriod;
            stats.dataRate_Bps = dataRateBps;
            stats.txDataRate_Bps = txDataRate_Bps.load(std::memory_order_relaxed);
            if (showStats)
            {
                ret = guarded_ioctl(fd, LITEPCIE_IOCTL_DMA_WRITER, &writer);
                // sprintf(ctemp, "%02X %02X %02X %02X %02X %02X %02X %02X", probeBuffer[0],probeBuffer[1],probeBuffer[2],probeBuffer[3],probeBuffer[4],probeBuffer[5], probeBuffer[6],probeBuffer[7]);
                printf("%s Rx: %.3f MB/s | FIFO:%i/%i/%i pktIn:%i pktOut:%i TS:%li overrun:%i loss:%i, txDrops:%i, shw:%li/%li\n",
                    trxPort->GetPathName().c_str(),
                    dataRateBps / 1000000.0,
                    rxOut.size(),
                    maxFIFOlevel,
                    rxOut.max_size(),
                    packetsRecv,
                    packetsOut,
                    lastTS,
                    rxOverrun,
                    rxDiscontinuity,
                    txDrops,
                    writer.sw_count,
                    writer.hw_count
                );
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
}

} // namespace lime
