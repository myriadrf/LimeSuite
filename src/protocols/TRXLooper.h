#ifndef TRXLooper_H
#define TRXLooper_H

#include <vector>
#include <atomic>
#include <thread>
#include "dataTypes.h"
#include "SDRDevice.h"

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
    void Start(const lime::SDRDevice::StreamConfig &config);
    void Stop();
    float GetDataRate(bool tx);

  protected:
    virtual void ReceivePacketsLoop() = 0;
    virtual void TransmitPacketsLoop() = 0;

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
    uint8_t rxActiveChannelCount;
    uint8_t txActiveChannelCount;

    // void AlignRxTSP();
    // void AlignRxRF(bool restoreValues);
    // void AlignQuadrature(bool restoreValues);
    // void RstRxIQGen();
    // double GetPhaseOffset(int bin);
    FPGA *fpga;
    LMS7002M *lms;
    int chipId;
};

} // namespace lime

#endif /* TRXLooper_H */
