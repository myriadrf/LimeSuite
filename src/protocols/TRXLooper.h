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
    void Setup(const lime::SDRDevice::StreamConfig &config);
    virtual void Start();
    virtual void Stop();
    float GetDataRate(bool tx);

    inline const lime::SDRDevice::StreamConfig& GetConfig() const {
      return mConfig;
    }

    virtual int StreamRx(void **samples, uint32_t count, SDRDevice::StreamMeta *meta);
    virtual int StreamTx(const void **samples, uint32_t count, const SDRDevice::StreamMeta *meta);

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

    PacketsFIFO<SamplesBlock> rxOut;
    PacketsFIFO<SamplesBlock> txIn;

    SamplesBlock rxStaging;
    SamplesBlock txStaging;

    MemoryPool rxOutPool;
    MemoryPool txInPool;

    std::thread rxParsingThread;
    std::thread txParsingThread;

    Profiler *rxProfiler;
    Profiler *txProfiler;

    int mMaxBufferSize;
    std::atomic<int> mThreadsReady;
    std::chrono::time_point<std::chrono::high_resolution_clock> pcStreamStart;
};

} // namespace lime

#endif /* TRXLooper_H */
