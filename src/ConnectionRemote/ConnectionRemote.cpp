// On some systems USB device can only be used by single application at a time.
// Remote connection is ONLY for debugging purposes to allow to forward communications
// from external application to inspect and adjust board configuration at runtime.
// It may cause performance issues (github #263)

#include "ConnectionRemote.h"
#include <string>
#include "string.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <chrono>
#include "FPGA_common.h"
#include "Logger.h"
#include "LMS64CProtocol.h"

#ifdef __unix__
    #include <errno.h>
    #include <unistd.h>
    #include <termios.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <stdlib.h>

    #include <termios.h>
    #include <unistd.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
#else
    #include <Winsock2.h>
#endif // LINUX

#if defined(__FreeBSD__)
    #include <sys/socket.h>
#endif

using namespace std;
using namespace lime;

static const int SOCKET_INVALID = -1;

ConnectionRemote::ConnectionRemote(const char *ipString, uint16_t port) :
    m_TargetPort(port)
{
    targetIP = std::string(ipString);
    m_ControlSocketFd = -1;
#ifndef __unix__
    WSADATA wsaData;
    if( int err = WSAStartup(0x0202, &wsaData))
        lime::log(lime::LOG_LEVEL_ERROR, "RemoteControl: WSAStartup error %i\n", err);
#endif
    Connect(ipString, port);
}

ConnectionRemote::~ConnectionRemote(void)
{
    Close();
#ifndef __unix__
    WSACleanup();
#endif
}

void ConnectionRemote::Close(void)
{
    if(m_ControlSocketFd)
    {
#ifndef __unix__
        //shutdown(m_ControlSocketFd, SD_BOTH);
        closesocket(m_ControlSocketFd);
#else
        shutdown(m_ControlSocketFd, SHUT_RDWR);
        close(m_ControlSocketFd);
#endif
    }
    m_ControlSocketFd = SOCKET_INVALID;
}

bool ConnectionRemote::IsOpen(void)
{
    return m_ControlSocketFd != SOCKET_INVALID;
}

int ConnectionRemote::Open()
{
    if (m_ControlSocketFd < 0)
        return Connect(targetIP.c_str(), m_TargetPort);
    return 0;
}

DeviceInfo ConnectionRemote::GetDeviceInfo(void)
{
    DeviceInfo info;
    CtrlPacketHeader pkt;
    pkt.cmd = CMD_GET_INFO;
    pkt.payloadLen = 0;

    const int timeout_ms = 2000;
    if( Write( (uint8_t*)&pkt, sizeof(pkt), timeout_ms) != sizeof(pkt))
        return info;

    // Read back status
    memset(&pkt, 0, sizeof(pkt));
    int bytesReceived = Read((uint8_t*)&pkt, sizeof(pkt), timeout_ms);
    if(bytesReceived != sizeof(CtrlPacketHeader))
        return info;

    if(pkt.payloadLen > 0)
    {
        std::vector<char> data(pkt.payloadLen);
        // Read back rest of payload
        bytesReceived = Read((uint8_t*)data.data(), pkt.payloadLen, timeout_ms);
        if(bytesReceived != pkt.payloadLen)
            return info;

        //parse strings
        char* start = data.data();
        info.deviceName = std::string(start);
        start += info.deviceName.length()+1;
        info.expansionName = std::string(start);
        start += info.expansionName.length()+1;
        info.firmwareVersion = std::string(start);
        start += info.firmwareVersion.length()+1;
        info.gatewareVersion = std::string(start);
        start += info.gatewareVersion.length()+1;
        info.gatewareRevision = std::string(start);
        start += info.gatewareRevision.length()+1;
        info.gatewareTargetBoard = std::string(start);
        start += info.gatewareTargetBoard.length()+1;
        info.hardwareVersion = std::string(start);
        start += info.hardwareVersion.length()+1;
        info.protocolVersion = std::string(start);
        start += info.hardwareVersion.length()+1;
        memcpy(&info.boardSerialNumber, start, sizeof(info.boardSerialNumber));
    }
    return info;
}

int ConnectionRemote::Connect(const char* ip, uint16_t port)
{
    struct sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(ip);
    clientService.sin_port = htons(port);

    m_ControlSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_ControlSocketFd < 0)
    {
#ifndef __unix__
        lime::log(lime::LOG_LEVEL_ERROR, "RemoteControl: control socket failed with error: %d\n", WSAGetLastError());
#else
        lime::log(lime::LOG_LEVEL_ERROR, "RemoteControl: control socket failed with error: %i\n", m_ControlSocketFd);
#endif
        return -1;
    }

    if(int result = connect(m_ControlSocketFd, (struct sockaddr*)&clientService, sizeof(clientService)) )
    {
#ifndef __unix__
        lime::log(lime::LOG_LEVEL_ERROR, "RemoteControl: control socket connect failed with error: %d\n", WSAGetLastError());
#else
        lime::log(lime::LOG_LEVEL_ERROR, "RemoteControl: control socket connect failed with error: %d\n", result);
#endif
        Close();
        return -1;
    }
    return 0;
}

int ConnectionRemote::Write(const unsigned char *data, int len, int timeout_ms)
{
    int bytesWritten = 0;
    while(bytesWritten < len)
    {
        int wrBytes = send(m_ControlSocketFd, (char*)data+bytesWritten, len-bytesWritten, 0);
        if(wrBytes < 0)
        {
            lime::log(lime::LOG_LEVEL_ERROR, "ConnectionRemote write failed: %d\n", wrBytes);
            return 0;
        }
        bytesWritten += wrBytes;
        printf("Socket send %i/%i\n", bytesWritten, len);
        std::this_thread::yield();
    }
    return bytesWritten;
}

int ConnectionRemote::Read(unsigned char *response, int len, int timeout_ms)
{
    int bytesRead = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    while(bytesRead < len)
    {
        int rdBytes = recv(m_ControlSocketFd, (char*)response+bytesRead, len-bytesRead, MSG_WAITALL);
        if(rdBytes < 0)
        {
            lime::log(lime::LOG_LEVEL_ERROR, "ConnectionRemote read failed: %d\n", rdBytes);
            return 0;
        }
        bytesRead += rdBytes;
        printf("Socket read %i/%i\n", bytesRead, len);
        if(rdBytes == 0 && (t1 - std::chrono::high_resolution_clock::now()) > std::chrono::milliseconds(timeout_ms))
        {
            printf("Read timeout\n");
            return bytesRead;
        }
        std::this_thread::yield();
    }
    return bytesRead;
}

int ConnectionRemote::TransactSPI(const int addr, const uint32_t *writeData, uint32_t *readData, const size_t size)
{
    //! TODO
    //! For multi-LMS7002M, RFIC # could be encoded with the slave number
    //! And the index would need to be encoded into the packet as well

    if (!IsOpen())
    {
        ReportError(ENOTCONN, "connection is not open");
        return -1;
    }

    //perform spi writes when there is no read data
    if (readData == nullptr)
        switch(addr)
        {
            case lime::LMS64CProtocol::SPI_Index::LMS7002M_SPI_INDEX:
                return this->WriteLMS7002MSPI(writeData, size);
            // case lime::LMS64CProtocol::SPI_Index::ADF4002_SPI_INDEX:
            //     return this->WriteADF4002SPI(writeData, size);
        }

    //otherwise perform reads into the provided buffer
    if (readData != nullptr)
        switch(addr)
        {
            case lime::LMS64CProtocol::SPI_Index::LMS7002M_SPI_INDEX:
                return this->ReadLMS7002MSPI(writeData, readData, size);
            // case lime::LMS64CProtocol::SPI_Index::ADF4002_SPI_INDEX:
            //     return this->ReadADF4002SPI(writeData, readData, size);
        }

    return ReportError(ENOTSUP, "unknown spi address");
}

int ConnectionRemote::WriteLMS7002MSPI(const uint32_t *writeData, size_t size, unsigned periphID)
{
    const uint16_t outBufLen = sizeof(CtrlPacketHeader)+size*4;
    std::vector<uint8_t> outBuffer(outBufLen);
    CtrlPacketHeader* pkt = (CtrlPacketHeader*)outBuffer.data();
    pkt->cmd = NETWORK_LMS7002_WR;
    //pkt->status = 0;
    pkt->periphID = periphID;
    pkt->payloadLen = size*4;

    memcpy(outBuffer.data()+sizeof(CtrlPacketHeader), writeData, pkt->payloadLen);
    const int timeout_ms = 2000;
    if( Write(outBuffer.data(), outBufLen, timeout_ms) != outBufLen)
        return -1;

    // Read back status
    memset(outBuffer.data(), 0, outBufLen);
    int bytesReceived = Read(outBuffer.data(), sizeof(CtrlPacketHeader), timeout_ms);
    if(bytesReceived != sizeof(CtrlPacketHeader))
        return -2;

    if(pkt->payloadLen > 0)
    {
        // Read back rest of payload
        bytesReceived = Read(outBuffer.data()+sizeof(CtrlPacketHeader), pkt->payloadLen, timeout_ms);
        if(bytesReceived != pkt->payloadLen)
            return -3;
    }

    return 0;
}

int ConnectionRemote::ReadLMS7002MSPI(const uint32_t *writeData, uint32_t *readData, size_t size, unsigned periphID)
{
    const uint16_t outBufLen = sizeof(CtrlPacketHeader)+size*4;
    std::vector<uint8_t> outBuffer(outBufLen);
    CtrlPacketHeader* pkt = (CtrlPacketHeader*)outBuffer.data();
    pkt->cmd = NETWORK_LMS7002_RD;
    //pkt->status = 0;
    pkt->periphID = periphID;
    pkt->payloadLen = size*4;

    memcpy(outBuffer.data()+sizeof(CtrlPacketHeader), writeData, pkt->payloadLen);
    const int timeout_ms = 2000;
    if( Write(outBuffer.data(), outBufLen, timeout_ms) != outBufLen)
        return -1;

    // Read back status
    memset(outBuffer.data(), 0, outBufLen);
    int bytesReceived = Read(outBuffer.data(), sizeof(CtrlPacketHeader), timeout_ms);
    if(bytesReceived != sizeof(CtrlPacketHeader))
        return -2;

    if(pkt->payloadLen > 0)
    {
        // Read back rest of payload
        bytesReceived = Read((uint8_t*)readData, pkt->payloadLen, timeout_ms);
        if(bytesReceived != pkt->payloadLen)
            return -3;
    }

    return 0;
}

int ConnectionRemote::WriteRegisters(const uint32_t *addrs, const uint32_t *data, const size_t size)
{
    const uint16_t outBufLen = sizeof(CtrlPacketHeader)+size*4;
    std::vector<uint8_t> outBuffer(outBufLen);
    CtrlPacketHeader* pkt = (CtrlPacketHeader*)outBuffer.data();
    pkt->cmd = NETWORK_BRDSPI_WR;
    //pkt->status = 0;
    pkt->periphID = 0;
    pkt->payloadLen = size*4;

    uint8_t* payload = outBuffer.data()+sizeof(CtrlPacketHeader);
    for (size_t i = 0; i < size; ++i)
    {
        payload[i*4] = (addrs[i] >> 8);
        payload[i*4 + 1] = (addrs[i] & 0xFF);
        payload[i*4 + 2] = (data[i] >> 8);
        payload[i*4 + 3] = (data[i] & 0xFF);
    }

    const int timeout_ms = 2000;
    if( Write(outBuffer.data(), outBufLen, timeout_ms) != outBufLen)
        return -1;

    // Read back status
    memset(outBuffer.data(), 0, outBufLen);
    int bytesReceived = Read(outBuffer.data(), sizeof(CtrlPacketHeader), timeout_ms);
    if(bytesReceived != sizeof(CtrlPacketHeader))
        return -2;

    // Read back rest of payload
    if(pkt->payloadLen > 0)
    {
        bytesReceived = Read(outBuffer.data()+sizeof(CtrlPacketHeader), pkt->payloadLen, timeout_ms);
        if(bytesReceived != pkt->payloadLen)
            return -3;
    }

    return 0;
}

int ConnectionRemote::ReadRegisters(const uint32_t *addrs, uint32_t *data, const size_t size)
{
    const uint16_t outBufLen = sizeof(CtrlPacketHeader)+size*2;
    std::vector<uint8_t> outBuffer(outBufLen);
    CtrlPacketHeader* pkt = (CtrlPacketHeader*)outBuffer.data();
    pkt->cmd = NETWORK_BRDSPI_RD;
    //pkt->status = 0;
    pkt->periphID = 0;
    pkt->payloadLen = size*2;

    uint8_t* payload = outBuffer.data()+sizeof(CtrlPacketHeader);
    for (size_t i = 0; i < size; ++i)
    {
        payload[i*2] = (addrs[i] >> 8);
        payload[i*2 + 1] = (addrs[i] & 0xFF);
    }

    const int timeout_ms = 2000;
    if( Write(outBuffer.data(), outBufLen, timeout_ms) != outBufLen)
        return -1;

    // Read back status
    memset(outBuffer.data(), 0, outBufLen);
    int bytesReceived = Read(outBuffer.data(), sizeof(CtrlPacketHeader), timeout_ms);
    if(bytesReceived != sizeof(CtrlPacketHeader))
        return -2;

    // Read back rest of payload
    if(pkt->payloadLen > 0)
    {
        bytesReceived = Read((uint8_t*)payload, pkt->payloadLen, timeout_ms);
        if(bytesReceived != pkt->payloadLen)
            return -3;

        for (int i = 0; i < bytesReceived/2; ++i)
        {
            int hi = payload[2*i];
            int lo = payload[2*i + 1];
            data[i] = (hi << 8) | lo;
        }
    }

    return 0;
}

int ConnectionRemote::WriteI2C(const int addr, const std::string &data)
{
    const uint16_t outBufLen = sizeof(CtrlPacketHeader)+data.length();
    std::vector<uint8_t> outBuffer(outBufLen);
    CtrlPacketHeader* pkt = (CtrlPacketHeader*)outBuffer.data();
    pkt->cmd = NETWORK_I2C_WR;
    //pkt->status = 0;
    pkt->periphID = addr; // repurposing periphID for address
    pkt->payloadLen = data.length();

    memcpy(outBuffer.data()+sizeof(CtrlPacketHeader), data.data(), pkt->payloadLen);
    const int timeout_ms = 2000;
    if( Write(outBuffer.data(), outBufLen, timeout_ms) != outBufLen)
        return -1;

    // Read back status
    memset(outBuffer.data(), 0, outBufLen);
    int bytesReceived = Read(outBuffer.data(), sizeof(CtrlPacketHeader), timeout_ms);
    if(bytesReceived != sizeof(CtrlPacketHeader))
        return -2;

    if(pkt->payloadLen > 0)
    {
        // Read back rest of payload
        bytesReceived = Read(outBuffer.data()+sizeof(CtrlPacketHeader), pkt->payloadLen, timeout_ms);
        if(bytesReceived != pkt->payloadLen)
            return -3;
    }

    return 0;
}

int ConnectionRemote::ReadI2C(const int addr, const size_t numBytes, std::string &data)
{
    const uint16_t outBufLen = sizeof(CtrlPacketHeader)+numBytes;
    std::vector<uint8_t> outBuffer(outBufLen);
    CtrlPacketHeader* pkt = (CtrlPacketHeader*)outBuffer.data();
    pkt->cmd = NETWORK_I2C_RD;
    //pkt->status = 0;
    pkt->periphID = addr; // repurposing periphID for address
    pkt->payloadLen = sizeof(uint16_t);

    uint8_t* payload = outBuffer.data()+sizeof(CtrlPacketHeader);
    payload[0] = (numBytes >> 8) & 0xFF;
    payload[1] = (numBytes & 0xFF);

    const int timeout_ms = 2000;
    const int bytesToSend = sizeof(CtrlPacketHeader) + sizeof(uint16_t);
    if( Write(outBuffer.data(), bytesToSend, timeout_ms) != bytesToSend)
        return -1;

    // Read back status
    memset(outBuffer.data(), 0, outBufLen);
    int bytesReceived = Read(outBuffer.data(), sizeof(CtrlPacketHeader), timeout_ms);
    if(bytesReceived != sizeof(CtrlPacketHeader))
        return -2;

    if(pkt->payloadLen > 0)
    {
        // Read back rest of payload
        bytesReceived = Read(outBuffer.data()+sizeof(CtrlPacketHeader), pkt->payloadLen, timeout_ms);
        if(bytesReceived != pkt->payloadLen)
            return -3;
        data.resize(pkt->payloadLen);
        data.replace(0, pkt->payloadLen, (char*)(outBuffer.data()+sizeof(CtrlPacketHeader)));
    }

    return 0;
}

int ConnectionRemote::DeviceReset(int ind)
{
    CtrlPacketHeader pkt;
    pkt.cmd = NETWORK_RESET_DEVICE;
    pkt.periphID = ind;
    pkt.payloadLen = 0;

    const int timeout_ms = 2000;
    if( Write( (uint8_t*)&pkt, sizeof(pkt), timeout_ms) != sizeof(pkt))
        return -1;

    // Read back status
    memset(&pkt, 0, sizeof(pkt));
    int bytesReceived = Read((uint8_t*)&pkt, sizeof(pkt), timeout_ms);
    if(bytesReceived != sizeof(CtrlPacketHeader))
        return -2;

    return 0;
}

int ConnectionRemote::GPIOWrite(const uint8_t *buffer, const size_t bufLength)
{
    return -1;
}
int ConnectionRemote::GPIORead(uint8_t *buffer, const size_t bufLength)
{
    return -1;
}
int ConnectionRemote::GPIODirWrite(const uint8_t *buffer, const size_t bufLength)
{
    return -1;
}
int ConnectionRemote::GPIODirRead(uint8_t *buffer, const size_t bufLength)
{
    return -1;
}
