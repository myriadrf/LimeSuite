#include "USBEntry.h"
#include "Logger.h"

using namespace lime;

#ifdef __unix__
libusb_context* USBEntry::ctx{ nullptr };
uint32_t USBEntry::ctxRefCount{ 0 };
#endif

USBEntry::USBEntry(const std::string& name, const std::set<VidPid>& deviceIds)
    : DeviceRegistryEntry(name)
    , mDeviceIds(deviceIds)
{
#ifdef __unix__
    ++ctxRefCount;

    if (ctx != nullptr)
    {
        return;
    }

    int returnCode = libusb_init(&ctx); // Initialize the library for the session we just declared
    if (returnCode < 0)
    {
        lime::error("Init Error %i", returnCode); // There was an error
    }

    #if LIBUSBX_API_VERSION < 0x01000106
    libusb_set_debug(ctx, 3); // Set verbosity level to 3, as suggested in the documentation
    #else
    libusb_set_option(
        ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO); // Set verbosity level to info, as suggested in the documentation
    #endif
#endif
}

USBEntry::~USBEntry()
{
#ifdef __unix__
    if (--ctxRefCount <= 0)
    {
        libusb_exit(ctx);
    }
#endif
}

std::vector<DeviceHandle> USBEntry::enumerate(const DeviceHandle& hint)
{
    std::vector<DeviceHandle> handles;

    if (!hint.media.empty() && hint.media.find("USB") == std::string::npos)
        return handles;

#ifdef __unix__
    libusb_device** devs; // Pointer to pointer of device, used to retrieve a list of devices
    int usbDeviceCount = libusb_get_device_list(ctx, &devs);

    if (usbDeviceCount < 0)
    {
        lime::error("Failed to get libusb device list: %s", libusb_strerror(libusb_error(usbDeviceCount)));
        return handles;
    }

    for (int i = 0; i < usbDeviceCount; ++i)
    {
        libusb_device_descriptor desc;
        int returnCode = libusb_get_device_descriptor(devs[i], &desc);
        if (returnCode < 0)
        {
            lime::error("Failed to get device description");
        }

        VidPid id{ desc.idVendor, desc.idProduct };

        if (mDeviceIds.find(id) == mDeviceIds.end())
        {
            continue;
        }

        libusb_device_handle* tempDev_handle(nullptr);
        if (libusb_open(devs[i], &tempDev_handle) != 0 || tempDev_handle == nullptr)
        {
            continue;
        }

        DeviceHandle handle = GetDeviceHandle(tempDev_handle, devs[i], desc);

        libusb_close(tempDev_handle);

        // Add handle conditionally, filter by serial number
        if (handle.IsEqualIgnoringEmpty(hint))
        {
            handles.push_back(handle);
        }
    }

    libusb_free_device_list(devs, 1);
#else
        // TODO: implement for windows
#endif
    return handles;
}

#ifdef __unix__
std::string USBEntry::GetUSBDeviceSpeedString(libusb_device* device)
{
    int speed = libusb_get_device_speed(device);

    switch (speed)
    {
    case LIBUSB_SPEED_HIGH:
        return "USB 2.0";
    case LIBUSB_SPEED_SUPER:
        return "USB 3.0";
    default:
        return "USB";
    }
}

DeviceHandle USBEntry::GetDeviceHandle(
    libusb_device_handle* tempHandle, libusb_device* device, const libusb_device_descriptor& desc)
{
    DeviceHandle handle;

    // Check operating speed
    handle.media = GetUSBDeviceSpeedString(device);

    // Read device name
    char characterBuffer[255];
    int descriptorStringLength = libusb_get_string_descriptor_ascii(
        tempHandle, LIBUSB_CLASS_COMM, reinterpret_cast<unsigned char*>(characterBuffer), sizeof(characterBuffer));
    if (descriptorStringLength > 0)
    {
        handle.name = std::string(characterBuffer, size_t(descriptorStringLength));
    }

    int addressLength = std::sprintf(characterBuffer, "%.4x:%.4x", desc.idVendor, desc.idProduct);
    if (addressLength > 0)
    {
        handle.addr = std::string(characterBuffer, size_t(addressLength));
    }

    if (desc.iSerialNumber > 0)
    {
        int serialStringLength = libusb_get_string_descriptor_ascii(
            tempHandle, desc.iSerialNumber, reinterpret_cast<unsigned char*>(characterBuffer), sizeof(characterBuffer));

        if (serialStringLength < 0)
        {
            lime::error("Failed to get serial number");
        }
        else
        {
            handle.serial = std::string(characterBuffer, size_t(serialStringLength));
        }
    }

    return handle;
}
#endif
