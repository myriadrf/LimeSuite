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

static inline void SetWriteBit(uint32_t& in)
{
    in |= (1 << 31);
}

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

TEST(LMS64CProtocol, FPGASPIEmptyTest)
{
    SerialPortMock mockPort{};

    EXPECT_CALL(mockPort, Write(_, sizeof(LMS64CPacket), _)).Times(0);
    EXPECT_CALL(mockPort, Read(_, sizeof(LMS64CPacket), _)).Times(0);

    int returnValue = LMS64CProtocol::FPGA_SPI(mockPort, nullptr, nullptr, 0);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, FPGASPIOneCountTestValueRead)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<std::byte*>(&packet), reinterpret_cast<std::byte*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_RD), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    uint32_t mosi = 1U;
    uint32_t miso = 2U;
    int returnValue = LMS64CProtocol::FPGA_SPI(mockPort, &mosi, &miso, 1);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, FPGASPIOneCountTestValueWrite)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<std::byte*>(&packet), reinterpret_cast<std::byte*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_WR), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    uint32_t mosi = 1U;
    SetWriteBit(mosi);
    int returnValue = LMS64CProtocol::FPGA_SPI(mockPort, &mosi, nullptr, 1);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, FPGASPIWriteReadReadWrite)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    Sequence writeSequence;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<std::byte*>(&packet), reinterpret_cast<std::byte*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_WR), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_RD), IsBlockCountCorrect(2)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_WR), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);

    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(3);

    std::vector<uint32_t> mosi{ 1U, 2U, 3U, 4U };
    SetWriteBit(mosi[0]);
    SetWriteBit(mosi[3]);

    std::vector<uint32_t> miso{ 1U, 2U, 3U, 4U };
    int returnValue = LMS64CProtocol::FPGA_SPI(mockPort, mosi.data(), miso.data(), 4);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, FPGASPIReadWriteReadReadWrite)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    Sequence writeSequence;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<std::byte*>(&packet), reinterpret_cast<std::byte*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_RD), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_WR), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_RD), IsBlockCountCorrect(2)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_WR), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);

    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(4);

    std::vector<uint32_t> mosi{ 1U, 2U, 3U, 4U, 5U };
    SetWriteBit(mosi[1]);
    SetWriteBit(mosi[4]);

    std::vector<uint32_t> miso{ 1U, 2U, 3U, 4U, 5U };
    int returnValue = LMS64CProtocol::FPGA_SPI(mockPort, mosi.data(), miso.data(), 5);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, FPGASPISixteenWrites)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    Sequence writeSequence;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<std::byte*>(&packet), reinterpret_cast<std::byte*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_WR), IsBlockCountCorrect(14)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_WR), IsBlockCountCorrect(2)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);

    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(2);

    uint32_t mosi = 1U;
    SetWriteBit(mosi);
    uint32_t miso = 2U;

    std::vector<uint32_t> mosis(16, mosi);
    std::vector<uint32_t> misos(16, miso);

    int returnValue = LMS64CProtocol::FPGA_SPI(mockPort, mosis.data(), misos.data(), 16);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, FPGASPISixteenReads)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    Sequence writeSequence;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<std::byte*>(&packet), reinterpret_cast<std::byte*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_RD), IsBlockCountCorrect(14)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_BRDSPI_RD), IsBlockCountCorrect(2)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);

    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(2);

    uint32_t mosi = 1U;
    uint32_t miso = 2U;

    std::vector<uint32_t> mosis(16, mosi);
    std::vector<uint32_t> misos(16, miso);

    int returnValue = LMS64CProtocol::FPGA_SPI(mockPort, mosis.data(), misos.data(), 16);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, FPGASPINotFullyWritten)
{
    SerialPortMock mockPort{};

    ON_CALL(mockPort, Write(_, PACKET_SIZE, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort, Write(_, PACKET_SIZE, _)).Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(0);

    uint32_t mosi = 1U;
    uint32_t miso = 2U;

    int returnValue = LMS64CProtocol::FPGA_SPI(mockPort, &mosi, &miso, 1);

    EXPECT_EQ(returnValue, -1);
}

TEST(LMS64CProtocol, FPGASPINotFullyRead)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<std::byte*>(&packet), reinterpret_cast<std::byte*>(&packet + 1)), ReturnArg<1>()));

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort, Write(_, PACKET_SIZE, _)).Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    uint32_t mosi = 1U;
    uint32_t miso = 2U;

    int returnValue = LMS64CProtocol::FPGA_SPI(mockPort, &mosi, &miso, 1);

    EXPECT_EQ(returnValue, -1);
}

TEST(LMS64CProtocol, FPGASPIWrongStatus)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_UNKNOWN_CMD;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<std::byte*>(&packet), reinterpret_cast<std::byte*>(&packet + 1)), ReturnArg<1>()));

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort, Write(_, PACKET_SIZE, _)).Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    uint32_t mosi = 1U;
    uint32_t miso = 2U;

    int returnValue = LMS64CProtocol::FPGA_SPI(mockPort, &mosi, &miso, 1);

    EXPECT_EQ(returnValue, -1);
}

TEST(LMS64CProtocol, FPGASPINotFullyWrittenOnSecondCall)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<std::byte*>(&packet), reinterpret_cast<std::byte*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(_, PACKET_SIZE, _)).Times(2).WillOnce(ReturnArg<1>()).WillRepeatedly(Return(0));
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    std::vector<uint32_t> mosi{ 1U, 2U };
    SetWriteBit(mosi[1]); // Swap between read and write for the test

    std::vector<uint32_t> miso{ 1U, 2U };

    int returnValue = LMS64CProtocol::FPGA_SPI(mockPort, mosi.data(), miso.data(), 2);

    EXPECT_EQ(returnValue, -1);
}

TEST(LMS64CProtocol, FPGASPINotFullyReadOnSecondCall)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    EXPECT_CALL(mockPort, Write(_, PACKET_SIZE, _)).Times(2);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .Times(2)
        .WillOnce(DoAll(
            SetArrayArgument<0>(reinterpret_cast<std::byte*>(&packet), reinterpret_cast<std::byte*>(&packet + 1)), ReturnArg<1>()))
        .WillRepeatedly(Return(0));

    std::vector<uint32_t> mosi{ 1U, 2U };
    SetWriteBit(mosi[1]);

    std::vector<uint32_t> miso{ 1U, 2U };

    int returnValue = LMS64CProtocol::FPGA_SPI(mockPort, mosi.data(), miso.data(), 2);

    EXPECT_EQ(returnValue, -1);
}
