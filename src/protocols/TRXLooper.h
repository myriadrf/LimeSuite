#ifndef TRXLooper_H
#define TRXLooper_H

#include <vector>
#include <atomic>
#include <thread>
#include "limesuite/SDRDevice.h"
#include "limesuite/complex.h"
#include "PacketsFIFO.h"
#include "MemoryPool.h"
#include "SamplesPacket.h"

namespace lime {
class FPGA;
class LMS7002M;

/** @brief Class responsible for receiving and transmitting continuous sample data */
class TRXLooper
{
  public:
    TRXLooper(FPGA* f, LMS7002M* chip, int id);
    virtual ~TRXLooper();

    uint64_t GetHardwareTimestamp() const;
    OpStatus SetHardwareTimestamp(const uint64_t now);
    virtual OpStatus Setup(const lime::SDRDevice::StreamConfig& cfg);
    virtual void Start();
    virtual void Stop();

    /// @brief Gets whether the stream is currently running or not.
    /// @return The current status of the stream (true if running).
    constexpr bool IsStreamRunning() const { return mStreamEnabled; }

    /// @brief Gets the current configuration of the stream.
    /// @return The current configuration of the stream.
    constexpr const lime::SDRDevice::StreamConfig& GetConfig() const { return mConfig; }

    virtual uint32_t StreamRx(lime::complex32f_t* const* samples, uint32_t count, SDRDevice::StreamMeta* meta);
    virtual uint32_t StreamRx(lime::complex16_t* const* samples, uint32_t count, SDRDevice::StreamMeta* meta);
    virtual uint32_t StreamTx(const lime::complex32f_t* const* samples, uint32_t count, const SDRDevice::StreamMeta* meta);
    virtual uint32_t StreamTx(const lime::complex16_t* const* samples, uint32_t count, const SDRDevice::StreamMeta* meta);

    /// @brief Sets the callback to use for message logging.
    /// @param callback The new callback to use.
    void SetMessageLogCallback(SDRDevice::LogCallbackType callback) { mCallback_logMessage = callback; }

    SDRDevice::StreamStats GetStats(TRXDir tx) const;

    /// @brief The type of a sample packet.
    typedef SamplesPacket<2> SamplesPacketType;

  protected:
    virtual int RxSetup() { return 0; };
    virtual void ReceivePacketsLoop() = 0;
    virtual void RxTeardown(){};

    virtual int TxSetup() { return 0; };
    virtual void TransmitPacketsLoop() = 0;
    virtual void TxTeardown(){};

    uint64_t mTimestampOffset;
    lime::SDRDevice::StreamConfig mConfig;

    FPGA* fpga;
    LMS7002M* lms;
    int chipId;

    std::chrono::time_point<std::chrono::steady_clock> streamClockStart;

    SDRDevice::LogCallbackType mCallback_logMessage;
    std::condition_variable streamActive;
    std::mutex streamMutex;
    bool mStreamEnabled;

    struct Stream {
        MemoryPool* memPool;
        SDRDevice::StreamStats stats;
        PacketsFIFO<SamplesPacketType*>* fifo;
        SamplesPacketType* stagingPacket;
        std::thread thread;
        std::atomic<uint64_t> lastTimestamp;
        std::atomic<bool> terminate;
        // how many packets to batch in data transaction
        // lower count will give better latency, but can cause problems with really high data rates
        uint16_t samplesInPkt;
        uint8_t packetsToBatch;

        Stream()
            : memPool(nullptr)
            , fifo(nullptr)
            , stagingPacket(nullptr)
        {
        }

        ~Stream()
        {
            if (fifo != nullptr)
            {
                delete fifo;
            }

            DeleteMemoryPool();
        }

        void DeleteMemoryPool()
        {
            if (memPool == nullptr)
            {
                return;
            }

            if (stagingPacket != nullptr)
            {
                memPool->Free(stagingPacket);
                stagingPacket = nullptr;
            }

            delete memPool;
            memPool = nullptr;
        }
    };

    Stream mRx;
    Stream mTx;

  private:
    template<class T> uint32_t StreamRxTemplate(T* const* dest, uint32_t count, SDRDevice::StreamMeta* meta);
    template<class T> uint32_t StreamTxTemplate(const T* const* samples, uint32_t count, const SDRDevice::StreamMeta* meta);
};

} // namespace lime

#endif /* TRXLooper_H */
