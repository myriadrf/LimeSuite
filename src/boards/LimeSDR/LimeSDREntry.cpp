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
    #ifdef __GNUC__
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpedantic"
    #endif
    #include <libusb.h>
    #ifdef __GNUC__
        #pragma GCC diagnostic pop
    #endif
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

void __loadLimeSDR(void) //TODO fixme replace with LoadLibrary/dlopen
{
    static LimeSDREntry limesdrSupport; // self register on initialization
}

// Device identifier vendor ID and product ID pairs.
static const std::set<VidPid> ids{ { 1204, 241 }, { 1204, 243 }, { 7504, 24840 } };

LimeSDREntry::LimeSDREntry()
    : USBEntry("LimeSDR", ids)
{
}

#ifndef __unix__
std::vector<DeviceHandle> LimeSDREntry::enumerate(const DeviceHandle& hint)
{
    std::vector<DeviceHandle> handles;

    if (!hint.media.empty() && hint.media.find("USB") == std::string::npos)
    {
        return handles;
    }

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

    return handles;
}
#endif

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

    FX3* usbComms = new FX3(
#ifdef __unix__
        ctx
#endif
    );
    if (!usbComms->Connect(vid, pid, handle.serial))
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
