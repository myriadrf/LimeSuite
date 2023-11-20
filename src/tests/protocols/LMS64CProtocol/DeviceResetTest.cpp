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
using ::testing::SetArrayArgument;

static constexpr std::size_t packetSize = sizeof(LMS64CPacket);

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
    LMS64CPacket* packet = reinterpret_cast<LMS64CPacket*>(const_cast<uint8_t*>(arg));

    return packet->payload[index] == byte;
}

TEST(LMS64CProtocol, DeviceResetTestCorrectCommand)
{
    ISerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    uint32_t socIndex = 1;
    uint32_t subdevice = 1;

    ON_CALL(mockPort, Read(_, packetSize, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RST),
                  IsSubdeviceCorrect(subdevice),
                  IsPeripheralIDCorrect(subdevice),
                  IsPayloadByteCorrect(0, 2)),
            packetSize,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(1);

    int returnValue = LMS64CProtocol::DeviceReset(mockPort, socIndex, 1);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, DeviceResetTestNotFullyWritten)
{
    ISerialPortMock mockPort{};

    uint32_t socIndex = 1;
    uint32_t subdevice = 1;

    ON_CALL(mockPort, Write(_, packetSize, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RST),
                  IsSubdeviceCorrect(subdevice),
                  IsPeripheralIDCorrect(subdevice),
                  IsPayloadByteCorrect(0, 2)),
            packetSize,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(0);

    EXPECT_THROW(LMS64CProtocol::DeviceReset(mockPort, socIndex, 1);, std::runtime_error);
}

TEST(LMS64CProtocol, DeviceResetTestNotFullyRead)
{
    ISerialPortMock mockPort{};

    uint32_t socIndex = 1;
    uint32_t subdevice = 1;

    ON_CALL(mockPort, Read(_, packetSize, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort,
        Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RST),
                  IsSubdeviceCorrect(subdevice),
                  IsPeripheralIDCorrect(subdevice),
                  IsPayloadByteCorrect(0, 2)),
            packetSize,
            _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, packetSize, _)).Times(1);

    EXPECT_THROW(LMS64CProtocol::DeviceReset(mockPort, socIndex, 1);, std::runtime_error);
}
