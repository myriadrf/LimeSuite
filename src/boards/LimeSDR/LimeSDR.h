#ifndef LIME_LIMESDR_H
#define LIME_LIMESDR_H

#include "SDRDevice.h"
#include "DeviceRegistry.h"
#include "DeviceHandle.h"
#include <vector>

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

    virtual void Configure(const SDRConfig config) override;

    virtual const SDRDevice::Descriptor &GetDescriptor() const override;

    virtual int Init() override;
    virtual DeviceInfo GetDeviceInfo() override;
    virtual void Reset() override;
    virtual int EnableChannel(SDRDevice::Dir dir, uint8_t channel, bool enabled) override;

    virtual double GetRate(SDRDevice::Dir dir, uint8_t channel) const override;
    virtual SDRDevice::Range GetRateRange(SDRDevice::Dir dir, uint8_t channel) const override;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) override;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) override;

    virtual SDRDevice::Range GetFrequencyRange(SDRDevice::Dir dir) const override;
    virtual std::vector<std::string> GetPathNames(SDRDevice::Dir dir, uint8_t channel) const;
    virtual uint8_t GetPath(SDRDevice::Dir dir, uint8_t channel) const override;

    virtual void Synchronize(bool toChip) override;
    virtual void EnableCache(bool enable) override;

    virtual uint16_t ReadParam(const LMS7Parameter &param, uint8_t channel = 0,
                               bool forceReadFromChip = false) const override;
    virtual int WriteParam(const LMS7Parameter &param, uint16_t val, uint8_t channel) override;

    virtual double GetTemperature(uint8_t id) override;

    virtual uint8_t GetNumChannels() const override
    {
        return 2;
    };

    virtual void SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, size_t count) override;
    virtual int I2CWrite(int address, const uint8_t *data, size_t length) override;
    virtual int I2CRead(int addr, uint8_t *dest, size_t length) override;

    virtual int StreamStart(const StreamConfig &config) override;
    virtual void StreamStop() override;

    virtual int StreamRx(uint8_t channel, void **samples, uint32_t count,
                         StreamMeta *meta) override;
    virtual int StreamTx(uint8_t channel, const void **samples, uint32_t count,
                         const StreamMeta *meta) override;
    virtual void StreamStatus(uint8_t channel, SDRDevice::StreamStats &status) override;

    virtual void *GetInternalChip(uint32_t index) override;

    virtual void SetFPGAInterfaceFreq(uint8_t interp, uint8_t dec, double txPhase,
                                      double rxPhase) override;

  protected:
    void ResetUSBFIFO();
    void SetSampleRate(double f_Hz, uint8_t oversample);
    LMS7002M* mLMSChip;
    FPGA *mFPGA;
    USBGeneric* comms;
    TRXLooper_USB *mStreamer;

    PacketsFIFO<FPGA_DataPacket> *rxFIFO;
    PacketsFIFO<FPGA_DataPacket> *txFIFO;
    StreamConfig mStreamConfig;
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
