#ifndef TRXLooper_H
#define TRXLooper_H

#include <vector>
#include <atomic>
#include <thread>
#include "dataTypes.h"
#include "SDRDevice.h"
#include "PacketsFIFO.h"
#include "Profiler.h"
#include "MemoryPool.h"

namespace lime {
class FPGA;
class LMS7002M;

class TRXLooper
{
public:
    TRXLooper(FPGA *f, LMS7002M *chip, int id);
    virtual ~TRXLooper();

    int GetStreamSize(bool tx);

    uint64_t GetHardwareTimestamp(void);
    void SetHardwareTimestamp(const uint64_t now);
    virtual void Setup(const lime::SDRDevice::StreamConfig &config);
    virtual void Start();
    virtual void Stop();
    float GetDataRate(bool tx);

    inline const lime::SDRDevice::StreamConfig& GetConfig() const {
      return mConfig;
    }

    virtual int StreamRx(void **samples, uint32_t count, SDRDevice::StreamMeta *meta);
    virtual int StreamTx(const void **samples, uint32_t count, const SDRDevice::StreamMeta *meta);

    void SetMessageLogCallback(SDRDevice::LogCallbackType callback) {
      mCallback_logMessage = callback;
    }
  protected:
    virtual void ReceivePacketsLoop() = 0;
    virtual void TransmitPacketsLoop() = 0;

    void ParseRxPacketsLoop();
    void ParseTxPacketsLoop();

    std::atomic<uint32_t> rxDataRate_Bps;
    std::atomic<uint32_t> txDataRate_Bps;
    std::thread rxThread;
    std::thread txThread;
    std::atomic<bool> terminateRx;
    std::atomic<bool> terminateTx;

    std::atomic<uint64_t> rxLastTimestamp;
    std::atomic<uint64_t> txLastTimestamp;
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

    typedef StagingPacket<lime::complex32f_t, 2> StagingPacketType;


    PacketsFIFO<StagingPacketType*> rxOut;
    PacketsFIFO<StagingPacketType*> txIn;

    PacketsFIFO<StagingPacketType*> rxPacketsPool;
    PacketsFIFO<StagingPacketType*> txPacketsPool;
    // MemoryPool rxOutPool;
    // MemoryPool txInPool;

    std::thread rxParsingThread;
    std::thread txParsingThread;

    Profiler *rxProfiler;
    Profiler *txProfiler;

    int mMaxBufferSize;
    std::atomic<int> mThreadsReady;
    std::chrono::time_point<std::chrono::steady_clock> steamClockStart;

    // how many packets to batch in data transaction
    // lower count will give better latency, but can cause problems with really high data rates
    uint8_t mRxPacketsToBatch;
    uint8_t mTxPacketsToBatch;
    SDRDevice::LogCallbackType mCallback_logMessage;
    std::condition_variable streamActive;
    std::mutex streamMutex;
    bool mStreamEnabled;
private:
    StagingPacketType *txStaging;
    StagingPacketType *rxStaging;
};

} // namespace lime

#endif /* TRXLooper_H */
