#ifndef LIME_LIMESDR_H
#define LIME_LIMESDR_H

#include "SDRDevice.h"
#include "DeviceRegistry.h"
#include "DeviceHandle.h"
#include <vector>
#include <memory>

#include "PacketsFIFO.h"
#include "dataTypes.h"
namespace lime
{

class USBGeneric;
class LMS7002M;
class Streamer;
class FPGA;
class TRXLooper_USB;

class LimeSDR : public SDRDevice
{
public:
    LimeSDR(lime::USBGeneric* conn);
    virtual ~LimeSDR();

    virtual void Configure(const SDRConfig config, uint8_t moduleIndex) override;

    virtual const SDRDevice::Descriptor &GetDescriptor() const override;

    virtual int Init() override;
    virtual void Reset() override;

    //virtual double GetRate(Dir dir, uint8_t channel) const override;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) override;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) override;

    virtual void Synchronize(bool toChip) override;
    virtual void EnableCache(bool enable) override;

    virtual void SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, size_t count) override;
    virtual int I2CWrite(int address, const uint8_t *data, size_t length) override;
    virtual int I2CRead(int addr, uint8_t *dest, size_t length) override;

    virtual int StreamSetup(const StreamConfig &config, uint8_t moduleIndex) override;
    virtual void StreamStart(uint8_t moduleIndex) override;
    virtual void StreamStop(uint8_t moduleIndex) override;

    virtual void StreamStatus(uint8_t channel, SDRDevice::StreamStats &status) override;

    virtual void *GetInternalChip(uint32_t index) override;

    virtual void SetFPGAInterfaceFreq(uint8_t interp, uint8_t dec, double txPhase,
                                      double rxPhase) override;

  protected:
    int EnableChannel(Dir dir, uint8_t channel, bool enabled);
    SDRDevice::DeviceInfo GetDeviceInfo();
    void ResetUSBFIFO();
    void SetSampleRate(double f_Hz, uint8_t oversample);
    std::unique_ptr<FPGA> mFPGA;
    USBGeneric* comms;
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
