#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "USBCommon.h"

#include "USB_CSR_Pipe_SDR.h"
#include "FX3_Mock.h"

using namespace lime;
using ::testing::_;

TEST(USB_CSR_Pipe_SDR, WriteTestBulkTransfer)
{
    FX3_Mock mockConnection{};
    EXPECT_CALL(mockConnection, BulkTransfer(_, _, _, _)).Times(1);

    USB_CSR_Pipe_SDR pipe{ mockConnection };

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GPIO_RD;
    pipe.Write((uint8_t*)&pkt, sizeof(pkt), 100);
}

TEST(USB_CSR_Pipe_SDR, WriteTestControlTransfer)
{
    FX3_Mock mockConnection{};
    EXPECT_CALL(mockConnection, ControlTransfer(_, _, _, _, _, _, _)).Times(1);

    USB_CSR_Pipe_SDR pipe{ mockConnection };

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GET_INFO;
    pipe.Write((uint8_t*)&pkt, sizeof(pkt), 100);
}
