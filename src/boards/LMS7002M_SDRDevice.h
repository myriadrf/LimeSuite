#ifndef LIME_LMS7002M_SDRDevice_H
#define LIME_LMS7002M_SDRDevice_H

#include <vector>
#include <unordered_map>
#include <functional>
#include <string.h>

#include "limesuite/SDRDevice.h"
#include "limesuite/LMS7002M.h"

namespace lime {

class TRXLooper;
class FPGA;

/** @brief Base class for device with multiple LMS7002M chips and FPGA */
class LIME_API LMS7002M_SDRDevice : public SDRDevice
{
  public:
    LMS7002M_SDRDevice();
    virtual ~LMS7002M_SDRDevice();

    virtual const Descriptor& GetDescriptor() const override;

    virtual void Reset() override;
    virtual void GetGPSLock(GPS_Lock* status) override;

    virtual double GetNCOOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel) override;
    virtual void SetNCOOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double offset) override;

    virtual double GetFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel) override;
    virtual void SetFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double frequency) override;

    virtual double GetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel) override;
    virtual void SetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double sampleRate, uint8_t oversample) override;

    virtual int SetGain(uint8_t moduleIndex, TRXDir direction, uint8_t channel, eGainTypes gain, double value) override;
    virtual int GetGain(uint8_t moduleIndex, TRXDir direction, uint8_t channel, eGainTypes gain, double& value) override;

    virtual double GetLowPassFilter(uint8_t moduleIndex, TRXDir trx, uint8_t channel) override;
    virtual void SetLowPassFilter(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double lpf) override;

    virtual uint8_t GetAntenna(uint8_t moduleIndex, TRXDir trx, uint8_t channel) override;
    virtual void SetAntenna(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t path) override;

    virtual void SetTestSignal(uint8_t moduleIndex,
        TRXDir direction,
        uint8_t channel,
        ChannelConfig::Direction::TestSignal signalConfiguration,
        int16_t dc_i = 0,
        int16_t dc_q = 0) override;
    virtual ChannelConfig::Direction::TestSignal GetTestSignal(uint8_t moduleIndex, TRXDir direction, uint8_t channel) override;

    virtual void Synchronize(bool toChip) override;
    virtual void EnableCache(bool enable) override;

    virtual void Calibrate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double bandwidth) override;
    virtual void ConfigureGFIR(
        uint8_t moduleIndex, TRXDir trx, uint8_t channel, ChannelConfig::Direction::GFIRFilter settings) override;

    virtual uint64_t GetHardwareTimestamp(uint8_t moduleIndex) override;
    virtual void SetHardwareTimestamp(uint8_t moduleIndex, const uint64_t now) override;

    virtual void StreamStart(uint8_t moduleIndex) override;
    virtual void StreamStop(uint8_t moduleIndex) override;

    virtual int StreamRx(uint8_t moduleIndex, complex32f_t* const* samples, uint32_t count, StreamMeta* meta) override;
    virtual int StreamRx(uint8_t moduleIndex, complex16_t* const* samples, uint32_t count, StreamMeta* meta) override;
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

    virtual void* GetInternalChip(uint32_t index) override;

    virtual bool UploadMemory(
        eMemoryDevice device, uint8_t moduleIndex, const char* data, size_t length, UploadMemoryCallback callback) override;

    virtual int ReadFPGARegister(uint32_t address);
    virtual int WriteFPGARegister(uint32_t address, uint32_t value);

  protected:
    static int UpdateFPGAInterfaceFrequency(LMS7002M& soc, FPGA& fpga, uint8_t chipIndex);
    void SetGainInformationInDescriptor(RFSOCDescriptor& descriptor);

    DataCallbackType mCallback_logData;
    LogCallbackType mCallback_logMessage;
    std::vector<LMS7002M*> mLMSChips;
    std::vector<TRXLooper*> mStreamers;

    Descriptor mDeviceDescriptor;
    StreamConfig mStreamConfig;
    FPGA* mFPGA;

  private:
    int SetGenericRxGain(lime::LMS7002M* device, LMS7002M::Channel channel, double value);
    int SetGenericTxGain(lime::LMS7002M* device, LMS7002M::Channel channel, double value);
};

} // namespace lime
#endif
