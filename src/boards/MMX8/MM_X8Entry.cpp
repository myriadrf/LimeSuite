#ifdef __unix__
    #include <unistd.h>
#endif

#include "LitePCIe.h"
#include "MM_X8.h"
#include "PCIeCommon.h"

#include <fstream>
#include <map>
#include <unistd.h>
#include <fcntl.h>

using namespace lime;

void __loadLimeSDR_MMX8(void) //TODO fixme replace with LoadLibrary/dlopen
{
    static LimeSDR_MMX8Entry limesdr_MMX8Support; // self register on initialization
}

LimeSDR_MMX8Entry::LimeSDR_MMX8Entry()
    : DeviceRegistryEntry("LimeSDR_MMX8")
{
}

LimeSDR_MMX8Entry::~LimeSDR_MMX8Entry()
{
}

std::vector<DeviceHandle> LimeSDR_MMX8Entry::enumerate(const DeviceHandle& hint)
{
    std::vector<DeviceHandle> handles;
    DeviceHandle handle;
    handle.media = "PCIe";

    if (!hint.media.empty() && hint.media != handle.media)
        return handles;

    const std::string searchDevName("LimeMM-X8");
    const std::vector<std::string> boardNames = { GetDeviceName(LMS_DEV_LIMESDR_MMX8), "LimeSDR-MMX8", searchDevName };
    if (!hint.name.empty())
    {
        bool foundMatch = false;
        for (const std::string& name : boardNames)
        {
            if (name.find(hint.name) != std::string::npos)
            {
                foundMatch = true;
                break;
            }
        }
        if (!foundMatch)
            return handles;
    }

    const std::string pattern(searchDevName + "[0-9]*_control");
    const std::vector<std::string> devices = LitePCIe::GetDevicesWithPattern(pattern);

    for (const auto& devPath : devices)
    {
        size_t pos = devPath.find(searchDevName);
        if (pos == std::string::npos)
            continue;

        if (!hint.addr.empty() && devPath.find(hint.addr) == std::string::npos)
            continue;

        std::string dev_nr(&devPath[pos + searchDevName.length()], &devPath[devPath.find("_")]);
        handle.name = GetDeviceName(LMS_DEV_LIMESDR_MMX8) + (dev_nr == "0" ? "" : " (" + dev_nr + ")");

        handle.addr = devPath.substr(0, devPath.find("_"));
        handles.push_back(handle);
    }
    return handles;
}

class LMS64C_LMS7002M_Over_PCIe_MMX8 : public lime::IComms
{
  public:
    LMS64C_LMS7002M_Over_PCIe_MMX8(std::shared_ptr<LitePCIe> dataPort, uint32_t subdeviceIndex)
        : pipe(dataPort)
        , subdeviceIndex(subdeviceIndex)
    {
    }
    virtual void SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override { SPI(0, MOSI, MISO, count); }
    virtual void SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override
    {
        LMS64CProtocol::LMS7002M_SPI(pipe, spiBusAddress, MOSI, MISO, count, subdeviceIndex);
    }
    virtual int ResetDevice(int chipSelect) override { return LMS64CProtocol::DeviceReset(pipe, chipSelect, subdeviceIndex); };

  private:
    PCIE_CSR_Pipe pipe;
    uint32_t subdeviceIndex;
};

class LMS64C_FPGA_Over_PCIe_MMX8 : public lime::IComms
{
  public:
    LMS64C_FPGA_Over_PCIe_MMX8(std::shared_ptr<LitePCIe> dataPort, uint32_t subdeviceIndex)
        : pipe(dataPort)
        , subdeviceIndex(subdeviceIndex)
    {
    }
    void SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override
    {
        LMS64CProtocol::FPGA_SPI(pipe, MOSI, MISO, count, subdeviceIndex);
    }
    void SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override
    {
        LMS64CProtocol::FPGA_SPI(pipe, MOSI, MISO, count, subdeviceIndex);
    }

    virtual int CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) override
    {
        return LMS64CProtocol::CustomParameterWrite(pipe, parameters, subdeviceIndex);
    };
    virtual int CustomParameterRead(std::vector<CustomParameterIO>& parameters) override
    {
        return LMS64CProtocol::CustomParameterRead(pipe, parameters, subdeviceIndex);
    }
    virtual int ProgramWrite(
        const char* data, size_t length, int prog_mode, int target, ProgressCallback callback = nullptr) override
    {
        return LMS64CProtocol::ProgramWrite(
            pipe, data, length, prog_mode, (LMS64CProtocol::ProgramWriteTarget)target, callback, subdeviceIndex);
    }

  private:
    PCIE_CSR_Pipe pipe;
    uint32_t subdeviceIndex;
};

class LMS64C_ADF_Over_PCIe_MMX8 : public lime::ISPI
{
  public:
    LMS64C_ADF_Over_PCIe_MMX8(std::shared_ptr<LitePCIe> dataPort, uint32_t subdeviceIndex)
        : pipe(dataPort)
        , subdeviceIndex(subdeviceIndex)
    {
    }
    void SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override
    {
        LMS64CProtocol::ADF4002_SPI(pipe, MOSI, count, subdeviceIndex);
    }
    void SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override
    {
        LMS64CProtocol::ADF4002_SPI(pipe, MOSI, count, subdeviceIndex);
    }

  private:
    PCIE_CSR_Pipe pipe;
    uint32_t subdeviceIndex;
};

SDRDevice* LimeSDR_MMX8Entry::make(const DeviceHandle& handle)
{
    std::shared_ptr<LitePCIe> control{ std::make_shared<LitePCIe>() };
    std::vector<std::shared_ptr<LitePCIe>> trxStreams(8);
    std::vector<std::shared_ptr<IComms>> controls(8);
    std::vector<std::shared_ptr<IComms>> fpga(8);
    ISPI* adfComms = new LMS64C_ADF_Over_PCIe_MMX8(control, 0);
    for (size_t i = 0; i < controls.size(); ++i)
    {
        controls[i] =
            std::shared_ptr<LMS64C_LMS7002M_Over_PCIe_MMX8>(std::make_shared<LMS64C_LMS7002M_Over_PCIe_MMX8>(control, i + 1));
        fpga[i] = std::shared_ptr<LMS64C_FPGA_Over_PCIe_MMX8>(std::make_shared<LMS64C_FPGA_Over_PCIe_MMX8>(control, i + 1));
    }
    fpga.push_back(std::shared_ptr<LMS64C_FPGA_Over_PCIe_MMX8>(std::make_shared<LMS64C_FPGA_Over_PCIe_MMX8>(control, 0)));

    try
    {
        std::string controlFile(handle.addr + "_control");
        control->Open(controlFile.c_str(), O_RDWR);

        std::string streamFile("");
        for (size_t i = 0; i < trxStreams.size(); ++i)
        {
            char portName[128];
            sprintf(portName, "%s_trx%li", handle.addr.c_str(), i);
            trxStreams[i] = std::shared_ptr<LitePCIe>(std::make_shared<LitePCIe>());
            trxStreams[i]->SetPathName(portName);
        }
        return new LimeSDR_MMX8(controls, fpga, std::move(trxStreams), adfComms);
    } catch (std::runtime_error& e)
    {
        char reason[256];
        sprintf(reason, "Unable to connect to device using handle(%s): %s", handle.Serialize().c_str(), e.what());
        throw std::runtime_error(reason);
    }
}
