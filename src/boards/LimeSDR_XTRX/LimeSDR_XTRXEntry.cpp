#ifdef __unix__
#include <unistd.h>
#endif

#include "LitePCIe.h"
#include "LimeSDR_XTRX.h"

#include <fstream>
#include <map>
#include <unistd.h>
#include <fcntl.h>
#include "LMSBoards.h"

using namespace lime;

void __loadLimeSDR_XTRX(void) //TODO fixme replace with LoadLibrary/dlopen
{
    static LimeSDR_XTRXEntry limesdr_XTRXSupport; // self register on initialization
}

LimeSDR_XTRXEntry::LimeSDR_XTRXEntry() : DeviceRegistryEntry("LimeSDR_XTRX")
{
}

LimeSDR_XTRXEntry::~LimeSDR_XTRXEntry()
{
}

std::vector<DeviceHandle> LimeSDR_XTRXEntry::enumerate(const DeviceHandle &hint)
{
    std::vector<DeviceHandle> handles;
    DeviceHandle handle;
    handle.media = "PCIe";

    if (!hint.media.empty() && hint.media != handle.media)
        return handles;

    const std::string searchDevName("LimeXTRX");
    if (!hint.name.empty() && searchDevName.find(hint.name) == std::string::npos)
        return handles;

    const std::string pattern(searchDevName + "[0-9]*_control");
    const std::vector<std::string> devices = LitePCIe::GetDevicesWithPattern(pattern);

    for(const auto& devPath : devices)
    {
        size_t pos = devPath.find(searchDevName);
        if(pos == std::string::npos)
            continue;

        if (!hint.addr.empty() && devPath.find(hint.addr) == std::string::npos)
            continue;

        std::string dev_nr(&devPath[pos+searchDevName.length()], &devPath[devPath.find("_")]);
        handle.name = searchDevName + (dev_nr == "0" ? "" : " (" + dev_nr + ")");

        handle.addr = devPath.substr(0, devPath.find("_"));
        handles.push_back(handle);
    }
    return handles;
}

SDRDevice* LimeSDR_XTRXEntry::make(const DeviceHandle &handle)
{
    LitePCIe* control = new LitePCIe();
    LitePCIe* stream = new LitePCIe();
    try {
        std::string controlFile(handle.addr + "_control");
        control->Open(controlFile.c_str(), O_RDWR);

        std::string streamFile(handle.addr + "_trx0");
        stream->SetPathName(streamFile.c_str());

        return new LimeSDR_XTRX(control, stream);
    }
    catch ( std::runtime_error &e )
    {
        delete control;
        delete stream;
        char reason[256];
        sprintf(reason, "Unable to connect to device using handle(%s)", handle.Serialize().c_str());
        throw std::runtime_error(reason);
    }
}
