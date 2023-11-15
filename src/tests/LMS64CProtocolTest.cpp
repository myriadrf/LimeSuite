#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "asserters/ISerialPortAsserter.h"
#include "LMS64CProtocol.h"

using namespace lime;
using namespace lime::testing;

TEST(LMS64CProtocolTest, LMS7002MSPIEmptyTest)
{
    ISerialPortAsserter asserter{ std::vector<LMS64CPacket>() };

    int returnValue = LMS64CProtocol::LMS7002M_SPI(asserter, 0, nullptr, nullptr, 0);

    asserter.AssertWriteCalled(0);
    asserter.AssertReadCalled(0);
    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocolTest, LMS7002MSPIOneCountTestValueRead)
{
    auto packetsToRead = std::vector<LMS64CPacket>();
    LMS64CPacket packet {};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD ;
    packetsToRead.push_back(packet);
    ISerialPortAsserter asserter{ packetsToRead };

    uint32_t mosi = 1;
    uint32_t miso = 2;
    int returnValue = LMS64CProtocol::LMS7002M_SPI(asserter, 0, &mosi, &miso, 1);

    asserter.AssertWriteCalled(1);
    asserter.AssertBlockCount(1, 0);
    asserter.AssertCommand(LMS64CProtocol::CMD_LMS7002_RD, 0);
    asserter.AssertSubDevice(0, 0);

    asserter.AssertReadCalled(1);
    EXPECT_EQ(returnValue, 0);
}

TEST(LMS64CProtocolTest, LMS7002MSPIOneCountTestValueWrite)
{
    auto packetsToRead = std::vector<LMS64CPacket>();
    LMS64CPacket packet {};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD ;
    packetsToRead.push_back(packet);
    ISerialPortAsserter asserter{ packetsToRead };

    uint32_t mosi = 1;
    mosi |= (1 << 31); // Set the write bit
    uint32_t miso = 2;
    int returnValue = LMS64CProtocol::LMS7002M_SPI(asserter, 0, &mosi, &miso, 1);

    asserter.AssertWriteCalled(1);
    asserter.AssertBlockCount(1, 0);
    asserter.AssertCommand(LMS64CProtocol::CMD_LMS7002_WR, 0);
    asserter.AssertSubDevice(0, 0);

    asserter.AssertReadCalled(1);
    EXPECT_EQ(returnValue, 0);
}