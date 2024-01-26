#ifndef LIME_SERIALPORTMOCK_H
#define LIME_SERIALPORTMOCK_H

#include "protocols/ISerialPort.h"
#include "LMS64CProtocol.h"

namespace lime::testing {

class SerialPortMock : public ISerialPort
{
  public:
    SerialPortMock()
    {
        ON_CALL(*this, Write).WillByDefault([this](const std::byte* data, size_t length, int timeout_ms) { return length; });
        ON_CALL(*this, Read).WillByDefault([this](std::byte* data, size_t length, int timeout_ms) { return length; });
    }

    MOCK_METHOD(int, Write, (const std::byte* data, size_t length, int timeout_ms), (override));
    MOCK_METHOD(int, Read, (std::byte * data, size_t length, int timeout_ms), (override));
};

} // namespace lime::testing

#endif // LIME_SERIALPORTMOCK_H
