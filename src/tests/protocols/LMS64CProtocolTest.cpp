#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ISerialPortAsserter.h"
#include "LMS64CProtocol.h"

using namespace lime;
using namespace lime::testing;

static inline void SetWriteBit(uint32_t& in)
{
    in |= (1 << 31);
}

TEST(LMS64CProtocol, LMS7002MSPIEmptyTest)
{
    ISerialPortAsserter asserter{};

    int returnValue = LMS64CProtocol::LMS7002M_SPI(asserter, 0, nullptr, nullptr, 0);

    asserter.AssertWriteCalled(0);
    asserter.AssertReadCalled(0);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, LMS7002MSPIOneCountTestValueRead)
{
    auto packetsToRead = std::vector<LMS64CPacket>();
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;
    packetsToRead.push_back(packet);
    ISerialPortAsserter asserter{ packetsToRead };

    uint32_t mosi = 1;
    uint32_t miso = 2;
    int returnValue = LMS64CProtocol::LMS7002M_SPI(asserter, 0, &mosi, &miso, 1);

    asserter.AssertWriteCalled(1);
    asserter.AssertReadCalled(1);

    asserter.AssertBlockCountAll(1);
    asserter.AssertCommandAll(LMS64CProtocol::CMD_LMS7002_RD);
    asserter.AssertSubDeviceAll(0);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, LMS7002MSPIOneCountTestValueWrite)
{
    auto packetsToRead = std::vector<LMS64CPacket>();
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;
    packetsToRead.push_back(packet);
    ISerialPortAsserter asserter{ packetsToRead };

    uint32_t mosi = 1;
    SetWriteBit(mosi);
    int returnValue = LMS64CProtocol::LMS7002M_SPI(asserter, 0, &mosi, nullptr, 1);

    asserter.AssertWriteCalled(1);
    asserter.AssertReadCalled(1);

    asserter.AssertBlockCountAll(1);
    asserter.AssertCommandAll(LMS64CProtocol::CMD_LMS7002_WR);
    asserter.AssertSubDeviceAll(0);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, LMS7002MSPIWriteReadReadWrite)
{
    auto packetsToRead = std::vector<LMS64CPacket>();
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;
    packetsToRead.push_back(packet);
    ISerialPortAsserter asserter{ packetsToRead };

    std::vector<uint32_t> mosi{ 1, 2, 3, 4 };
    SetWriteBit(mosi[0]);
    SetWriteBit(mosi[3]);

    std::vector<uint32_t> miso{ 1, 2, 3, 4 };
    int returnValue = LMS64CProtocol::LMS7002M_SPI(asserter, 0, mosi.data(), miso.data(), 4);

    asserter.AssertWriteCalled(3);
    asserter.AssertReadCalled(3);

    asserter.AssertBlockCountSequence({ 1, 2, 1 });
    asserter.AssertCommandSequence(
        { LMS64CProtocol::CMD_LMS7002_WR, LMS64CProtocol::CMD_LMS7002_RD, LMS64CProtocol::CMD_LMS7002_WR });
    asserter.AssertSubDeviceAll(0);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, LMS7002MSPIReadWriteReadReadWrite)
{
    auto packetsToRead = std::vector<LMS64CPacket>();
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;
    packetsToRead.push_back(packet);
    ISerialPortAsserter asserter{ packetsToRead };

    std::vector<uint32_t> mosi{ 1, 2, 3, 4, 5 };
    SetWriteBit(mosi[1]);
    SetWriteBit(mosi[4]);

    std::vector<uint32_t> miso{ 1, 2, 3, 4, 5 };
    int returnValue = LMS64CProtocol::LMS7002M_SPI(asserter, 0, mosi.data(), miso.data(), 5);

    asserter.AssertWriteCalled(4);
    asserter.AssertReadCalled(4);

    asserter.AssertBlockCountSequence({ 1, 1, 2, 1 });
    asserter.AssertCommandSequence({ LMS64CProtocol::CMD_LMS7002_RD,
        LMS64CProtocol::CMD_LMS7002_WR,
        LMS64CProtocol::CMD_LMS7002_RD,
        LMS64CProtocol::CMD_LMS7002_WR });
    asserter.AssertSubDeviceAll(0);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, LMS7002MSPISixteenWrites)
{
    auto packetsToRead = std::vector<LMS64CPacket>();
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;
    packetsToRead.push_back(packet);
    ISerialPortAsserter asserter{ packetsToRead };

    uint32_t mosi = 1;
    SetWriteBit(mosi);
    uint32_t miso = 2;

    std::vector<uint32_t> mosis(16, mosi);
    std::vector<uint32_t> misos(16, miso);

    int returnValue = LMS64CProtocol::LMS7002M_SPI(asserter, 0, mosis.data(), misos.data(), 16);

    asserter.AssertWriteCalled(2);
    asserter.AssertReadCalled(2);

    asserter.AssertBlockCountSequence({ 14, 2 });
    asserter.AssertCommandAll(LMS64CProtocol::CMD_LMS7002_WR);
    asserter.AssertSubDeviceAll(0);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, LMS7002MSPISixteenReads)
{
    auto packetsToRead = std::vector<LMS64CPacket>();
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;
    packetsToRead.push_back(packet);
    ISerialPortAsserter asserter{ packetsToRead };

    uint32_t mosi = 1;
    uint32_t miso = 2;

    std::vector<uint32_t> mosis(16, mosi);
    std::vector<uint32_t> misos(16, miso);

    int returnValue = LMS64CProtocol::LMS7002M_SPI(asserter, 0, mosis.data(), misos.data(), 16);

    asserter.AssertWriteCalled(2);
    asserter.AssertReadCalled(2);

    asserter.AssertBlockCountSequence({ 14, 2 });
    asserter.AssertCommandAll(LMS64CProtocol::CMD_LMS7002_RD);
    asserter.AssertSubDeviceAll(0);

    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocol, LMS7002MSPINotFullyWritten)
{
    ISerialPortAsserter asserter{ {}, { 0 } };

    uint32_t mosi = 1;
    uint32_t miso = 2;

    int returnValue = LMS64CProtocol::LMS7002M_SPI(asserter, 0, &mosi, &miso, 1);

    asserter.AssertWriteCalled(1);
    asserter.AssertReadCalled(0);

    asserter.AssertBlockCountSequence({ 1 });
    asserter.AssertCommandAll(LMS64CProtocol::CMD_LMS7002_RD);
    asserter.AssertSubDeviceAll(0);

    EXPECT_EQ(returnValue, -1);
}

TEST(LMS64CProtocol, LMS7002MSPINotFullyRead)
{
    auto packetsToRead = std::vector<LMS64CPacket>();
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;
    packetsToRead.push_back(packet);

    ISerialPortAsserter asserter{ packetsToRead, { sizeof(LMS64CPacket) }, { 0 } };

    uint32_t mosi = 1;
    uint32_t miso = 2;

    int returnValue = LMS64CProtocol::LMS7002M_SPI(asserter, 0, &mosi, &miso, 1);

    asserter.AssertWriteCalled(1);
    asserter.AssertReadCalled(1);

    asserter.AssertBlockCountSequence({ 1 });
    asserter.AssertCommandAll(LMS64CProtocol::CMD_LMS7002_RD);
    asserter.AssertSubDeviceAll(0);

    EXPECT_EQ(returnValue, -1);
}

TEST(LMS64CProtocol, LMS7002MSPINotFullyWrittenOnSecondCall)
{
    auto packetsToRead = std::vector<LMS64CPacket>();
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;
    packetsToRead.push_back(packet);

    ISerialPortAsserter asserter{ packetsToRead, { sizeof(LMS64CPacket), 0 } };

    std::vector<uint32_t> mosi{ 1, 2 };
    SetWriteBit(mosi[1]);

    std::vector<uint32_t> miso{ 1, 2 };

    int returnValue = LMS64CProtocol::LMS7002M_SPI(asserter, 0, mosi.data(), miso.data(), 2);

    asserter.AssertWriteCalled(2);
    asserter.AssertReadCalled(1);

    asserter.AssertBlockCountSequence({ 1 });
    asserter.AssertCommandSequence({ LMS64CProtocol::CMD_LMS7002_RD, LMS64CProtocol::CMD_LMS7002_WR });
    asserter.AssertSubDeviceAll(0);

    EXPECT_EQ(returnValue, -1);
}

TEST(LMS64CProtocol, LMS7002MSPINotFullyReadOnSecondCall)
{
    auto packetsToRead = std::vector<LMS64CPacket>();
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;
    packetsToRead.push_back(packet);

    ISerialPortAsserter asserter{ packetsToRead, { sizeof(LMS64CPacket) }, { sizeof(LMS64CPacket), 0 } };

    std::vector<uint32_t> mosi{ 1, 2 };
    SetWriteBit(mosi[1]);

    std::vector<uint32_t> miso{ 1, 2 };

    int returnValue = LMS64CProtocol::LMS7002M_SPI(asserter, 0, mosi.data(), miso.data(), 2);

    asserter.AssertWriteCalled(2);
    asserter.AssertReadCalled(2);

    asserter.AssertBlockCountSequence({ 1 });
    asserter.AssertCommandSequence({ LMS64CProtocol::CMD_LMS7002_RD, LMS64CProtocol::CMD_LMS7002_WR });
    asserter.AssertSubDeviceAll(0);

    EXPECT_EQ(returnValue, -1);
}
