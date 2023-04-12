#ifndef TRXLooper_H
#define TRXLooper_H

#include <vector>
#include <atomic>
#include <thread>
#include "dataTypes.h"
#include "limesuite/SDRDevice.h"
#include "PacketsFIFO.h"
#include "MemoryPool.h"
#include "SamplesPacket.h"

namespace lime {
class FPGA;
class LMS7002M;

class TRXLooper
{
public:
    TRXLooper(FPGA *f, LMS7002M *chip, int id);
    virtual ~TRXLooper();

    uint64_t GetHardwareTimestamp(void);
    void SetHardwareTimestamp(const uint64_t now);
    virtual void Setup(const lime::SDRDevice::StreamConfig &config);
    virtual void Start();
    virtual void Stop();

    inline const lime::SDRDevice::StreamConfig& GetConfig() const {
      return mConfig;
    }

    virtual int StreamRx(void **samples, uint32_t count, SDRDevice::StreamMeta *meta);
    virtual int StreamTx(const void **samples, uint32_t count, const SDRDevice::StreamMeta *meta);

    void SetMessageLogCallback(SDRDevice::LogCallbackType callback) {
      mCallback_logMessage = callback;
    }

    SDRDevice::StreamStats GetStats(bool tx);

    typedef SamplesPacket<2> SamplesPacketType;
protected:
    virtual int RxSetup() { return 0; };
    virtual void ReceivePacketsLoop() = 0;
    virtual void RxTeardown() {};

    virtual int TxSetup() { return 0;};
    virtual void TransmitPacketsLoop() = 0;
    virtual void TxTeardown() {};

    uint64_t mTimestampOffset;
    lime::SDRDevice::StreamConfig mConfig;

    // void AlignRxTSP();
    // void AlignRxRF(bool restoreValues);
    // void AlignQuadrature(bool restoreValues);
    // void RstRxIQGen();
    // double GetPhaseOffset(int bin);
    FPGA *fpga;
    LMS7002M *lms;
    int chipId;

    typedef DataBlock RawDataBlock;
    typedef DataBlock SamplesBlock;

    std::atomic<int> mThreadsReady;
    std::chrono::time_point<std::chrono::steady_clock> steamClockStart;

    SDRDevice::LogCallbackType mCallback_logMessage;
    std::condition_variable streamActive;
    std::mutex streamMutex;
    bool mStreamEnabled;

    struct Stream
    {
      MemoryPool *memPool;
      SDRDevice::StreamStats stats;
      PacketsFIFO<SamplesPacketType*> *fifo;
      SamplesPacketType* stagingPacket;
      std::thread thread;
      std::atomic<uint64_t> lastTimestamp;
      std::atomic<bool> terminate;
      // how many packets to batch in data transaction
      // lower count will give better latency, but can cause problems with really high data rates
      uint16_t samplesInPkt;
      uint8_t packetsToBatch;

      Stream() : memPool(nullptr), fifo(nullptr), stagingPacket(nullptr)
      {
      }
    };

    Stream mRx;
    Stream mTx;
};

} // namespace lime

#endif /* TRXLooper_H */
