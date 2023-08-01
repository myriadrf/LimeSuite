#include "LimeSDR.h"
#include "DeviceExceptions.h"
#include "DeviceRegistry.h"
#include "DeviceHandle.h"
#include "Logger.h"

#include "FX3.h"

#ifndef __unix__
#include "windows.h"
#include "CyAPI.h"
#else
#include <libusb.h>
#include <mutex>
#endif

using namespace lime;

static libusb_context* ctx; //a libusb session

void __loadLimeSDR(void) //TODO fixme replace with LoadLibrary/dlopen
{
    static LimeSDREntry limesdrSupport; // self register on initialization
}

LimeSDREntry::LimeSDREntry() : DeviceRegistryEntry("LimeSDR")
{
#ifdef __unix__
    int r = libusb_init(&ctx); //initialize the library for the session we just declared
    if(r < 0)
        lime::error("Init Error %i", r); //there was an error
#if LIBUSBX_API_VERSION < 0x01000106
    libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation
#else
    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, 3); //set verbosity level to 3, as suggested in the documentation
#endif

#endif
}

LimeSDREntry::~LimeSDREntry()
{
#ifdef __unix__
    libusb_exit(ctx);
#endif
}

std::vector<DeviceHandle> LimeSDREntry::enumerate(const DeviceHandle &hint)
{
    std::vector<DeviceHandle> handles;

#ifndef __unix__
    CCyUSBDevice device;
    if (device.DeviceCount())
    {
        for (int i = 0; i<device.DeviceCount(); ++i)
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
            handle.serial = std::string(ws.begin(),ws.end());
            if (hint.serial.empty() or handle.serial.find(hint.serial) != std::string::npos)
                handles.push_back(handle); //filter on serial
            device.Close();
        }
    }
#else
    libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
    int usbDeviceCount = libusb_get_device_list(ctx, &devs);

    if (usbDeviceCount < 0) {
        lime::error("failed to get libusb device list: %s", libusb_strerror(libusb_error(usbDeviceCount)));
        return handles;
    }

    for(int i=0; i<usbDeviceCount; ++i)
    {
        libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(devs[i], &desc);
        if(r<0)
            lime::error("failed to get device description");
        int pid = desc.idProduct;
        int vid = desc.idVendor;

        if((vid == 1204 && pid == 241) || (vid == 1204 && pid == 243) || (vid == 7504 && pid == 24840))
        {
            libusb_device_handle *tempDev_handle(nullptr);
            if(libusb_open(devs[i], &tempDev_handle) != 0 || tempDev_handle == nullptr)
                continue;

            DeviceHandle handle;

            //check operating speed
            int speed = libusb_get_device_speed(devs[i]);
            if(speed == LIBUSB_SPEED_HIGH)
                handle.media = "USB 2.0";
            else if(speed == LIBUSB_SPEED_SUPER)
                handle.media = "USB 3.0";
            else
                handle.media = "USB";

            //read device name
            char data[255];
            r = libusb_get_string_descriptor_ascii(tempDev_handle,  LIBUSB_CLASS_COMM, (unsigned char*)data, sizeof(data));
            if(r > 0) handle.name = std::string(data, size_t(r));

            r = std::sprintf(data, "%.4x:%.4x", int(vid), int(pid));
            if (r > 0) handle.addr = std::string(data, size_t(r));

            if (desc.iSerialNumber > 0)
            {
                r = libusb_get_string_descriptor_ascii(tempDev_handle,desc.iSerialNumber,(unsigned char*)data, sizeof(data));
                if(r<0)
                    lime::error("failed to get serial number");
                else
                    handle.serial = std::string(data, size_t(r));
            }
            libusb_close(tempDev_handle);

            //add handle conditionally, filter by serial number
            if (hint.serial.empty() or handle.serial.find(hint.serial) != std::string::npos)
                handles.push_back(handle);
        }
    }

    libusb_free_device_list(devs, 1);
#endif
    return handles;
}

SDRDevice *LimeSDREntry::make(const DeviceHandle &handle)
{
    FX3* usbComms = nullptr;

    const auto splitPos = handle.addr.find(":");
    const uint16_t vid = std::stoi(handle.addr.substr(0, splitPos), nullptr, 16);
    const uint16_t pid = std::stoi(handle.addr.substr(splitPos+1), nullptr, 16);

    usbComms = new FX3();
    if (usbComms->Connect(vid, pid, handle.serial) != 0)
    {
        delete usbComms;
        char reason[256];
        sprintf(reason, "Unable to connect to device using handle(%s)", handle.Serialize().c_str());
        throw std::runtime_error(reason);
    }
    return new LimeSDR(usbComms);//ctx, handle.addr, handle.serial, handle.index);
}
