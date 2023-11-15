#pragma once

#include "protocols/ISerialPort.h"
#include "LMS64CProtocol.h"

#include <vector>

namespace lime::testing {

enum class eReturnValue { Minimum, MinusOne, Zero, One, OneLess, Expected, OneMore, Maximum };

struct ReturnValue {
    eReturnValue returnValue;
    int expectedReturnValue;
};

class ISerialPortAsserter : public lime::ISerialPort
{
  public:
    ISerialPortAsserter(std::vector<lime::LMS64CPacket> packetsToReturnOnRead = std::vector<LMS64CPacket>(),
        std::vector<ReturnValue> returnValuesWrite = { { eReturnValue::Expected, sizeof(LMS64CPacket) } },
        std::vector<ReturnValue> returnValuesRead = { { eReturnValue::Expected, sizeof(LMS64CPacket) } });
    virtual int Write(const uint8_t* data, size_t length, int timeout_ms) override;
    virtual int Read(uint8_t* data, size_t length, int timeout_ms) override;

    void AssertCommand(const LMS64CProtocol::eCMD_LMS command, const std::size_t index) const;
    void AssertStatus(const LMS64CProtocol::eCMD_STATUS status, const std::size_t index) const;
    void AssertBlockCount(const uint8_t count, const std::size_t index) const;
    void AssertPeriphID(const uint8_t id, const std::size_t index) const;
    void AssertSubDevice(const uint8_t subdevice, const std::size_t index) const;

    void AssertCommandAll(const LMS64CProtocol::eCMD_LMS command) const;
    void AssertStatusAll(const LMS64CProtocol::eCMD_STATUS status) const;
    void AssertBlockCountAll(const uint8_t count) const;
    void AssertPeriphIDAll(const uint8_t id) const;
    void AssertSubDeviceAll(const uint8_t subdevice) const;

    void AssertCommandSequence(const std::vector<LMS64CProtocol::eCMD_LMS>& command) const;
    void AssertStatusSequence(const std::vector<LMS64CProtocol::eCMD_STATUS>& status) const;
    void AssertBlockCountSequence(const std::vector<uint8_t>& count) const;
    void AssertPeriphIDSequence(const std::vector<uint8_t>& id) const;
    void AssertSubDeviceSequence(const std::vector<uint8_t>& subdevice) const;

    void AssertWriteCalled(const uint times) const;
    void AssertReadCalled(const uint times) const;

  private:
    int GetReturnValue(const ReturnValue& returnValue) const;

    std::size_t mWriteCallCount;
    std::size_t mReadCallCount;
    std::vector<lime::LMS64CPacket> mWrittenPackets;
    std::vector<lime::LMS64CPacket> mPacketsToReturnOnRead;
    std::vector<ReturnValue> mReturnValuesWrite;
    std::vector<ReturnValue> mReturnValuesRead;
};

} // namespace lime::testing
