#ifdef __unix__
    #include <unistd.h>
#endif

#include "MM_X8Entry.h"
#include "LitePCIe.h"
#include "MM_X8.h"
#include "PCIE_CSR_Pipe.h"
#include "LMS64C_ADF_Over_PCIe_MMX8.h"
#include "LMS64C_FPGA_Over_PCIe_MMX8.h"
#include "LMS64C_LMS7002M_Over_PCIe_MMX8.h"

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

SDRDevice* LimeSDR_MMX8Entry::make(const DeviceHandle& handle)
{
    auto control = std::make_shared<LitePCIe>();
    std::vector<std::shared_ptr<LitePCIe>> trxStreams(8);
    std::vector<std::shared_ptr<IComms>> controls(8);
    std::vector<std::shared_ptr<IComms>> fpga(8);
    auto adfComms = std::make_shared<LMS64C_ADF_Over_PCIe_MMX8>(control, 0);
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
