#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "tests/protocols/ISerialPortMock.h"
#include "LMS64CProtocol.h"
#include <array>

using namespace lime;
using namespace lime::testing;
using ::testing::_;
using ::testing::AllOf;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::ReturnArg;
using ::testing::SetArrayArgument;

static constexpr std::size_t packetSize = sizeof(LMS64CPacket);

MATCHER_P(IsCommandCorrect, command, "Checks if the packet has the correct command")
{
    auto packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->cmd == command;
}

MATCHER_P(IsBlockCountCorrect, blockCount, "Checks if the packet has the correct block count")
{
    auto packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->blockCount == blockCount;
}

TEST(LMS64CProtocol, GPIOReadTestOneBlock)
{
    ISerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    const uint8_t value = 0b01010101;
    packet.payload[0] = value;

    ON_CALL(mockPort, Read(_, packetSize, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_GPIO_RD), IsBlockCountCorrect(1)), packetSize, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(1);

    uint8_t actual = 0;
    int returnValue = LMS64CProtocol::GPIORead(mockPort, &actual, 1);

    EXPECT_EQ(returnValue, 0);
    EXPECT_EQ(actual, value);
}

TEST(LMS64CProtocol, GPIOReadTestTwoBlocks)
{
    ISerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    const uint8_t value1 = 0b01010101;
    const uint8_t value2 = 0b10101010;

    packet.payload[0] = value1;
    packet.payload[1] = value2;

    ON_CALL(mockPort, Read(_, packetSize, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_GPIO_RD), IsBlockCountCorrect(2)), packetSize, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(1);

    std::array<uint8_t, 2> actual{ 0, 0 };
    int returnValue = LMS64CProtocol::GPIORead(mockPort, actual.data(), 2);

    EXPECT_EQ(returnValue, 0);
    EXPECT_EQ(actual[0], value1);
    EXPECT_EQ(actual[1], value2);
}

TEST(LMS64CProtocol, GPIOReadTestNotFullyWritten)
{
    ISerialPortMock mockPort{};

    ON_CALL(mockPort, Write(_, packetSize, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_GPIO_RD), IsBlockCountCorrect(1)), packetSize, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(0);

    uint8_t actual = 0;
    EXPECT_THROW(LMS64CProtocol::GPIORead(mockPort, &actual, 1);, std::runtime_error);
}

TEST(LMS64CProtocol, GPIOReadTestNotFullyRead)
{
    ISerialPortMock mockPort{};

    ON_CALL(mockPort, Read(_, packetSize, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_GPIO_RD), IsBlockCountCorrect(1)), packetSize, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(1);

    uint8_t actual = 0;
    EXPECT_THROW(LMS64CProtocol::GPIORead(mockPort, &actual, 1);, std::runtime_error);
}

TEST(LMS64CProtocol, GPIOReadTestWrongStatus)
{
    ISerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_BUSY_CMD;

    ON_CALL(mockPort, Read(_, packetSize, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_GPIO_RD), IsBlockCountCorrect(1)), packetSize, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(1);

    uint8_t actual = 0;
    EXPECT_THROW(LMS64CProtocol::GPIORead(mockPort, &actual, 1);, std::runtime_error);
}

TEST(LMS64CProtocol, GPIOReadTestBufferSizeTooBig)
{
    ISerialPortMock mockPort{};

    EXPECT_CALL(mockPort, Write(_, packetSize, _)).Times(0);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(0);

    uint8_t actual = 0;
    EXPECT_THROW(LMS64CProtocol::GPIORead(mockPort, &actual, 64);, std::invalid_argument);
}
