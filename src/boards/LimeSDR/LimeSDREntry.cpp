#include "LimeSDREntry.h"
#include "LimeSDR.h"
#include "FX3/FX3.h"
#include "USB_CSR_Pipe_SDR.h"
#include "LMS64C_LMS7002M_Over_USB.h"
#include "LMS64C_FPGA_Over_USB.h"

#include <memory>
#include <stdexcept>

#ifndef __unix__
    #include "windows.h"
    #include "CyAPI.h"
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

void __loadLimeSDR(void) //TODO fixme replace with LoadLibrary/dlopen
{
    static LimeSDREntry limesdrSupport; // self register on initialization
}

// Device identifier vendor ID and product ID pairs.
static const std::set<VidPid> ids{ { 1204, 241 }, { 1204, 243 }, { 7504, 24840 } };

LimeSDREntry::LimeSDREntry()
    : USBEntry("LimeSDR", ids)
{
}

#ifndef __unix__
std::vector<DeviceHandle> LimeSDREntry::enumerate(const DeviceHandle& hint)
{
    std::vector<DeviceHandle> handles;
    if (!hint.media.empty() && hint.media.find("USB") == std::string::npos)
    {
        return handles;
    }

    CCyUSBDevice device;
    if (device.DeviceCount() > 0)
    {
        for (int i = 0; i < device.DeviceCount(); ++i)
        {
            if (device.IsOpen())
                device.Close();
            device.Open(i);
            DeviceHandle handle;
            if (device.bSuperSpeed == true)
                handle.media = "USB 3.0";
            else if (device.bHighSpeed == true)
                handle.media = "USB 2.0";
            else
                handle.media = "USB";
            handle.name = device.DeviceName;
            std::wstring ws(device.SerialNumber);
            handle.serial = std::string(ws.begin(), ws.end());
            if (hint.serial.empty() or handle.serial.find(hint.serial) != std::string::npos)
                handles.push_back(handle); //filter on serial
            device.Close();
        }
    }
    return handles;
}
#endif

SDRDevice* LimeSDREntry::make(const DeviceHandle& handle)
{
    const auto splitPos = handle.addr.find(":");
    uint16_t vid = 0;
    uint16_t pid = 0;

    if (splitPos != std::string::npos)
    {
        vid = std::stoi(handle.addr.substr(0, splitPos), nullptr, 16);
        pid = std::stoi(handle.addr.substr(splitPos + 1), nullptr, 16);
    }
 
    auto usbComms = std::make_shared<FX3>(
#ifdef __unix__
        ctx
#endif
    );
    if (!usbComms->Connect(vid, pid, handle.serial))
    {
        const std::string reason = "Unable to connect to device using handle (" + handle.Serialize() + ")";
        throw std::runtime_error(reason);
    }

    auto usbPipe = std::make_shared<USB_CSR_Pipe_SDR>(*usbComms);

    // protocol layer
    auto route_lms7002m = std::make_shared<LMS64C_LMS7002M_Over_USB>(usbPipe);
    auto route_fpga = std::make_shared<LMS64C_FPGA_Over_USB>(usbPipe);

    return new LimeSDR(route_lms7002m, route_fpga, usbComms, usbPipe);
}
