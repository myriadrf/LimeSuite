#include "LimeSDR_Mini.h"
#include "DeviceExceptions.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/DeviceHandle.h"
#include "protocols/LMS64CProtocol.h"
#include "Logger.h"
#include "USBCommon.h"

#include "FT601/FT601.h"

#ifndef __unix__
#include "windows.h"
#include "FTD3XXLibrary/FTD3XX.h"
#else
#include <libusb.h>
#include <mutex>
#endif

using namespace lime;

static libusb_context* ctx; // A libusb session

void __loadLimeSDR_Mini(void) // TODO: fixme replace with LoadLibrary/dlopen
{
    static LimeSDR_MiniEntry limesdr_miniSupport; // Self register on initialization
}

LimeSDR_MiniEntry::LimeSDR_MiniEntry() : DeviceRegistryEntry("LimeSDR_Mini")
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

LimeSDR_MiniEntry::~LimeSDR_MiniEntry()
{
#ifdef __unix__
    libusb_exit(ctx);
#endif
}

std::vector<DeviceHandle> LimeSDR_MiniEntry::enumerate(const DeviceHandle &hint)
{
    std::vector<DeviceHandle> handles;

    if (!hint.media.empty() && hint.media.find("USB") == std::string::npos)
    {
        return handles;
    }

#ifndef __unix__
    FT_STATUS ftStatus=FT_OK;
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
                handle.name = Description;
                handle.index = i;
                handle.serial = SerialNumber;
                // Add handle conditionally, filter by serial number
                if (hint.serial.empty() || handle.serial.find(hint.serial) != std::string::npos)
                    handles.push_back(handle);
            }
        }
    }
#else
    libusb_device **devs; // Pointer to pointer of device, used to retrieve a list of devices
    int usbDeviceCount = libusb_get_device_list(ctx, &devs);

    if (usbDeviceCount < 0) {
        lime::error("failed to get libusb device list: %s", libusb_strerror(libusb_error(usbDeviceCount)));
        return handles;
    }

    for (int i = 0; i < usbDeviceCount; ++i)
    {
        libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(devs[i], &desc);
        if (r < 0)
        {
            lime::error("failed to get device description");
        }

        int pid = desc.idProduct;
        int vid = desc.idVendor;

        if (vid == 0x0403 && pid == 0x601F)
        {
            libusb_device_handle *tempDev_handle(nullptr);
            if (libusb_open(devs[i], &tempDev_handle) != 0 || tempDev_handle == nullptr)
            {
                continue;
            }

            DeviceHandle handle;

            //check operating speed
            int speed = libusb_get_device_speed(devs[i]);
            if (speed == LIBUSB_SPEED_HIGH)
            {
                handle.media = "USB 2.0";
            }
            else if (speed == LIBUSB_SPEED_SUPER)
            {
                handle.media = "USB 3.0";
            }
            else
            {
                handle.media = "USB";
            }

            //read device name
            char data[255];
            r = libusb_get_string_descriptor_ascii(tempDev_handle,  LIBUSB_CLASS_COMM, (unsigned char*)data, sizeof(data));
            if (r > 0)
            {
                handle.name = std::string(data, size_t(r));
            }

            r = std::sprintf(data, "%.4x:%.4x", int(vid), int(pid));
            if (r > 0) 
            {
                handle.addr = std::string(data, size_t(r));
            }

            if (desc.iSerialNumber > 0)
            {
                r = libusb_get_string_descriptor_ascii(tempDev_handle,desc.iSerialNumber,(unsigned char*)data, sizeof(data));
                if (r < 0)
                {
                    lime::error("failed to get serial number");
                }
                else
                {
                    handle.serial = std::string(data, size_t(r));
                }
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

static constexpr int streamBulkWriteAddr = 0x03;
static constexpr int streamBulkReadAddr = 0x83;

static constexpr int ctrlBulkWriteAddr = 0x02;
static constexpr int ctrlBulkReadAddr = 0x82;

class USB_CSR_Pipe_Mini : public USB_CSR_Pipe
{
public:
    explicit USB_CSR_Pipe_Mini(FT601& port) : USB_CSR_Pipe(), port(port) {};

    virtual int Write(const uint8_t* data, size_t length, int timeout_ms) override
    {    
        return port.BulkTransfer(ctrlBulkWriteAddr, const_cast<uint8_t *>(data), length, timeout_ms);
    }

    virtual int Read(uint8_t* data, size_t length, int timeout_ms) override
    {
        return port.BulkTransfer(ctrlBulkReadAddr, data, length, timeout_ms);
    }
protected:
    FT601& port;
};

SDRDevice *LimeSDR_MiniEntry::make(const DeviceHandle &handle)
{
    const auto splitPos = handle.addr.find(":");
    const uint16_t vid = std::stoi(handle.addr.substr(0, splitPos), nullptr, 16);
    const uint16_t pid = std::stoi(handle.addr.substr(splitPos+1), nullptr, 16);

    FT601* usbComms = new FT601(ctx);
    if (usbComms->Connect(vid, pid, handle.serial) != 0)
    {
        delete usbComms;
        char reason[256];
        sprintf(reason, "Unable to connect to device using handle(%s)", handle.Serialize().c_str());
        throw std::runtime_error(reason);
    }

    USB_CSR_Pipe* usbPipe = new USB_CSR_Pipe_Mini(*usbComms);

    // protocol layer
    IComms* route_lms7002m = new LMS64C_LMS7002M_Over_USB(*usbPipe);
    IComms* route_fpga = new LMS64C_FPGA_Over_USB(*usbPipe);

    return new LimeSDR_Mini(route_lms7002m, route_fpga, usbComms, usbPipe);
}
