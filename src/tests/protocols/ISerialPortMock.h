#pragma once

#include "protocols/ISerialPort.h"
#include "LMS64CProtocol.h"

namespace lime::testing {

class ISerialPortMock : public ISerialPort
{
  public:
    ISerialPortMock()
    {
        ON_CALL(*this, Write).WillByDefault([this](const uint8_t* data, size_t length, int timeout_ms) { return length; });
        ON_CALL(*this, Read).WillByDefault([this](uint8_t* data, size_t length, int timeout_ms) { return length; });
    }

    MOCK_METHOD(int, Write, (const uint8_t* data, size_t length, int timeout_ms), (override));
    MOCK_METHOD(int, Read, (uint8_t * data, size_t length, int timeout_ms), (override));
};

} // namespace lime::testing
