#include "USBCommon.h"
#include "Logger.h"

#ifdef __unix__
#include <libusb.h>
#endif

namespace lime
{

USBTransferContext::USBTransferContext()
{
#ifdef __unix__
    transfer = libusb_alloc_transfer(0);
    bytesXfered = 0;
    done = 0;
#endif
}

USBTransferContext::~USBTransferContext()
{
#ifdef __unix__
    if (transfer)
    {
        libusb_free_transfer(transfer);
    }
#endif
}

bool USBTransferContext::Reset()
{
    if (used)
    {
        return false;
    }

    return true;
}

#ifdef __unix__
libusb_context* USBEntry::ctx {nullptr}; 
#endif

USBEntry::USBEntry(const std::string &name, std::set<VidPid> deviceIds) : DeviceRegistryEntry(name), mDeviceIds(deviceIds)
{
#ifdef __unix__
    if (ctx == nullptr) 
    {
        int returnCode = libusb_init(&ctx); // Initialize the library for the session we just declared
        if (returnCode < 0)
        {
            lime::error("Init Error %i", returnCode); // There was an error
        }
#if LIBUSBX_API_VERSION < 0x01000106
        libusb_set_debug(ctx, 3); // Set verbosity level to 3, as suggested in the documentation
#else
        libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO); // Set verbosity level to info, as suggested in the documentation
#endif
    }

#endif
}

USBEntry::~USBEntry()
{
#ifdef __unix__
    if (ctx != nullptr)
    {
        libusb_exit(ctx);
        ctx = nullptr;
    }
#endif
}

std::vector<DeviceHandle> USBEntry::enumerate(const DeviceHandle &hint)
{
#ifdef __unix__
    std::vector<DeviceHandle> handles;

    if (!hint.media.empty() && hint.media.find("USB") == std::string::npos)
    {
        return handles;
    }

    libusb_device **devs; // Pointer to pointer of device, used to retrieve a list of devices
    int usbDeviceCount = libusb_get_device_list(ctx, &devs);

    if (usbDeviceCount < 0) {
        lime::error("failed to get libusb device list: %s", libusb_strerror(libusb_error(usbDeviceCount)));
        return handles;
    }

    for (int i = 0; i < usbDeviceCount; ++i)
    {
        libusb_device_descriptor desc;
        int returnCode = libusb_get_device_descriptor(devs[i], &desc);
        if (returnCode < 0)
        {
            lime::error("failed to get device description");
        }

        VidPid id {desc.idVendor, desc.idProduct};

        if (mDeviceIds.find(id) != mDeviceIds.end())
        {
            libusb_device_handle *tempDev_handle(nullptr);
            if (libusb_open(devs[i], &tempDev_handle) != 0 || tempDev_handle == nullptr)
            {
                continue;
            }

            DeviceHandle handle;

            //check operating speed
            int speed = libusb_get_device_speed(devs[i]);

            switch (speed)
            {
            case LIBUSB_SPEED_SUPER:
                handle.media = "USB 3.0";
                break;
            case LIBUSB_SPEED_HIGH:
                handle.media = "USB 2.0";
                break;
            default:
                handle.media = "USB";
                break;
            }

            //read device name
            char characterBuffer[255];
            int descriptorStringLength = libusb_get_string_descriptor_ascii(tempDev_handle,  LIBUSB_CLASS_COMM, reinterpret_cast<unsigned char*>(characterBuffer), sizeof(characterBuffer));
            if (descriptorStringLength > 0)
            {
                handle.name = std::string(characterBuffer, size_t(descriptorStringLength));
            }

            int addressLength = std::sprintf(characterBuffer, "%.4x:%.4x", id.vid, id.pid);
            if (addressLength > 0) 
            {
                handle.addr = std::string(characterBuffer, size_t(addressLength));
            }

            if (desc.iSerialNumber > 0)
            {
                int serialStringLength = libusb_get_string_descriptor_ascii(tempDev_handle, desc.iSerialNumber, reinterpret_cast<unsigned char*>(characterBuffer), sizeof(characterBuffer));

                if (serialStringLength < 0)
                {
                    lime::error("failed to get serial number");
                }
                else
                {
                    handle.serial = std::string(characterBuffer, size_t(serialStringLength));
                }
            }

            libusb_close(tempDev_handle);

            //add handle conditionally, filter by serial number
            if (hint.serial.empty() or handle.serial.find(hint.serial) != std::string::npos)
            {
                handles.push_back(handle);
            }
        }
    }

    libusb_free_device_list(devs, 1);

    return handles;
#endif
}

LMS64C_LMS7002M_Over_USB::LMS64C_LMS7002M_Over_USB(USB_CSR_Pipe& dataPort) : pipe(dataPort) {}

void LMS64C_LMS7002M_Over_USB::SPI(const uint32_t *MOSI, uint32_t *MISO, uint32_t count)
{
    LMS64CProtocol::LMS7002M_SPI(pipe, 0, MOSI, MISO, count);
}

void LMS64C_LMS7002M_Over_USB::SPI(uint32_t spiBusAddress, const uint32_t *MOSI, uint32_t *MISO, uint32_t count)
{
    LMS64CProtocol::LMS7002M_SPI(pipe, spiBusAddress, MOSI, MISO, count);
}

int LMS64C_LMS7002M_Over_USB::ResetDevice(int chipSelect)
{
    return LMS64CProtocol::DeviceReset(pipe, chipSelect);
};

LMS64C_FPGA_Over_USB::LMS64C_FPGA_Over_USB(USB_CSR_Pipe &dataPort) : pipe(dataPort) {}

void LMS64C_FPGA_Over_USB::SPI(const uint32_t *MOSI, uint32_t *MISO, uint32_t count)
{
    SPI(0, MOSI, MISO, count);
}

void LMS64C_FPGA_Over_USB::SPI(uint32_t spiBusAddress, const uint32_t *MOSI, uint32_t *MISO, uint32_t count)
{
    LMS64CProtocol::FPGA_SPI(pipe, MOSI, MISO, count);
}

int LMS64C_FPGA_Over_USB::GPIODirRead(uint8_t *buffer, const size_t bufLength) 
{
    return LMS64CProtocol::GPIODirRead(pipe, buffer, bufLength);
}

int LMS64C_FPGA_Over_USB::GPIORead(uint8_t *buffer, const size_t bufLength) 
{
    return LMS64CProtocol::GPIORead(pipe, buffer, bufLength);
}

int LMS64C_FPGA_Over_USB::GPIODirWrite(const uint8_t *buffer, const size_t bufLength) 
{
    return LMS64CProtocol::GPIODirWrite(pipe, buffer, bufLength);
}

int LMS64C_FPGA_Over_USB::GPIOWrite(const uint8_t *buffer, const size_t bufLength) 
{
    return LMS64CProtocol::GPIOWrite(pipe, buffer, bufLength);
}

int LMS64C_FPGA_Over_USB::CustomParameterWrite(const int32_t *ids, const double *values, const size_t count, const std::string& units)
{
    return LMS64CProtocol::CustomParameterWrite(pipe, ids, values, count, units);
}

int LMS64C_FPGA_Over_USB::CustomParameterRead(const int32_t *ids, double *values, const size_t count, std::string* units)
{
    return LMS64CProtocol::CustomParameterRead(pipe, ids, values, count, units);
}

int LMS64C_FPGA_Over_USB::ProgramWrite(const char* data, size_t length, int prog_mode, int target, ProgressCallback callback)
{
    return LMS64CProtocol::ProgramWrite(pipe, data, length, prog_mode, (LMS64CProtocol::ProgramWriteTarget)target, callback);
}

}