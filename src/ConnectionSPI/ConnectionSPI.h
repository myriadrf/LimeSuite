/**
    @file ConnectionSTREAM.h
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/

#pragma once
#include <ConnectionRegistry.h>
#include "LMS64CProtocol.h"
#include <vector>
#include <string>
#include <atomic>
#include <memory>
#include <thread>
#include "dataTypes.h"

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>

namespace lime{

class ConnectionSPI : public IConnection
{
public:
    ConnectionSPI(const unsigned index);
    ~ConnectionSPI(void);

    int Open(const unsigned index);
    void Close();
    bool IsOpen();
    int GetOpenedIndex();

    int WriteLMS7002MSPI(const uint32_t *writeData, size_t size, unsigned periphID) override;
    int ReadLMS7002MSPI(const uint32_t *writeData, uint32_t *readData, size_t size, unsigned periphID) override;
    int WriteRegisters(const uint32_t *addrs, const uint32_t *data, const size_t size) override;
    int ReadRegisters(const uint32_t *addrs, uint32_t *data, const size_t size) override;
    int CustomParameterRead(const uint8_t *ids, double *values, const size_t count, std::string* units) override;
    int CustomParameterWrite(const uint8_t *ids, const double *values, const size_t count, const std::string& units) override;
    DeviceInfo GetDeviceInfo(void) override;
    int DeviceReset(int ind) override;
protected:
    int GetBuffersCount() const override;
    int CheckStreamSize(int size) const override;
    
    int ReceiveData(char* buffer, int length, int epIndex, int timeout = 100) override;
    int SendData(const char* buffer, int length, int epIndex, int timeout = 100) override;
    
    int BeginDataReading(char* buffer, uint32_t length, int ep) override;
    bool WaitForReading(int contextHandle, unsigned int timeout_ms) override;
    int FinishDataReading(char* buffer, uint32_t length, int contextHandle) override;
    void AbortReading(int epIndex);

    int BeginDataSending(const char* buffer, uint32_t length, int ep) override;
    bool WaitForSending(int contextHandle, uint32_t timeout_ms) override;
    int FinishDataSending(const char* buffer, uint32_t length, int contextHandle) override;
    void AbortSending(int epIndex);

private:
    
    static int TransferSPI(int fd, const void *tx, void *rx, uint32_t len);
    static ConnectionSPI* pthis;
    static void SPIcallback();
    int fd_stream;
    int fd_stream_clocks;
    int fd_control_lms;
    int fd_control_fpga;
    int fd_control_dac;
    int dac_value;
    std::mutex mTxStreamLock;
    std::mutex mRxStreamLock;
    
    std::queue<FPGA_DataPacket> rxQueue;
    std::queue<FPGA_DataPacket> txQueue;  
    std::chrono::time_point<std::chrono::high_resolution_clock> last_int_time;
};

class ConnectionSPIEntry : public ConnectionRegistryEntry
{
public:
    ConnectionSPIEntry(void);

    ~ConnectionSPIEntry(void);

    std::vector<ConnectionHandle> enumerate(const ConnectionHandle &hint);

    IConnection *make(const ConnectionHandle &handle);

private:
};

}
