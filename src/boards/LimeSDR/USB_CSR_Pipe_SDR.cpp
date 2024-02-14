#include "USB_CSR_Pipe_SDR.h"

#include <cstdint>
#include <set>

#include "FX3/FX3.h"
#include "LMS64CProtocol.h"

using namespace lime;

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

USB_CSR_Pipe_SDR::USB_CSR_Pipe_SDR(FX3& port)
    : USB_CSR_Pipe()
    , port(port){};

int USB_CSR_Pipe_SDR::Write(const uint8_t* data, size_t length, int timeout_ms)
{
    const LMS64CPacket* pkt = reinterpret_cast<const LMS64CPacket*>(data);

    if (commandsToBulkTransfer.find(pkt->cmd) != commandsToBulkTransfer.end())
    {
        return port.BulkTransfer(FX3::CONTROL_BULK_OUT_ADDRESS, const_cast<uint8_t*>(data), length, timeout_ms);
    }

    return port.ControlTransfer(
#ifdef __unix__
        LIBUSB_REQUEST_TYPE_VENDOR
#else
        0
#endif // __unix__
        ,
        FX3::CTR_W_REQCODE,
        FX3::CTR_W_VALUE,
        FX3::CTR_W_INDEX,
        const_cast<uint8_t*>(data),
        length,
        timeout_ms);
}

int USB_CSR_Pipe_SDR::Read(uint8_t* data, size_t length, int timeout_ms)
{
    const LMS64CPacket* pkt = reinterpret_cast<const LMS64CPacket*>(data);

    if (commandsToBulkTransfer.find(pkt->cmd) != commandsToBulkTransfer.end())
    {
        return port.BulkTransfer(FX3::CONTROL_BULK_IN_ADDRESS, data, length, timeout_ms);
    }

    return port.ControlTransfer(
#ifdef __unix__
        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN
#else
        1
#endif // __unix__
        ,
        FX3::CTR_R_REQCODE,
        FX3::CTR_R_VALUE,
        FX3::CTR_R_INDEX,
        data,
        length,
        timeout_ms);
}
