#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "tests/comms/USB/FX3/FX3Mock.h"
#include "LimeSDR.h"
#include "LMS64C_LMS7002M_Over_USB.h"
#include "LMS64C_FPGA_Over_USB.h"
#include "tests/comms/USB/USB_CSR_PipeMock.h"

using namespace lime;
using namespace lime::testing;
using ::testing::_;
using ::testing::DoAll;
using ::testing::ReturnArg;
using ::testing::SetArrayArgument;

TEST(LimeSDR, Constructor)
{
    auto usbCommsMock{ std::make_shared<FX3Mock>() };
    auto usbPipeMock{ std::make_shared<USB_CSR_PipeMock>() };
    auto route_lms7002m{ std::make_shared<LMS64C_LMS7002M_Over_USB>(usbPipeMock) };
    auto route_fpga{ std::make_shared<LMS64C_FPGA_Over_USB>(usbPipeMock) };

    LMS64CPacket packet{};
    packet.status = LMS64CProtocol::STATUS_COMPLETED_CMD;

    ON_CALL(*usbPipeMock, Read(_, sizeof(LMS64CPacket), _))
        .WillByDefault(DoAll(
            SetArrayArgument<0>(reinterpret_cast<uint8_t*>(&packet), reinterpret_cast<uint8_t*>(&packet + 1)), ReturnArg<1>()));

    EXPECT_CALL(*usbPipeMock, Write(_, sizeof(LMS64CPacket), _)).Times(4);
    EXPECT_CALL(*usbPipeMock, Read(_, sizeof(LMS64CPacket), _)).Times(4);

    auto usbPipe = std::static_pointer_cast<USB_CSR_Pipe>(usbPipeMock);

    LimeSDR(route_lms7002m, route_fpga, usbCommsMock, usbPipe);
}
