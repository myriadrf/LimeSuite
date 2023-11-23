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
using ::testing::SetArrayArgument;

static constexpr std::size_t PACKET_SIZE = sizeof(LMS64CPacket);

MATCHER_P(IsCommandCorrect, command, "Checks if the packet has the correct command")
{
    auto packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->cmd == command;
}

MATCHER_P(IsSubdeviceCorrect, subDevice, "Checks if the packet has the correct subdevice")
{
    auto packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->subDevice == subDevice;
}

TEST(LMS64CProtocol, GetFirmwareInfoGetsInfo)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    LMS64CProtocol::FirmwareInfo info;

    uint32_t subdevice = 1U;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(
        mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_GET_INFO), IsSubdeviceCorrect(subdevice)), PACKET_SIZE, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    int returnValue = LMS64CProtocol::GetFirmwareInfo(mockPort, info, subdevice);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, GetFirmwareInfoNotFullyWritten)
{
    SerialPortMock mockPort{};
    LMS64CProtocol::FirmwareInfo info;

    uint32_t subdevice = 1U;

    ON_CALL(mockPort, Write(_, PACKET_SIZE, _)).WillByDefault(Return(0));

    EXPECT_CALL(
        mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_GET_INFO), IsSubdeviceCorrect(subdevice)), PACKET_SIZE, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(0);

    int returnValue = LMS64CProtocol::GetFirmwareInfo(mockPort, info, subdevice);

    EXPECT_EQ(returnValue, -1);
}

TEST(LMS64CProtocol, GetFirmwareInfoNotFullyRead)
{
    SerialPortMock mockPort{};
    LMS64CProtocol::FirmwareInfo info;

    uint32_t subdevice = 1U;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _)).WillByDefault(Return(0));

    EXPECT_CALL(
        mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_GET_INFO), IsSubdeviceCorrect(subdevice)), PACKET_SIZE, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    int returnValue = LMS64CProtocol::GetFirmwareInfo(mockPort, info, subdevice);

    EXPECT_EQ(returnValue, -1);
}

TEST(LMS64CProtocol, GetFirmwareInfoWrongStatus)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_RESOURCE_DENIED_CMD;

    LMS64CProtocol::FirmwareInfo info;

    uint32_t subdevice = 1U;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(
        mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_GET_INFO), IsSubdeviceCorrect(subdevice)), PACKET_SIZE, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    int returnValue = LMS64CProtocol::GetFirmwareInfo(mockPort, info, subdevice);

    EXPECT_EQ(returnValue, -1);
}
