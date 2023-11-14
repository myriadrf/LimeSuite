#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ISerialPortAsserter.h"

#include <cstring>
#include <limits>

namespace lime::testing {

ISerialPortAsserter::ISerialPortAsserter(
    std::vector<lime::LMS64CPacket> packetsToReturnOnRead, ReturnValue returnValue, int expectedReturnValue)
    : mWriteCallCount(0)
    , mReadCallCount(0)
    , mWrittenPackets()
    , mPacketsToReturnOnRead(packetsToReturnOnRead)
    , mReturnValue(returnValue)
    , mExpectedReturnValue(expectedReturnValue)
{
    if (mPacketsToReturnOnRead.size() == 0)
    {
        mPacketsToReturnOnRead.push_back(LMS64CPacket());
    }
}

int ISerialPortAsserter::Write(const uint8_t* data, size_t length, int timeout_ms)
{
    ++mWriteCallCount;

    lime::LMS64CPacket packet;
    std::memcpy(&packet, data, length);

    mWrittenPackets.push_back(packet);

    return GetReturnValue();
}

int ISerialPortAsserter::Read(uint8_t* data, size_t length, int timeout_ms)
{
    std::size_t index = std::min(mReadCallCount, mPacketsToReturnOnRead.size() - 1);
    lime::LMS64CPacket& packet = mPacketsToReturnOnRead[index];

    std::memcpy(data, &packet, length);

    ++mReadCallCount;

    return GetReturnValue();
}

int ISerialPortAsserter::GetReturnValue()
{
    switch (mReturnValue)
    {
    case ReturnValue::Minimum:
        return std::numeric_limits<int>::min();
    case ReturnValue::MinusOne:
        return -1;
    case ReturnValue::Zero:
        return 0;
    case ReturnValue::One:
        return 1;
    case ReturnValue::OneLess:
        return mExpectedReturnValue - 1;
    case ReturnValue::Expected:
        return mExpectedReturnValue;
    case ReturnValue::OneMore:
        return mExpectedReturnValue + 1;
    case ReturnValue::Maximum:
        return std::numeric_limits<int>::max();
    default:
        throw std::domain_error("ReturnValue enumerator outside the expected values");
    }
}

void ISerialPortAsserter::AssertCommand(LMS64CProtocol::eCMD_LMS command, std::size_t index)
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    ASSERT_EQ(packet.cmd, command);
}

void ISerialPortAsserter::AssertStatus(LMS64CProtocol::eCMD_STATUS status, std::size_t index)
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    ASSERT_EQ(packet.status, status);
}

void ISerialPortAsserter::AssertBlockCount(uint8_t count, std::size_t index)
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    ASSERT_EQ(packet.blockCount, count);
}

void ISerialPortAsserter::AssertPeriphID(uint8_t id, std::size_t index)
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    ASSERT_EQ(packet.periphID, id);
}

void ISerialPortAsserter::AssertSubDevice(uint8_t subdevice, std::size_t index)
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    ASSERT_EQ(packet.subDevice, subdevice);
}

void ISerialPortAsserter::AssertCommandAll(LMS64CProtocol::eCMD_LMS command)
{
    for (const lime::LMS64CPacket& packet : mWrittenPackets)
    {
        ASSERT_EQ(packet.cmd, command);
    }
}

void ISerialPortAsserter::AssertStatusAll(LMS64CProtocol::eCMD_STATUS status)
{
    for (const lime::LMS64CPacket& packet : mWrittenPackets)
    {
        ASSERT_EQ(packet.status, status);
    }
}

void ISerialPortAsserter::AssertBlockCountAll(uint8_t count)
{
    for (const lime::LMS64CPacket& packet : mWrittenPackets)
    {
        ASSERT_EQ(packet.blockCount, count);
    }
}

void ISerialPortAsserter::AssertPeriphIDAll(uint8_t id)
{
    for (const lime::LMS64CPacket& packet : mWrittenPackets)
    {
        ASSERT_EQ(packet.periphID, id);
    }
}

void ISerialPortAsserter::AssertSubDeviceAll(uint8_t subdevice)
{
    for (const lime::LMS64CPacket& packet : mWrittenPackets)
    {
        ASSERT_EQ(packet.subDevice, subdevice);
    }
}

void ISerialPortAsserter::AssertWriteCalled(uint times)
{
    ASSERT_EQ(mWriteCallCount, times);
}

void ISerialPortAsserter::AssertReadCalled(uint times)
{
    ASSERT_EQ(mReadCallCount, times);
}

} // namespace lime::testing