#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "tests/include/limesuite/ICommsMock.h"
#include "SlaveSelectShim.h"

using namespace lime;
using namespace lime::testing;

TEST(SlaveSelectShim, ChoosesCorrectSlaveOnSPICall)
{
    auto mockComms{ std::make_shared<ICommsMock>() };
    constexpr int slaveId{ 1 };

    SlaveSelectShim shim{ mockComms, slaveId };

    uint32_t MOSI = 2;
    uint32_t MISO = 3;
    uint32_t count = 4;

    EXPECT_CALL(*mockComms, SPI(slaveId, &MOSI, &MISO, count)).Times(1);
    shim.SPI(&MOSI, &MISO, count);
}

TEST(SlaveSelectShim, SPICalledCorrectly)
{
    auto mockComms{ std::make_shared<ICommsMock>() };
    constexpr int slaveId{ 1 };

    SlaveSelectShim shim{ mockComms, slaveId };

    uint32_t spiBusAddress = 2;
    uint32_t MOSI = 3;
    uint32_t MISO = 4;
    uint32_t count = 5;

    EXPECT_CALL(*mockComms, SPI(spiBusAddress, &MOSI, &MISO, count)).Times(1);
    shim.SPI(spiBusAddress, &MOSI, &MISO, count);
}

TEST(SlaveSelectShim, ResetsCorrectDevice)
{
    auto mockComms{ std::make_shared<ICommsMock>() };
    constexpr int slaveId{ 1 };

    SlaveSelectShim shim{ mockComms, slaveId };

    EXPECT_CALL(*mockComms, ResetDevice(slaveId)).Times(1);
    shim.ResetDevice();
}
