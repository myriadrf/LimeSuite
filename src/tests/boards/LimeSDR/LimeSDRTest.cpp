#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "tests/comms/USB/FX3/FX3Mock.h"
#include "boards/LimeSDR/LimeSDR.h"
#include "comms/USB/LMS64C_LMS7002M_Over_USB.h"
#include "comms/USB/LMS64C_FPGA_Over_USB.h"
#include "tests/comms/USB/USB_CSR_PipeMock.h"

using namespace lime;
using namespace lime::testing;
using ::testing::_;
using ::testing::AllOf;
using ::testing::AnyNumber;
using ::testing::DoAll;
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

MATCHER_P(IsPeripheralIDCorrect, periphID, "Checks if the packet has the correct peripheral ID")
{
    const LMS64CPacket* packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->periphID == periphID;
}

MATCHER_P2(ArePayloadBytesCorrect, count, bytes, "Checks if the packet has the correct block count")
{
    auto packet = reinterpret_cast<const LMS64CPacket*>(arg);
    bool correct = true;

    for (int i = 0; i < count; i++)
    {
        correct &= packet->payload[i] == bytes[i];
    }

    return correct;
}

TEST(LimeSDR, Constructor)
{
    auto usbCommsMock = std::make_shared<FX3Mock>();
    auto usbPipeMock = std::make_shared<USB_CSR_PipeMock>();
    auto lms7002mRoute = std::make_shared<LMS64C_LMS7002M_Over_USB>(usbPipeMock);
    auto fpgaRoute = std::make_shared<LMS64C_FPGA_Over_USB>(usbPipeMock);

    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    ON_CALL(*usbPipeMock, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    // Sink uninteresting calls so that any new changes don't immediately break tests
    EXPECT_CALL(*usbPipeMock, Write(_, PACKET_SIZE, _)).Times(AnyNumber());
    EXPECT_CALL(*usbPipeMock, Read(_, PACKET_SIZE, _)).Times(AnyNumber());

    // Calls about getting the device information
    auto getInfoMatcher = IsCommandCorrect(LMS64CProtocol::CMD_GET_INFO);
    EXPECT_CALL(*usbPipeMock, Write(getInfoMatcher, PACKET_SIZE, _)).Times(1).RetiresOnSaturation();
    EXPECT_CALL(*usbPipeMock, Read(getInfoMatcher, PACKET_SIZE, _)).Times(1).RetiresOnSaturation();

    // Calls the FPGA's device information
    const uint8_t GetDeviceInformationPayloadBytes[]{ 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03 };
    auto callFPGAForDeviceInformationMatcher = AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_RD),
        IsBlockCountCorrect(4),
        IsPeripheralIDCorrect(1),
        ArePayloadBytesCorrect(8, GetDeviceInformationPayloadBytes));
    EXPECT_CALL(*usbPipeMock, Write(callFPGAForDeviceInformationMatcher, PACKET_SIZE, _)).Times(1).RetiresOnSaturation();
    EXPECT_CALL(*usbPipeMock, Read(callFPGAForDeviceInformationMatcher, PACKET_SIZE, _)).Times(1).RetiresOnSaturation();

    // Calls the LMS7002M chip for its chip revision
    const uint8_t LMS7002ChipRevisionBytes[]{ 0x00, 0x2f };
    auto callForLMS7002MChipRevisionMatcher = AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RD),
        IsBlockCountCorrect(1),
        IsPeripheralIDCorrect(0),
        ArePayloadBytesCorrect(2, LMS7002ChipRevisionBytes));
    EXPECT_CALL(*usbPipeMock, Write(callForLMS7002MChipRevisionMatcher, PACKET_SIZE, _)).Times(1).RetiresOnSaturation();
    EXPECT_CALL(*usbPipeMock, Read(callForLMS7002MChipRevisionMatcher, PACKET_SIZE, _)).Times(1).RetiresOnSaturation();

    // Calls the FPGA for its gateware information
    auto callFPGAForGatewareInformationMatcher = AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_RD),
        IsBlockCountCorrect(4),
        IsPeripheralIDCorrect(0),
        ArePayloadBytesCorrect(8, GetDeviceInformationPayloadBytes));
    EXPECT_CALL(*usbPipeMock, Write(callFPGAForGatewareInformationMatcher, PACKET_SIZE, _)).Times(1).RetiresOnSaturation();
    EXPECT_CALL(*usbPipeMock, Read(callFPGAForGatewareInformationMatcher, PACKET_SIZE, _)).Times(1).RetiresOnSaturation();

    auto usbPipe = std::static_pointer_cast<USB_CSR_Pipe>(usbPipeMock);

    LimeSDR(lms7002mRoute, fpgaRoute, usbCommsMock, usbPipe);
}
