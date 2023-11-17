#ifndef LIME_LIMESDR_5G_H
#define LIME_LIMESDR_5G_H

#include "CDCM6208/CDCM6208_Dev.h"
#include "LMS7002M_SDRDevice.h"
#include "limesuite/DeviceRegistry.h"

#include "protocols/LMS64CProtocol.h"

#include <vector>
#include <array>
#include <memory>

#include "dataTypes.h"

namespace lime {

class LMS7002M;
class LitePCIe;
class FPGA;
class Equalizer;
class TRXLooper_PCIE;
class SlaveSelectShim;

class LimeSDR_X3 : public LMS7002M_SDRDevice
{
  public:
    LimeSDR_X3() = delete;
    LimeSDR_X3(
        std::shared_ptr<IComms> spiLMS7002M, std::shared_ptr<IComms> spiFPGA, std::vector<std::shared_ptr<LitePCIe>> trxStreams);
    virtual ~LimeSDR_X3();

    virtual void Configure(const SDRConfig& config, uint8_t socIndex) override;

    virtual int Init() override;
    virtual void Reset() override;

    virtual double GetSampleRate(uint8_t moduleIndex, TRXDir trx) override;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) override;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) override;

    virtual void SPI(uint32_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual int StreamSetup(const StreamConfig& config, uint8_t moduleIndex) override;
    virtual void StreamStop(uint8_t moduleIndex) override;

    virtual int CustomParameterWrite(const int32_t id, const double value, const std::string& units) override;
    virtual int CustomParameterRead(const int32_t id, double& value, std::string& units) override;

    virtual bool UploadMemory(uint32_t id, const char* data, size_t length, UploadMemoryCallback callback) override;
    virtual int UploadTxWaveform(const StreamConfig& config, uint8_t moduleIndex, const void** samples, uint32_t count) override;

  protected:
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
    static int LMS1_UpdateFPGAInterface(void* userData);

    void LMS2_SetSampleRate(double f_Hz, uint8_t oversample);

    enum class ePathLMS1_Rx { NONE = 0, LNAH = 1, LNAL = 2 };
    enum class ePathLMS1_Tx { NONE = 0, BAND1 = 1, BAND2 = 2 };
    enum class ePathLMS2_Rx { NONE = 0, TDD = 1, FDD = 2, CALIBRATION = 3 };
    enum class ePathLMS2_Tx { NONE = 0, TDD = 1, FDD = 2 };

    enum class eMemoryDevice { FPGA_RAM = 0, FPGA_FLASH, COUNT };

  private:
    void ConfigureDirection(TRXDir dir, LMS7002M* chip, const SDRConfig& cfg, int ch, uint8_t socIndex);
    void SetLMSPath(const TRXDir dir, const SDRDevice::ChannelConfig::Direction& trx, const int ch, const uint8_t socIndex);

    CDCM_Dev* mClockGeneratorCDCM;
    Equalizer* mEqualizer;
    std::vector<std::shared_ptr<LitePCIe>> mTRXStreamPorts;

    std::array<std::shared_ptr<SlaveSelectShim>, 3> mLMS7002Mcomms;
    std::shared_ptr<IComms> fpgaPort;
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

} // namespace lime

#endif // LIME_LIMESDR_5G_H
