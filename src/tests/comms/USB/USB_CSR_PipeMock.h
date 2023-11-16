#pragma once

#include "tests/protocols/ISerialPortMock.h"
#include "USB_CSR_Pipe.h"

namespace lime::testing {

class USB_CSR_PipeMock
    : public ISerialPortMock
    , public USB_CSR_Pipe
{
  public:
    virtual int Write(const uint8_t* data, size_t length, int timeout_ms)
    {
        return ISerialPortMock::Write(data, length, timeout_ms);
    }

    virtual int Read(uint8_t* data, size_t length, int timeout_ms) { return ISerialPortMock::Read(data, length, timeout_ms); }
};

} // namespace lime::testing
