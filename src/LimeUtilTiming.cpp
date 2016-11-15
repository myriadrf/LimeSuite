/**
    @file LimeUtil.cpp
    @author Lime Microsystems
    @brief Command line test app
*/

#include <VersionInfo.h>
#include <ConnectionRegistry.h>
#include <IConnection.h>
#include <LMS7002M.h>
#include <iostream>
#include <chrono>

using namespace lime;

int deviceTestTiming(const std::string &argStr)
{
    auto handles = ConnectionRegistry::findConnections(argStr);
    if(handles.size() == 0)
    {
        std::cout << "No devices found" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Connected to [" << handles[0].serialize() << "]" << std::endl;
    auto conn = ConnectionRegistry::makeConnection(handles[0]);

    std::cout << "Creating instance of LMS7002M:" << std::endl;
    auto lms7 = new LMS7002M;
    lms7->SetConnection(conn);

    //time spi write access
    {
        const size_t numIters(1000);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->SPI_write(0x0000, 0x0000);
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << ">>> SPI write operation:\t" << (secsPerOp/1e-6) << " us" << std::endl;
    }

    //time spi read access
    {
        const size_t numIters(1000);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->SPI_read(0x0000, true);
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << ">>> SPI read operation: \t" << (secsPerOp/1e-6) << " us" << std::endl;
    }

    //time CGEN tuning
    {
        const size_t numIters(100);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->SetFrequencyCGEN(1e6*(100+i), false);
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << ">>> CGEN re-tune operation:  \t" << (secsPerOp/1e-3) << " ms" << std::endl;
    }

    //time LO tuning
    {
        const size_t numIters(100);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numIters; i++)
        {
            lms7->SetFrequencySX(LMS7002M::Tx, 1e6*(100+i));
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto secsPerOp = std::chrono::duration<double>(t1-t0).count()/numIters;
        std::cout << ">>> RF LO re-tune operation:\t" << (secsPerOp/1e-3) << " ms" << std::endl;
    }

    delete lms7;
    ConnectionRegistry::freeConnection(conn);
    return EXIT_SUCCESS;
}
