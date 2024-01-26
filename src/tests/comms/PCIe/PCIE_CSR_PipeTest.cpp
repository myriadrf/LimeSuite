#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "LitePCIeMock.h"
#include "PCIeCommon.h"
#include "LMS64CProtocol.h"

using namespace lime;
using namespace lime::testing;
using ::testing::Pointer;

TEST(PCIE_CSR_Pipe, Write)
{
    auto mockConnection = std::make_shared<LitePCIeMock>();
    PCIE_CSR_Pipe pipe{ mockConnection };

    LMS64CPacket pkt;
    int length = sizeof(LMS64CPacket);
    int timeout = 100;

    EXPECT_CALL(*mockConnection, WriteControl(Pointer(reinterpret_cast<std::byte*>(&pkt)), length, timeout)).Times(1);
    pipe.Write(reinterpret_cast<std::byte*>(&pkt), length, timeout);
}

TEST(PCIE_CSR_Pipe, Read)
{
    auto mockConnection = std::make_shared<LitePCIeMock>();
    PCIE_CSR_Pipe pipe{ mockConnection };

    LMS64CPacket pkt;
    int length = sizeof(LMS64CPacket);
    int timeout = 100;

    EXPECT_CALL(*mockConnection, ReadControl(Pointer(reinterpret_cast<std::byte*>(&pkt)), length, timeout)).Times(1);
    pipe.Read(reinterpret_cast<std::byte*>(&pkt), length, timeout);
}
