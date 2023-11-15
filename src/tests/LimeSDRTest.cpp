#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "asserters/ISerialPortAsserter.h"
#include "asserters/USB_CSR_PipeAsserter.h"
#include "mocks/FX3Mock.h"
#include "LimeSDR.h"
#include "USB_CSR_Pipe_SDR.h"

using namespace lime;
using namespace lime::testing;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

TEST(LimeSDR, Constructor)
{
    auto packetsToRead = std::vector<LMS64CPacket>();
    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;
    packetsToRead.push_back(packet);

    std::shared_ptr<FX3Mock> usbComms{ new FX3Mock() };
    EXPECT_CALL(*usbComms, BulkTransfer(_, _, sizeof(LMS64CPacket), _)).WillRepeatedly(Return(sizeof(LMS64CPacket)));

    std::shared_ptr<USB_CSR_Pipe> usbPipe{ new USB_CSR_PipeAsserter{ packetsToRead } };
    std::shared_ptr<IComms> route_lms7002m{ new LMS64C_LMS7002M_Over_USB(usbPipe) };
    std::shared_ptr<IComms> route_fpga{ new LMS64C_FPGA_Over_USB(usbPipe) };

    auto asserter = std::static_pointer_cast<USB_CSR_PipeAsserter>(usbPipe);
    asserter->SetWillRepeatLastWrittenBlockCount(true);

    LimeSDR(route_lms7002m, route_fpga, usbComms, usbPipe);

    asserter->AssertWriteCalled(4);
    asserter->AssertReadCalled(4);
}