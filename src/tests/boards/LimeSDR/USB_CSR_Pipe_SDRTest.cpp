#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "tests/comms/USB/FX3/FX3Mock.h"
#include "boards/LimeSDR/USB_CSR_Pipe_SDR.h"
#include "LMS64CProtocol.h"

using namespace lime;
using namespace lime::testing;
using ::testing::_;
using ::testing::Pointer;

TEST(USB_CSR_Pipe_SDR, WriteBulkTransfer)
{
    FX3Mock mockConnection{};
    USB_CSR_Pipe_SDR pipe{ mockConnection };

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GPIO_RD;

    int timeout = 100;
    int length = sizeof(LMS64CPacket);

    EXPECT_CALL(
        mockConnection, BulkTransfer(FX3::CONTROL_BULK_OUT_ADDRESS, Pointer(reinterpret_cast<uint8_t*>(&pkt)), length, timeout))
        .Times(1);
    pipe.Write(reinterpret_cast<uint8_t*>(&pkt), length, timeout);
}

TEST(USB_CSR_Pipe_SDR, WriteControlTransfer)
{
    FX3Mock mockConnection{};
    USB_CSR_Pipe_SDR pipe{ mockConnection };

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GET_INFO;

    int timeout = 100;
    int length = sizeof(LMS64CPacket);

#ifdef __unix__
    EXPECT_CALL(mockConnection,
        ControlTransfer(LIBUSB_REQUEST_TYPE_VENDOR,
            FX3::CTR_W_REQCODE,
            FX3::CTR_W_VALUE,
            FX3::CTR_W_INDEX,
            Pointer(reinterpret_cast<uint8_t*>(&pkt)),
            length,
            timeout))
        .Times(1);
#else
    EXPECT_CALL(mockConnection,
        ControlTransfer(
            0, FX3::CTR_W_REQCODE, FX3::CTR_W_VALUE, FX3::CTR_W_INDEX, Pointer(reinterpret_cast<uint8_t*>(&pkt)), length, timeout))
        .Times(1);
#endif

    pipe.Write(reinterpret_cast<uint8_t*>(&pkt), length, timeout);
}

TEST(USB_CSR_Pipe_SDR, ReadBulkTransfer)
{
    FX3Mock mockConnection{};
    USB_CSR_Pipe_SDR pipe{ mockConnection };

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GPIO_RD;

    int timeout = 100;
    int length = sizeof(LMS64CPacket);

    EXPECT_CALL(
        mockConnection, BulkTransfer(FX3::CONTROL_BULK_IN_ADDRESS, Pointer(reinterpret_cast<uint8_t*>(&pkt)), length, timeout))
        .Times(1);
    pipe.Read(reinterpret_cast<uint8_t*>(&pkt), length, timeout);
}

TEST(USB_CSR_Pipe_SDR, ReadControlTransfer)
{
    FX3Mock mockConnection{};
    USB_CSR_Pipe_SDR pipe{ mockConnection };

    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_GET_INFO;

    int timeout = 100;
    int length = sizeof(LMS64CPacket);

#ifdef __unix__
    EXPECT_CALL(mockConnection,
        ControlTransfer(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
            FX3::CTR_R_REQCODE,
            FX3::CTR_R_VALUE,
            FX3::CTR_R_INDEX,
            Pointer(reinterpret_cast<uint8_t*>(&pkt)),
            length,
            timeout))
        .Times(1);
#else
    EXPECT_CALL(mockConnection,
        ControlTransfer(
            1, FX3::CTR_R_REQCODE, FX3::CTR_R_VALUE, FX3::CTR_R_INDEX, Pointer(reinterpret_cast<uint8_t*>(&pkt)), length, timeout))
        .Times(1);
#endif
    pipe.Read(reinterpret_cast<uint8_t*>(&pkt), length, timeout);
}
