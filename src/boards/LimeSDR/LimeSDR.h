#ifndef LIME_LIMESDR_H
#define LIME_LIMESDR_H

#include "LMS7002M_SDRDevice.h"
#include "protocols/LMS64CProtocol.h"
#include "dataTypes.h"

#include <vector>
#include <memory>

namespace lime {

class USBGeneric;

class LimeSDR : public LMS7002M_SDRDevice
{
  public:
    LimeSDR(std::shared_ptr<IComms> spiLMS,
        std::shared_ptr<IComms> spiFPGA,
        std::shared_ptr<USBGeneric> mStreamPort,
        std::shared_ptr<ISerialPort> commsPort);
    virtual ~LimeSDR();

    virtual void Configure(const SDRConfig& config, uint8_t moduleIndex) override;

    virtual int Init() override;
    virtual void Reset() override;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) override;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) override;

    virtual void Synchronize(bool toChip) override;
    virtual void EnableCache(bool enable) override;

    virtual int SPI(uint32_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual int StreamSetup(const StreamConfig& config, uint8_t moduleIndex) override;

    virtual void StreamStart(uint8_t moduleIndex) override;
    virtual void StreamStop(uint8_t moduleIndex) override;

    virtual void StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats* rx, SDRDevice::StreamStats* tx) override;

    virtual void* GetInternalChip(uint32_t index) override;

    virtual int GPIODirRead(uint8_t* buffer, const size_t bufLength) override;
    virtual int GPIORead(uint8_t* buffer, const size_t bufLength) override;
    virtual int GPIODirWrite(const uint8_t* buffer, const size_t bufLength) override;
    virtual int GPIOWrite(const uint8_t* buffer, const size_t bufLength) override;

    virtual int CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) override;
    virtual int CustomParameterRead(std::vector<CustomParameterIO>& parameters) override;

    virtual int ReadFPGARegister(uint32_t address);
    virtual int WriteFPGARegister(uint32_t address, uint32_t value);

  protected:
    int EnableChannel(TRXDir dir, uint8_t channel, bool enabled);
    SDRDevice::Descriptor GetDeviceInfo();
    void ResetUSBFIFO();
    void SetSampleRate(double f_Hz, uint8_t oversample);
    static int UpdateFPGAInterface(void* userData);

  private:
    std::shared_ptr<USBGeneric> mStreamPort;
    std::shared_ptr<ISerialPort> mSerialPort;
    std::shared_ptr<IComms> mlms7002mPort;
    std::shared_ptr<IComms> mfpgaPort;
};

} // namespace lime

#endif /* LIME_LIMESDR_H */
