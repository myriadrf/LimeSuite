#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "tests/protocols/SerialPortMock.h"
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

static constexpr std::size_t PACKET_SIZE = sizeof(LMS64CPacket);

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

MATCHER_P2(IsPayloadByteCorrect, index, byte, "Checks if the packet has the correct block count")
{
    LMS64CPacket* packet = reinterpret_cast<LMS64CPacket*>(const_cast<uint8_t*>(arg));

    return packet->payload[index] == byte;
}

TEST(LMS64CProtocol, GPIOWriteTestOneBlock)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    const uint8_t value = 0b01010101;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_GPIO_WR), IsBlockCountCorrect(1), IsPayloadByteCorrect(0, value)),
            PACKET_SIZE,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    OpStatus returnValue = LMS64CProtocol::GPIOWrite(mockPort, &value, 1);

    EXPECT_EQ(returnValue, OpStatus::SUCCESS);
}

TEST(LMS64CProtocol, GPIOWriteTestTwoBlocks)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    const std::array<uint8_t, 2> values{ 0b01010101, 0b10101010 };

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_GPIO_WR),
                  IsBlockCountCorrect(2),
                  IsPayloadByteCorrect(0, values[0]),
                  IsPayloadByteCorrect(1, values[1])),
            PACKET_SIZE,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    OpStatus returnValue = LMS64CProtocol::GPIOWrite(mockPort, values.data(), 2);

    EXPECT_EQ(returnValue, OpStatus::SUCCESS);
}

TEST(LMS64CProtocol, GPIOWriteTestNotFullyWritten)
{
    SerialPortMock mockPort{};

    ON_CALL(mockPort, Write(_, PACKET_SIZE, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_GPIO_WR), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(0);

    uint8_t actual = 0U;
    EXPECT_THROW(LMS64CProtocol::GPIOWrite(mockPort, &actual, 1);, std::runtime_error);
}

TEST(LMS64CProtocol, GPIOWriteTestNotFullyRead)
{
    SerialPortMock mockPort{};

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_GPIO_WR), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    uint8_t actual = 0U;
    EXPECT_THROW(LMS64CProtocol::GPIOWrite(mockPort, &actual, 1);, std::runtime_error);
}

TEST(LMS64CProtocol, GPIOWriteTestWrongStatus)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_BUSY_CMD;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_GPIO_WR), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    uint8_t actual = 0U;
    EXPECT_THROW(LMS64CProtocol::GPIOWrite(mockPort, &actual, 1);, std::runtime_error);
}

TEST(LMS64CProtocol, GPIOWriteTestBufferSizeTooBig)
{
    SerialPortMock mockPort{};

    EXPECT_CALL(mockPort, Write(_, PACKET_SIZE, _)).Times(0);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(0);

    uint8_t actual = 0U;
    EXPECT_THROW(LMS64CProtocol::GPIOWrite(mockPort, &actual, 64);, std::invalid_argument);
}
