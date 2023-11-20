#ifndef LIME_LIMESDR_XTRX_H
#define LIME_LIMESDR_XTRX_H

#include "LMS7002M_SDRDevice.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/IComms.h"

#include <vector>
#include <mutex>
#include <array>
#include <memory>

#include "dataTypes.h"

namespace lime {

class LMS7002M;
class IGenericComms;
class LitePCIe;
class FPGA;
class Equalizer;
class TRXLooper_PCIE;

class LimeSDR_XTRX : public LMS7002M_SDRDevice
{
  public:
    LimeSDR_XTRX() = delete;
    LimeSDR_XTRX(std::shared_ptr<IComms> spiLMS7002M, std::shared_ptr<IComms> spiFPGA, std::shared_ptr<LitePCIe> sampleStream);
    virtual ~LimeSDR_XTRX();

    virtual void Configure(const SDRConfig& config, uint8_t socIndex) override;

    virtual int Init() override;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) override;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) override;

    virtual void SPI(uint32_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual int StreamSetup(const StreamConfig& config, uint8_t moduleIndex) override;
    virtual void StreamStop(uint8_t moduleIndex) override;

    virtual int CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) override;
    virtual int CustomParameterRead(std::vector<CustomParameterIO>& parameters) override;

    virtual bool UploadMemory(uint32_t id, const char* data, size_t length, UploadMemoryCallback callback);

  protected:
    void LMS1SetPath(bool tx, uint8_t chan, uint8_t path);
    void LMS1_SetSampleRate(double f_Hz, uint8_t rxDecimation, uint8_t txInterpolation);
    static int LMS1_UpdateFPGAInterface(void* userData);

    enum class ePathLMS1_Rx { NONE = 0, LNAH = 1, LNAL = 2, LNAW = 3 };
    enum class ePathLMS1_Tx { NONE = 0, BAND1 = 1, BAND2 = 2 };
    enum class ePathLMS2_Rx { NONE = 0, TDD = 1, FDD = 2, CALIBRATION = 3 };
    enum class ePathLMS2_Tx { NONE = 0, TDD = 1, FDD = 2 };

    enum class eMemoryDevice { FPGA_RAM = 0, FPGA_FLASH, COUNT };

  private:
    std::shared_ptr<IComms> lms7002mPort;
    std::shared_ptr<IComms> fpgaPort;
    std::shared_ptr<LitePCIe> mStreamPort;

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

} // namespace lime

#endif // LIME_LIMESDR_5G_H
