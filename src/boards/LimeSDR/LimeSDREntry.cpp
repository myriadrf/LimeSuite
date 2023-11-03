#include "LimeSDR.h"
#include "DeviceExceptions.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/DeviceHandle.h"
#include "protocols/LMS64CProtocol.h"
#include "Logger.h"
#include "USBCommon.h"

#include "FX3/FX3.h"

#ifndef __unix__
    #include "windows.h"
    #include "CyAPI.h"
#else
    #include <libusb.h>
    #include <mutex>
#endif

#define CTR_W_REQCODE 0xC1
#define CTR_W_VALUE 0x0000
#define CTR_W_INDEX 0x0000

#define CTR_R_REQCODE 0xC0
#define CTR_R_VALUE 0x0000
#define CTR_R_INDEX 0x0000

using namespace lime;

static constexpr uint8_t ctrlBulkOutAddr = 0x0F;
static constexpr uint8_t ctrlBulkInAddr = 0x8F;

static libusb_context* ctx; //a libusb session

void __loadLimeSDR(void) //TODO fixme replace with LoadLibrary/dlopen
{
    static LimeSDREntry limesdrSupport; // self register on initialization
}

LimeSDREntry::LimeSDREntry()
    : DeviceRegistryEntry("LimeSDR")
{
#ifdef __unix__
    if (ctx == nullptr)
    {
        int returnCode = libusb_init(&ctx); //initialize the library for the session we just declared
        if (returnCode < 0)
        {
            lime::error("Init Error %i", returnCode); //there was an error
        }
    #if LIBUSBX_API_VERSION < 0x01000106
        libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation
    #else
        libusb_set_option(
            ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO); //set verbosity level to info, as suggested in the documentation
    #endif
    }

#endif
}

LimeSDREntry::~LimeSDREntry()
{
#ifdef __unix__
    libusb_exit(ctx);
#endif
}

std::vector<DeviceHandle> LimeSDREntry::enumerate(const DeviceHandle& hint)
{
    std::vector<DeviceHandle> handles;

    if (!hint.media.empty() && hint.media.find("USB") == std::string::npos)
    {
        return handles;
    }

#ifndef __unix__
    CCyUSBDevice device;
    if (device.DeviceCount())
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
#else
    libusb_device** devs; //pointer to pointer of device, used to retrieve a list of devices
    int usbDeviceCount = libusb_get_device_list(ctx, &devs);

    if (usbDeviceCount < 0)
    {
        lime::error("failed to get libusb device list: %s", libusb_strerror(libusb_error(usbDeviceCount)));
        return handles;
    }

    for (int i = 0; i < usbDeviceCount; ++i)
    {
        libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(devs[i], &desc);
        if (r < 0)
            lime::error("failed to get device description");
        int pid = desc.idProduct;
        int vid = desc.idVendor;

        if ((vid == 1204 && pid == 241) || (vid == 1204 && pid == 243) || (vid == 7504 && pid == 24840))
        {
            libusb_device_handle* tempDev_handle(nullptr);
            if (libusb_open(devs[i], &tempDev_handle) != 0 || tempDev_handle == nullptr)
                continue;

            DeviceHandle handle;

            //check operating speed
            int speed = libusb_get_device_speed(devs[i]);
            if (speed == LIBUSB_SPEED_HIGH)
                handle.media = "USB 2.0";
            else if (speed == LIBUSB_SPEED_SUPER)
                handle.media = "USB 3.0";
            else
                handle.media = "USB";

            //read device name
            char data[255];
            r = libusb_get_string_descriptor_ascii(tempDev_handle, LIBUSB_CLASS_COMM, (unsigned char*)data, sizeof(data));
            if (r > 0)
                handle.name = std::string(data, size_t(r));

            r = std::sprintf(data, "%.4x:%.4x", int(vid), int(pid));
            if (r > 0)
                handle.addr = std::string(data, size_t(r));

            if (desc.iSerialNumber > 0)
            {
                r = libusb_get_string_descriptor_ascii(tempDev_handle, desc.iSerialNumber, (unsigned char*)data, sizeof(data));
                if (r < 0)
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

static const std::set<uint8_t> commandsToBulkTransfer = {
    LMS64CProtocol::CMD_BRDSPI_WR,
    LMS64CProtocol::CMD_BRDSPI_RD,
    LMS64CProtocol::CMD_LMS7002_WR,
    LMS64CProtocol::CMD_LMS7002_RD,
    LMS64CProtocol::CMD_ANALOG_VAL_WR,
    LMS64CProtocol::CMD_ANALOG_VAL_RD,
    LMS64CProtocol::CMD_ADF4002_WR,
    LMS64CProtocol::CMD_LMS7002_RST,
    LMS64CProtocol::CMD_GPIO_DIR_WR,
    LMS64CProtocol::CMD_GPIO_DIR_RD,
    LMS64CProtocol::CMD_GPIO_WR,
    LMS64CProtocol::CMD_GPIO_RD,
};

class USB_CSR_Pipe_SDR : public USB_CSR_Pipe
{
  public:
    explicit USB_CSR_Pipe_SDR(FX3& port)
        : USB_CSR_Pipe()
        , port(port){};

    virtual int Write(const uint8_t* data, size_t length, int timeout_ms) override
    {
        const LMS64CPacket* pkt = reinterpret_cast<const LMS64CPacket*>(data);

        if (commandsToBulkTransfer.find(pkt->cmd) != commandsToBulkTransfer.end())
        {
            return port.BulkTransfer(ctrlBulkOutAddr, const_cast<uint8_t*>(data), length, timeout_ms);
        }

        return port.ControlTransfer(
            LIBUSB_REQUEST_TYPE_VENDOR, CTR_W_REQCODE, CTR_W_VALUE, CTR_W_INDEX, const_cast<uint8_t*>(data), length, 1000);
    }

    virtual int Read(uint8_t* data, size_t length, int timeout_ms) override
    {
        const LMS64CPacket* pkt = reinterpret_cast<const LMS64CPacket*>(data);

        if (commandsToBulkTransfer.find(pkt->cmd) != commandsToBulkTransfer.end())
        {
            return port.BulkTransfer(ctrlBulkInAddr, data, length, timeout_ms);
        }

        return port.ControlTransfer(
            LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, CTR_R_REQCODE, CTR_R_VALUE, CTR_R_INDEX, data, length, 1000);
    }

  protected:
    FX3& port;
};

SDRDevice* LimeSDREntry::make(const DeviceHandle& handle)
{
    const auto splitPos = handle.addr.find(":");
    const uint16_t vid = std::stoi(handle.addr.substr(0, splitPos), nullptr, 16);
    const uint16_t pid = std::stoi(handle.addr.substr(splitPos + 1), nullptr, 16);

    FX3* usbComms = new FX3(ctx);
    if (usbComms->Connect(vid, pid, handle.serial) != 0)
    {
        delete usbComms;
        char reason[256];
        sprintf(reason, "Unable to connect to device using handle(%s)", handle.Serialize().c_str());
        throw std::runtime_error(reason);
    }

    USB_CSR_Pipe* usbPipe = new USB_CSR_Pipe_SDR(*usbComms);

    // protocol layer
    IComms* route_lms7002m = new LMS64C_LMS7002M_Over_USB(*usbPipe);
    IComms* route_fpga = new LMS64C_FPGA_Over_USB(*usbPipe);

    return new LimeSDR(route_lms7002m, route_fpga, usbComms, usbPipe);
}
