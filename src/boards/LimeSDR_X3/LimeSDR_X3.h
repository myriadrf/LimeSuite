#ifndef LIME_LIMESDR_5G_H
#define LIME_LIMESDR_5G_H

#include "CDCM6208/CDCM6208_Dev.h"
#include "LMS7002M_SDRDevice.h"
#include "protocols/LMS64CProtocol.h"

#include <vector>
#include <array>
#include <memory>

namespace lime {

class LitePCIe;
class Equalizer;
class SlaveSelectShim;

class LimeSDR_X3 : public LMS7002M_SDRDevice
{
  public:
    LimeSDR_X3() = delete;
    LimeSDR_X3(std::shared_ptr<IComms> spiLMS7002M,
        std::shared_ptr<IComms> spiFPGA,
        std::vector<std::shared_ptr<LitePCIe>> trxStreams,
        std::shared_ptr<ISerialPort> control);
    virtual ~LimeSDR_X3();

    virtual void Configure(const SDRConfig& config, uint8_t socIndex) override;

    virtual int Init() override;
    virtual void Reset() override;

    virtual double GetSampleRate(uint8_t moduleIndex, TRXDir trx) override;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) override;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) override;

    virtual int SPI(uint32_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual int StreamSetup(const StreamConfig& config, uint8_t moduleIndex) override;
    virtual void StreamStop(uint8_t moduleIndex) override;

    virtual int CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) override;
    virtual int CustomParameterRead(std::vector<CustomParameterIO>& parameters) override;

    virtual bool UploadMemory(
        eMemoryDevice device, uint8_t moduleIndex, const char* data, size_t length, UploadMemoryCallback callback) override;
    virtual int MemoryWrite(std::shared_ptr<DataStorage> storage, eMemoryRegion memoryRegion, const void* data) override;
    virtual int MemoryRead(std::shared_ptr<DataStorage> storage, eMemoryRegion memoryRegion, void* data) override;
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

    enum class ePathLMS1_Rx : uint8_t { NONE, LNAH, LNAL };
    enum class ePathLMS1_Tx : uint8_t { NONE, BAND1, BAND2 };
    enum class ePathLMS2_Rx : uint8_t { NONE, TDD, FDD, CALIBRATION };
    enum class ePathLMS2_Tx : uint8_t { NONE, TDD, FDD };

  private:
    void ConfigureDirection(TRXDir dir, LMS7002M* chip, const SDRConfig& cfg, int ch, uint8_t socIndex);
    void SetLMSPath(const TRXDir dir, const SDRDevice::ChannelConfig::Direction& trx, const int ch, const uint8_t socIndex);

    CDCM_Dev* mClockGeneratorCDCM;
    Equalizer* mEqualizer;
    std::vector<std::shared_ptr<LitePCIe>> mTRXStreamPorts;

    std::array<std::shared_ptr<SlaveSelectShim>, 3> mLMS7002Mcomms;
    std::shared_ptr<IComms> mfpgaPort;
    std::shared_ptr<ISerialPort> mSerialPort;
    std::mutex mCommsMutex;
    bool mConfigInProgress;
};

} // namespace lime

#endif // LIME_LIMESDR_5G_H
