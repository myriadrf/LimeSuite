#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ISerialPortAsserter.h"
#include "LMS64CProtocol.h"

#include <cstring>
#include <limits>

namespace lime::testing {

ISerialPortAsserter::ISerialPortAsserter(
    std::vector<LMS64CPacket> packetsToReturnOnRead, std::vector<int> returnValuesWrite, std::vector<int> returnValuesRead)
    : mWriteCallCount(0)
    , mReadCallCount(0)
    , mWrittenPackets()
    , mPacketsToReturnOnRead(packetsToReturnOnRead)
    , mReturnValuesWrite(returnValuesWrite)
    , mReturnValuesRead(returnValuesRead)
    , mWillRepeatLastWrittenBlockCount(false)
{
    if (mPacketsToReturnOnRead.size() == 0)
    {
        mPacketsToReturnOnRead.push_back(LMS64CPacket());
    }
}

int ISerialPortAsserter::Write(const uint8_t* data, size_t length, int timeout_ms)
{
    EXPECT_EQ(length, sizeof(LMS64CPacket));

    LMS64CPacket packet;
    std::memcpy(&packet, data, length);

    mWrittenPackets.push_back(packet);

    const std::size_t returnValueIndex = std::min(mWriteCallCount, mReturnValuesWrite.size() - 1);
    ++mWriteCallCount;

    return mReturnValuesWrite.at(returnValueIndex);
}

int ISerialPortAsserter::Read(uint8_t* data, size_t length, int timeout_ms)
{
    EXPECT_EQ(length, sizeof(LMS64CPacket));

    const std::size_t packetToReturnIndex = std::min(mReadCallCount, mPacketsToReturnOnRead.size() - 1);
    LMS64CPacket& packet = mPacketsToReturnOnRead.at(packetToReturnIndex);

    std::memcpy(data, &packet, length);

    if (mWillRepeatLastWrittenBlockCount)
    {
        packet.blockCount = mWrittenPackets.back().blockCount;
    }

    const std::size_t returnValueIndex = std::min(mReadCallCount, mReturnValuesRead.size() - 1);
    ++mReadCallCount;

    return mReturnValuesRead.at(returnValueIndex);
}

void ISerialPortAsserter::AssertCommand(const LMS64CProtocol::eCMD_LMS command, const std::size_t index) const
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    EXPECT_EQ(packet.cmd, command);
}

void ISerialPortAsserter::AssertStatus(const LMS64CProtocol::eCMD_STATUS status, const std::size_t index) const
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    EXPECT_EQ(packet.status, status);
}

void ISerialPortAsserter::AssertBlockCount(const uint8_t count, const std::size_t index) const
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    EXPECT_EQ(packet.blockCount, count);
}

void ISerialPortAsserter::AssertPeriphID(const uint8_t id, const std::size_t index) const
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    EXPECT_EQ(packet.periphID, id);
}

void ISerialPortAsserter::AssertSubDevice(const uint8_t subdevice, const std::size_t index) const
{
    LMS64CPacket packet = mWrittenPackets.at(index);

    EXPECT_EQ(packet.subDevice, subdevice);
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
    EXPECT_EQ(mWriteCallCount, times);
}

void ISerialPortAsserter::AssertReadCalled(const uint times) const
{
    EXPECT_EQ(mReadCallCount, times);
}

void ISerialPortAsserter::SetWillRepeatLastWrittenBlockCount(const bool newValue)
{
    mWillRepeatLastWrittenBlockCount = newValue;
}

} // namespace lime::testing