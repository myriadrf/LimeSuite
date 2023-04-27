#ifndef LIME_LIMESDR_5G_H
#define	LIME_LIMESDR_5G_H

#include "CDCM6208/CDCM6208_Dev.h"
#include "LMS7002M_SDRDevice.h"
#include "limesuite/DeviceRegistry.h"

#include "protocols/LMS64CProtocol.h"

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

class LimeSDR_X3 : public LMS7002M_SDRDevice
{
public:
    LimeSDR_X3() = delete;
    LimeSDR_X3(lime::LitePCIe* control, std::vector<lime::LitePCIe*> rxStreams, std::vector<lime::LitePCIe*> txStreams);
    virtual ~LimeSDR_X3();

    virtual void Configure(const SDRConfig& config, uint8_t socIndex) override;

    virtual int Init() override;
    virtual void Reset() override;

    virtual double GetSampleRate(uint8_t moduleIndex, TRXDir trx) override;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) override;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) override;

    virtual void SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override;

    virtual int StreamSetup(const StreamConfig &config, uint8_t moduleIndex) override;
    virtual void StreamStop(uint8_t moduleIndex) override;

    virtual void SetFPGAInterfaceFreq(uint8_t interp, uint8_t dec, double txPhase,
                                      double rxPhase) override;
    virtual int CustomParameterWrite(const int32_t *ids, const double *values, const size_t count, const std::string& units) override;
    virtual int CustomParameterRead(const int32_t *ids, double *values, const size_t count, std::string* units) override;

    virtual bool UploadMemory(uint32_t id, const char* data, size_t length, UploadMemoryCallback callback) override;
    virtual int UploadTxWaveform(const StreamConfig &config, uint8_t moduleIndex, const void** samples, uint32_t count) override;
protected:
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

    int InitLMS1(bool skipTune = false);
    int InitLMS2(bool skipTune = false);
    int InitLMS3(bool skipTune = false);
    void PreConfigure(const SDRConfig& cfg, uint8_t socIndex);
    void PostConfigure(const SDRConfig& cfg, uint8_t socIndex);
    void LMS1_PA_Enable(uint8_t chan, bool enabled);
    void LMS1_SetSampleRate(double f_Hz, uint8_t rxDecimation, uint8_t txInterpolation);
    void LMS1SetPath(TRXDir dir, uint8_t chan, uint8_t pathId);
    void LMS2SetPath(TRXDir dir, uint8_t chan, uint8_t path);
    void LMS2_PA_LNA_Enable(uint8_t chan, bool PAenabled, bool LNAenabled);
    void LMS3SetPath(TRXDir dir, uint8_t chan, uint8_t path);
    void LMS3_SetSampleRate_ExternalDAC(double chA_Hz, double chB_Hz);
    static void LMS1_UpdateFPGAInterface(void* userData);

    void LMS2_SetSampleRate(double f_Hz, uint8_t oversample);

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

private:
    CDCM_Dev* mClockGeneratorCDCM;
    LitePCIe *mControlPort;
    Equalizer* mEqualizer;
    std::vector<LitePCIe*> mRXStreamPorts;
    std::vector<LitePCIe*> mTXStreamPorts;

    CommsRouter* mLMS7002Mcomms[3];
    CommsRouter mFPGAcomms;
    std::mutex mCommsMutex;
    bool mConfigInProgress;
};

class LimeSDR_X3Entry : public DeviceRegistryEntry
{
public:
    LimeSDR_X3Entry();
    virtual ~LimeSDR_X3Entry();
    std::vector<DeviceHandle> enumerate(const DeviceHandle& hint) override;
    SDRDevice* make(const DeviceHandle& handle) override;
};

}

#endif // LIME_LIMESDR_5G_H

