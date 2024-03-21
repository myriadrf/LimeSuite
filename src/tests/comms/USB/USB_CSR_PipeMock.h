#ifndef LIME_USB_CSR_PIPEMOCK_H
#define LIME_USB_CSR_PIPEMOCK_H

#include "tests/protocols/SerialPortMock.h"
#include "comms/USB/USB_CSR_Pipe.h"

namespace lime::testing {

class USB_CSR_PipeMock
    : public SerialPortMock
    , public USB_CSR_Pipe
{
  public:
    virtual int Write(const uint8_t* data, size_t length, int timeout_ms)
    {
        return SerialPortMock::Write(data, length, timeout_ms);
    }

    virtual int Read(uint8_t* data, size_t length, int timeout_ms) { return SerialPortMock::Read(data, length, timeout_ms); }
};

} // namespace lime::testing

#endif // LIME_USB_CSR_PIPEMOCK_H
