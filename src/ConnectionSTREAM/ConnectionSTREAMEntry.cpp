/**
    @file ConnectionSTREAMEntry.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/

#include "ConnectionSTREAM.h"
#include <iostream>
using namespace lime;

#ifdef __unix__
void ConnectionSTREAMEntry::handle_libusb_events()
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 250000;
    while(mProcessUSBEvents.load() == true)
    {
        int r = libusb_handle_events_timeout_completed(ctx, &tv, NULL);
        if(r != 0) printf("error libusb_handle_events %s\n", libusb_strerror(libusb_error(r)));
    }
}
#endif // __UNIX__

//! make a static-initialized entry in the registry
void __loadConnectionSTREAMEntry(void) //TODO fixme replace with LoadLibrary/dlopen
{
static ConnectionSTREAMEntry STREAMEntry;
}

int USBTransferContext::idCounter = 0;

ConnectionSTREAMEntry::ConnectionSTREAMEntry(void):
    ConnectionRegistryEntry("STREAM")
{
#ifdef __unix__
    int r = libusb_init(&ctx); //initialize the library for the session we just declared
    if(r < 0)
        printf("Init Error %i\n", r); //there was an error
    libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation
    mProcessUSBEvents.store(true);
    mUSBProcessingThread = std::thread(&ConnectionSTREAMEntry::handle_libusb_events, this);
#endif
}

ConnectionSTREAMEntry::ConnectionSTREAMEntry(const std::string entryName):
    ConnectionRegistryEntry(entryName)
{
#ifdef __unix__
    int r = libusb_init(&ctx); //initialize the library for the session we just declared
    if(r < 0)
        printf("Init Error %i\n", r); //there was an error
    libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation
    mProcessUSBEvents.store(true);
    mUSBProcessingThread = std::thread(&ConnectionSTREAMEntry::handle_libusb_events, this);
#endif
}

ConnectionSTREAMEntry::~ConnectionSTREAMEntry(void)
{
#ifdef __unix__
    mProcessUSBEvents.store(false);
    mUSBProcessingThread.join();
    libusb_exit(ctx);
#endif
}

#ifndef __unix__
/** @return name of usb device as string.
    @param index device index in list
*/
std::string ConnectionSTREAMEntry::DeviceName(unsigned int index)
{
    std::string name;
    char tempName[USB_STRING_MAXLEN];
    CCyUSBDevice device;
    if (index >= device.DeviceCount())
        return "";

    for (int i = 0; i < USB_STRING_MAXLEN; ++i)
            tempName[i] = device.DeviceName[i];
    if (device.bSuperSpeed == true)
        name = "USB 3.0";
    else if (device.bHighSpeed == true)
        name = "USB 2.0";
    else
        name = "USB";
    name += " (";
    name += tempName;
    name += ")";
    return name;
}
#endif

std::vector<ConnectionHandle> ConnectionSTREAMEntry::enumerate(const ConnectionHandle &hint)
{
    std::vector<ConnectionHandle> handles;

#ifndef __unix__
	CCyUSBDevice device;
	if (device.DeviceCount())
    {
		for (int i = 0; i<device.DeviceCount(); ++i)
        {
			if (hint.index >= 0 && hint.index != i)
				continue;
			if (device.IsOpen())
				device.Close();
            device.Open(i);
            ConnectionHandle handle;
            handle.media = "USB";
            handle.name = DeviceName(i);
            handle.index = i;
            std::wstring ws(device.SerialNumber);
            handle.serial = std::string(ws.begin(),ws.end());
            handles.push_back(handle);
			device.Close();
        }
    }
#else
    libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
    int usbDeviceCount = libusb_get_device_list(ctx, &devs);
    if(usbDeviceCount > 0)
    {
        libusb_device_descriptor desc;
        for(int i=0; i<usbDeviceCount; ++i)
        {
            int r = libusb_get_device_descriptor(devs[i], &desc);
            if(r<0)
                printf("failed to get device description\n");
            int pid = desc.idProduct;
            int vid = desc.idVendor;

            if( vid == 1204)
            {
                if(pid == 34323)
                {
                    ConnectionHandle handle;
                    handle.media = "USB";
                    handle.name = "DigiGreen";
                    handle.addr = std::to_string(int(pid))+":"+std::to_string(int(vid));
                    handles.push_back(handle);
                }
                else if(pid == 241 || pid == 243)
                {
                    libusb_device_handle *tempDev_handle;
                    tempDev_handle = libusb_open_device_with_vid_pid(ctx, vid, pid);
                    if(libusb_kernel_driver_active(tempDev_handle, 0) == 1)   //find out if kernel driver is attached
                    {
                        if(libusb_detach_kernel_driver(tempDev_handle, 0) == 0) //detach it
                            printf("Kernel Driver Detached!\n");
                    }
                    if(libusb_claim_interface(tempDev_handle, 0) < 0) //claim interface 0 (the first) of device
                    {
                        printf("Cannot Claim Interface\n");
                    }

                    std::string fullName;
                    //check operating speed
                    int speed = libusb_get_device_speed(devs[i]);
                    if(speed == LIBUSB_SPEED_HIGH)
                        fullName = "USB 2.0";
                    else if(speed == LIBUSB_SPEED_SUPER)
                        fullName = "USB 3.0";
                    else
                        fullName = "USB";
                    fullName += " (";
                    //read device name
                    char data[255];
                    memset(data, 0, 255);
                    libusb_get_string_descriptor_ascii(tempDev_handle, 2, (unsigned char*)data, 255);
                    if(strlen(data) > 0)
                        fullName += data;
                    fullName += ")";

                    ConnectionHandle handle;
                    handle.media = "USB";
                    handle.name = fullName;
                    handle.addr = std::to_string(int(pid))+":"+std::to_string(int(vid));

                    if (desc.iSerialNumber > 0)
                    {
                        r = libusb_get_string_descriptor_ascii(tempDev_handle,desc.iSerialNumber,(unsigned char*)data, 255);
                        if(r<0)
                            printf("failed to get serial number\n");
                        else if (strlen(data) > 0)
                            handle.serial = std::string((const char*)data);
                    }
                    libusb_close(tempDev_handle);

                    handles.push_back(handle);
                }
            }
        }
    }
    else
    {
        libusb_free_device_list(devs, 1);
    }
#endif
    return handles;
}

IConnection *ConnectionSTREAMEntry::make(const ConnectionHandle &handle)
{
#ifndef __unix__
    return new ConnectionSTREAM(nullptr, handle.index);
#else
    const auto pidvid = handle.addr;
    const auto splitPos = pidvid.find(":");
    const auto pid = std::stoi(pidvid.substr(0, splitPos));
    const auto vid = std::stoi(pidvid.substr(splitPos+1));
    return new ConnectionSTREAM(ctx, handle.index, vid, pid);
#endif
}
