#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "FX3/FX3Mock.h"
#include "USB_CSR_Pipe_SDR.h"
#include "LMS64CProtocol.h"

using namespace lime;
using namespace lime::testing;
using ::testing::_;
using ::testing::NiceMock;

TEST(USB_CSR_Pipe_SDR, WriteBulkTransfer)
{
    NiceMock<FX3Mock> mockConnection{};
    USB_CSR_Pipe_SDR pipe{ mockConnection };

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GPIO_RD;

    EXPECT_CALL(mockConnection, BulkTransfer(_, _, _, _)).Times(1);
    pipe.Write((uint8_t*)&pkt, sizeof(pkt), 100);
}

TEST(USB_CSR_Pipe_SDR, WriteControlTransfer)
{
    NiceMock<FX3Mock> mockConnection{};
    USB_CSR_Pipe_SDR pipe{ mockConnection };

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GET_INFO;

    EXPECT_CALL(mockConnection, ControlTransfer(_, _, _, _, _, _, _)).Times(1);
    pipe.Write((uint8_t*)&pkt, sizeof(pkt), 100);
}

TEST(USB_CSR_Pipe_SDR, ReadBulkTransfer)
{
    NiceMock<FX3Mock> mockConnection{};
    USB_CSR_Pipe_SDR pipe{ mockConnection };

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GPIO_RD;

    EXPECT_CALL(mockConnection, BulkTransfer(_, _, _, _)).Times(1);
    pipe.Read((uint8_t*)&pkt, sizeof(pkt), 100);
}

TEST(USB_CSR_Pipe_SDR, ReadControlTransfer)
{
    NiceMock<FX3Mock> mockConnection{};
    USB_CSR_Pipe_SDR pipe{ mockConnection };

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GET_INFO;

    EXPECT_CALL(mockConnection, ControlTransfer(_, _, _, _, _, _, _)).Times(1);
    pipe.Read((uint8_t*)&pkt, sizeof(pkt), 100);
}
