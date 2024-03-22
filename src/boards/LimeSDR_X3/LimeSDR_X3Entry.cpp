#ifdef __unix__
    #include <unistd.h>
#endif

#include "LimeSDR_X3Entry.h"
#include "LitePCIe.h"
#include "LimeSDR_X3.h"
#include "PCIE_CSR_Pipe.h"
#include "LMS64C_FPGA_Over_PCIe.h"
#include "LMS64C_LMS7002M_Over_PCIe.h"

#include <fstream>
#include <map>
#include <fcntl.h>

using namespace lime;

void __loadLimeSDR_X3(void) //TODO fixme replace with LoadLibrary/dlopen
{
    static LimeSDR_X3Entry limesdr_X3Support; // self register on initialization
}

LimeSDR_X3Entry::LimeSDR_X3Entry()
    : DeviceRegistryEntry("LimeSDR_X3")
{
}

LimeSDR_X3Entry::~LimeSDR_X3Entry()
{
}

std::vector<DeviceHandle> LimeSDR_X3Entry::enumerate(const DeviceHandle& hint)
{
    std::vector<DeviceHandle> handles;
    DeviceHandle handle;
    handle.media = "PCIe";

    if (!hint.media.empty() && hint.media != handle.media)
        return handles;

    const std::string searchDevName("LimeX3");
    const std::vector<std::string> boardNames = { GetDeviceName(LMS_DEV_LIMESDR_X3), "LimeSDR-X3", searchDevName };
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
        handle.name = boardNames[0]; // + (dev_nr == "0" ? "" : " (" + dev_nr + ")");

        handle.addr = devPath.substr(0, devPath.find("_"));

        if (handle.IsEqualIgnoringEmpty(hint))
        {
            handles.push_back(handle);
        }
    }
    return handles;
}

SDRDevice* LimeSDR_X3Entry::make(const DeviceHandle& handle)
{
    // Data transmission layer
    auto control = std::make_shared<LitePCIe>();
    std::vector<std::shared_ptr<LitePCIe>> trxStreams(3);

    // protocol layer
    auto route_lms7002m = std::make_shared<LMS64C_LMS7002M_Over_PCIe>(control);
    auto route_fpga = std::make_shared<LMS64C_FPGA_Over_PCIe>(control);

    try
    {
        std::string controlFile(handle.addr + "_control");
        control->Open(controlFile, O_RDWR);

        std::string streamFile("");
        for (int i = 0; i < 3; ++i)
        {
            const std::string portName = handle.addr + "_trx" + std::to_string(i);
            trxStreams[i] = std::make_shared<LitePCIe>();
            trxStreams[i]->SetPathName(portName);
        }

        auto controlPipe = std::make_shared<PCIE_CSR_Pipe>(control);
        return new LimeSDR_X3(route_lms7002m, route_fpga, std::move(trxStreams), controlPipe);
    } catch (std::runtime_error& e)
    {
        const std::string reason = "Unable to connect to device using handle (" + handle.Serialize() + ")";
        throw std::runtime_error(reason);
    }
}
