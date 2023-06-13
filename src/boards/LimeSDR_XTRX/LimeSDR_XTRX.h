#ifndef LIME_LIMESDR_XTRX_H
#define	LIME_LIMESDR_XTRX_H

#include "LMS7002M_SDRDevice.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/IComms.h"

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

class LimeSDR_XTRX : public LMS7002M_SDRDevice
{
public:
    LimeSDR_XTRX() = delete;
    LimeSDR_XTRX(lime::LitePCIe* control, lime::LitePCIe* stream);
    virtual ~LimeSDR_XTRX();

    virtual void Configure(const SDRConfig& config, uint8_t socIndex) override;

    virtual int Init() override;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) override;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) override;

    virtual void SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override;

    virtual int StreamSetup(const StreamConfig &config, uint8_t moduleIndex) override;
    virtual void StreamStop(uint8_t moduleIndex) override;

    virtual void SetFPGAInterfaceFreq(uint8_t interp, uint8_t dec, double txPhase,
                                      double rxPhase) override;
    virtual int CustomParameterWrite(const int32_t *ids, const double *values, const size_t count, const std::string& units) override;
    virtual int CustomParameterRead(const int32_t *ids, double *values, const size_t count, std::string* units) override;

    virtual bool UploadMemory(uint32_t id, const char* data, size_t length, UploadMemoryCallback callback);
protected:
    void LMS1SetPath(bool tx, uint8_t chan, uint8_t path);
    void LMS1_SetSampleRate(double f_Hz, uint8_t rxDecimation, uint8_t txInterpolation);
    static int LMS1_UpdateFPGAInterface(void* userData);

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

    enum class eMemoryDevice {
        FPGA_RAM = 0,
        FPGA_FLASH,
        COUNT
    };

    // Communications helper to divert data to specific device
    class CommsRouter : public ISPI, public II2C
    {
    public:
        CommsRouter(LitePCIe* port, uint32_t slaveID);
        virtual ~CommsRouter();
        virtual void SPI(const uint32_t *MOSI, uint32_t *MISO, uint32_t count);
        virtual void SPI(uint32_t spiBusAddress, const uint32_t *MOSI, uint32_t *MISO, uint32_t count);
        virtual int I2CWrite(int address, const uint8_t *data, uint32_t length);
        virtual int I2CRead(int addres, uint8_t *dest, uint32_t length);
    private:
        LitePCIe* port;
        uint32_t mDefaultSlave;
    };

private:
    LitePCIe *mControlPort;
    LitePCIe *mStreamPort;

    CommsRouter mLMS7002Mcomms;
    CommsRouter mFPGAcomms;
    std::mutex mCommsMutex;
    bool mConfigInProgress;
};

class LimeSDR_XTRXEntry : public DeviceRegistryEntry
{
public:
    LimeSDR_XTRXEntry();
    virtual ~LimeSDR_XTRXEntry();
    std::vector<DeviceHandle> enumerate(const DeviceHandle& hint) override;
    SDRDevice* make(const DeviceHandle& handle) override;
};

}

#endif // LIME_LIMESDR_5G_H

