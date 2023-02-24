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
#include <thread>

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

template<class T>
class DeltaVariable
{
public:
    DeltaVariable(T init) : mValue(init), mLastValue(0) {};
    void set(T val) { mValue = val;}
    void add(T val) { mValue += val;}
    T delta() { T d = mValue - mLastValue; return d; } // value change since last reset
    T value() const { return mValue;}
    void checkpoint() { mLastValue = mValue; }
private:
    T mValue;
    T mLastValue;
};

TRXLooper_PCIE::TRXLooper_PCIE(LitePCIe *rxPort, LitePCIe *txPort, FPGA *f, LMS7002M *chip, uint8_t moduleIndex)
    : TRXLooper(f, chip, moduleIndex)
{
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
    mTxPacketsToBatch = 6;
    mRxPacketsToBatch = 6;
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
    const int chCount = std::max(mConfig.rxCount, mConfig.txCount);
    const int sampleSize = (mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 4 : 3); // sizeof IQ pair
    const int samplesInPkt = 256;//(mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I16 ? 1020 : 1360) / chCount;
    const int packetSize = sizeof(TxHeader) + samplesInPkt * sampleSize * chCount;

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
    const int upperAllocationLimit = 65536;//sizeof(complex32f_t) * mTxPacketsToBatch * samplesInPkt * chCount + SamplesPacketType::headerSize;
    mTx.memPool = new MemoryPool(1024, upperAllocationLimit, 4096, name);
    return 0;
}

template<class T>
class TxBufferManager
{
public:
    TxBufferManager(bool mimo, bool compressed, uint32_t maxSamplesInPkt, uint32_t maxPacketsInBatch) :
    header(nullptr), payloadPtr(nullptr), mData(nullptr),
        bytesUsed(0), mCapacity(0),
        maxPacketsInBatch(maxPacketsInBatch), maxSamplesInPkt(maxSamplesInPkt),
        packetsCreated(0), payloadSize(0)
    {
        bytesForFrame = (compressed ? 3 : 4) * (mimo ? 2: 1);
        conversion.srcFormat = SDRDevice::StreamConfig::DataFormat::F32;
        conversion.destFormat = compressed ? SDRDevice::StreamConfig::DataFormat::I12 : SDRDevice::StreamConfig::DataFormat::I16;
        conversion.channelCount = mimo ? 2 : 1;
        maxPayloadSize = std::min(4080u, bytesForFrame*maxSamplesInPkt);
    }

    void Reset(uint8_t* memPtr, uint32_t capacity)
    {
        packetsCreated = 0;
        bytesUsed = 0;
        mData = memPtr;
        mCapacity = capacity;
        memset(mData, 0, capacity);
        header = reinterpret_cast<TxHeader*>(mData);
        header->Clear();
        payloadSize = 0;
        payloadPtr = (uint8_t*)header + sizeof(TxHeader);
    }

    inline bool hasSpace() const {
        const bool packetNotFull = payloadSize < maxPayloadSize;
        const int spaceAvailable = mCapacity-bytesUsed > sizeof(TxHeader);
        return packetNotFull && spaceAvailable;
    }

    inline bool consume(T* src)
    {
        bool sendBuffer = false;
        while(!src->empty())
        {
            if (payloadSize >= maxPayloadSize || payloadSize == maxSamplesInPkt*bytesForFrame)
            {
                header = reinterpret_cast<TxHeader*>(mData+bytesUsed);
                header->Clear();
                payloadPtr = (uint8_t*)header + sizeof(TxHeader);
                payloadSize = 0;
            }

            header->ignoreTimestamp(!src->useTimestamp);
            if(payloadSize == 0)
            {
                ++packetsCreated;
                header->counter = src->timestamp;
                bytesUsed += sizeof(TxHeader);
            }
            const int freeSpace = std::min(maxPayloadSize - payloadSize, mCapacity - bytesUsed - 16);
            uint32_t transferCount = std::min(freeSpace/bytesForFrame, src->size());
            transferCount = std::min(transferCount, maxSamplesInPkt);
            if (transferCount > 0)
            {
                int samplesDataSize = Interleave(src, transferCount, conversion, payloadPtr);
                payloadPtr = payloadPtr+samplesDataSize;
                payloadSize += samplesDataSize;
                bytesUsed += samplesDataSize;
                header->SetPayloadSize(payloadSize);
                assert(payloadSize > 0);
                assert(payloadSize <= maxPayloadSize);
            }
            else
                sendBuffer = true;

            if (packetsCreated >= maxPacketsInBatch && !hasSpace())
                sendBuffer = true;

            if (bytesUsed >= mCapacity - sizeof(TxHeader))
                sendBuffer = true; // not enough space for more packets, need to flush
            if ((uint64_t)payloadPtr & 0xF)
                sendBuffer = true; // next packets payload memory is not suitably aligned for vectorized filling

            if (sendBuffer)
            {
                const int busWidthBytes = 16;
                int extraBytes = bytesUsed % busWidthBytes;
                if (extraBytes != 0)
                {
                    //printf("Patch buffer, bytes %i, extra: %i, last payload: %i\n", bytesUsed, extraBytes, payloadSize);
                    // patch last packet so that whole buffer size would be multiple of bus width
                    int padding = busWidthBytes-extraBytes;
                    memset(payloadPtr, 0, padding);
                    payloadSize += padding;
                    bytesUsed += padding;
                    header->SetPayloadSize(payloadSize);
                    //printf("Patch buffer, bytes %i, last payload: %i\n", bytesUsed, payloadSize);
                }
                break;
            }
        }
        if (!hasSpace())
            return true;
        return src->flush || sendBuffer;
    }

    inline int size() const { return bytesUsed; };
    inline uint8_t* data() const { return mData; };
    inline int packetCount() const { return packetsCreated; };
private:
    DataConversion conversion;
    TxHeader* header;
    uint8_t* payloadPtr;
    uint8_t* mData;
    uint32_t bytesUsed;
    uint32_t mCapacity;
    uint32_t maxPacketsInBatch;
    uint32_t maxSamplesInPkt;
    uint32_t maxPayloadSize;
    uint16_t packetsCreated;
    uint16_t payloadSize;
    uint8_t bytesForFrame;
};

void FPGATxState(FPGA* fpga)
{
    uint32_t words[4];
    for(int i=0; i<5; ++i)
    {
        uint64_t pendingTxTS = 0;
        const uint32_t addrs[4] = {0x61u+i*4, 0x62u+i*4, 0x63u+i*4, 0x64u+i*4};
        fpga->ReadRegisters(addrs, words, 4);
        pendingTxTS |= words[0];
        pendingTxTS |= words[1] << 16;
        pendingTxTS |= (uint64_t)words[2] << 32;
        pendingTxTS |= (uint64_t)words[3] << 48;
        if (i < 4)
            printf("Buf%i: %08lX\n", i, pendingTxTS);
        else
            printf("Rx: %08lX\n", pendingTxTS);
    }

    uint16_t bufs = fpga->ReadRegister(0x0075);
    printf("currentIndex: %i, ready: %0X\n", bufs & 0xF, (bufs>>4) & 0xF);
}

void TRXLooper_PCIE::TransmitPacketsLoop()
{
    const bool mimo = std::max(mConfig.txCount, mConfig.rxCount) > 1;
    const bool compressed = mConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::I12;
    const int irqPeriod = 4;

    const int32_t bufferCount = mTxArgs.buffers.size();
    const uint32_t overflowLimit = bufferCount-irqPeriod;
    const int32_t bufferSize = mTxArgs.bufferSize;
    const std::vector<uint8_t*> &dmaBuffers = mTxArgs.buffers;

    SDRDevice::StreamStats &stats = mTx.stats;

    auto fifo = mTx.fifo;

    int64_t totalBytesSent = 0; //for data rate calculation
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

    uint32_t stagingBufferIndex = 0;
    SamplesPacketType* srcPkt = nullptr;

    TxBufferManager<SamplesPacketType> output(mimo, compressed, mTxArgs.samplesInPacket, mTxArgs.packetsToBatch);

    mTxArgs.port->CacheFlush(true, false, 0);
    output.Reset(dmaBuffers[0], mTxArgs.bufferSize);

    bool outputReady = false;

    AvgRmsCounter txTSAdvance;
    AvgRmsCounter transferSize;

    // Initialize DMA
    mTxArgs.port->TxDMAEnable(true);
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

    DeltaVariable<int32_t> underrun(0);
    DeltaVariable<int32_t> loss(0);

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
                if(!fifo->pop(&srcPkt, true, 100))
                {
                    std::this_thread::yield();
                    break;
                }
            }

            // drop old packets before forming that
            int64_t rxNow = rxLastTimestamp.load(std::memory_order_relaxed);
            const int64_t txAdvance = srcPkt->timestamp - rxNow;
            if(mConfig.hintSampleRate)
            {
                int64_t timeAdvance = ts_to_us(mConfig.hintSampleRate, txAdvance);
                txTSAdvance.Add(timeAdvance);
            }
            else
                txTSAdvance.Add(txAdvance);
            if(txAdvance <= 0)
            {
                underrun.add(1);
                ++stats.underrun;
                mTx.memPool->Free(srcPkt);
                srcPkt = nullptr;
                continue;
            }

            const bool doFlush = output.consume(srcPkt);
            
            if(srcPkt->empty())
            {
                mTx.memPool->Free(srcPkt);
                srcPkt = nullptr;
            }
            if(doFlush)
            {
                stats.packets += output.packetCount();
                outputReady = true;
                mTxArgs.port->CacheFlush(true, true, stagingBufferIndex % bufferCount);
                break;
            }
        }
        uint16_t pendingBuffers = stagingBufferIndex - state.hwIndex;
        bool canSend = pendingBuffers < overflowLimit;
        if (!canSend)
        {
            if (mConfig.extraConfig == nullptr || mConfig.extraConfig->usePoll)
                canSend = mTxArgs.port->WaitTx();
            else
                std::this_thread::yield();
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

            TxHeader* pkt = reinterpret_cast<TxHeader*>(output.data());
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
                underrun.add(1);
                ++stats.underrun;
                // TODO: first packet in the buffer is already late, could just skip this
                // buffer transmission, but packets at the end of buffer might just still
                // make it in time.
                // outputReady = false;
                // output.Reset(dmaBuffers[stagingBufferIndex % bufferCount], mTxArgs.bufferSize);
                // continue;
            }

            // DMA memory is write only, to read from the buffer will trigger Bus errors
            int ret = mTxArgs.port->SetTxDMAState(state);
            if(ret)
            {
                if (errno == EINVAL)
                {
                    printf("Failed to submit dma write (%i) %s\n", errno, strerror(errno));
                    return;
                }
                mTxArgs.port->WaitTx();
                ++stats.overrun;
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
                mTxArgs.port->CacheFlush(true, false, stagingBufferIndex % bufferCount);
                output.Reset(dmaBuffers[stagingBufferIndex % bufferCount], mTxArgs.bufferSize);
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
            loss.set(stats.loss);
            if(showStats || mCallback_logMessage)
            {
                fpga->WriteRegister(0xFFFF, 1 << chipId);
                const uint16_t addr = 0x7FE2 + (chipId*5);
                uint32_t addrs[] = {addr, addr+1u};
                uint32_t values[2];
                fpga->ReadRegisters(addrs, values, 2);
                const uint32_t fpgaTxPktIngressCount = (values[0] << 16) | values[1];

                char msg[512];
                snprintf(msg, sizeof(msg)-1, "%s Tx: %3.3f MB/s | TS:%li pkt:%li o:%i totalOut:(x%08X)-fpga(x%08X)=%i, shw:%u/%u(%+i) u:%i(%+i) l:%i(%+i) tsAdvance:%+.0f/%+.0f/%+.0f%s, f:%i",
                    mRxArgs.port->GetPathName().c_str(),
                    dataRate / 1000000.0,
                    lastTS,
                    stats.packets,
                    stats.overrun,
                    totalPacketSent&0xFFFFFFFF,
                    fpgaTxPktIngressCount,
                    totalPacketSent - fpgaTxPktIngressCount,
                    state.swIndex,
                    state.hwIndex,
                    state.swIndex-state.hwIndex,
                    underrun.value(), underrun.delta(),
                    loss.value(), loss.delta(),
                    txTSAdvance.Min(),
                    avgTxAdvance,
                    txTSAdvance.Max(),
                    (mConfig.hintSampleRate ? "us" : ""),
                    fifo->size()
                );
                if(showStats)
                    printf("%s\n", msg);
                if(mCallback_logMessage)
                {
                    bool showAsWarning = underrun.delta() || loss.delta();
                    SDRDevice::LogLevel level = showAsWarning ? SDRDevice::LogLevel::WARNING : SDRDevice::LogLevel::DEBUG;
                    mCallback_logMessage(level, msg);
                }
            }
            loss.checkpoint();
            underrun.checkpoint();
            packetsSent = 0;
            totalBytesSent = 0;
            txDataRate_Bps.store((uint32_t)dataRate, std::memory_order_relaxed);
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
        sprintf(msg, "Tx Loop totals: packets sent: %li (0x%08lX) , FPGA packet counter: %i (0x%08X), diff: %li, FPGA tx drops: %i\n",
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

    int irqPeriod = 16;
    float bufferTimeDuration = 0;
    if (mConfig.hintSampleRate > 0)
    {
        bufferTimeDuration = float(samplesInPkt*mRxPacketsToBatch) / mConfig.hintSampleRate;
        irqPeriod = 80e-6 / bufferTimeDuration;
    }
    irqPeriod = clamp(irqPeriod, 1, 16);
    irqPeriod = 4;

    if(mCallback_logMessage)
    {
        char msg[256];
        sprintf(msg, "Stream%i usePoll:%i rxSamplesInPkt:%i rxPacketsInBatch:%i, DMA_ReadSize:%i, batchSizeInTime:%gus\n",
            chipId, usePoll ? 1 : 0, samplesInPkt, mRxPacketsToBatch, mRxPacketsToBatch*packetSize, bufferTimeDuration*1e6);
        mCallback_logMessage(SDRDevice::LogLevel::DEBUG, msg);
    }

    // float expectedDataRateBps = 0;
    // if (mConfig.hintSampleRate != 0)
    //     expectedDataRateBps = (mConfig.hintSampleRate/samplesInPkt) * (16 + samplesInPkt * sampleSize * chCount);

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

    const int32_t readSize = mRxArgs.packetSize * mRxArgs.packetsToBatch;
    mRxArgs.port->RxDMAEnable(true, readSize, irqPeriod);
    return 0;
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

    DeltaVariable<int32_t> overrun(0);
    DeltaVariable<int32_t> loss(0);

    // thread ready for work, just wait for stream enable
    {
        std::unique_lock<std::mutex> lk(streamMutex);
        while (!mStreamEnabled && !terminateRx.load(std::memory_order_relaxed))
            streamActive.wait_for(lk, milliseconds(100));
        lk.unlock();
    }

    uint8_t irqPeriod = 8;
    // Anticipate the overflow 2 interrupts early, just in case of missing an interrupt
    // Avoid situations where CPU and device is at the same buffer index
    // CPU reading while device writing creates coherency issues.
    const uint16_t overrunLimit = std::max(bufferCount-4, bufferCount/2);
//    mRxArgs.port->RxDMAEnable(true, readSize, irqPeriod);
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

            char msg[512];
            snprintf(msg, sizeof(msg)-1, "%s Rx: %3.3f MB/s | TS:%li pkt:%li o:%i(%+i) l:%i(%+i) dma:%u/%u(%u) swFIFO:%i",
                mRxArgs.port->GetPathName().c_str(),
                stats.dataRate_Bps / 1e6,
                stats.timestamp,
                stats.packets,
                overrun.value(), overrun.delta(),
                loss.value(), loss.delta(),
                dma.swIndex,
                dma.hwIndex,
                dma.hwIndex-dma.swIndex,
                mRx.fifo->size()
            );
            if(showStats)
                printf("%s\n", msg);
            if(mCallback_logMessage)
            {
                bool showAsWarning = overrun.delta() || loss.delta();
                SDRDevice::LogLevel level = showAsWarning ? SDRDevice::LogLevel::WARNING : SDRDevice::LogLevel::DEBUG;
                mCallback_logMessage(level, msg);
            }
            overrun.checkpoint();
            loss.checkpoint();
            Bps = 0;
        }

        uint16_t buffersAvailable = dma.hwIndex - dma.swIndex;

        // process received data
        bool reportProblems = false;
        if(buffersAvailable >= overrunLimit) // data overflow
        {
            // jump CPU to 1 buffer behind hardware index, to avoid device starting to write into buffer being read by CPU
            dma.swIndex = (dma.hwIndex - 1);
            ++stats.loss;
            overrun.add(1);
        }

        if (!buffersAvailable)
        {
            if(!mConfig.extraConfig || (mConfig.extraConfig && mConfig.extraConfig->usePoll))
            {
                if (mRxArgs.port->WaitRx())
                    continue;
                else
                    std::this_thread::yield();
            }
            continue;
        }

        mRxArgs.port->CacheFlush(false, false, dma.swIndex % bufferCount);
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
                loss.add(1);
            }
            if (pkt->txWasDropped())
                ++mTx.stats.loss;

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
        mRxArgs.port->CacheFlush(false, true, dma.swIndex % bufferCount);

        ++dma.swIndex;
        mRxArgs.port->SetRxDMAState(dma);

        mRxArgs.sw = dma.swIndex;
        mRxArgs.hw = dma.hwIndex;
        ++mRxArgs.cnt;
        // one callback for the entire batch
        if(reportProblems && mConfig.statusCallback)
            mConfig.statusCallback(&stats, mConfig.userData);
        std::this_thread::yield();
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
