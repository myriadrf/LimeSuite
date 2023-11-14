#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "USBCommon.h"

#include "USB_CSR_Pipe_SDR.h"
#include "FX3/FX3.h"

using namespace lime;

TEST(Success, Successful)
{
    EXPECT_TRUE(true);
}

TEST(USB_CSR_Pipe_SDR, Successful)
{
    FX3 connection{};
    USB_CSR_Pipe_SDR pipe{connection};
}
