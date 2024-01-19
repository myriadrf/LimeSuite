#include "LimeSDR_MiniEntry.h"
#include "LimeSDR_Mini.h"
#include "DeviceExceptions.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/DeviceHandle.h"
#include "protocols/LMS64CProtocol.h"
#include "Logger.h"
#include "USB_CSR_Pipe_Mini.h"
#include "LMS64C_LMS7002M_Over_USB.h"
#include "LMS64C_FPGA_Over_USB.h"

#include "FT601/FT601.h"

#ifndef __unix__
    #include "windows.h"
    #include "FTD3XXLibrary/FTD3XX.h"
#else
    #ifdef __GNUC__
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpedantic"
    #endif
    #include <libusb.h>
    #ifdef __GNUC__
        #pragma GCC diagnostic pop
    #endif
    #include <mutex>
#endif

using namespace lime;

void __loadLimeSDR_Mini(void) // TODO: fixme replace with LoadLibrary/dlopen
{
    static LimeSDR_MiniEntry limesdr_miniSupport; // Self register on initialization
}

// Device identifier vendor ID and product ID pairs.
static const std::set<VidPid> ids{ { 1027, 24607 } };

LimeSDR_MiniEntry::LimeSDR_MiniEntry()
    : USBEntry("LimeSDR_Mini", ids)
{
}

std::vector<DeviceHandle> LimeSDR_MiniEntry::enumerate(const DeviceHandle& hint)
{
    std::vector<DeviceHandle> handles;
#ifdef __unix__
    const std::string_view moduleName = "FT601";

    auto hintCopy = hint;
    if (hint.module.empty() || hint.module == moduleName)
    {
        hintCopy.module = "";
        handles = USBEntry::enumerate(hintCopy);
    }

    for (auto& handle : handles)
    {
        handle.module = std::string{ moduleName };
    }
#else
    if (!hint.media.empty() && hint.media.find("USB") == std::string::npos)
    {
        return handles;
    }

    FT_STATUS ftStatus = FT_OK;
    static DWORD numDevs = 0;

    ftStatus = FT_CreateDeviceInfoList(&numDevs);

    if (!FT_FAILED(ftStatus) && numDevs > 0)
    {
        DWORD Flags = 0;
        char SerialNumber[16] = { 0 };
        char Description[32] = { 0 };
        for (DWORD i = 0; i < numDevs; i++)
        {
            ftStatus = FT_GetDeviceInfoDetail(i, &Flags, nullptr, nullptr, nullptr, SerialNumber, Description, nullptr);
            if (!FT_FAILED(ftStatus))
            {
                ConnectionHandle handle;
                handle.media = Flags & FT_FLAGS_SUPERSPEED ? "USB 3" : Flags & FT_FLAGS_HISPEED ? "USB 2" : "USB";
                handle.module = "FT601";
                handle.name = Description;
                handle.index = i;
                handle.serial = SerialNumber;
                // Add handle conditionally, filter by serial number
                if (hint.serial.empty() || handle.serial.find(hint.serial) != std::string::npos)
                    handles.push_back(handle);
            }
        }
    }
#endif
    return handles;
}

SDRDevice* LimeSDR_MiniEntry::make(const DeviceHandle& handle)
{
    const auto splitPos = handle.addr.find(":");
    const uint16_t vid = std::stoi(handle.addr.substr(0, splitPos), nullptr, 16);
    const uint16_t pid = std::stoi(handle.addr.substr(splitPos + 1), nullptr, 16);

    auto usbComms = std::make_shared<FT601>(
#ifdef __unix__
        ctx
#endif
    );
    if (!usbComms->Connect(vid, pid, handle.serial))
    {
        const std::string reason = "Unable to connect to device using handle (" + handle.Serialize() + ")";
        throw std::runtime_error(reason);
    }

    auto usbPipe = std::make_shared<USB_CSR_Pipe_Mini>(*usbComms);

    // protocol layer
    auto route_lms7002m = std::make_shared<LMS64C_LMS7002M_Over_USB>(usbPipe);
    auto route_fpga = std::make_shared<LMS64C_FPGA_Over_USB>(usbPipe);

    return new LimeSDR_Mini(route_lms7002m, route_fpga, usbComms, usbPipe);
}
