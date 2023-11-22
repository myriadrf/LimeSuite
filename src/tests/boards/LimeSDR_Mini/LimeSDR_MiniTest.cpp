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
using ::testing::AnyNumber;
using ::testing::DoAll;
using ::testing::ReturnArg;
using ::testing::Sequence;
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

TEST(LimeSDR_Mini, Constructor)
{
    auto usbComms = std::make_shared<FT601Mock>();
    auto usbPipeMock = std::make_shared<USB_CSR_PipeMock>();
    auto lms7002mRoute = std::make_shared<LMS64C_LMS7002M_Over_USB>(usbPipeMock);
    auto fpgaRoute = std::make_shared<LMS64C_FPGA_Over_USB>(usbPipeMock);

    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    Sequence FPGADetectRefClockSequence;

    ON_CALL(*usbPipeMock, Read(_, packetSize, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    // Sink uninteresting calls so that any new changes don't immediately break tests
    EXPECT_CALL(*usbPipeMock, Write(_, packetSize, _)).Times(AnyNumber());
    EXPECT_CALL(*usbPipeMock, Read(_, packetSize, _)).Times(AnyNumber());

    // Calls about getting the device information
    auto getInfoMatcher = IsCommandCorrect(LMS64CProtocol::CMD_GET_INFO);
    EXPECT_CALL(*usbPipeMock, Write(getInfoMatcher, packetSize, _)).Times(1).RetiresOnSaturation();
    EXPECT_CALL(*usbPipeMock, Read(getInfoMatcher, packetSize, _)).Times(1).RetiresOnSaturation();

    // Calls the FPGA's device information
    const uint8_t GetDeviceInformationPayloadBytes[]{ 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03 };
    auto callFPGAForDeviceInformationMatcher = AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_RD),
        IsBlockCountCorrect(4),
        IsPeripheralIDCorrect(1),
        ArePayloadBytesCorrect(8, GetDeviceInformationPayloadBytes));
    EXPECT_CALL(*usbPipeMock, Write(callFPGAForDeviceInformationMatcher, packetSize, _)).Times(1).RetiresOnSaturation();
    EXPECT_CALL(*usbPipeMock, Read(callFPGAForDeviceInformationMatcher, packetSize, _)).Times(1).RetiresOnSaturation();

    // Calls the LMS7002M chip for its chip revision
    const uint8_t LMS7002ChipRevisionBytes[]{ 0x00, 0x2f };
    auto callForLMS7002MChipRevisionMatcher = AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RD),
        IsBlockCountCorrect(1),
        IsPeripheralIDCorrect(0),
        ArePayloadBytesCorrect(2, LMS7002ChipRevisionBytes));
    EXPECT_CALL(*usbPipeMock, Write(callForLMS7002MChipRevisionMatcher, packetSize, _)).Times(1).RetiresOnSaturation();
    EXPECT_CALL(*usbPipeMock, Read(callForLMS7002MChipRevisionMatcher, packetSize, _)).Times(1).RetiresOnSaturation();

    // Writes to the FPGA values to set up the reference clock detection
    const uint8_t FPGADetectRefClockSetupBytes[]{ 0x80, 0x61, 0x00, 0x00, 0x80, 0x63, 0x00, 0x00 };
    auto FPGADetectRefClockSetupMatcher = AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_WR),
        IsBlockCountCorrect(2),
        IsPeripheralIDCorrect(0),
        ArePayloadBytesCorrect(8, FPGADetectRefClockSetupBytes));
    EXPECT_CALL(*usbPipeMock, Write(FPGADetectRefClockSetupMatcher, packetSize, _))
        .Times(1)
        .InSequence(FPGADetectRefClockSequence)
        .RetiresOnSaturation();
    EXPECT_CALL(*usbPipeMock, Read(FPGADetectRefClockSetupMatcher, packetSize, _))
        .Times(1)
        .InSequence(FPGADetectRefClockSequence)
        .RetiresOnSaturation();

    // Writes to the FPGA value to start the reference clock detection
    const uint8_t FPGADetectRefClockStartBytes[]{ 0x80, 0x61, 0x00, 0x04 };
    auto FPGADetectRefClockStartMatcher = AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_WR),
        IsBlockCountCorrect(1),
        IsPeripheralIDCorrect(0),
        ArePayloadBytesCorrect(4, FPGADetectRefClockStartBytes));
    EXPECT_CALL(*usbPipeMock, Write(FPGADetectRefClockStartMatcher, packetSize, _))
        .Times(1)
        .InSequence(FPGADetectRefClockSequence)
        .RetiresOnSaturation();
    EXPECT_CALL(*usbPipeMock, Read(FPGADetectRefClockStartMatcher, packetSize, _))
        .Times(1)
        .InSequence(FPGADetectRefClockSequence)
        .RetiresOnSaturation();

    // When the clock is found, the device responds with a specific bit set
    LMS64CPacket detectRefClkPacket{};
    detectRefClkPacket.status = LMS64CProtocol::STATUS_COMPLETED_CMD;
    detectRefClkPacket.blockCount = 1;
    detectRefClkPacket.payload[3] |= 0b100;

    auto FPGADetectRefClkReadRegister0x65Matcher =
        AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_RD), IsBlockCountCorrect(1), IsPayloadByteCorrect(1, 0x65));
    EXPECT_CALL(*usbPipeMock, Write(FPGADetectRefClkReadRegister0x65Matcher, packetSize, _))
        .Times(1)
        .InSequence(FPGADetectRefClockSequence)
        .RetiresOnSaturation();
    EXPECT_CALL(*usbPipeMock, Read(FPGADetectRefClkReadRegister0x65Matcher, packetSize, _))
        .Times(1)
        .InSequence(FPGADetectRefClockSequence)
        .WillOnce(DoAll(SetArrayArgument<0>(
                            reinterpret_cast<uint8_t*>(&detectRefClkPacket), reinterpret_cast<uint8_t*>(&detectRefClkPacket + 1)),
            ReturnArg<1>()))
        .RetiresOnSaturation();

    // When the reference clock detection is finished, read the clock counter from the device
    const uint8_t FPGADetectRefClockReadClockCounterBytes[]{ 0x00, 0x72, 0x00, 0x73 };
    auto FPGADetectRefClockReadClockCounterMatcher = AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_RD),
        IsBlockCountCorrect(2),
        IsPeripheralIDCorrect(0),
        ArePayloadBytesCorrect(4, FPGADetectRefClockReadClockCounterBytes));
    EXPECT_CALL(*usbPipeMock, Write(FPGADetectRefClockReadClockCounterMatcher, packetSize, _))
        .Times(1)
        .InSequence(FPGADetectRefClockSequence)
        .RetiresOnSaturation();
    EXPECT_CALL(*usbPipeMock, Read(FPGADetectRefClockReadClockCounterMatcher, packetSize, _))
        .Times(1)
        .InSequence(FPGADetectRefClockSequence)
        .RetiresOnSaturation();

    // Calls the FPGA for its gateware information
    auto callFPGAForGatewareInformationMatcher = AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_RD),
        IsBlockCountCorrect(4),
        IsPeripheralIDCorrect(0),
        ArePayloadBytesCorrect(8, GetDeviceInformationPayloadBytes));
    EXPECT_CALL(*usbPipeMock, Write(callFPGAForGatewareInformationMatcher, packetSize, _)).Times(1).RetiresOnSaturation();
    EXPECT_CALL(*usbPipeMock, Read(callFPGAForGatewareInformationMatcher, packetSize, _)).Times(1).RetiresOnSaturation();

    auto usbPipe = std::static_pointer_cast<USB_CSR_Pipe>(usbPipeMock);

    LimeSDR_Mini(lms7002mRoute, fpgaRoute, usbComms, usbPipe);
}
