#ifndef LIME_LIMESDR_H
#define LIME_LIMESDR_H

#include "LMS7002M_SDRDevice.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/DeviceHandle.h"
#include <vector>
#include <memory>

#include <FX3.h>

#include "dataTypes.h"
namespace lime
{

class USBGeneric;
class LMS7002M;
class Streamer;
class FPGA;
class TRXLooper_USB;

class LimeSDR : public LMS7002M_SDRDevice
{
public:
    LimeSDR(lime::USBGeneric* conn);
    virtual ~LimeSDR();

    virtual void Configure(const SDRConfig& config, uint8_t moduleIndex) override;

    virtual const Descriptor &GetDescriptor() override;

    virtual int Init() override;
    virtual void Reset() override;

    //virtual double GetRate(Dir dir, uint8_t channel) const override;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) override;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) override;

    virtual void Synchronize(bool toChip) override;
    virtual void EnableCache(bool enable) override;

    virtual void SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override;

    // virtual int I2CWrite(int address, const uint8_t *data, uint32_t length) override;
    // virtual int I2CRead(int addr, uint8_t *dest, uint32_t length) override;

    virtual int StreamSetup(const StreamConfig &config, uint8_t moduleIndex) override;

    virtual void StreamStart(uint8_t moduleIndex) override;
    virtual void StreamStop(uint8_t moduleIndex) override;

    virtual void StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats* rx, SDRDevice::StreamStats* tx) override;

    virtual void *GetInternalChip(uint32_t index) override;

    virtual void SetFPGAInterfaceFreq(uint8_t interp, uint8_t dec, double txPhase,
                                      double rxPhase) override;

    virtual int GPIODirRead(uint8_t *buffer, const size_t bufLength) override;
    virtual int GPIORead(uint8_t *buffer, const size_t bufLength) override;
    virtual int GPIODirWrite(const uint8_t *buffer, const size_t bufLength) override;
    virtual int GPIOWrite(const uint8_t *buffer, const size_t bufLength) override;
  protected:
    class CommsRouter : public ISPI, public II2C
    {
    public:
        CommsRouter(FX3* port, uint32_t slaveID);
        virtual ~CommsRouter();
        virtual void SPI(const uint32_t *MOSI, uint32_t *MISO, uint32_t count);
        virtual void SPI(uint32_t spiBusAddress, const uint32_t *MOSI, uint32_t *MISO, uint32_t count);
        virtual int I2CWrite(int address, const uint8_t *data, uint32_t length);
        virtual int I2CRead(int addres, uint8_t *dest, uint32_t length);
    private:
        FX3* port;
        uint32_t mDefaultSlave;
    };

    int EnableChannel(TRXDir dir, uint8_t channel, bool enabled);
    SDRDevice::Descriptor GetDeviceInfo();
    void ResetUSBFIFO();
    void SetSampleRate(double f_Hz, uint8_t oversample);
    USBGeneric* comms;
    CommsRouter mFPGAComms;
    CommsRouter mLMSComms;
private:
};

class LimeSDREntry : public DeviceRegistryEntry
{
public:
    LimeSDREntry();
    virtual ~LimeSDREntry();
    std::vector<DeviceHandle> enumerate(const DeviceHandle& hint) override;
    SDRDevice* make(const DeviceHandle& handle) override;
};

}

#endif	/* LIME_LIMESDR_H */
