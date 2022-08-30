#include "FX3.h"
#include <cstring>
#include "Logger.h"
#include <ciso646>
#include <fstream>
#include <thread>
#include <chrono>
#include <assert.h>

using namespace std;
using namespace lime;

static int activeUSBconnections = 0;
static std::thread gUSBProcessingThread;

#ifdef __unix__
void FX3::handle_libusb_events()
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    while(activeUSBconnections > 0)
    {
        int r = libusb_handle_events_timeout_completed(ctx, &tv, NULL);
        if(r != 0)
            lime::error("error libusb_handle_events %s", libusb_strerror(libusb_error(r)));
    }
}
#endif // __UNIX__

FX3::FX3(void *usbContext) : contexts(nullptr), isConnected(false)
{
    isConnected = false;
#ifdef __unix__
    dev_handle = nullptr;
    ctx = (libusb_context *)usbContext;

    if(activeUSBconnections == 0)
    {
        ++activeUSBconnections;
        gUSBProcessingThread = std::thread(&FX3::handle_libusb_events, this);
    }
    else
        ++activeUSBconnections;
#endif
}

FX3::~FX3()
{
    Disconnect();
    --activeUSBconnections;
    if(activeUSBconnections == 0)
    {
        if(gUSBProcessingThread.joinable())
            gUSBProcessingThread.join();
    }
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
        if (r < 0) {
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
    int e = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device
    if (e != LIBUSB_SUCCESS) {
        throw std::runtime_error(libusb_strerror((libusb_error)e));
        return ReportError(-1, "Cannot claim interface - %s", libusb_strerror(libusb_error(e)));
    }
#endif
    isConnected = true;
    contexts = new USBTransferContext[USB_MAX_CONTEXTS];
    return 0;
}
/**	@brief Closes communication to device.
*/
void FX3::Disconnect()
{
    if (contexts) {
#ifdef __unix__
    const libusb_version* ver = libusb_get_version();
    // Fix #358 libusb crash when freeing transfers(never used ones) without valid device handle. Bug in libusb 1.0.25 https://github.com/libusb/libusb/issues/1059
    const bool isBuggy_libusb_free_transfer = ver->major==1 && ver->minor==0 && ver->micro == 25;
    if (isBuggy_libusb_free_transfer && contexts) {
        for (int i = 0; i < USB_MAX_CONTEXTS; ++i)
            contexts[i].transfer->dev_handle = dev_handle;
    }
#endif
    delete[] contexts;
    contexts = nullptr;
    }

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

    assert(data);

#ifdef __unix__
    int actualTransferred = 0;
    int status = libusb_bulk_transfer(dev_handle, endPointAddr, data, length, &actualTransferred, timeout_ms);
    len = actualTransferred;
    if (status != 0)
        printf("FX3::BulkTransfer(0x%02X) : %s, transferred: %i, expected: %i\n", endPointAddr, libusb_error_name(status), actualTransferred, length);
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

    assert(data);
#ifdef __unix__
    len = libusb_control_transfer(dev_handle, requestType, request, value, index, data, length, timeout_ms);
#endif
    return len;
}

#ifdef __unix__
/** @brief Function for handling libusb callbacks
*/
static void process_libusbtransfer(libusb_transfer *trans)
{
    USBTransferContext *context = static_cast<USBTransferContext *>(trans->user_data);
    std::unique_lock<std::mutex> lck(context->transferLock);
    switch (trans->status) {
    case LIBUSB_TRANSFER_CANCELLED:
        context->bytesXfered = trans->actual_length;
        context->done.store(true);
        break;
    case LIBUSB_TRANSFER_COMPLETED:
        context->bytesXfered = trans->actual_length;
        context->done.store(true);
        break;
    case LIBUSB_TRANSFER_ERROR:
        lime::error("USB TRANSFER ERROR");
        context->bytesXfered = trans->actual_length;
        context->done.store(true);
        break;
    case LIBUSB_TRANSFER_TIMED_OUT:
        context->bytesXfered = trans->actual_length;
        context->done.store(true);
        break;
    case LIBUSB_TRANSFER_OVERFLOW:
        lime::error("USB transfer overflow");
        break;
    case LIBUSB_TRANSFER_STALL:
        lime::error("USB transfer stalled");
        break;
    case LIBUSB_TRANSFER_NO_DEVICE:
        lime::error("USB transfer no device");
        context->done.store(true);
        break;
    }
    lck.unlock();
    context->cv.notify_one();
}
#endif

/**
    @brief Starts asynchronous data reading from board
    @param *buffer buffer where to store received data
    @param length number of bytes to read
    @param streamBulkInAddr endpoint index?
    @return handle of transfer context
*/
int FX3::BeginDataXfer(const uint8_t *buffer, uint32_t length, uint8_t endPointAddr)
{
    int i = 0;
    bool contextFound = false;
    //find not used context
    for (i = 0; i < USB_MAX_CONTEXTS; i++) {
        if (!contexts[i].used) {
            contextFound = true;
            break;
        }
    }
    if (!contextFound) {
        printf("No contexts left for reading data\n");
        return -1;
    }
    contexts[i].used = true;
#ifndef __unix__
    if (InEndPt[endPointAddr & 0xF]) {
        contexts[i].EndPt = InEndPt[endPointAddr & 0xF];
        contexts[i].context =
            contexts[i].EndPt->BeginDataXfer((unsigned char *)buffer, length, contexts[i].inOvLap);
    }
    return i;
#else
    libusb_transfer *tr = contexts[i].transfer;
    libusb_fill_bulk_transfer(tr, dev_handle, endPointAddr, (unsigned char *)buffer, length,
                              process_libusbtransfer, &contexts[i], 0);
    contexts[i].done = false;
    contexts[i].bytesXfered = 0;
    int status = libusb_submit_transfer(tr);
    if (status != 0) {
        printf("BEGIN DATA READING %s", libusb_error_name(status));
        contexts[i].used = false;
        return -1;
    }
#endif
    return i;
}

/**
@brief Waits for asynchronous data reception
@param contextHandle handle of which context data to wait
@param timeout_ms number of miliseconds to wait
@return  true - wait finished, false - still waiting for transfer to complete
*/
bool FX3::WaitForXfer(int contextHandle, uint32_t timeout_ms)
{
    if (contextHandle >= 0 && contexts[contextHandle].used == true) {
#ifndef __unix__
        int status = 0;
        status =
            contexts[contextHandle].EndPt->WaitForXfer(contexts[contextHandle].inOvLap, timeout_ms);
        return status;
#else
        //blocking not to waste CPU
        std::unique_lock<std::mutex> lck(contexts[contextHandle].transferLock);
        return contexts[contextHandle].cv.wait_for(lck, chrono::milliseconds(timeout_ms), [&]() {
            return contexts[contextHandle].done.load();
        });
#endif
    }
    return true; //there is nothing to wait for (signal wait finished)
}

/**
    @brief Finishes asynchronous data reading from board
    @param buffer array where to store received data
    @param length number of bytes to read
    @param contextHandle handle of which context to finish
    @return negative values failure, positive number of bytes received
*/
int FX3::FinishDataXfer(const uint8_t *buffer, uint32_t length, int contextHandle)
{
    if (contextHandle >= 0 && contexts[contextHandle].used == true) {
#ifndef __unix__
        int status = 0;
        long len = length;
        status = contexts[contextHandle].EndPt->FinishDataXfer((unsigned char *)buffer, len,
                                                               contexts[contextHandle].inOvLap,
                                                               contexts[contextHandle].context);
        contexts[contextHandle].used = false;
        contexts[contextHandle].reset();
        return len;
#else
        length = contexts[contextHandle].bytesXfered;
        contexts[contextHandle].used = false;
        contexts[contextHandle].reset();
        return length;
#endif
    }
    else
        return 0;
}

/**
    @brief Aborts reading operations
*/
void FX3::AbortEndpointXfers(uint8_t endPointAddr)
{
#ifndef __unix__
    for (int i = 0; i < MAX_EP_CNT; i++)
        if (InEndPt[i] && InEndPt[i]->Address == endPointAddr)
            InEndPt[i]->Abort();
#else
    for (int i = 0; i < USB_MAX_CONTEXTS; ++i) {
        if (contexts[i].used && contexts[i].transfer->endpoint == endPointAddr)
            libusb_cancel_transfer(contexts[i].transfer);
    }
#endif
    for (int i = 0; i < USB_MAX_CONTEXTS; ++i) {
        if (contexts[i].used) {
            WaitForXfer(i, 250);
            FinishDataXfer(nullptr, 0, i);
        }
    }
}
