#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "tests/protocols/ISerialPortMock.h"
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

static constexpr std::size_t packetSize = sizeof(LMS64CPacket);

MATCHER_P(IsCommandCorrect, command, "Checks if the packet has the correct command")
{
    const LMS64CPacket* packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->cmd == command;
}

MATCHER_P(IsBlockCountCorrect, blockCount, "Checks if the packet has the correct block count")
{
    const LMS64CPacket* packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->blockCount == blockCount;
}

MATCHER_P(IsSubdeviceCorrect, subDevice, "Checks if the packet has the correct subdevice")
{
    const LMS64CPacket* packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->subDevice == subDevice;
}

MATCHER_P2(IsPayloadByteCorrect, index, byte, "Checks if the packet has the correct block count")
{
    LMS64CPacket* packet = reinterpret_cast<LMS64CPacket*>(const_cast<uint8_t*>(arg));

    return packet->payload[index] == byte;
}

TEST(LMS64CProtocol, CustomParameterWriteTestEmptyDoesNothing)
{
    ISerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    uint32_t subdevice = 1;

    EXPECT_CALL(mockPort, Write(_, packetSize, _)).Times(0);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(0);

    int returnValue = LMS64CProtocol::CustomParameterWrite(mockPort, {}, subdevice);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, CustomParameterWriteTestOneParameter)
{
    ISerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    uint32_t subdevice = 1;

    ON_CALL(mockPort, Read(_, packetSize, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR), IsBlockCountCorrect(1), IsSubdeviceCorrect(subdevice)),
            packetSize,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(1);

    int returnValue = LMS64CProtocol::CustomParameterWrite(mockPort, { { 16, 127.0, "C" } }, subdevice);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, CustomParameterWriteTestSixteenParameters)
{
    ISerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    uint32_t subdevice = 1;

    Sequence writeSequence;

    std::vector<CustomParameterIO> parameters;
    parameters.reserve(16);

    for (int i = 0; i < 16; i++)
    {
        parameters.push_back({ i, 127, "Ohm" });
    }

    ON_CALL(mockPort, Read(_, packetSize, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR), IsBlockCountCorrect(14), IsSubdeviceCorrect(subdevice)),
            packetSize,
            _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR), IsBlockCountCorrect(2), IsSubdeviceCorrect(subdevice)),
            packetSize,
            _))
        .Times(1)
        .InSequence(writeSequence);

    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(2);

    int returnValue = LMS64CProtocol::CustomParameterWrite(mockPort, parameters, subdevice);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, CustomParameterWriteTestLowValue)
{
    ISerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    uint32_t subdevice = 1;

    ON_CALL(mockPort, Read(_, packetSize, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR),
                  IsBlockCountCorrect(1),
                  IsSubdeviceCorrect(subdevice),
                  IsPayloadByteCorrect(1, static_cast<uint8_t>(-2))),
            packetSize,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(1);

    int returnValue = LMS64CProtocol::CustomParameterWrite(mockPort, { { 9, 0.00127, "W" } }, subdevice);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, CustomParameterWriteTestHighValue)
{
    ISerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    uint32_t subdevice = 1;

    ON_CALL(mockPort, Read(_, packetSize, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR),
                  IsBlockCountCorrect(1),
                  IsSubdeviceCorrect(subdevice),
                  IsPayloadByteCorrect(1, static_cast<uint8_t>(1))),
            packetSize,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(1);

    int returnValue = LMS64CProtocol::CustomParameterWrite(mockPort, { { 4, 131268, "A" } }, subdevice);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, CustomParameterWriteNotFullyWritten)
{
    ISerialPortMock mockPort{};

    uint32_t subdevice = 1;

    ON_CALL(mockPort, Write(_, packetSize, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR), IsBlockCountCorrect(1), IsSubdeviceCorrect(subdevice)),
            packetSize,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(0);

    EXPECT_THROW(LMS64CProtocol::CustomParameterWrite(mockPort, { { 16, 127.0, "C" } }, subdevice);, std::runtime_error);
}

TEST(LMS64CProtocol, CustomParameterWriteNotFullyRead)
{
    ISerialPortMock mockPort{};

    uint32_t subdevice = 1;

    ON_CALL(mockPort, Read(_, packetSize, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR), IsBlockCountCorrect(1), IsSubdeviceCorrect(subdevice)),
            packetSize,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(1);

    EXPECT_THROW(LMS64CProtocol::CustomParameterWrite(mockPort, { { 16, 127.0, "C" } }, subdevice);, std::runtime_error);
}

TEST(LMS64CProtocol, CustomParameterWriteWrongStatus)
{
    ISerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_MANY_BLOCKS_CMD;

    uint32_t subdevice = 1;

    ON_CALL(mockPort, Read(_, packetSize, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_ANALOG_VAL_WR), IsBlockCountCorrect(1), IsSubdeviceCorrect(subdevice)),
            packetSize,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(1);

    EXPECT_THROW(LMS64CProtocol::CustomParameterWrite(mockPort, { { 16, 127.0, "C" } }, subdevice);, std::runtime_error);
}
