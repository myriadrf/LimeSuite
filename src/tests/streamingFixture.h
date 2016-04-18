
#ifndef STREAMING_FIXTURE_H
#define STREAMING_FIXTURE_H
#include "gtest/gtest.h"

#include "IConnection.h"
#include <ConnectionRegistry.h>

class StreamingFixture : public ::testing::Test
{
public:
    StreamingFixture( ) : serPort(nullptr), initialized(false)
    {
        auto cachedHandles = lime::ConnectionRegistry::findConnections();
        if(cachedHandles.size() > 0)
            serPort = lime::ConnectionRegistry::makeConnection(cachedHandles.at(0));
        if(serPort == nullptr)
            return;
        if (serPort != nullptr && !serPort->IsOpen())
        {
            lime::ConnectionRegistry::freeConnection(serPort);
            printf("failed to open LMS7 control device");
            initialized = false;
        }
        else
        {
            initialized = true;
            auto info = serPort->GetDeviceInfo();
            printf("Running tests with %s FW:%s HW:%s\n", info.deviceName.c_str(), info.firmwareVersion.c_str(), info.hardwareVersion.c_str());
        }
    }

    void SetUp( )
    {
        ASSERT_EQ(true, initialized);
        ASSERT_EQ(true, serPort->IsOpen());
        // code here will execute just before the test ensues
    }

    void TearDown( )
    {
        // code here will be called just after the test completes
        // ok to through exceptions from here if need be
    }

    ~StreamingFixture( )
    {
       lime::ConnectionRegistry::freeConnection(serPort);
    }

    lime::IConnection *serPort;
    bool initialized;
};

#endif
