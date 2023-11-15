#pragma once

#include "ISerialPortAsserter.h"
#include "USBCommon.h"

namespace lime::testing {

class USB_CSR_PipeAsserter
    : public USB_CSR_Pipe
    , public ISerialPortAsserter
{
  public:
    USB_CSR_PipeAsserter(std::vector<LMS64CPacket> packetsToReturnOnRead = std::vector<LMS64CPacket>(),
        std::vector<int> returnValuesWrite = { sizeof(LMS64CPacket) },
        std::vector<int> returnValuesRead = { sizeof(LMS64CPacket) })
        : ISerialPortAsserter(packetsToReturnOnRead, returnValuesWrite, returnValuesRead){}
    ~USB_CSR_PipeAsserter(){}

  private:
    virtual int Write(const uint8_t* data, size_t length, int timeout_ms)
    {
        return ISerialPortAsserter::Write(data, length, timeout_ms);
    }
    virtual int Read(uint8_t* data, size_t length, int timeout_ms) { return ISerialPortAsserter::Read(data, length, timeout_ms); }
};

} // namespace lime::testing