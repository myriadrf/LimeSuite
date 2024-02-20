#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "tests/include/limesuite/CommsMock.h"
#include "LimeSDR_X3/SlaveSelectShim.h"

using namespace lime;
using namespace lime::testing;

TEST(SlaveSelectShim, ChoosesCorrectSlaveOnSPICall)
{
    auto mockComms = std::make_shared<CommsMock>();
    uint32_t slaveId = 4U;

    SlaveSelectShim shim{ mockComms, slaveId };

    uint32_t MOSI = 3U;
    uint32_t MISO = 2U;
    uint32_t count = 1U;

    EXPECT_CALL(*mockComms, SPI(slaveId, &MOSI, &MISO, count)).Times(1);
    shim.SPI(&MOSI, &MISO, count);
}

TEST(SlaveSelectShim, SPICalledCorrectly)
{
    auto mockComms = std::make_shared<CommsMock>();
    uint32_t slaveId = 5U;

    SlaveSelectShim shim{ mockComms, slaveId };

    uint32_t spiBusAddress = 4U;
    uint32_t MOSI = 3U;
    uint32_t MISO = 2U;
    uint32_t count = 1U;

    EXPECT_CALL(*mockComms, SPI(spiBusAddress, &MOSI, &MISO, count)).Times(1);
    shim.SPI(spiBusAddress, &MOSI, &MISO, count);
}

TEST(SlaveSelectShim, ResetsCorrectDevice)
{
    auto mockComms = std::make_shared<CommsMock>();
    uint32_t slaveId = 1U;

    SlaveSelectShim shim{ mockComms, slaveId };

    EXPECT_CALL(*mockComms, ResetDevice(slaveId)).Times(1);
    shim.ResetDevice();
}
