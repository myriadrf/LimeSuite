#ifndef LIME_LMS7002M_SDRDevice_H
#define LIME_LMS7002M_SDRDevice_H

#include "dataTypes.h"

#include <vector>
#include <unordered_map>
#include <functional>
#include <string.h>

#include "limesuite/SDRDevice.h"

namespace lime {

class LMS7002M;
class TRXLooper;
class FPGA;

// Base class for device with multiple LMS7002M chips and FPGA
class LIME_API LMS7002M_SDRDevice : public SDRDevice
{
  public:
    LMS7002M_SDRDevice();
    virtual ~LMS7002M_SDRDevice();

    virtual void Configure(const SDRConfig& config, uint8_t moduleIndex) = 0;

    /// Returns SPI slave names and chip select IDs for use with SDRDevice::SPI()
    virtual const Descriptor& GetDescriptor();

    virtual int Init() = 0;
    virtual void Reset() override;
    virtual void GetGPSLock(GPS_Lock* status) override;

    virtual double GetSampleRate(uint8_t moduleIndex, TRXDir trx) override;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) = 0;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) = 0;

    virtual void Synchronize(bool toChip) override;
    virtual void EnableCache(bool enable) override;

    virtual int StreamSetup(const StreamConfig& config, uint8_t moduleIndex) = 0;
    virtual void StreamStart(uint8_t moduleIndex) override;
    virtual void StreamStop(uint8_t moduleIndex) override;

    virtual int StreamRx(uint8_t moduleIndex, complex32f_t** samples, uint32_t count, StreamMeta* meta) override;
    virtual int StreamRx(uint8_t moduleIndex, complex16_t** samples, uint32_t count, StreamMeta* meta) override;
    virtual int StreamTx(uint8_t moduleIndex, const complex32f_t* const* samples, uint32_t count, const StreamMeta* meta) override;
    virtual int StreamTx(uint8_t moduleIndex, const complex16_t* const* samples, uint32_t count, const StreamMeta* meta) override;
    virtual void StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats* rx, SDRDevice::StreamStats* tx) override;

    virtual int SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual int I2CWrite(int address, const uint8_t* data, uint32_t length) override;
    virtual int I2CRead(int addr, uint8_t* dest, uint32_t length) override;
    virtual int GPIOWrite(const uint8_t* buffer, const size_t bufLength) override;
    virtual int GPIORead(uint8_t* buffer, const size_t bufLength) override;
    virtual int GPIODirWrite(const uint8_t* buffer, const size_t bufLength) override;
    virtual int GPIODirRead(uint8_t* buffer, const size_t bufLength) override;
    virtual int CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) override;
    virtual int CustomParameterRead(std::vector<CustomParameterIO>& parameters) override;

    virtual void SetDataLogCallback(DataCallbackType callback) override;
    virtual void SetMessageLogCallback(LogCallbackType callback) override;

    virtual void* GetInternalChip(uint32_t index);

    virtual bool UploadMemory(uint32_t id, const char* data, size_t length, UploadMemoryCallback callback) override;

  protected:
    static int UpdateFPGAInterfaceFrequency(LMS7002M& soc, FPGA& fpga, uint8_t chipIndex);
    DataCallbackType mCallback_logData;
    LogCallbackType mCallback_logMessage;
    std::vector<LMS7002M*> mLMSChips;
    std::vector<TRXLooper*> mStreamers;

    Descriptor mDeviceDescriptor;
    StreamConfig mStreamConfig;
    FPGA* mFPGA;

  private:
    friend class DeviceRegistry;
};

} // namespace lime
#endif
