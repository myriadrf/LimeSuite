/**
    @file ConnectionRemote.h
    @author Lime Microsystems
    @brief Implementation of EVB7 connection of serial COM port.
*/

#pragma once
#include <ConnectionRegistry.h>
#include <IConnection.h>
#include <vector>
#include <string>
#include <mutex>
#include "IConnection.h"
#include "LMS64CCommands.h"

namespace lime{

class ConnectionRemote : public IConnection
{
public:
    ConnectionRemote(const char *ipString, uint16_t port);
    ~ConnectionRemote(void);
    virtual bool IsOpen(void) override;

    virtual DeviceInfo GetDeviceInfo(void);

    virtual int TransactSPI(const int addr, const uint32_t *writeData, uint32_t *readData, const size_t size) override;

    virtual int WriteLMS7002MSPI(const uint32_t *writeData, size_t size,unsigned periphID = 0) override;
    virtual int ReadLMS7002MSPI(const uint32_t *writeData, uint32_t *readData, size_t size, unsigned periphID = 0) override;

    virtual int WriteRegisters(const uint32_t *addrs, const uint32_t *data, const size_t size) override;
    virtual int ReadRegisters(const uint32_t *addrs, uint32_t *data, const size_t size) override;

    virtual int WriteI2C(const int addr, const std::string &data) override;
    virtual int ReadI2C(const int addr, const size_t numBytes, std::string &data) override;
    virtual int DeviceReset(int ind=0) override;
    //virtual int ProgramWrite(const char *buffer, const size_t length, const int programmingMode, const int index, ProgrammingCallback callback = 0) override;
    
    virtual int GPIOWrite(const uint8_t *buffer, const size_t bufLength) override;
    virtual int GPIORead(uint8_t *buffer, const size_t bufLength) override;
    virtual int GPIODirWrite(const uint8_t *buffer, const size_t bufLength) override;
    virtual int GPIODirRead(uint8_t *buffer, const size_t bufLength) override;

    // Network commands don't have to match LMS64C commands, just using them for convenience
    enum NetworkCommand
    {
        NETWORK_GET_INFO = 0,
        NETWORK_RESET_DEVICE = CMD_LMS7002_RST,
        NETWORK_I2C_WR = CMD_SI5351_WR,
        NETWORK_I2C_RD = CMD_SI5351_RD,
        NETWORK_LMS7002_WR = CMD_LMS7002_WR,
        NETWORK_LMS7002_RD = CMD_LMS7002_RD,
        NETWORK_BRDSPI_WR = CMD_BRDSPI_WR,
        NETWORK_BRDSPI_RD = CMD_BRDSPI_RD
    };

    struct CtrlPacketHeader
    {
        uint8_t cmd;
        uint8_t status;
        uint8_t periphID;
        uint16_t payloadLen;
    };

protected:
    int Write(const unsigned char *data, int len, int timeout_ms);
    int Read(unsigned char *response, int len, int timeout_ms);

    uint16_t m_TargetPort;
    int m_ControlSocketFd;
    int Connect(const char* ip, uint16_t port);
private:
    int Open();
    void Close(void);
    std::mutex mTransferLock;
    std::string targetIP;
};

class ConnectionRemoteEntry : public ConnectionRegistryEntry
{
public:
    ConnectionRemoteEntry(void);
    std::vector<ConnectionHandle> enumerate(const ConnectionHandle &hint);
    IConnection *make(const ConnectionHandle &handle);
};

}
