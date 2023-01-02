#ifndef LIME_LIMESDR_5G_H
#define	LIME_LIMESDR_5G_H

#include "CDCM6208/CDCM6208_Dev.h"
#include "SDRDevice.h"
#include "DeviceRegistry.h"

#include <vector>
#include <array>

#include "PacketsFIFO.h"
#include "dataTypes.h"

namespace lime
{

class LMS7002M;
class LitePCIe;
class FPGA;
class Equalizer;
class TRXLooper_PCIE;

class LimeSDR_5GRadio : public SDRDevice
{
public:
    LimeSDR_5GRadio(lime::LitePCIe* control, std::vector<lime::LitePCIe*> rxStreams, std::vector<lime::LitePCIe*> txStreams);
    virtual ~LimeSDR_5GRadio();

    virtual void Configure(const SDRConfig config, uint8_t socIndex) override;

    virtual const SDRDevice::Descriptor &GetDescriptor() const override;

    virtual int Init() override;
    virtual void Reset() override;
    //virtual int EnableChannel(SDRDevice::Dir dir, uint8_t channel, bool enabled) override;

    //virtual double GetRate(Dir dir, uint8_t channel) const override;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) override;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) override;

    //virtual std::vector<std::string> GetPathNames(SDRDevice::Dir dir, uint8_t channel) const;

    virtual void Synchronize(bool toChip) override;

    // virtual uint16_t ReadParam(const LMS7Parameter &param, uint8_t channel = 0,
    //                            bool forceReadFromChip = false) const override;
    // virtual int WriteParam(const LMS7Parameter &param, uint16_t val, uint8_t channel) override;

    virtual void SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override;
    virtual int I2CWrite(int address, const uint8_t *data, uint32_t length) override;
    virtual int I2CRead(int addr, uint8_t *dest, uint32_t length) override;

    virtual int StreamSetup(const StreamConfig &config, uint8_t moduleIndex) override;
    virtual void StreamStop(uint8_t moduleIndex) override;
    virtual void StreamStart(uint8_t moduleIndex) override;

    virtual void StreamStatus(uint8_t channel, SDRDevice::StreamStats &status) override;

    virtual void SetFPGAInterfaceFreq(uint8_t interp, uint8_t dec, double txPhase,
                                      double rxPhase) override;
    virtual int CustomParameterWrite(const uint8_t *ids, const double *values, const size_t count, const std::string& units) override;
protected:
    void LMS1SetPath(bool tx, uint8_t chan, uint8_t path);
    void LMS2SetPath(bool tx, uint8_t chan, uint8_t path);
    void LMS3SetPath(bool tx, uint8_t chan, uint8_t path);
    void LMS1_SetSampleRate(double f_Hz, uint8_t rxDecimation, uint8_t txInterpolation);
    void LMS2_SetSampleRate(double f_Hz, uint8_t oversample);
    SDRDevice::DeviceInfo GetDeviceInfo();

    enum class ePathLMS1_Rx {
        NONE = 0, LNAH = 1, LNAL = 2
    };
    enum class ePathLMS1_Tx {
        NONE = 0, BAND1 = 1, BAND2 = 2
    };
    enum class ePathLMS2_Rx {
        NONE = 0, TDD = 1, FDD = 2, CALIBRATION = 3
    };
    enum class ePathLMS2_Tx {
        NONE = 0, TDD = 1, FDD = 2
    };

private:
    CDCM_Dev* cdcm[2];
    LitePCIe *mControlPort;
    Equalizer* mEqualizer;
    std::vector<LitePCIe*> mRXStreamPorts;
    std::vector<LitePCIe*> mTXStreamPorts;
    std::mutex mCommsMutex;
};

class LimeSDR_5GRadioEntry : public DeviceRegistryEntry
{
public:
    LimeSDR_5GRadioEntry();
    virtual ~LimeSDR_5GRadioEntry();
    std::vector<DeviceHandle> enumerate(const DeviceHandle& hint) override;
    SDRDevice* make(const DeviceHandle& handle) override;
};

}

#endif // LIME_LIMESDR_5G_H

