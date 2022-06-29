#include "FX3.h"
#include <cstring>
#include "Si5351C.h"
#include "FPGA_common.h"
#include "LMS7002M.h"
#include "Logger.h"
#include <ciso646>
#include <fstream>
#include <thread>
#include <chrono>

using namespace std;
using namespace lime;

FX3::FX3(void* usbContext) : isConnected(false)
{
    isConnected = false;
#ifdef __unix__
    dev_handle = nullptr;
    ctx = (libusb_context *)usbContext;
#endif
}

FX3::~FX3()
{
    Disconnect();
}

bool FX3::Connect(uint16_t vid, uint16_t pid, const std::string &serial)
{
    Disconnect();
#ifdef __unix__
    libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
    int usbDeviceCount = libusb_get_device_list(ctx, &devs);

    if (usbDeviceCount < 0)
        return ReportError(-1, "libusb_get_device_list failed: %s", libusb_strerror(libusb_error(usbDeviceCount)));

    for(int i=0; i<usbDeviceCount; ++i)
    {
        libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(devs[i], &desc);
        if(r<0) 
        {
            lime::error("failed to get device description");
            continue;
        }
        if (desc.idProduct != pid || desc.idVendor != vid)
            continue;

        if(libusb_open(devs[i], &dev_handle) != 0)
            continue;

        std::string foundSerial;
        if (desc.iSerialNumber > 0)
        {
            char data[255];
            r = libusb_get_string_descriptor_ascii(dev_handle,desc.iSerialNumber,(unsigned char*)data, sizeof(data));
            if(r<0)
                lime::error("failed to get serial number");
            else
                foundSerial = std::string(data, size_t(r));
        }

        if (serial == foundSerial)
            break; //found it
        libusb_close(dev_handle);
        dev_handle = nullptr;
    }
    libusb_free_device_list(devs, 1);

    if(dev_handle == nullptr)
        return ReportError(-1, "libusb_open failed");
    if(libusb_kernel_driver_active(dev_handle, 0) == 1)   //find out if kernel driver is attached
    {
        lime::info("Kernel Driver Active");
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
            lime::info("Kernel Driver Detached!");
    }
    int r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device
    if(r < 0)
        return ReportError(-1, "Cannot claim interface - %s", libusb_strerror(libusb_error(r)));
#endif
    isConnected = true;
    //contexts = new USBTransferContext[USB_MAX_CONTEXTS];
    //contextsToSend = new USBTransferContext[USB_MAX_CONTEXTS];
    return 0;
}
/**	@brief Closes communication to device.
*/
void FX3::Disconnect()
{
#ifdef __unix__
    const libusb_version* ver = libusb_get_version();
    // Fix #358 libusb crash when freeing transfers(never used ones) without valid device handle. Bug in libusb 1.0.25 https://github.com/libusb/libusb/issues/1059
    const bool isBuggy_libusb_free_transfer = ver->major==1 && ver->minor==0 && ver->micro == 25;
    if(isBuggy_libusb_free_transfer)
    {
        if(contexts)
            for(int i=0; i<USB_MAX_CONTEXTS; ++i)
                contexts[i].transfer->dev_handle = dev_handle;
        if(contextsToSend)
            for(int i=0; i<USB_MAX_CONTEXTS; ++i)
                contextsToSend[i].transfer->dev_handle = dev_handle;
    }
#endif

    // if(contexts)
    // {
    //     delete[] contexts;
    //     contexts = nullptr;
    // }
    // if(contextsToSend)
    // {
    //     delete[] contextsToSend;
    //     contextsToSend = nullptr;
    // }

#ifdef __unix__
    if(dev_handle != 0)
    {
        libusb_release_interface(dev_handle, 0);
        libusb_close(dev_handle);
        dev_handle = 0;
    }
#endif
    isConnected = false;
}

/** @brief Returns connection status
    @return 1-connection open, 0-connection closed.
*/
inline bool FX3::IsConnected()
{
#ifdef __unix__
    return isConnected;
#endif
}

int32_t FX3::BulkTransfer(uint8_t endPointAddr, uint8_t *data, int length, int32_t timeout_ms)
{
    long len = 0;
    if (not IsConnected())
        throw runtime_error("BulkTransfer: USB device is not connected");

#ifdef __unix__
    int actual = 0;
    libusb_bulk_transfer(dev_handle, endPointAddr, data, length, &actual, timeout_ms);
    len = actual;
#endif
    return len;
}

int32_t FX3::ControlTransfer(int requestType, int request, int value, int index,
        uint8_t* data, uint32_t length,
        int32_t timeout_ms)
{
    long len = length;
    if (not IsConnected())
        throw runtime_error("BulkTransfer: USB device is not connected");

#ifdef __unix__
    len = libusb_control_transfer(dev_handle, requestType, request, value, index, data, length, timeout_ms);
#endif
    return len;
}
