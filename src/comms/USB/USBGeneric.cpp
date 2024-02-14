#include "USBGeneric.h"
#include "Logger.h"

#include <cassert>

using namespace std::literals::string_literals;

namespace lime {

#ifdef __unix__
int USBGeneric::activeUSBconnections = 0;
std::thread USBGeneric::gUSBProcessingThread{};

void USBGeneric::HandleLibusbEvents()
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

USBGeneric::USBGeneric(void* usbContext)
    : contexts(nullptr)
    , isConnected(false)
{
#ifdef __unix__
    dev_handle = nullptr;
    ctx = reinterpret_cast<libusb_context*>(usbContext);

    if (ctx == nullptr)
    {
        return;
    }

    if (activeUSBconnections == 0)
    {
        ++activeUSBconnections;
        gUSBProcessingThread = std::thread(&USBGeneric::HandleLibusbEvents, this);
    }
    else
    {
        ++activeUSBconnections;
    }
#endif
}

USBGeneric::~USBGeneric()
{
    Disconnect();
#ifdef __unix__
    --activeUSBconnections;

    if (activeUSBconnections == 0)
    {
        if (gUSBProcessingThread.joinable())
        {
            gUSBProcessingThread.join();
        }
    }
#endif
}

bool USBGeneric::Connect(uint16_t vid, uint16_t pid, const std::string& serial)
{
#ifdef __unix__
    libusb_device** devs; // Pointer to pointer of device, used to retrieve a list of devices
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
            int stringLength = libusb_get_string_descriptor_ascii(
                dev_handle, desc.iSerialNumber, reinterpret_cast<unsigned char*>(data), sizeof(data));

            if (stringLength < 0)
            {
                lime::error("failed to get serial number");
            }
            else
            {
                foundSerial = std::string(data, static_cast<size_t>(stringLength));
            }
        }

        if (serial == foundSerial)
        {
            break; //found it
        }

        libusb_close(dev_handle);
        dev_handle = nullptr;
    }

    libusb_free_device_list(devs, 1);

    if (dev_handle == nullptr)
    {
        return ReportError(-1, "libusb_open failed");
    }

    if (libusb_kernel_driver_active(dev_handle, 0) == 1) // Find out if kernel driver is attached
    {
        lime::info("Kernel Driver Active");

        if (libusb_detach_kernel_driver(dev_handle, 0) == 0) // Detach it
        {
            lime::info("Kernel Driver Detached!");
        }
    }

    int returnCode = libusb_claim_interface(dev_handle, 0); // Claim interface 0 (the first) of device
    if (returnCode != LIBUSB_SUCCESS)
    {
        return ReportError(returnCode, "Cannot claim interface - %s", libusb_strerror(libusb_error(returnCode)));
    }
#endif
    isConnected = true;
    return true;
}

inline bool USBGeneric::IsConnected()
{
    return isConnected;
}

void USBGeneric::Disconnect()
{
    if (contexts == nullptr)
    {
        return;
    }

#ifdef __unix__
    const libusb_version* ver = libusb_get_version();
    // Fix #358 libusb crash when freeing transfers(never used ones) without valid device handle. Bug in libusb 1.0.25 https://github.com/libusb/libusb/issues/1059
    const bool isBuggy_libusb_free_transfer = ver->major == 1 && ver->minor == 0 && ver->micro == 25;

    if (isBuggy_libusb_free_transfer)
    {
        for (int i = 0; i < USB_MAX_CONTEXTS; ++i)
        {
            contexts[i].transfer->dev_handle = dev_handle;
        }
    }

    std::unique_lock<std::mutex> lock{ contextsLock };

    for (int i = 0; i < USB_MAX_CONTEXTS; ++i)
    {
        if (contexts[i].used)
        {
            AbortEndpointXfers(contexts[i].transfer->endpoint);
        }
    }
#endif

    delete[] contexts;
    contexts = nullptr;
}

int32_t USBGeneric::BulkTransfer(uint8_t endPointAddr, uint8_t* data, int length, int32_t timeout_ms)
{
    long len = 0;
    if (!IsConnected())
    {
        throw std::runtime_error("BulkTransfer: USB device is not connected");
    }

    assert(data);

#ifdef __unix__
    int actualTransferred = 0;
    int status = libusb_bulk_transfer(dev_handle, endPointAddr, data, length, &actualTransferred, timeout_ms);
    len = actualTransferred;

    if (status != 0)
    {
        lime::error("USBGeneric::BulkTransfer(0x%02X) : %s, transferred: %i, expected: %i",
            endPointAddr,
            libusb_error_name(status),
            actualTransferred,
            length);
    }
#endif
    return len;
}

int32_t USBGeneric::ControlTransfer(
    int requestType, int request, int value, int index, uint8_t* data, uint32_t length, int32_t timeout_ms)
{
    long len = length;
    if (!IsConnected())
    {
        throw std::runtime_error("ControlTransfer: USB device is not connected");
    }

    assert(data);
#ifdef __unix__
    len = libusb_control_transfer(dev_handle, requestType, request, value, index, data, length, timeout_ms);
#endif
    return len;
}

#ifdef __unix__
/** @brief Function for handling libusb callbacks
*/
static void process_libusbtransfer(libusb_transfer* trans)
{
    USBTransferContext* context = static_cast<USBTransferContext*>(trans->user_data);
    std::unique_lock<std::mutex> lck(context->transferLock);
    switch (trans->status)
    {
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

int USBGeneric::BeginDataXfer(uint8_t* buffer, uint32_t length, uint8_t endPointAddr)
{
#ifdef __unix__
    int i = GetUSBContextIndex();

    if (i < 0)
    {
        return -1;
    }

    libusb_transfer* tr = contexts[i].transfer;
    libusb_fill_bulk_transfer(tr, dev_handle, endPointAddr, buffer, length, process_libusbtransfer, &contexts[i], 0);
    contexts[i].done = false;
    contexts[i].bytesXfered = 0;
    int status = libusb_submit_transfer(tr);

    if (status != 0)
    {
        lime::error("BEGIN DATA TRANSFER %s", libusb_error_name(status));
        contexts[i].used = false;
        return -1;
    }

    return i;
#endif
    return 0;
}

bool USBGeneric::WaitForXfer(int contextHandle, uint32_t timeout_ms)
{
#ifdef __unix__
    if (contextHandle >= 0 && contexts[contextHandle].used == true)
    {
        // Blocking not to waste CPU
        std::unique_lock<std::mutex> lck(contexts[contextHandle].transferLock);
        return contexts[contextHandle].cv.wait_for(
            lck, std::chrono::milliseconds(timeout_ms), [&]() { return contexts[contextHandle].done.load(); });
    }
#endif
    return true; // There is nothing to wait for (signal wait finished)
}

int USBGeneric::FinishDataXfer(uint8_t* buffer, uint32_t length, int contextHandle)
{
#ifdef __unix__
    if (contextHandle >= 0 && contexts[contextHandle].used == true)
    {
        length = contexts[contextHandle].bytesXfered;
        contexts[contextHandle].used = false;
        contexts[contextHandle].Reset();
        return length;
    }
#endif

    return 0;
}

void USBGeneric::AbortEndpointXfers(uint8_t endPointAddr)
{
    if (contexts == nullptr)
    {
        return;
    }

#ifdef __unix__
    for (int i = 0; i < USB_MAX_CONTEXTS; ++i)
    {
        if (contexts[i].used && contexts[i].transfer->endpoint == endPointAddr)
        {
            libusb_cancel_transfer(contexts[i].transfer);
        }
    }
#endif
    WaitForXfers(endPointAddr);
}

int USBGeneric::GetUSBContextIndex()
{
    std::unique_lock<std::mutex> lock{ contextsLock };

    if (contexts == nullptr)
    {
        return -1;
    }

    int i = 0;
    bool contextFound = false;
    // Find not used context
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
        lime::error("No contexts left for reading or sending data"s);
        return -1;
    }

    contexts[i].used = true;

    return i;
}

void USBGeneric::WaitForXfers(uint8_t endPointAddr)
{
    for (int i = 0; i < USB_MAX_CONTEXTS; ++i)
    {
#ifdef __unix__
        if (contexts[i].used && contexts[i].transfer->endpoint == endPointAddr)
#else
        if (contexts[i].used)
#endif
        {
            WaitForXfer(i, 250);
            FinishDataXfer(nullptr, 0, i);
        }
    }
}

} // namespace lime
