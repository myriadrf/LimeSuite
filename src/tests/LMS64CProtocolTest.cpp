#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "asserters/ISerialPortAsserter.h"
#include "LMS64CProtocol.h"

using namespace lime;
using namespace lime::testing;

TEST(LMS64CProtocolTest, LMS7002MSPIEmptyTest)
{
    ISerialPortAsserter asserter{ std::vector<LMS64CPacket>(), ReturnValue::Expected, sizeof(LMS64CPacket) };

    LMS64CProtocol::LMS7002M_SPI(asserter, 0, nullptr, nullptr, 0);

    asserter.AssertWriteCalled(0);
    asserter.AssertReadCalled(0);
}

TEST(LMS64CProtocolTest, LMS7002MSPIOneCountTest)
{
    ISerialPortAsserter asserter{ std::vector<LMS64CPacket>(), ReturnValue::Expected, sizeof(LMS64CPacket) };

    uint32_t mosi = 0;
    uint32_t miso = 0;
    LMS64CProtocol::LMS7002M_SPI(asserter, 0, &mosi, &miso, 1);

    asserter.AssertWriteCalled(1);
    asserter.AssertBlockCount(1, 0);
    asserter.AssertCommand(LMS64CProtocol::CMD_LMS7002_RD, 0);
    asserter.AssertSubDevice(0, 0);

    asserter.AssertReadCalled(1);
}