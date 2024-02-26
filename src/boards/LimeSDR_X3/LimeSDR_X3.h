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
class ISerialPort;

/** @brief Class for managing the LimeSDR X3 device. */
class LimeSDR_X3 : public LMS7002M_SDRDevice
{
  public:
    LimeSDR_X3() = delete;
    LimeSDR_X3(std::shared_ptr<IComms> spiLMS7002M,
        std::shared_ptr<IComms> spiFPGA,
        std::vector<std::shared_ptr<LitePCIe>> trxStreams,
        std::shared_ptr<ISerialPort> control);
    virtual ~LimeSDR_X3();

    virtual OpStatus Configure(const SDRConfig& config, uint8_t socIndex) override;

    virtual OpStatus Init() override;
    virtual OpStatus Reset() override;

    virtual double GetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel) override;
    virtual OpStatus SetSampleRate(
        uint8_t moduleIndex, TRXDir trx, uint8_t channel, double sampleRate, uint8_t oversample) override;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) override;
    virtual OpStatus SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) override;

    virtual OpStatus SPI(uint32_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual OpStatus StreamSetup(const StreamConfig& config, uint8_t moduleIndex) override;
    virtual void StreamStop(uint8_t moduleIndex) override;

    virtual OpStatus CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) override;
    virtual OpStatus CustomParameterRead(std::vector<CustomParameterIO>& parameters) override;

    virtual OpStatus UploadMemory(
        eMemoryDevice device, uint8_t moduleIndex, const char* data, size_t length, UploadMemoryCallback callback) override;
    virtual OpStatus MemoryWrite(std::shared_ptr<DataStorage> storage, Region region, const void* data) override;
    virtual OpStatus MemoryRead(std::shared_ptr<DataStorage> storage, Region region, void* data) override;
    virtual OpStatus UploadTxWaveform(
        const StreamConfig& config, uint8_t moduleIndex, const void** samples, uint32_t count) override;

  protected:
    OpStatus InitLMS1(bool skipTune = false);
    OpStatus InitLMS2(bool skipTune = false);
    OpStatus InitLMS3(bool skipTune = false);
    void PreConfigure(const SDRConfig& cfg, uint8_t socIndex);
    void PostConfigure(const SDRConfig& cfg, uint8_t socIndex);
    void LMS1_PA_Enable(uint8_t chan, bool enabled);
    void LMS1_SetSampleRate(double f_Hz, uint8_t rxDecimation, uint8_t txInterpolation);
    void LMS1SetPath(TRXDir dir, uint8_t chan, uint8_t pathId);
    void LMS2SetPath(TRXDir dir, uint8_t chan, uint8_t path);
    void LMS2_PA_LNA_Enable(uint8_t chan, bool PAenabled, bool LNAenabled);
    void LMS3SetPath(TRXDir dir, uint8_t chan, uint8_t path);
    void LMS3_SetSampleRate_ExternalDAC(double chA_Hz, double chB_Hz);
    static OpStatus LMS1_UpdateFPGAInterface(void* userData);

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
    std::mutex mCommsMutex;
    bool mConfigInProgress;
};

} // namespace lime

#endif // LIME_LIMESDR_5G_H
