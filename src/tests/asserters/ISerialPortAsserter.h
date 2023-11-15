#pragma once

#include "protocols/ISerialPort.h"
#include "LMS64CProtocol.h"

#include <vector>

namespace lime::testing {

enum class ReturnValue { Minimum, MinusOne, Zero, One, OneLess, Expected, OneMore, Maximum };

class ISerialPortAsserter : public lime::ISerialPort
{
  public:
    ISerialPortAsserter(std::vector<lime::LMS64CPacket> packetsToReturnOnRead,
        ReturnValue returnValue = ReturnValue::Expected,
        int expectedReturnValue = sizeof(LMS64CPacket));
    virtual int Write(const uint8_t* data, size_t length, int timeout_ms) override;
    virtual int Read(uint8_t* data, size_t length, int timeout_ms) override;

    void AssertCommand(LMS64CProtocol::eCMD_LMS command, std::size_t index);
    void AssertStatus(LMS64CProtocol::eCMD_STATUS status, std::size_t index);
    void AssertBlockCount(uint8_t count, std::size_t index);
    void AssertPeriphID(uint8_t id, std::size_t index);
    void AssertSubDevice(uint8_t subdevice, std::size_t index);

    void AssertCommandAll(LMS64CProtocol::eCMD_LMS command);
    void AssertStatusAll(LMS64CProtocol::eCMD_STATUS status);
    void AssertBlockCountAll(uint8_t count);
    void AssertPeriphIDAll(uint8_t id);
    void AssertSubDeviceAll(uint8_t subdevice);

    void AssertWriteCalled(uint times);
    void AssertReadCalled(uint times);

  private:
    int GetReturnValue();

    std::size_t mWriteCallCount;
    std::size_t mReadCallCount;
    std::vector<lime::LMS64CPacket> mWrittenPackets;
    std::vector<lime::LMS64CPacket> mPacketsToReturnOnRead;
    ReturnValue mReturnValue;
    int mExpectedReturnValue;
};

} // namespace lime::testing
