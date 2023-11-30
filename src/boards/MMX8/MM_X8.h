#ifndef LIME_LIMESDR_5G_H
#define LIME_LIMESDR_5G_H

#include "CDCM6208/CDCM6208_Dev.h"
#include "LMS7002M_SDRDevice.h"
#include "limesuite/IComms.h"
#include "ADF4002/ADF4002.h"
#include "protocols/LMS64CProtocol.h"

#include <vector>
#include <array>
#include <memory>

#include "PacketsFIFO.h"
#include "dataTypes.h"

namespace lime {

class LitePCIe;
class LimeSDR_XTRX;

class LimeSDR_MMX8 : public lime::SDRDevice
{
  public:
    LimeSDR_MMX8() = delete;
    LimeSDR_MMX8(std::vector<std::shared_ptr<IComms>>& spiLMS7002M,
        std::vector<std::shared_ptr<IComms>>& spiFPGA,
        std::vector<std::shared_ptr<LitePCIe>> trxStreams,
        ISPI* adfComms);
    virtual ~LimeSDR_MMX8();

    virtual void Configure(const SDRConfig& config, uint8_t socIndex) override;
    virtual const Descriptor& GetDescriptor() override;

    virtual int Init() override;
    virtual void Reset() override;
    virtual void GetGPSLock(GPS_Lock* status) override;

    virtual double GetSampleRate(uint8_t moduleIndex, TRXDir trx) override;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) override;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) override;

    virtual void Synchronize(bool toChip) override;
    virtual void EnableCache(bool enable) override;

    virtual int StreamSetup(const StreamConfig& config, uint8_t moduleIndex) override;
    virtual void StreamStart(uint8_t moduleIndex) override;
    virtual void StreamStop(uint8_t moduleIndex) override;

    virtual int StreamRx(uint8_t moduleIndex, lime::complex32f_t** samples, uint32_t count, StreamMeta* meta) override;
    virtual int StreamRx(uint8_t moduleIndex, lime::complex16_t** samples, uint32_t count, StreamMeta* meta) override;
    virtual int StreamTx(
        uint8_t moduleIndex, const lime::complex32f_t* const* samples, uint32_t count, const StreamMeta* meta) override;
    virtual int StreamTx(
        uint8_t moduleIndex, const lime::complex16_t* const* samples, uint32_t count, const StreamMeta* meta) override;
    virtual void StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats* rx, SDRDevice::StreamStats* tx) override;

    virtual int SPI(uint32_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual int I2CWrite(int address, const uint8_t* data, uint32_t length) override;
    virtual int I2CRead(int addres, uint8_t* dest, uint32_t length) override;

    virtual int CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) override;
    virtual int CustomParameterRead(std::vector<CustomParameterIO>& parameters) override;

    virtual void SetDataLogCallback(DataCallbackType callback){};
    virtual void SetMessageLogCallback(LogCallbackType callback) override;

    virtual void* GetInternalChip(uint32_t index) override;

    virtual bool UploadMemory(uint32_t id, const char* data, size_t length, UploadMemoryCallback callback) override;
    virtual int MemoryWrite(uint32_t id, uint32_t address, const void* data, size_t length) override;
    virtual int MemoryRead(uint32_t id, uint32_t address, void* data, size_t length) override;
    virtual int UploadTxWaveform(const StreamConfig& config, uint8_t moduleIndex, const void** samples, uint32_t count) override;

    virtual Range GetRateRange(uint8_t moduleIndex) const override;
    virtual Range GetFrequencyRange(uint8_t moduleIndex) const override;
    virtual Range GetAntennaRange(uint8_t moduleIndex, TRXDir direction, unsigned path) const override;

  protected:
    enum class eMemoryDevice : uint8_t { FPGA_FLASH, EEPROM, COUNT };

  private:
    std::shared_ptr<IComms> mMainFPGAcomms;
    Descriptor mDeviceDescriptor;
    std::vector<std::shared_ptr<LitePCIe>> mTRXStreamPorts;
    std::vector<LimeSDR_XTRX*> mSubDevices;
    std::map<uint32_t, LimeSDR_XTRX*> chipSelectToDevice;
    std::map<uint32_t, LimeSDR_XTRX*> memorySelectToDevice;
    std::map<uint32_t, LimeSDR_XTRX*> customParameterToDevice;
    lime::ADF4002* mADF;
};

} // namespace lime

#endif // LIME_LIMESDR_5G_H
