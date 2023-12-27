#include <assert.h>
#include "FPGA_common.h"
#include "limesuite/LMS7002M.h"
#include <ciso646>
#include "Logger.h"
#include <complex>
#include "LMSBoards.h"
#include "threadHelper.h"

#include "TRXLooper.h"

using namespace std;

namespace lime {

using namespace std::chrono;

static constexpr uint16_t defaultSamplesInPkt = 256;

/// @brief Constructs a new TRXLooper object.
/// @param f The FPGA device to use for streaming.
/// @param chip The LMS7002M device to use for streaming.
/// @param id The ID of the chip to use.
TRXLooper::TRXLooper(FPGA* f, LMS7002M* chip, int id)
    : mCallback_logMessage(nullptr)
    , mStreamEnabled(false)
{
    mRx.packetsToBatch = 4;
    mTx.packetsToBatch = 4;
    mTx.samplesInPkt = defaultSamplesInPkt;
    mRx.samplesInPkt = defaultSamplesInPkt;
    const int fifoLen = 512;
    mRx.fifo = new PacketsFIFO<SamplesPacketType*>(fifoLen);
    mTx.fifo = new PacketsFIFO<SamplesPacketType*>(fifoLen);

    lms = chip, fpga = f;
    chipId = id;
    mTimestampOffset = 0;
    mRx.lastTimestamp.store(0, std::memory_order_relaxed);
    mRx.terminate.store(false, std::memory_order_relaxed);
    mTx.terminate.store(false, std::memory_order_relaxed);
}

TRXLooper::~TRXLooper()
{
    Stop();
}

/// @brief Gets the current timestamp of the hardware.
/// @return The current timestamp of the hardware.
uint64_t TRXLooper::GetHardwareTimestamp() const
{
    return mRx.lastTimestamp.load(std::memory_order_relaxed) + mTimestampOffset;
}

/// @brief Sets the hardware timestamp.
/// @param now The current timestamp to set.
void TRXLooper::SetHardwareTimestamp(const uint64_t now)
{
    mTimestampOffset = now - mRx.lastTimestamp.load(std::memory_order_relaxed);
}

/// @brief Sets up the stream of this looper.
/// @param cfg The configuration settings to set up the stream with.
void TRXLooper::Setup(const SDRDevice::StreamConfig& cfg)
{
    if (mRx.thread.joinable() || mTx.thread.joinable())
        throw std::logic_error("Samples streaming already running");

    bool needTx = cfg.txCount > 0;
    bool needRx = cfg.rxCount > 0; // always need Rx to know current timestamps, cfg.rxCount > 0;
    //bool needMIMO = cfg.rxCount > 1 || cfg.txCount > 1; // TODO: what if using only B channel, does it need MIMO configuration?
    uint8_t channelEnables = 0;

    for (int i = 0; i < cfg.rxCount; ++i)
    {
        if (cfg.rxChannels[i] > 1)
            throw std::logic_error("Invalid Rx channel, only [0,1] channels supported");
        else
            channelEnables |= (1 << cfg.rxChannels[i]);
    }
    for (int i = 0; i < cfg.txCount; ++i)
    {
        if (cfg.txChannels[i] > 1)
            throw std::logic_error("Invalid Tx channel, only [0,1] channels supported");
        else
            channelEnables |= (1 << cfg.txChannels[i]); // << 8;
    }
    if ((cfg.linkFormat != SDRDevice::StreamConfig::DataFormat::I12) &&
        (cfg.linkFormat != SDRDevice::StreamConfig::DataFormat::I16))
        throw std::logic_error("Unsupported stream link format");
    mConfig = cfg;

    //configure FPGA on first start, or disable FPGA when not streaming
    if (!needTx && !needRx)
        return;

    assert(fpga);
    fpga->WriteRegister(0xFFFF, 1 << chipId);
    fpga->StopStreaming();
    fpga->WriteRegister(0xD, 0); //stop WFM
    mRx.lastTimestamp.store(0, std::memory_order_relaxed);

    // const uint16_t MIMO_EN = needMIMO << 8;
    // const uint16_t TRIQ_PULSE = lms->Get_SPI_Reg_bits(LMS7param(LML1_TRXIQPULSE)) << 7; // 0-OFF, 1-ON
    // const uint16_t DDR_EN = lms->Get_SPI_Reg_bits(LMS7param(LML1_SISODDR)) << 6; // 0-SDR, 1-DDR
    // const uint16_t MODE = 0 << 5; // 0-TRXIQ, 1-JESD207 (not impelemented)
    // const uint16_t smpl_width =
    //     cfg.linkFormat == SDRDevice::StreamConfig::DataFormat::I12 ? 2 : 0;
    // printf("TRIQ:%i, DDR_EN:%i, MIMO_EN:%i\n", TRIQ_PULSE, DDR_EN, MIMO_EN);
    // const uint16_t reg8 = MIMO_EN | TRIQ_PULSE | DDR_EN | MODE | smpl_width;

    uint16_t mode = 0x0100;
    if (lms->Get_SPI_Reg_bits(LMS7param(LML1_SISODDR)))
        mode = 0x0040;
    else if (lms->Get_SPI_Reg_bits(LMS7param(LML1_TRXIQPULSE)))
        mode = 0x0180;

    const uint16_t smpl_width = cfg.linkFormat == SDRDevice::StreamConfig::DataFormat::I12 ? 2 : 0;
    fpga->WriteRegister(0x0008, mode | smpl_width);
    fpga->WriteRegister(0x0007, channelEnables);
    fpga->ResetTimestamp();

    constexpr uint16_t waitGPS_PPS = 1 << 2;
    int interface_ctrl_000A = fpga->ReadRegister(0x000A);
    interface_ctrl_000A &= ~waitGPS_PPS; // disable by default
    if (cfg.extraConfig && cfg.extraConfig->waitPPS)
        interface_ctrl_000A |= waitGPS_PPS;
    fpga->WriteRegister(0x000A, interface_ctrl_000A);

    // Don't just use REALTIME scheduling, or at least be cautious with it.
    // if the thread blocks for too long, Linux can trigger RT throttling
    // which can cause unexpected data packet losses and timing issues.
    // Also need to set policy to default here, because if host process is running
    // with REALTIME policy, these threads would inherit it and exhibit mentioned
    // issues.
    const auto schedulingPolicy = ThreadPolicy::PREEMPTIVE;
    if (needRx)
    {
        mRx.terminate.store(false, std::memory_order_relaxed);
        auto RxLoopFunction = std::bind(&TRXLooper::ReceivePacketsLoop, this);
        mRx.thread = std::thread(RxLoopFunction);
        SetOSThreadPriority(ThreadPriority::HIGHEST, schedulingPolicy, &mRx.thread);
        pthread_setname_np(mRx.thread.native_handle(), "lime:RxLoop");

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(2, &cpuset);
        //int rc = pthread_setaffinity_np(mRx.thread.native_handle(), sizeof(cpu_set_t), &cpuset);
        // if (rc != 0) {
        //   printf("Error calling pthread_setaffinity_np: %i\n", rc);
        // }
    }
    if (needTx)
    {
        mTx.terminate.store(false, std::memory_order_relaxed);
        auto TxLoopFunction = std::bind(&TRXLooper::TransmitPacketsLoop, this);
        mTx.thread = std::thread(TxLoopFunction);
        SetOSThreadPriority(ThreadPriority::HIGHEST, schedulingPolicy, &mTx.thread);
        pthread_setname_np(mTx.thread.native_handle(), "lime:TxLoop");

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(3, &cpuset);
        //int rc = pthread_setaffinity_np(mTx.thread.native_handle(), sizeof(cpu_set_t), &cpuset);
        // if (rc != 0) {
        //   printf("Error calling pthread_setaffinity_np: %i\n", rc);
        // }
    }

    // if (cfg.alignPhase)
    //     TODO: AlignRxRF(true);
    //enable FPGA streaming
}

/// @brief Starts the stream of this looper.
void TRXLooper::Start()
{
    mRx.fifo->clear();
    mTx.fifo->clear();

    fpga->StartStreaming();

    {
        std::lock_guard<std::mutex> lock(streamMutex);
        mStreamEnabled = true;
        streamActive.notify_all();
    }

    streamClockStart = steady_clock::now();
    //int64_t startPoint = std::chrono::time_point_cast<std::chrono::microseconds>(pcStreamStart).time_since_epoch().count();
    //printf("Stream%i start %lius\n", chipId, startPoint);
    // if (!mConfig.alignPhase)
    //     lms->ResetLogicregisters();
}

/// @brief Stops the stream and cleans up all the memory.
void TRXLooper::Stop()
{
    if (!mStreamEnabled)
    {
        return;
    }

    mTx.terminate.store(true, std::memory_order_relaxed);
    mRx.terminate.store(true, std::memory_order_relaxed);
    try
    {
        if (mRx.thread.joinable())
            mRx.thread.join();

        if (mTx.thread.joinable())
            mTx.thread.join();
    } catch (...)
    {
        printf("Failed to join TRXLooper threads\n");
    }
    fpga->StopStreaming();

    RxTeardown();
    TxTeardown();

    if (mRx.stagingPacket != nullptr)
    {
        mRx.memPool->Free(mRx.stagingPacket);
        mRx.stagingPacket = nullptr;
    }

    if (mTx.stagingPacket != nullptr)
    {
        mTx.memPool->Free(mTx.stagingPacket);
        mTx.stagingPacket = nullptr;
    }

    if (mRx.memPool != nullptr)
    {
        delete mRx.memPool;
        mRx.memPool = nullptr;
    }

    if (mTx.memPool != nullptr)
    {
        delete mTx.memPool;
        mTx.memPool = nullptr;
    }

    mStreamEnabled = false;
}

template<class T> uint32_t TRXLooper::StreamRxTemplate(T** dest, uint32_t count, SDRDevice::StreamMeta* meta)
{
    bool timestampSet = false;
    uint32_t samplesProduced = 0;
    const bool useChannelB = mConfig.rxCount > 1;

    bool firstIteration = true;

    //auto start = high_resolution_clock::now();
    while (samplesProduced < count)
    {
        if (!mRx.stagingPacket && !mRx.fifo->pop(&mRx.stagingPacket, firstIteration, 250))
        {
            return samplesProduced;
        }

        if (!timestampSet && meta)
        {
            meta->timestamp = mRx.stagingPacket->metadata.timestamp;
            timestampSet = true;
        }

        uint32_t expectedCount = count - samplesProduced;
        const uint32_t samplesToCopy = std::min(expectedCount, mRx.stagingPacket->size());

        T* const* src = reinterpret_cast<T* const*>(mRx.stagingPacket->front());

        std::memcpy(&dest[0][samplesProduced], src[0], samplesToCopy * sizeof(T));

        if (useChannelB)
        {
            std::memcpy(&dest[1][samplesProduced], src[1], samplesToCopy * sizeof(T));
        }

        mRx.stagingPacket->pop(samplesToCopy);
        samplesProduced += samplesToCopy;

        if (mRx.stagingPacket->empty())
        {
            mRx.memPool->Free(mRx.stagingPacket);
            mRx.stagingPacket = nullptr;
        }

        // int duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
        // if(duration > 300) // TODO: timeout duration in meta
        //     return samplesProduced;
    }

    return samplesProduced;
}

/// @brief Reveives samples from this specific stream.
/// @param samples The buffer to put the received samples in.
/// @param count The amount of samples to reveive.
/// @param meta The metadata of the packets of the stream.
/// @return The amount of samples received.
uint32_t TRXLooper::StreamRx(complex32f_t** samples, uint32_t count, SDRDevice::StreamMeta* meta)
{
    return StreamRxTemplate<complex32f_t>(samples, count, meta);
}

/// @copydoc TRXLooper::StreamRx()
uint32_t TRXLooper::StreamRx(complex16_t** samples, uint32_t count, SDRDevice::StreamMeta* meta)
{
    return StreamRxTemplate<complex16_t>(samples, count, meta);
}

template<class T> uint32_t TRXLooper::StreamTxTemplate(const T* const* samples, uint32_t count, const SDRDevice::StreamMeta* meta)
{
    const bool useChannelB = mConfig.txCount > 1;
    const bool useTimestamp = meta ? meta->waitForTimestamp : false;
    const bool flush = meta && meta->flushPartialPacket;
    int64_t ts = meta ? meta->timestamp : 0;

    uint32_t samplesRemaining = count;

    const int samplesInPkt = mTx.samplesInPkt;
    const int packetsToBatch = mTx.packetsToBatch;
    const int32_t outputPktSize = SamplesPacketType::headerSize + packetsToBatch * samplesInPkt * sizeof(T);

    if (mTx.stagingPacket && mTx.stagingPacket->metadata.timestamp + mTx.stagingPacket->size() != meta->timestamp)
    {
        if (!mTx.fifo->push(mTx.stagingPacket))
        {
            return 0;
        }

        mTx.stagingPacket = nullptr;
    }

    const T* src[2] = { samples[0], useChannelB ? samples[1] : nullptr };
    while (samplesRemaining > 0)
    {
        if (!mTx.stagingPacket)
        {
            mTx.stagingPacket = SamplesPacketType::ConstructSamplesPacket(
                mTx.memPool->Allocate(outputPktSize), samplesInPkt * packetsToBatch, sizeof(T));

            if (!mTx.stagingPacket)
            {
                break;
            }

            mTx.stagingPacket->Reset();
            mTx.stagingPacket->metadata.timestamp = ts;
            mTx.stagingPacket->metadata.waitForTimestamp = useTimestamp;
        }

        int consumed = mTx.stagingPacket->push(src, samplesRemaining);
        src[0] += consumed;
        if (useChannelB)
        {
            src[1] += consumed;
        }

        samplesRemaining -= consumed;
        ts += consumed;

        if (mTx.stagingPacket->isFull() || flush)
        {
            if (samplesRemaining == 0)
            {
                mTx.stagingPacket->metadata.flushPartialPacket = flush;
            }

            if (!mTx.fifo->push(mTx.stagingPacket))
            {
                break;
            }

            mTx.stagingPacket = nullptr;
        }
    }

    return count - samplesRemaining;
}

/// @brief Transmits packets from from this specific stream.
/// @param samples The buffer of the samples to transmit.
/// @param count The amount of samples to transmit.
/// @param meta The metadata of the packets of the stream.
/// @return The amount of samples transmitted.
uint32_t TRXLooper::StreamTx(const lime::complex32f_t* const* samples, uint32_t count, const SDRDevice::StreamMeta* meta)
{
    return StreamTxTemplate(samples, count, meta);
}

/// @copydoc TRXLooper::StreamTx()
uint32_t TRXLooper::StreamTx(const lime::complex16_t* const* samples, uint32_t count, const SDRDevice::StreamMeta* meta)
{
    return StreamTxTemplate(samples, count, meta);
}

/// @brief Gets statistics from a specified transfer direction.
/// @param dir The direction of which to get the statistics.
/// @return The statistics of the transfers.
SDRDevice::StreamStats TRXLooper::GetStats(TRXDir dir) const
{
    SDRDevice::StreamStats stats;

    if (dir == TRXDir::Tx)
    {
        stats = mTx.stats;
        stats.FIFO = { mTx.fifo->max_size(), mTx.fifo->size() };
    }
    else
    {
        stats = mRx.stats;
        stats.FIFO = { mRx.fifo->max_size(), mRx.fifo->size() };
    }

    return stats;
}

} // namespace lime
