#include "FT601.h"
#include <thread>
#include "Logger.h"
#include <assert.h>
#include "dataTypes.h"

#ifndef __unix__
#include "windows.h"
#include "FTD3XXLibrary/FTD3XX.h"
#else
#include <libusb.h>
#include <mutex>
#include <condition_variable>
#include <chrono>
#endif

using namespace lime;

static constexpr int streamBulkWriteAddr = 0x03;
static constexpr int streamBulkReadAddr = 0x83;

static constexpr int ctrlBulkWriteAddr = 0x02;
static constexpr int ctrlBulkReadAddr = 0x82;

static int activeUSBconnections = 0;
static std::thread gUSBProcessingThread;

#ifdef __unix__
void FT601::handle_libusb_events()
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;

    while (activeUSBconnections > 0)
    {
        int returnCode = libusb_handle_events_timeout_completed(ctx, &tv, NULL);

        if (returnCode != 0)
        {
            lime::error("error libusb_handle_events %s", libusb_strerror(static_cast<libusb_error>(returnCode)));
        }
    }
}
#endif // __UNIX__

FT601::FT601(void *usbContext) : contexts(nullptr), isConnected(false)
{
    isConnected = false;
#ifdef __unix__
    dev_handle = nullptr;
    ctx = reinterpret_cast<libusb_context*>(usbContext);

    if (activeUSBconnections == 0)
    {
        ++activeUSBconnections;
        gUSBProcessingThread = std::thread(&FT601::handle_libusb_events, this);
    }
    else
    {
        ++activeUSBconnections;
    }
#endif
}

FT601::~FT601()
{
    Disconnect();
    --activeUSBconnections;

    if (activeUSBconnections == 0)
    {
        if (gUSBProcessingThread.joinable())
        {
            gUSBProcessingThread.join();
        }
    }
}

bool FT601::Connect(uint16_t vid, uint16_t pid, const std::string &serial)
{
#ifndef __unix__
    DWORD devCount;
    FT_STATUS ftStatus = FT_OK;
    DWORD dwNumDevices = 0;
    // Open a device
    ftStatus = FT_Create(reinterpret_cast<void*>(const_cast<char*>(serial.c_str())), FT_OPEN_BY_SERIAL_NUMBER, &mFTHandle);

    if (FT_FAILED(ftStatus))
    {
        ReportError(ENODEV, "Failed to list USB Devices");
        return -1;
    }

    FT_AbortPipe(mFTHandle, streamBulkReadAddr);
    FT_AbortPipe(mFTHandle, ctrlBulkReadAddr);
    FT_AbortPipe(mFTHandle, ctrlBulkWriteAddr);
    FT_AbortPipe(mFTHandle, streamBulkWriteAddr);
    FT_SetStreamPipe(mFTHandle, FALSE, FALSE, ctrlBulkReadAddr, 64);
    FT_SetStreamPipe(mFTHandle, FALSE, FALSE, ctrlBulkWriteAddr, 64);
    FT_SetStreamPipe(mFTHandle, FALSE, FALSE, streamBulkReadAddr, sizeof(FPGA_DataPacket));
    FT_SetStreamPipe(mFTHandle, FALSE, FALSE, streamBulkWriteAddr, sizeof(FPGA_DataPacket));
    FT_SetPipeTimeout(mFTHandle, ctrlBulkWriteAddr, 500);
    FT_SetPipeTimeout(mFTHandle, ctrlBulkReadAddr, 500);
    FT_SetPipeTimeout(mFTHandle, streamBulkReadAddr, 0);
    FT_SetPipeTimeout(mFTHandle, streamBulkWriteAddr, 0);
    isConnected = true;
    return 0;
#else
    libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
    int usbDeviceCount = libusb_get_device_list(ctx, &devs);

    if (usbDeviceCount < 0)
    {
        return ReportError(-1, "libusb_get_device_list failed: %s", libusb_strerror(static_cast<libusb_error>(usbDeviceCount)));
    }

    for (int i = 0; i < usbDeviceCount; ++i)
    {
        libusb_device_descriptor desc;
        int returnCode = libusb_get_device_descriptor(devs[i], &desc);

        if (returnCode < 0)
        {
            lime::error("failed to get device description");
            continue;
        }

        if (desc.idProduct != pid || desc.idVendor != vid)
        {
            continue;
        }

        if (libusb_open(devs[i], &dev_handle) != 0)
        {
            continue;
        }

        std::string foundSerial;

        if (desc.iSerialNumber > 0)
        {
            char data[255];
            int stringLength = libusb_get_string_descriptor_ascii(dev_handle, desc.iSerialNumber, reinterpret_cast<unsigned char*>(data), sizeof(data));

            if (stringLength < 0)
            {
                lime::error("failed to get serial number");
            }
            else
            {
                foundSerial = std::string(data, static_cast<size_t>(stringLength));
            }
        }

        if (serial == foundSerial) // found it
        {
            break;
        }

        libusb_close(dev_handle);
        dev_handle = nullptr;
    }

    libusb_free_device_list(devs, 1);

    if (dev_handle == nullptr)
    {
        return ReportError(ENODEV, "libusb_open failed");
    }

    if (libusb_kernel_driver_active(dev_handle, 1) == 1)   //find out if kernel driver is attached
    {
        lime::debug("Kernel Driver Active");

        if(libusb_detach_kernel_driver(dev_handle, 1) == 0) //detach it
        {
            lime::debug("Kernel Driver Detached!");
        }
    }

    int returnCode = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device
    if (returnCode < 0)
    {
        return ReportError(-1, "Cannot claim interface - %s", libusb_strerror(static_cast<libusb_error>(returnCode)));
    }

    returnCode = libusb_claim_interface(dev_handle, 1); // claim interface 1 of device
    if (returnCode < 0)
    {
        return ReportError(-1, "Cannot claim interface - %s", libusb_strerror(static_cast<libusb_error>(returnCode)));
    }
    lime::debug("Claimed Interface");

    if (libusb_reset_device(dev_handle) != 0)
    {
        return ReportError(-1, "USB reset failed", libusb_strerror(static_cast<libusb_error>(returnCode)));
    }

    FT_FlushPipe(ctrlBulkReadAddr);  //clear ctrl ep rx buffer
    FT_SetStreamPipe(ctrlBulkReadAddr, 64);
    FT_SetStreamPipe(ctrlBulkWriteAddr, 64);
    isConnected = true;
    return 0;
#endif
}

bool FT601::IsConnected()
{
    return isConnected;
}

void FT601::Disconnect()
{
#ifndef __unix__
    FT_Close(mFTHandle);
#else
    if (dev_handle != 0)
    {
        FT_FlushPipe(streamBulkReadAddr);
        FT_FlushPipe(ctrlBulkReadAddr);
        libusb_release_interface(dev_handle, 1);
        libusb_close(dev_handle);
        dev_handle = 0;
    }
#endif
    isConnected = false;
}

int32_t FT601::BulkTransfer(uint8_t endPointAddr, uint8_t *data, int length, int32_t timeout_ms)
{
    long len = 0;
    if (not IsConnected())
    {
        throw std::runtime_error("BulkTransfer: USB device is not connected");
    }

    assert(data);

#ifndef __unix__
    ULONG ulBytesTransferred = 0;
    FT_STATUS ftStatus = FT_OK;
    OVERLAPPED vOverlapped = { 0 };
    FT_InitializeOverlapped(mFTHandle, &vOverlapped);

    if (endPointAddr == ctrlBulkWriteAddr)
    {
        ftStatus = FT_WritePipe(mFTHandle, ctrlBulkWriteAddr, data, length, &ulBytesTransferred, &vOverlapped);
    }
    else
    {
        ftStatus = FT_ReadPipe(mFTHandle, ctrlBulkReadAddr, data, length, &ulBytesTransferred, &vOverlapped);
    }

    if (ftStatus != FT_IO_PENDING)
    {
        FT_ReleaseOverlapped(mFTHandle, &vOverlapped);
        ReinitPipe(endPointAddr);
        return -1;
    }

    DWORD dwRet = WaitForSingleObject(vOverlapped.hEvent, timeout_ms);
    if (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_TIMEOUT)
    {
        if (FT_GetOverlappedResult(mFTHandle, &vOverlapped, &ulBytesTransferred, FALSE) == FALSE)
        {
            ReinitPipe(endPointAddr);
            ulBytesTransferred = -1;
        }
    }
    else
    {
        ReinitPipe(endPointAddr);
        ulBytesTransferred = -1;
    }

    FT_ReleaseOverlapped(mFTHandle, &vOverlapped);
    return ulBytesTransferred;
#else
    int actualTransferred = 0;
    int status = libusb_bulk_transfer(dev_handle, endPointAddr, data, length, &actualTransferred, timeout_ms);
    len = actualTransferred;
    if (status != 0)
    {
        printf("FT601::BulkTransfer(0x%02X) : %s, transferred: %i, expected: %i\n", endPointAddr, libusb_error_name(status), actualTransferred, length);
    }
#endif
    return len;
}

#ifdef __unix__
/** @brief Function for handling libusb callbacks
*/
static void process_libusbtransfer(libusb_transfer *trans)
{
    USBTransferContext_FT601 *context = static_cast<USBTransferContext_FT601 *>(trans->user_data);
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

int FT601::BeginDataXfer(uint8_t *buffer, uint32_t length, uint8_t endPointAddr)
{
    std::unique_lock<std::mutex> lock {contextsLock};

    int i = 0;
    bool contextFound = false;
    //find not used context
    for (i = 0; i < USB_MAX_CONTEXTS; i++)
    {
        if (!contexts[i].used) 
        {
            contextFound = true;
            break;
        }
    }

    if (!contextFound) 
    {
        printf("No contexts left for reading or sending data, address %i\n", endPointAddr);
        return -1;
    }

    contexts[i].used = true;

    lock.unlock();
#ifndef __unix__
	ULONG ulActual;
    FT_STATUS ftStatus = FT_OK;
    FT_InitializeOverlapped(mFTHandle, &contexts[i].inOvLap);

    if (endPointAddr == streamBulkReadAddr)
    {
        ftStatus = FT_ReadPipe(mFTHandle, streamBulkReadAddr, buffer, length, &ulActual, &contexts[i].inOvLap);
        
    }
    else
    {
        ftStatus = FT_WritePipe(mFTHandle, streamBulkWriteAddr, buffer, length, &ulActual, &contexts[i].inOvLap);
    }

    contexts[i].endPointAddr = endPointAddr;

    if (ftStatus != FT_IO_PENDING)
    {
        lime::error("ERROR BEGIN DATA TRANSFER %d", ftStatus);
        contexts[i].used = false;
        return -1;
    }
#else
    libusb_transfer *tr = contexts[i].transfer;
    libusb_fill_bulk_transfer(tr, dev_handle, endPointAddr, buffer, length, process_libusbtransfer, &contexts[i], 0);
    contexts[i].done = false;
    contexts[i].bytesXfered = 0;
    int status = libusb_submit_transfer(tr);
    if (status != 0)
    {
        printf("BEGIN DATA READING %s\n", libusb_error_name(status));
        contexts[i].used = false;
        return -1;
    }
#endif
    return i;
}

bool FT601::WaitForXfer(int contextHandle, uint32_t timeout_ms)
{    
    if (contextHandle >= 0 && contexts[contextHandle].used == true)
    {
#ifndef __unix__
        DWORD dwRet = WaitForSingleObject(contexts[contextHandle].inOvLap.hEvent, timeout_ms);

        if (dwRet == WAIT_OBJECT_0)
        {
            return true;
        }
#else
        //blocking not to waste CPU
        std::unique_lock<std::mutex> lck(contexts[contextHandle].transferLock);
        return contexts[contextHandle].cv.wait_for(lck, std::chrono::milliseconds(timeout_ms), [&]() {
            return contexts[contextHandle].done.load();
        });
#endif
    }

    return true; //there is nothing to wait for (signal wait finished)
}

int FT601::FinishDataXfer(uint8_t *buffer, uint32_t length, int contextHandle)
{
    if (contextHandle >= 0 && contexts[contextHandle].used == true) 
    {
#ifndef __unix__
        ULONG ulActualBytesTransferred;
        FT_STATUS ftStatus = FT_OK;

        ftStatus = FT_GetOverlappedResult(mFTHandle, &contexts[contextHandle].inOvLap, &ulActualBytesTransferred, FALSE);

        if (ftStatus != FT_OK)
        {
            length = 0;
        }
        else
        {
            length = ulActualBytesTransferred;
        }

        FT_ReleaseOverlapped(mFTHandle, &contexts[contextHandle].inOvLap);
        contexts[contextHandle].used = false;
        return length;
#else
        length = contexts[contextHandle].bytesXfered;
        contexts[contextHandle].used = false;
        contexts[contextHandle].reset();
        return length;
#endif
    }
    
    return 0;
}

void FT601::AbortEndpointXfers(uint8_t endPointAddr)
{
#ifndef __unix__
    FT_AbortPipe(mFTHandle, endPointAddr);

    for (int i = 0; i < USB_MAX_CONTEXTS; ++i)
    {
        if (contexts[i].used == true && contexts[i].endPointAddr == endPointAddr)
        {
            FT_ReleaseOverlapped(mFTHandle, &contexts[i].inOvLap);
            contexts[i].used = false;
        }
    }

    if (endPointAddr == streamBulkReadAddr)
    {
        FT_FlushPipe(mFTHandle, streamBulkReadAddr);
    }

    FT_SetStreamPipe(mFTHandle, FALSE, FALSE, endPointAddr, sizeof(FPGA_DataPacket));
#else
    for (int i = 0; i < USB_MAX_CONTEXTS; ++i)
    {
        if (contexts[i].used && contexts[i].transfer->endpoint == endPointAddr)
        {
            libusb_cancel_transfer(contexts[i].transfer);
        }
    }
#endif
    for (int i = 0; i < USB_MAX_CONTEXTS; ++i)
    {
        if (contexts[i].used)
        {
            WaitForXfer(i, 250);
            FinishDataXfer(nullptr, 0, i);
        }
    }
}

#ifndef __unix__
int FT601::ReinitPipe(unsigned char ep)
{
    FT_AbortPipe(mFTHandle, ep);
    FT_FlushPipe(mFTHandle, ep);
    FT_SetStreamPipe(mFTHandle, FALSE, FALSE, ep, 64);
    return 0;
}
#endif

#ifdef __unix__
int FT601::FT_FlushPipe(unsigned char ep)
{
    int actual = 0;
    unsigned char wbuffer[20] = {0};

    mUsbCounter++;
    wbuffer[0] = (mUsbCounter) & 0xFF;
    wbuffer[1] = (mUsbCounter >> 8) & 0xFF;
    wbuffer[2] = (mUsbCounter >> 16) & 0xFF;
    wbuffer[3] = (mUsbCounter >> 24) & 0xFF;
    wbuffer[4] = ep;

    libusb_bulk_transfer(dev_handle, 0x01, wbuffer, 20, &actual, 1000);
    if (actual != 20)
    {
        return -1;
    }

    mUsbCounter++;
    wbuffer[0] = (mUsbCounter) & 0xFF;
    wbuffer[1] = (mUsbCounter >> 8) & 0xFF;
    wbuffer[2] = (mUsbCounter >> 16) & 0xFF;
    wbuffer[3] = (mUsbCounter >> 24) & 0xFF;
    wbuffer[4] = ep;
    wbuffer[5] = 0x03;

    libusb_bulk_transfer(dev_handle, 0x01, wbuffer, 20, &actual, 1000);
    if (actual != 20)
    {
        return -1;
    }

    return 0;
}

int FT601::FT_SetStreamPipe(unsigned char ep, size_t size)
{
    int actual = 0;
    unsigned char wbuffer[20] = {0};

    mUsbCounter++;
    wbuffer[0] = (mUsbCounter) & 0xFF;
    wbuffer[1] = (mUsbCounter >> 8) & 0xFF;
    wbuffer[2] = (mUsbCounter >> 16) & 0xFF;
    wbuffer[3] = (mUsbCounter >> 24) & 0xFF;
    wbuffer[4] = ep;

    libusb_bulk_transfer(dev_handle, 0x01, wbuffer, 20, &actual, 1000);
    if (actual != 20)
    {
        return -1;
    }

    mUsbCounter++;
    wbuffer[0] = (mUsbCounter) & 0xFF;
    wbuffer[1] = (mUsbCounter >> 8) & 0xFF;
    wbuffer[2] = (mUsbCounter >> 16) & 0xFF;
    wbuffer[3] = (mUsbCounter >> 24) & 0xFF;
    wbuffer[5] = 0x02;
    wbuffer[8] = (size) & 0xFF;
    wbuffer[9] = (size >> 8) & 0xFF;
    wbuffer[10] = (size >> 16) & 0xFF;
    wbuffer[11] = (size >> 24) & 0xFF;

    libusb_bulk_transfer(dev_handle, 0x01, wbuffer, 20, &actual, 1000);

    if (actual != 20)
    {
        return -1;
    }

    return 0;
}
#endif
