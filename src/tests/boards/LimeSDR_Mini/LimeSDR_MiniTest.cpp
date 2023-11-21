#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "tests/comms/USB/FT601/FT601Mock.h"
#include "tests/comms/USB/USB_CSR_PipeMock.h"
#include "LimeSDR_Mini.h"
#include "LMS64C_FPGA_Over_USB.h"
#include "LMS64C_LMS7002M_Over_USB.h"
#include "USB_CSR_Pipe_Mini.h"

using namespace lime;
using namespace lime::testing;
using ::testing::_;
using ::testing::AllOf;
using ::testing::DoAll;
using ::testing::ReturnArg;
using ::testing::SetArrayArgument;

static constexpr std::size_t packetSize = sizeof(LMS64CPacket);

MATCHER_P(IsCommandCorrect, command, "Checks if the packet has the correct command")
{
    LMS64CPacket* packet = reinterpret_cast<LMS64CPacket*>(const_cast<uint8_t*>(arg));

    return packet->cmd == command;
}

MATCHER_P(IsBlockCountCorrect, blockCount, "Checks if the packet has the correct block count")
{
    LMS64CPacket* packet = reinterpret_cast<LMS64CPacket*>(const_cast<uint8_t*>(arg));

    return packet->blockCount == blockCount;
}

MATCHER_P2(IsPayloadByteCorrect, index, byte, "Checks if the packet has the correct block count")
{
    LMS64CPacket* packet = reinterpret_cast<LMS64CPacket*>(const_cast<uint8_t*>(arg));

    return packet->payload[index] == byte;
}

TEST(LimeSDR_Mini, Constructor)
{
    auto usbComms{ std::make_shared<FT601Mock>() };
    auto usbPipeMock{ std::make_shared<USB_CSR_PipeMock>() };
    auto route_lms7002m{ std::make_shared<LMS64C_LMS7002M_Over_USB>(usbPipeMock) };
    auto route_fpga{ std::make_shared<LMS64C_FPGA_Over_USB>(usbPipeMock) };

    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    LMS64CPacket detectRefClkPacket{};
    detectRefClkPacket.status = LMS64CProtocol::STATUS_COMPLETED_CMD;
    detectRefClkPacket.blockCount = 1;
    detectRefClkPacket.payload[3] |= 0x4;

    ON_CALL(*usbPipeMock, Read(_, packetSize, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    // Expectations are matched in reverse order (last written to first written)
    EXPECT_CALL(*usbPipeMock, Write(_, packetSize, _)).Times(7);
    EXPECT_CALL(*usbPipeMock, Read(_, packetSize, _)).Times(7);

    auto FPGADetectRefClkReadRegister0x65Matcher =
        AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_RD), IsBlockCountCorrect(1), IsPayloadByteCorrect(1, 0x65));

    EXPECT_CALL(*usbPipeMock, Write(FPGADetectRefClkReadRegister0x65Matcher, packetSize, _)).Times(1).RetiresOnSaturation();
    EXPECT_CALL(*usbPipeMock, Read(FPGADetectRefClkReadRegister0x65Matcher, packetSize, _))
        .Times(1)
        .WillOnce(DoAll(SetArrayArgument<0>(
                            reinterpret_cast<uint8_t*>(&detectRefClkPacket), reinterpret_cast<uint8_t*>(&detectRefClkPacket + 1)),
            ReturnArg<1>()))
        .RetiresOnSaturation();

    auto usbPipe = std::static_pointer_cast<USB_CSR_Pipe>(usbPipeMock);

    LimeSDR_Mini(route_lms7002m, route_fpga, usbComms, usbPipe);
}
