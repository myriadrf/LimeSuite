#include "USB_CSR_Pipe_Mini.h"

using namespace lime;

static const int CONTROL_BULK_WRITE_ADDRESS = 0x02;
static const int CONTROL_BULK_READ_ADDRESS = 0x82;

USB_CSR_Pipe_Mini::USB_CSR_Pipe_Mini(FT601& port)
    : USB_CSR_Pipe()
    , port(port){}

int USB_CSR_Pipe_Mini::Write(const uint8_t* data, size_t length, int timeout_ms)
{
    return port.BulkTransfer(CONTROL_BULK_WRITE_ADDRESS, const_cast<uint8_t*>(data), length, timeout_ms);
}

int USB_CSR_Pipe_Mini::Read(uint8_t* data, size_t length, int timeout_ms)
{
    return port.BulkTransfer(CONTROL_BULK_READ_ADDRESS, data, length, timeout_ms);
}
