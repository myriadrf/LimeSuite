#include "gtest/gtest.h"
#include <chrono>

#include "IConnection.h"
#include "ConnectionRegistry.h"

using namespace std;
using namespace lime;

TEST(Comms, TransferSPI)
{
    // TODO get chip address dynamically
    const uint8_t chipAddr = 0x10;
    auto handles = ConnectionRegistry::findConnections();
    ASSERT_NE(handles.size(), 0);
    auto conn = ConnectionRegistry::makeConnection(handles[0]);

    std::vector<uint32_t> addr (14, 0x002f << 16);
    std::vector<uint32_t> data (14, 0);

    int status = 0;
    const int tryCount = 1000;
    printf("Try count = %i\n", tryCount);
    float AvgResult = 0;
    auto startT = std::chrono::high_resolution_clock::now();
    for(int i=0; i<tryCount ; ++i)
        conn->GetDeviceInfo();
    auto endT = std::chrono::high_resolution_clock::now();
    AvgResult = chrono::duration_cast<chrono::milliseconds>(endT-startT).count()/float(tryCount);
    printf("Avg GetInfo: %.3f ms\n", AvgResult);

    //generates packet without SPI write/read
    startT = std::chrono::high_resolution_clock::now();
    for(int i=0; i<tryCount && status == 0; ++i)
        status = conn->TransactSPI(chipAddr, addr.data(), data.data(), 0);
    endT = std::chrono::high_resolution_clock::now();
    AvgResult = chrono::duration_cast<chrono::milliseconds>(endT-startT).count()/float(tryCount);
    printf("Avg communication roundtrip: %.3f ms\n", AvgResult);

    startT = std::chrono::high_resolution_clock::now();
    for(int i=0; i<tryCount && status == 0; ++i)
        status = conn->TransactSPI(chipAddr, addr.data(), data.data(), 1);
    endT = std::chrono::high_resolution_clock::now();
    AvgResult = chrono::duration_cast<chrono::milliseconds>(endT-startT).count()/float(tryCount);
    printf("Avg Single register read: %.3f ms\n", AvgResult);

    startT = std::chrono::high_resolution_clock::now();
    for(int i=0; i<tryCount && status == 0; ++i)
        status = conn->TransactSPI(chipAddr, addr.data(), data.data(), addr.size());
    endT = std::chrono::high_resolution_clock::now();
    AvgResult = chrono::duration_cast<chrono::milliseconds>(endT-startT).count()/float(tryCount);
    printf("Avg Single batch read(%li registers): %.3f ms, \n", addr.size(), AvgResult);

    startT = std::chrono::high_resolution_clock::now();
    for(int i=0; i<tryCount && status == 0; ++i)
        status = conn->ReadRegisters(addr.data(), data.data(), 0);
    endT = std::chrono::high_resolution_clock::now();
    AvgResult = chrono::duration_cast<chrono::milliseconds>(endT-startT).count()/float(tryCount);
    printf("FPGA Avg Single register read: %.3f ms\n", AvgResult);

    startT = std::chrono::high_resolution_clock::now();
    for(int i=0; i<tryCount && status == 0; ++i)
        status = conn->ReadRegisters(addr.data(), data.data(), addr.size());
    endT = std::chrono::high_resolution_clock::now();
    AvgResult = chrono::duration_cast<chrono::milliseconds>(endT-startT).count()/float(tryCount);
    printf("FPGA Avg Single batch read(%li registers): %.3f ms\n", addr.size(), AvgResult);

    ConnectionRegistry::freeConnection(conn);
}
