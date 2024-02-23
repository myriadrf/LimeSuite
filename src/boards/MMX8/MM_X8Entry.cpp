#ifdef __unix__
    #include <unistd.h>
#endif

#include "MM_X8Entry.h"
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
        std::size_t pos = devPath.find(searchDevName);
        if (pos == std::string::npos)
        {
            continue;
        }

        if (!hint.addr.empty() && devPath.find(hint.addr) == std::string::npos)
        {
            continue;
        }

        std::string dev_nr(&devPath[pos + searchDevName.length()], &devPath[devPath.find("_")]);
        handle.name = GetDeviceName(LMS_DEV_LIMESDR_MMX8) + (dev_nr == "0" ? "" : " (" + dev_nr + ")");

        handle.addr = devPath.substr(0, devPath.find("_"));

        if (handle.IsEqualIgnoringEmpty(hint))
        {
            handles.push_back(handle);
        }
    }
    return handles;
}

/** @brief A class for communicating with MMX8's subdevice's LMS7002M chips. */
class LMS64C_LMS7002M_Over_PCIe_MMX8 : public lime::IComms
{
  public:
    LMS64C_LMS7002M_Over_PCIe_MMX8(std::shared_ptr<LitePCIe> dataPort, uint32_t subdeviceIndex)
        : pipe(dataPort)
        , subdeviceIndex(subdeviceIndex)
    {
    }
    virtual OpStatus SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override { return SPI(0, MOSI, MISO, count); }
    virtual OpStatus SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override
    {
        return LMS64CProtocol::LMS7002M_SPI(pipe, spiBusAddress, MOSI, MISO, count, subdeviceIndex);
    }
    virtual OpStatus ResetDevice(int chipSelect) override { return LMS64CProtocol::DeviceReset(pipe, chipSelect, subdeviceIndex); };

  private:
    PCIE_CSR_Pipe pipe;
    uint32_t subdeviceIndex;
};

/** @brief A class for communicating with MMX8's subdevice's FPGA chips. */
class LMS64C_FPGA_Over_PCIe_MMX8 : public lime::IComms
{
  public:
    LMS64C_FPGA_Over_PCIe_MMX8(std::shared_ptr<LitePCIe> dataPort, uint32_t subdeviceIndex)
        : pipe(dataPort)
        , subdeviceIndex(subdeviceIndex)
    {
    }
    OpStatus SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override
    {
        return LMS64CProtocol::FPGA_SPI(pipe, MOSI, MISO, count, subdeviceIndex);
    }

    OpStatus SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override
    {
        return LMS64CProtocol::FPGA_SPI(pipe, MOSI, MISO, count, subdeviceIndex);
    }

    virtual OpStatus CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) override
    {
        return LMS64CProtocol::CustomParameterWrite(pipe, parameters, subdeviceIndex);
    };
    virtual OpStatus CustomParameterRead(std::vector<CustomParameterIO>& parameters) override
    {
        return LMS64CProtocol::CustomParameterRead(pipe, parameters, subdeviceIndex);
    }
    virtual OpStatus ProgramWrite(
        const char* data, size_t length, int prog_mode, int target, ProgressCallback callback = nullptr) override
    {
        return LMS64CProtocol::ProgramWrite(
            pipe, data, length, prog_mode, static_cast<LMS64CProtocol::ProgramWriteTarget>(target), callback, subdeviceIndex);
    }

    OpStatus MemoryWrite(uint32_t address, const void* data, uint32_t dataLength)
    {
        return LMS64CProtocol::MemoryWrite(pipe, address, data, dataLength, subdeviceIndex);
    }

    OpStatus MemoryRead(uint32_t address, void* data, uint32_t dataLength)
    {
        return LMS64CProtocol::MemoryRead(pipe, address, data, dataLength, subdeviceIndex);
    }

  private:
    PCIE_CSR_Pipe pipe;
    uint32_t subdeviceIndex;
};

/** @brief A class for communicating with MMX8's subdevice's ADF4002 chips. */
class LMS64C_ADF_Over_PCIe_MMX8 : public lime::ISPI
{
  public:
    LMS64C_ADF_Over_PCIe_MMX8(std::shared_ptr<LitePCIe> dataPort, uint32_t subdeviceIndex)
        : pipe(dataPort)
        , subdeviceIndex(subdeviceIndex)
    {
    }

    OpStatus SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override
    {
        return LMS64CProtocol::ADF4002_SPI(pipe, MOSI, count, subdeviceIndex);
    }
    OpStatus SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override
    {
        return LMS64CProtocol::ADF4002_SPI(pipe, MOSI, count, subdeviceIndex);
    }

  private:
    PCIE_CSR_Pipe pipe;
    uint32_t subdeviceIndex;
};

SDRDevice* LimeSDR_MMX8Entry::make(const DeviceHandle& handle)
{
    auto control = std::make_shared<LitePCIe>();
    std::vector<std::shared_ptr<LitePCIe>> trxStreams(8);
    std::vector<std::shared_ptr<IComms>> controls(8);
    std::vector<std::shared_ptr<IComms>> fpga(8);
    ISPI* adfComms = new LMS64C_ADF_Over_PCIe_MMX8(control, 0);
    for (size_t i = 0; i < controls.size(); ++i)
    {
        controls[i] = std::make_shared<LMS64C_LMS7002M_Over_PCIe_MMX8>(control, i + 1);
        fpga[i] = std::make_shared<LMS64C_FPGA_Over_PCIe_MMX8>(control, i + 1);
    }
    fpga.push_back(std::make_shared<LMS64C_FPGA_Over_PCIe_MMX8>(control, 0));

    try
    {
        std::string controlFile(handle.addr + "_control");
        control->Open(controlFile, O_RDWR);

        std::string streamFile("");
        for (size_t i = 0; i < trxStreams.size(); ++i)
        {
            const std::string portName = handle.addr + "_trx" + std::to_string(i);
            trxStreams[i] = std::make_shared<LitePCIe>();
            trxStreams[i]->SetPathName(portName);
        }
        auto controlPipe = std::make_shared<PCIE_CSR_Pipe>(control);
        return new LimeSDR_MMX8(controls, fpga, std::move(trxStreams), controlPipe, adfComms);
    } catch (std::runtime_error& e)
    {
        const std::string reason = "Unable to connect to device using handle (" + handle.Serialize() + "): " + e.what();
        throw std::runtime_error(reason);
    }
}
