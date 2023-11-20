#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "LMS64CProtocol.h"

using namespace lime;

TEST(LMS64CProtocol, FirmwareToDescriptorSetsInfo)
{
    LMS64CProtocol::FirmwareInfo firmwareInfo{ 3, 3, 3, 3, 3, 3 };
    SDRDevice::Descriptor descriptor;

    LMS64CProtocol::FirmwareToDescriptor(firmwareInfo, descriptor);

    EXPECT_EQ(descriptor.name, "DigiRed");
    EXPECT_EQ(descriptor.expansionName, "Myriad1");
    EXPECT_EQ(descriptor.firmwareVersion, "3");
    EXPECT_EQ(descriptor.hardwareVersion, "3");
    EXPECT_EQ(descriptor.protocolVersion, "3");
    EXPECT_EQ(descriptor.serialNumber, 3);
}

TEST(LMS64CProtocol, FirmwareToDescriptorUnknownsLow)
{
    LMS64CProtocol::FirmwareInfo firmwareInfo{ 0, 0, 0, 0, 0, 0 };
    SDRDevice::Descriptor descriptor;

    LMS64CProtocol::FirmwareToDescriptor(firmwareInfo, descriptor);

    EXPECT_EQ(descriptor.name, "UNKNOWN");
    EXPECT_EQ(descriptor.expansionName, "UNKNOWN");
    EXPECT_EQ(descriptor.firmwareVersion, "0");
    EXPECT_EQ(descriptor.hardwareVersion, "0");
    EXPECT_EQ(descriptor.protocolVersion, "0");
    EXPECT_EQ(descriptor.serialNumber, 0);
}

TEST(LMS64CProtocol, FirmwareToDescriptorUnknownsHigh)
{
    LMS64CProtocol::FirmwareInfo firmwareInfo{ 2000000, 2000000, 2000000, 2000000, 2000000, 2000000 };
    SDRDevice::Descriptor descriptor;

    LMS64CProtocol::FirmwareToDescriptor(firmwareInfo, descriptor);

    EXPECT_EQ(descriptor.name, "Unknown (0x1E8480)");
    EXPECT_EQ(descriptor.expansionName, "Unknown (0x1E8480)");
    EXPECT_EQ(descriptor.firmwareVersion, "2000000");
    EXPECT_EQ(descriptor.hardwareVersion, "2000000");
    EXPECT_EQ(descriptor.protocolVersion, "2000000");
    EXPECT_EQ(descriptor.serialNumber, 2000000);
}
