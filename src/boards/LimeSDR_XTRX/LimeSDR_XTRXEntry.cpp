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

    const std::string searchDevName("LimeXTRX");
    const std::string pattern(searchDevName + "[0-9]*_control");
    const std::vector<std::string> devices = LitePCIe::GetDevicesWithPattern(pattern);

    for(const auto& port : devices)
    {
        size_t pos = port.find(searchDevName);
        if(pos == std::string::npos)
            continue;

        std::string dev_nr(&port[pos+searchDevName.length()], &port[port.find("_")]);
        handle.name = GetDeviceName(LMS_DEV_LIMESDR_XTRX) + (dev_nr == "0" ? "" : " (" + dev_nr + ")");

        handle.addr = port.substr(0, port.find("_"));
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
        sprintf(reason, "Unable to connect to device using handle(%s)", handle.serialize().c_str());
        throw std::runtime_error(reason);
    }
}
