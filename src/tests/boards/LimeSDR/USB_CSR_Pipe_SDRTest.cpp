#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "tests/comms/USB/FX3/FX3Mock.h"
#include "USB_CSR_Pipe_SDR.h"
#include "LMS64CProtocol.h"

using namespace lime;
using namespace lime::testing;
using ::testing::_;
using ::testing::Pointer;

static const int CTR_W_REQCODE = 0xC1;
static const int CTR_W_VALUE = 0x0000;
static const int CTR_W_INDEX = 0x0000;

static const int CTR_R_REQCODE = 0xC0;
static const int CTR_R_VALUE = 0x0000;
static const int CTR_R_INDEX = 0x0000;

static const uint8_t CONTROL_BULK_OUT_ADDRESS = 0x0F;
static const uint8_t CONTROL_BULK_IN_ADDRESS = 0x8F;

TEST(USB_CSR_Pipe_SDR, WriteBulkTransfer)
{
    FX3Mock mockConnection{};
    USB_CSR_Pipe_SDR pipe{ mockConnection };

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GPIO_RD;

    int timeout = 100;
    int length = sizeof(LMS64CPacket);

    EXPECT_CALL(
        mockConnection, BulkTransfer(CONTROL_BULK_OUT_ADDRESS, Pointer(reinterpret_cast<std::byte*>(&pkt)), length, timeout))
        .Times(1);
    pipe.Write(reinterpret_cast<std::byte*>(&pkt), length, timeout);
}

TEST(USB_CSR_Pipe_SDR, WriteControlTransfer)
{
    FX3Mock mockConnection{};
    USB_CSR_Pipe_SDR pipe{ mockConnection };

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GET_INFO;

    int timeout = 100;
    int length = sizeof(LMS64CPacket);

    EXPECT_CALL(mockConnection,
        ControlTransfer(LIBUSB_REQUEST_TYPE_VENDOR,
            CTR_W_REQCODE,
            CTR_W_VALUE,
            CTR_W_INDEX,
            Pointer(reinterpret_cast<std::byte*>(&pkt)),
            length,
            timeout))
        .Times(1);
    pipe.Write(reinterpret_cast<std::byte*>(&pkt), length, timeout);
}

TEST(USB_CSR_Pipe_SDR, ReadBulkTransfer)
{
    FX3Mock mockConnection{};
    USB_CSR_Pipe_SDR pipe{ mockConnection };

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GPIO_RD;

    int timeout = 100;
    int length = sizeof(LMS64CPacket);

    EXPECT_CALL(mockConnection, BulkTransfer(CONTROL_BULK_IN_ADDRESS, Pointer(reinterpret_cast<std::byte*>(&pkt)), length, timeout))
        .Times(1);
    pipe.Read(reinterpret_cast<std::byte*>(&pkt), length, timeout);
}

TEST(USB_CSR_Pipe_SDR, ReadControlTransfer)
{
    FX3Mock mockConnection{};
    USB_CSR_Pipe_SDR pipe{ mockConnection };

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GET_INFO;

    int timeout = 100;
    int length = sizeof(LMS64CPacket);

    EXPECT_CALL(mockConnection,
        ControlTransfer(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
            CTR_R_REQCODE,
            CTR_R_VALUE,
            CTR_R_INDEX,
            Pointer(reinterpret_cast<std::byte*>(&pkt)),
            length,
            timeout))
        .Times(1);
    pipe.Read(reinterpret_cast<std::byte*>(&pkt), length, timeout);
}
