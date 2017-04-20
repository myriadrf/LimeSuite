/**
    @file ConnectionSTREAM.h
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/

#pragma once
#include <ConnectionRegistry.h>
#include <ILimeSDRStreaming.h>
#include <vector>
#include <string>
#include <atomic>
#include <memory>
#include <thread>
#include "fifo.h"

#ifndef __unix__
#include "windows.h"
#else
#include <mutex>
#include <condition_variable>
#include <chrono>
#endif

namespace lime{

class ConnectionXillybus : public ILimeSDRStreaming
{
public:
    ConnectionXillybus(const unsigned index);
    ~ConnectionXillybus(void);

    int Open(const unsigned index);
    void Close();
    bool IsOpen();
    int GetOpenedIndex();

    int Write(const unsigned char *buffer, int length, int timeout_ms = 100) override;
    int Read(unsigned char *buffer, int length, int timeout_ms = 100) override;

    //hooks to update FPGA plls when baseband interface data rate is changed
    int UpdateExternalDataRate(const size_t channel, const double txRate, const double rxRate) override;
    int UpdateExternalDataRate(const size_t channel, const double txRate, const double rxRate, const double txPhase, const double rxPhase)override;
    int ReadRawStreamData(char* buffer, unsigned length, int timeout_ms = 100)override;
    int TransferPacket(GenericPacket &pkt) override;
protected:
    virtual void ReceivePacketsLoop(const ThreadData args) override;
    virtual void TransmitPacketsLoop(const ThreadData args) override;

    virtual int ReceiveData(char* buffer, const int length, const int timeout = 100) override;
    virtual int SendData(const char* buffer, const int length, const int timeout = 100) override;
    virtual void AbortReading();
    virtual void AbortSending();

    int ConfigureFPGA_PLL(unsigned int pllIndex, const double interfaceClk_Hz, const double phaseShift_deg);
private:
    eConnectionType GetType(void)
    {
        return PCIE_PORT;
    }

    std::string m_hardwareName;
    int m_hardwareVer;

    bool isConnected;
    std::mutex mTransferLock;
#ifndef __unix__
    HANDLE hWrite;
    HANDLE hRead;
    HANDLE hWriteStream;
    HANDLE hReadStream;
#else
    int hWrite;
    int hRead;
    int hWriteStream;
    int hReadStream;
#endif
    std::string writeStreamPort;
    std::string readStreamPort;
};



class ConnectionXillybusEntry : public ConnectionRegistryEntry
{
public:
    ConnectionXillybusEntry(void);

    ~ConnectionXillybusEntry(void);

    std::vector<ConnectionHandle> enumerate(const ConnectionHandle &hint);

    IConnection *make(const ConnectionHandle &handle);

private:
    #ifndef __unix__
    std::string DeviceName(unsigned int index);
    #else
    #endif
};

}
