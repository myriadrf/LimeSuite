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

MATCHER_P(IsPeripheralIDCorrect, periphID, "Checks if the packet has the correct peripheral ID")
{
    auto packet = reinterpret_cast<const LMS64CPacket*>(arg);

    return packet->periphID == periphID;
}

TEST(LMS64CProtocol, LMS7002MSPIEmptyTest)
{
    SerialPortMock mockPort{};

    EXPECT_CALL(mockPort, Write(_, sizeof(LMS64CPacket), _)).Times(0);
    EXPECT_CALL(mockPort, Read(_, sizeof(LMS64CPacket), _)).Times(0);

    int returnValue = LMS64CProtocol::LMS7002M_SPI(mockPort, 0, nullptr, nullptr, 0);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, LMS7002MSPIOneCountTestValueRead)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RD), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    uint32_t mosi = 1U;
    uint32_t miso = 2U;
    int returnValue = LMS64CProtocol::LMS7002M_SPI(mockPort, 0, &mosi, &miso, 1);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, LMS7002MSPIOneCountTestValueWrite)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_WR), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    uint32_t mosi = 1U;
    SetWriteBit(mosi);
    int returnValue = LMS64CProtocol::LMS7002M_SPI(mockPort, 0, &mosi, nullptr, 1);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, LMS7002MSPIWriteReadReadWrite)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    Sequence writeSequence;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_WR), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RD), IsBlockCountCorrect(2)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_WR), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);

    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(3);

    std::vector<uint32_t> mosi{ 1U, 2U, 3U, 4U };
    SetWriteBit(mosi[0]);
    SetWriteBit(mosi[3]);

    std::vector<uint32_t> miso{ 1U, 2U, 3U, 4U };
    int returnValue = LMS64CProtocol::LMS7002M_SPI(mockPort, 0, mosi.data(), miso.data(), 4);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, LMS7002MSPIReadWriteReadReadWrite)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    Sequence writeSequence;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RD), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_WR), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RD), IsBlockCountCorrect(2)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_WR), IsBlockCountCorrect(1)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);

    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(4);

    std::vector<uint32_t> mosi{ 1U, 2U, 3U, 4U, 5U };
    SetWriteBit(mosi[1]);
    SetWriteBit(mosi[4]);

    std::vector<uint32_t> miso{ 1U, 2U, 3U, 4U, 5U };
    int returnValue = LMS64CProtocol::LMS7002M_SPI(mockPort, 0, mosi.data(), miso.data(), 5);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, LMS7002MSPISixteenWrites)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    Sequence writeSequence;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_WR), IsBlockCountCorrect(14)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_WR), IsBlockCountCorrect(2)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);

    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(2);

    uint32_t mosi = 1U;
    SetWriteBit(mosi);
    uint32_t miso = 2U;

    std::vector<uint32_t> mosis(16, mosi);
    std::vector<uint32_t> misos(16, miso);

    int returnValue = LMS64CProtocol::LMS7002M_SPI(mockPort, 0, mosis.data(), misos.data(), 16);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, LMS7002MSPISixteenReads)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    Sequence writeSequence;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RD), IsBlockCountCorrect(14)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);
    EXPECT_CALL(mockPort, Write(AllOf(IsCommandCorrect(LMS64CProtocol::CMD_LMS7002_RD), IsBlockCountCorrect(2)), PACKET_SIZE, _))
        .Times(1)
        .InSequence(writeSequence);

    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(2);

    uint32_t mosi = 1U;
    uint32_t miso = 2U;

    std::vector<uint32_t> mosis(16, mosi);
    std::vector<uint32_t> misos(16, miso);

    int returnValue = LMS64CProtocol::LMS7002M_SPI(mockPort, 0, mosis.data(), misos.data(), 16);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, LMS7002MSPINotFullyWritten)
{
    SerialPortMock mockPort{};

    ON_CALL(mockPort, Write(_, PACKET_SIZE, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort, Write(_, PACKET_SIZE, _)).Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(0);

    uint32_t mosi = 1U;
    uint32_t miso = 2U;

    int returnValue = LMS64CProtocol::LMS7002M_SPI(mockPort, 0, &mosi, &miso, 1);

    EXPECT_EQ(returnValue, -1);
}

TEST(LMS64CProtocol, LMS7002MSPINotFullyRead)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort, Write(_, PACKET_SIZE, _)).Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    uint32_t mosi = 1U;
    uint32_t miso = 2U;

    int returnValue = LMS64CProtocol::LMS7002M_SPI(mockPort, 0, &mosi, &miso, 1);

    EXPECT_EQ(returnValue, -1);
}

TEST(LMS64CProtocol, LMS7002MSPIWrongStatus)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_ERROR_CMD;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _)).WillByDefault(Return(0));

    EXPECT_CALL(mockPort, Write(_, PACKET_SIZE, _)).Times(1);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    uint32_t mosi = 1U;
    uint32_t miso = 2U;

    int returnValue = LMS64CProtocol::LMS7002M_SPI(mockPort, 0, &mosi, &miso, 1);

    EXPECT_EQ(returnValue, -1);
}

TEST(LMS64CProtocol, LMS7002MSPINotFullyWrittenOnSecondCall)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(_, PACKET_SIZE, _)).Times(2).WillOnce(ReturnArg<1>()).WillRepeatedly(Return(0));
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _)).Times(1);

    std::vector<uint32_t> mosi{ 1U, 2U };
    SetWriteBit(mosi[1]); // Swap between read and write for the test

    std::vector<uint32_t> miso{ 1U, 2U };

    int returnValue = LMS64CProtocol::LMS7002M_SPI(mockPort, 0, mosi.data(), miso.data(), 2);

    EXPECT_EQ(returnValue, -1);
}

TEST(LMS64CProtocol, LMS7002MSPINotFullyReadOnSecondCall)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    EXPECT_CALL(mockPort, Write(_, PACKET_SIZE, _)).Times(2);
    EXPECT_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .Times(2)
        .WillOnce(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()))
        .WillRepeatedly(Return(0));

    std::vector<uint32_t> mosi{ 1U, 2U };
    SetWriteBit(mosi[1]);

    std::vector<uint32_t> miso{ 1U, 2U };

    int returnValue = LMS64CProtocol::LMS7002M_SPI(mockPort, 0, mosi.data(), miso.data(), 2);

    EXPECT_EQ(returnValue, -1);
}

TEST(LMS64CProtocol, LMS7002MSPIPassesThroughCorrectChip)
{
    SerialPortMock mockPort{};
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    const uint8_t chip = 1U;

    ON_CALL(mockPort, Read(_, PACKET_SIZE, _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(mockPort, Write(IsPeripheralIDCorrect(chip), PACKET_SIZE, _)).Times(2);
    EXPECT_CALL(mockPort, Read(IsPeripheralIDCorrect(chip), PACKET_SIZE, _)).Times(2);

    std::vector<uint32_t> mosi{ 1U, 2U };
    SetWriteBit(mosi[1]);

    std::vector<uint32_t> miso{ 1U, 2U };

    int returnValue = LMS64CProtocol::LMS7002M_SPI(mockPort, chip, mosi.data(), miso.data(), 2);

    EXPECT_EQ(returnValue, 0);
}
