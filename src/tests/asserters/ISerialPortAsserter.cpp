#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ISerialPortAsserter.h"

#include <cstring>
#include <limits>

namespace lime::testing {

ISerialPortAsserter::ISerialPortAsserter(std::vector<lime::LMS64CPacket> packetsToReturnOnRead,
    std::vector<ReturnValue> returnValuesWrite,
    std::vector<ReturnValue> returnValuesRead)
    : mWriteCallCount(0)
    , mReadCallCount(0)
    , mWrittenPackets()
    , mPacketsToReturnOnRead(packetsToReturnOnRead)
    , mReturnValuesWrite(returnValuesWrite)
    , mReturnValuesRead(returnValuesRead)
{
    if (mPacketsToReturnOnRead.size() == 0)
    {
        mPacketsToReturnOnRead.push_back(LMS64CPacket());
    }
}

int ISerialPortAsserter::Write(const uint8_t* data, size_t length, int timeout_ms)
{
    const std::size_t returnValueIndex = std::min(mWriteCallCount, mReturnValuesWrite.size() - 1);
    ++mWriteCallCount;

    lime::LMS64CPacket packet;
    std::memcpy(&packet, data, length);

    mWrittenPackets.push_back(packet);

    return GetReturnValue(mReturnValuesWrite.at(returnValueIndex));
}

int ISerialPortAsserter::Read(uint8_t* data, size_t length, int timeout_ms)
{
    const std::size_t returnValueIndex = std::min(mReadCallCount, mReturnValuesRead.size() - 1);

    const std::size_t packetToReturnIndex = std::min(mReadCallCount, mPacketsToReturnOnRead.size() - 1);
    const lime::LMS64CPacket& packet = mPacketsToReturnOnRead.at(packetToReturnIndex);

    std::memcpy(data, &packet, length);

    ++mReadCallCount;

    return GetReturnValue(mReturnValuesRead.at(returnValueIndex));
}

int ISerialPortAsserter::GetReturnValue(const ReturnValue& returnValue) const
{
    switch (returnValue.returnValue)
    {
    case eReturnValue::Minimum:
        return std::numeric_limits<int>::min();
    case eReturnValue::MinusOne:
        return -1;
    case eReturnValue::Zero:
        return 0;
    case eReturnValue::One:
        return 1;
    case eReturnValue::OneLess:
        return returnValue.expectedReturnValue - 1;
    case eReturnValue::Expected:
        return returnValue.expectedReturnValue;
    case eReturnValue::OneMore:
        return returnValue.expectedReturnValue + 1;
    case eReturnValue::Maximum:
        return std::numeric_limits<int>::max();
    default:
        throw std::domain_error("eReturnValue enumerator outside the expected values");
    }
}

void ISerialPortAsserter::AssertCommand(const LMS64CProtocol::eCMD_LMS command, const std::size_t index) const
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    ASSERT_EQ(packet.cmd, command);
}

void ISerialPortAsserter::AssertStatus(const LMS64CProtocol::eCMD_STATUS status, const std::size_t index) const
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    ASSERT_EQ(packet.status, status);
}

void ISerialPortAsserter::AssertBlockCount(const uint8_t count, const std::size_t index) const
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    ASSERT_EQ(packet.blockCount, count);
}

void ISerialPortAsserter::AssertPeriphID(const uint8_t id, const std::size_t index) const
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    ASSERT_EQ(packet.periphID, id);
}

void ISerialPortAsserter::AssertSubDevice(const uint8_t subdevice, const std::size_t index) const
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    ASSERT_EQ(packet.subDevice, subdevice);
}

void ISerialPortAsserter::AssertCommandAll(const LMS64CProtocol::eCMD_LMS command) const
{
    for (std::size_t i = 0; i < mWrittenPackets.size(); ++i)
    {
        AssertCommand(command, i);
    }
}

void ISerialPortAsserter::AssertStatusAll(const LMS64CProtocol::eCMD_STATUS status) const
{
    for (std::size_t i = 0; i < mWrittenPackets.size(); ++i)
    {
        AssertStatus(status, i);
    }
}

void ISerialPortAsserter::AssertBlockCountAll(const uint8_t count) const
{
    for (std::size_t i = 0; i < mWrittenPackets.size(); ++i)
    {
        AssertBlockCount(count, i);
    }
}

void ISerialPortAsserter::AssertPeriphIDAll(const uint8_t id) const
{
    for (std::size_t i = 0; i < mWrittenPackets.size(); ++i)
    {
        AssertPeriphID(id, i);
    }
}

void ISerialPortAsserter::AssertSubDeviceAll(const uint8_t subdevice) const
{
    for (std::size_t i = 0; i < mWrittenPackets.size(); ++i)
    {
        AssertSubDevice(subdevice, i);
    }
}

void ISerialPortAsserter::AssertCommandSequence(const std::vector<LMS64CProtocol::eCMD_LMS>& commands) const
{
    for (std::size_t i = 0; i < mWrittenPackets.size(); ++i)
    {
        const std::size_t comparisonIndex = std::min(i, commands.size() - 1);
        AssertCommand(commands.at(comparisonIndex), i);
    }
}

void ISerialPortAsserter::AssertStatusSequence(const std::vector<LMS64CProtocol::eCMD_STATUS>& statuses) const
{
    for (std::size_t i = 0; i < mWrittenPackets.size(); ++i)
    {
        const std::size_t comparisonIndex = std::min(i, statuses.size() - 1);
        AssertStatus(statuses.at(comparisonIndex), i);
    }
}

void ISerialPortAsserter::AssertBlockCountSequence(const std::vector<uint8_t>& counts) const
{
    for (std::size_t i = 0; i < mWrittenPackets.size(); ++i)
    {
        const std::size_t comparisonIndex = std::min(i, counts.size() - 1);
        AssertBlockCount(counts.at(comparisonIndex), i);
    }
}

void ISerialPortAsserter::AssertPeriphIDSequence(const std::vector<uint8_t>& ids) const
{
    for (std::size_t i = 0; i < mWrittenPackets.size(); ++i)
    {
        const std::size_t comparisonIndex = std::min(i, ids.size() - 1);
        AssertPeriphID(ids.at(comparisonIndex), i);
    }
}

void ISerialPortAsserter::AssertSubDeviceSequence(const std::vector<uint8_t>& subdevices) const
{
    for (std::size_t i = 0; i < mWrittenPackets.size(); ++i)
    {
        const std::size_t comparisonIndex = std::min(i, subdevices.size() - 1);
        AssertSubDevice(subdevices.at(comparisonIndex), i);
    }
}

void ISerialPortAsserter::AssertWriteCalled(const uint times) const
{
    ASSERT_EQ(mWriteCallCount, times);
}

void ISerialPortAsserter::AssertReadCalled(const uint times) const
{
    ASSERT_EQ(mReadCallCount, times);
}

} // namespace lime::testing