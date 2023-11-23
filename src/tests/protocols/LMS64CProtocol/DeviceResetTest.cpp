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
    const LMS64CPacket* packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->cmd == command;
}

MATCHER_P(IsSubdeviceCorrect, subDevice, "Checks if the packet has the correct subdevice")
{
    const LMS64CPacket* packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->subDevice == subDevice;
}

MATCHER_P(IsPeripheralIDCorrect, periphID, "Checks if the packet has the correct peripheral ID")
{
    const LMS64CPacket* packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->periphID == periphID;
}

MATCHER_P2(IsPayloadByteCorrect, index, byte, "Checks if the packet has the correct block count")
{
    auto packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->payload[index] == byte;
}

TEST(LMS64CProtocol, DeviceResetTestCorrectCommand)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    uint32_t socIndex = 6U;
    uint32_t subdevice = 1U;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RST),
                  IsSubdeviceCorrect(subdevice),
                  IsPeripheralIDCorrect(socIndex),
                  IsPayloadByteCorrect(0, 2)),
            PACKET_SIZE,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    int returnValue = LMS64CProtocol::DeviceReset(mockPort, socIndex, subdevice);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, DeviceResetTestNotFullyWritten)
{
    SerialPortMock mockPort{};

    uint32_t socIndex = 6U;
    uint32_t subdevice = 1U;

    ON_CALL(mockPort, Write(_, PACKET_SIZE, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RST),
                  IsSubdeviceCorrect(subdevice),
                  IsPeripheralIDCorrect(socIndex),
                  IsPayloadByteCorrect(0, 2)),
            PACKET_SIZE,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(0);

    EXPECT_THROW(LMS64CProtocol::DeviceReset(mockPort, socIndex, subdevice);, std::runtime_error);
}

TEST(LMS64CProtocol, DeviceResetTestNotFullyRead)
{
    SerialPortMock mockPort{};

    uint32_t socIndex = 6U;
    uint32_t subdevice = 1U;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RST),
                  IsSubdeviceCorrect(subdevice),
                  IsPeripheralIDCorrect(socIndex),
                  IsPayloadByteCorrect(0, 2)),
            PACKET_SIZE,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    EXPECT_THROW(LMS64CProtocol::DeviceReset(mockPort, socIndex, subdevice);, std::runtime_error);
}

TEST(LMS64CProtocol, DeviceResetTestWrongStatus)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_BUSY_CMD;

    uint32_t socIndex = 6U;
    uint32_t subdevice = 1U;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RST),
                  IsSubdeviceCorrect(subdevice),
                  IsPeripheralIDCorrect(socIndex),
                  IsPayloadByteCorrect(0, 2)),
            PACKET_SIZE,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    EXPECT_THROW(LMS64CProtocol::DeviceReset(mockPort, socIndex, subdevice);, std::runtime_error);
}
