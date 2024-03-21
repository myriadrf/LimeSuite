#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "tests/comms/USB/FT601/FT601Mock.h"
#include "boards/LimeSDR_Mini/USB_CSR_Pipe_Mini.h"
#include "LMS64CProtocol.h"

using namespace lime;
using namespace lime::testing;
using ::testing::Pointer;

static const int CONTROL_BULK_WRITE_ADDRESS = 0x02;
static const int CONTROL_BULK_READ_ADDRESS = 0x82;

TEST(USB_CSR_Pipe_Mini, Write)
{
    FT601Mock mockConnection{};
    USB_CSR_Pipe_Mini pipe{ mockConnection };

    LMS64CPacket pkt;

    int timeout = 100;
    int length = sizeof(LMS64CPacket);

    EXPECT_CALL(
        mockConnection, BulkTransfer(CONTROL_BULK_WRITE_ADDRESS, Pointer(reinterpret_cast<uint8_t*>(&pkt)), length, timeout))
        .Times(1);
    pipe.Write(reinterpret_cast<uint8_t*>(&pkt), length, timeout);
}

TEST(USB_CSR_Pipe_Mini, Read)
{
    FT601Mock mockConnection{};
    USB_CSR_Pipe_Mini pipe{ mockConnection };

    LMS64CPacket pkt;

    int timeout = 100;
    int length = sizeof(LMS64CPacket);

    EXPECT_CALL(mockConnection, BulkTransfer(CONTROL_BULK_READ_ADDRESS, Pointer(reinterpret_cast<uint8_t*>(&pkt)), length, timeout))
        .Times(1);
    pipe.Read(reinterpret_cast<uint8_t*>(&pkt), length, timeout);
}
