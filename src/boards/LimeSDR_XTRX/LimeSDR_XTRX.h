#ifndef LIME_LIMESDR_XTRX_H
#define LIME_LIMESDR_XTRX_H

#include "LMS7002M_SDRDevice.h"
#include "limesuite/IComms.h"

#include <vector>
#include <mutex>
#include <array>
#include <memory>

namespace lime {

class LitePCIe;

static const float XTRX_DEFAULT_REFERENCE_CLOCK = 26e6;

/** @brief Class for managing the LimeSDR XTRX device. */
class LimeSDR_XTRX : public LMS7002M_SDRDevice
{
  public:
    LimeSDR_XTRX() = delete;
    LimeSDR_XTRX(std::shared_ptr<IComms> spiLMS7002M,
        std::shared_ptr<IComms> spiFPGA,
        std::shared_ptr<LitePCIe> sampleStream,
        double refClk = XTRX_DEFAULT_REFERENCE_CLOCK);
    virtual ~LimeSDR_XTRX();

    virtual void Configure(const SDRConfig& config, uint8_t socIndex) override;

    virtual int Init() override;

    virtual void SetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double sampleRate, uint8_t oversample) override;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) override;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) override;

    virtual int SPI(uint32_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual int StreamSetup(const StreamConfig& config, uint8_t moduleIndex) override;
    virtual void StreamStop(uint8_t moduleIndex) override;

    virtual int CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) override;
    virtual int CustomParameterRead(std::vector<CustomParameterIO>& parameters) override;

    virtual int UploadMemory(
        eMemoryDevice device, uint8_t moduleIndex, const char* data, size_t length, UploadMemoryCallback callback) override;
    virtual int MemoryWrite(std::shared_ptr<DataStorage> storage, Region region, const void* data) override;
    virtual int MemoryRead(std::shared_ptr<DataStorage> storage, Region region, void* data) override;

  protected:
    void LMS1SetPath(bool tx, uint8_t chan, uint8_t path);
    void LMS1_SetSampleRate(double f_Hz, uint8_t rxDecimation, uint8_t txInterpolation);
    static int LMS1_UpdateFPGAInterface(void* userData);

    enum class ePathLMS1_Rx : uint8_t { NONE, LNAH, LNAL, LNAW };
    enum class ePathLMS1_Tx : uint8_t { NONE, BAND1, BAND2 };

  private:
    std::shared_ptr<IComms> lms7002mPort;
    std::shared_ptr<IComms> fpgaPort;
    std::shared_ptr<LitePCIe> mStreamPort;

    std::mutex mCommsMutex;
    bool mConfigInProgress;
};

} // namespace lime

#endif // LIME_LIMESDR_5G_H
