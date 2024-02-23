#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "tests/protocols/SerialPortMock.h"
#include "LMS64CProtocol.h"

using namespace lime;
using namespace lime::testing;
using ::testing::_;
using ::testing::AllOf;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::ReturnArg;
using ::testing::Sequence;
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

MATCHER_P(IsSubdeviceCorrect, subDevice, "Checks if the packet has the correct subdevice")
{
    auto packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->subDevice == subDevice;
}

MATCHER_P2(IsPayloadByteCorrect, index, byte, "Checks if the packet has the correct block count")
{
    auto packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->payload[index] == byte;
}

TEST(LMS64CProtocol, CustomParameterWriteTestEmptyDoesNothing)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    uint32_t subdevice = 1U;

    EXPECT_CALL(mockPort, Write(_, PACKET_SIZE, _)).Times(0);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(0);

    OpStatus returnValue = LMS64CProtocol::CustomParameterWrite(mockPort, {}, subdevice);

    EXPECT_EQ(returnValue, OpStatus::SUCCESS);
}

TEST(LMS64CProtocol, CustomParameterWriteTestOneParameter)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    uint32_t subdevice = 1U;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR), IsBlockCountCorrect(1), IsSubdeviceCorrect(subdevice)),
            PACKET_SIZE,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    OpStatus returnValue = LMS64CProtocol::CustomParameterWrite(mockPort, { { 16, 127.0, "C" } }, subdevice);

    EXPECT_EQ(returnValue, OpStatus::SUCCESS);
}

TEST(LMS64CProtocol, CustomParameterWriteTestSixteenParameters)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    uint32_t subdevice = 1U;

    Sequence writeSequence;

    std::vector<CustomParameterIO> parameters(16);

    for (int i = 0; i < 16; i++)
    {
        parameters[i] = { i, 127, "Ohm" };
    }

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR), IsBlockCountCorrect(14), IsSubdeviceCorrect(subdevice)),
            PACKET_SIZE,
            _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR), IsBlockCountCorrect(2), IsSubdeviceCorrect(subdevice)),
            PACKET_SIZE,
            _))
        .Times(1)
        .InSequence(writeSequence);

    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(2);

    OpStatus returnValue = LMS64CProtocol::CustomParameterWrite(mockPort, parameters, subdevice);

    EXPECT_EQ(returnValue, OpStatus::SUCCESS);
}

TEST(LMS64CProtocol, CustomParameterWriteTestLowValue)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    uint32_t subdevice = 1U;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR),
                  IsBlockCountCorrect(1),
                  IsSubdeviceCorrect(subdevice),
                  IsPayloadByteCorrect(1, static_cast<uint8_t>(-2))),
            PACKET_SIZE,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    OpStatus returnValue = LMS64CProtocol::CustomParameterWrite(mockPort, { { 9, 0.00127, "W" } }, subdevice);

    EXPECT_EQ(returnValue, OpStatus::SUCCESS);
}

TEST(LMS64CProtocol, CustomParameterWriteTestHighValue)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    uint32_t subdevice = 1U;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR),
                  IsBlockCountCorrect(1),
                  IsSubdeviceCorrect(subdevice),
                  IsPayloadByteCorrect(1, static_cast<uint8_t>(1))),
            PACKET_SIZE,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    OpStatus returnValue = LMS64CProtocol::CustomParameterWrite(mockPort, { { 4, 131268, "A" } }, subdevice);

    EXPECT_EQ(returnValue, OpStatus::SUCCESS);
}

TEST(LMS64CProtocol, CustomParameterWriteNotFullyWritten)
{
    SerialPortMock mockPort{};

    uint32_t subdevice = 1U;

    ON_CALL(mockPort, Write(_, PACKET_SIZE, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR), IsBlockCountCorrect(1), IsSubdeviceCorrect(subdevice)),
            PACKET_SIZE,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(0);

    EXPECT_THROW(LMS64CProtocol::CustomParameterWrite(mockPort, { { 16, 127.0, "C" } }, subdevice);, std::runtime_error);
}

TEST(LMS64CProtocol, CustomParameterWriteNotFullyRead)
{
    SerialPortMock mockPort{};

    uint32_t subdevice = 1U;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR), IsBlockCountCorrect(1), IsSubdeviceCorrect(subdevice)),
            PACKET_SIZE,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    EXPECT_THROW(LMS64CProtocol::CustomParameterWrite(mockPort, { { 16, 127.0, "C" } }, subdevice);, std::runtime_error);
}

TEST(LMS64CProtocol, CustomParameterWriteWrongStatus)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_MANY_BLOCKS_CMD;

    uint32_t subdevice = 1U;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR), IsBlockCountCorrect(1), IsSubdeviceCorrect(subdevice)),
            PACKET_SIZE,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    EXPECT_THROW(LMS64CProtocol::CustomParameterWrite(mockPort, { { 16, 127.0, "C" } }, subdevice);, std::runtime_error);
}
