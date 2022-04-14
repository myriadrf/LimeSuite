#include <ciso646>
#include <string.h>
#ifdef __unix__
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <windows.h>
#endif
#include <fcntl.h>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include "Logger.h"
#include "threadHelper.h"
#include "IConnection.h"
#include "ConnectionRemote.h"
#include "ConnectionRegistry.h"

#include "ConnectionRemoteServer.h"

namespace lime
{

ConnectionRemoteServer::ConnectionRemoteServer() :
    device(nullptr), m_Active(false)
{
    socketFd = -1;
}

void ConnectionRemoteServer::SetDevice(IConnection* dev)
{
    device = dev;
}

int ConnectionRemoteServer::Start(uint16_t listenPort)
{
    std::vector<lime::ConnectionHandle> handles;
    handles = lime::ConnectionRegistry::findConnections();
    printf("Devices available for remote:\n");
    for (size_t i = 0; i < handles.size(); i++)
    {
        std::string str = handles[i].serialize();
        printf("%s", str.c_str());
        printf("\n");
    }

    if(socketFd != -1)
        return -1;

    const int port = listenPort;
#ifndef __unix__
    WSADATA wsaData;
    if( int err = WSAStartup(0x0202, &wsaData))
        printf("ConnectionRemoteServer: WSAStartup error %i\n", err);
#endif

    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFd < 0)
    {
        printf("ConnectionRemoteServer: socket error %i\n", socketFd);
        return -2;
    }
#ifdef __unix__
    //fcntl(socketFd, F_SETFL, O_NONBLOCK);
#else
    u_long NonBlock = 1;
    ioctlsocket(socketFd, FIONBIO, &NonBlock);
#endif

    struct sockaddr_in host;
    memset((void *)&host, 0, sizeof(sockaddr_in));
    host.sin_family = AF_INET;
    host.sin_addr.s_addr = INADDR_ANY;
    host.sin_port = htons(port);
    if(bind(socketFd, (struct sockaddr*)&host, sizeof(host)) < 0)
    {
        printf("ConnectionRemoteServer: bind error on port %i\n", port);
        Shutdown();
        return -3;
    }

    if(int error = listen(socketFd, SOMAXCONN) )
    {
#ifndef __unix__
        error =  WSAGetLastError();
#endif
        printf("ConnectionRemoteServer: listen error %i\n", error);
        Shutdown();
        return -4;
    }
    printf("ConnectionRemoteServer: listening on port: %i\n", port);
    m_Active = true;
    remoteThread = std::thread(&ConnectionRemoteServer::ProcessConnections, this);
    return 0;
}

ConnectionRemoteServer::~ConnectionRemoteServer()
{
    Shutdown();
}

void ConnectionRemoteServer::Shutdown()
{
    m_Active = false;
    if(remoteThread.joinable())
        remoteThread.join();

    if(socketFd > 0)
    {
#ifndef __unix__
    //shutdown(m_ControlSocketFd, SD_BOTH);
    closesocket(socketFd);
#else
    shutdown(socketFd, SHUT_RDWR);
    close(socketFd);
#endif
    }

#ifndef __unix__
    WSACleanup();
#endif
}

void ConnectionRemoteServer::ProcessConnections()
{
    struct sockaddr_in cli_addr;
#ifndef __unix__
    SOCKET clientFd;
    int clilen = sizeof(cli_addr);
#else
    int clientFd;
    unsigned int clilen = sizeof(cli_addr);
#endif // __unix__

    uint8_t* pktBuffer = new uint8_t[1024*32];
    ConnectionRemote::CtrlPacketHeader* header = reinterpret_cast<ConnectionRemote::CtrlPacketHeader*>(pktBuffer);
    const int headerSize = sizeof(ConnectionRemote::CtrlPacketHeader);

    while(m_Active && socketFd >= 0)
    {
        memset(&cli_addr, 0, sizeof(cli_addr));
        clientFd = accept(socketFd, (struct sockaddr*)&cli_addr, &clilen);
        printf("ACCEPTED CONNECTION\n");

        if(clientFd == EAGAIN || clientFd == EWOULDBLOCK)
            continue;
        if(clientFd < 0)
            continue;

        bool connected = true;
        while(connected && m_Active)
        {
            int r = 0;
            // read header
            int brecv = recv(clientFd, pktBuffer, headerSize, MSG_WAITALL);
            int msgSize = header->payloadLen;
            printf("recv header: %i, payloadLen: %i\n", brecv, msgSize);
            while(r < msgSize)
            {
                brecv = recv(clientFd, pktBuffer+headerSize+r, msgSize-r, MSG_WAITALL);
                printf("Recv: %i\n", brecv);
                r += brecv;
                if(brecv == 0)
                {
                    connected = false;
                    break;
                }
                if(brecv < 0 )
                {
#ifndef __unix__
                    brecv = WSAGetLastError();
#endif
                    printf("RemoteControl recv with error: %d\n", brecv);
                    connected = false;
                    break;
                }
            }
            

            if(not connected)
                break;
            if(device != nullptr)
            {
                uint32_t* src = reinterpret_cast<uint32_t*>(pktBuffer+headerSize);
                switch(header->cmd)
                {
                    case ConnectionRemote::NETWORK_GET_INFO:
                    {
                        printf("GETINFO\n");
                        DeviceInfo info = device->GetDeviceInfo();
                        std::vector<char> output;

                        auto appendCString = [](std::vector<char> &dest, std::string &src)
                        {
                            const int insertPos = dest.size();
                            dest.resize(dest.size() + src.length() + 1);
                            memcpy(&dest[insertPos], src.data(), src.length());
                            dest[dest.size()-1] = 0; // null terminate;
                        };

                        appendCString(output, info.deviceName);
                        appendCString(output, info.expansionName);
                        appendCString(output, info.firmwareVersion);
                        appendCString(output, info.gatewareVersion);
                        appendCString(output, info.gatewareRevision);
                        appendCString(output, info.gatewareTargetBoard);

                        appendCString(output, info.hardwareVersion);
                        appendCString(output, info.protocolVersion);
                        output.resize(output.size() + sizeof(info.boardSerialNumber));
                        memcpy(&output[output.size()-sizeof(info.boardSerialNumber)], &info.boardSerialNumber, sizeof(info.boardSerialNumber));
                        msgSize = output.size();
                        header->payloadLen = output.size();
                        memcpy(pktBuffer+headerSize, output.data(), output.size());
                        break;
                    }
                    case ConnectionRemote::NETWORK_RESET_DEVICE:
                    {
                        printf("Reset device\n");
                        device->DeviceReset(header->periphID);
                        header->status = 0;
                        header->payloadLen = 0;
                        msgSize = 0;
                        break;
                    }
                    case ConnectionRemote::NETWORK_LMS7002_WR:
                        printf("LMS7002_WR\n");
                        header->status = device->WriteLMS7002MSPI(src, msgSize/4, header->periphID);
                        break;

                    case ConnectionRemote::NETWORK_LMS7002_RD:
                    {
                        printf("LMS7002_RD\n");
                        uint32_t* dest = src;
                        header->status = device->ReadLMS7002MSPI(src, dest, msgSize/4, header->periphID);
                        header->payloadLen = msgSize;
                        break;
                    }
                    case ConnectionRemote::NETWORK_BRDSPI_WR:
                    {
                        printf("BRDSPI_WR\n");
                        std::vector<uint32_t> addrs(msgSize/4);
                        std::vector<uint32_t> data(msgSize/4);
                        const uint8_t* payload = pktBuffer+headerSize;
                        for(int i=0; i<msgSize/4; ++i)
                        {
                            addrs[i] = payload[i*4] << 8 | payload[i*4 + 1];
                            data[i] = payload[i*4 + 2] << 8 | payload[i*4 + 3];
                        }
                        header->status = device->WriteRegisters(addrs.data(), data.data(), addrs.size());
                        break;
                    }

                    case ConnectionRemote::NETWORK_BRDSPI_RD:
                    {
                        printf("BRDSPI_RD\n");
                        std::vector<uint32_t> addrs(msgSize/2);
                        std::vector<uint32_t> data(msgSize/2);
                        uint8_t* payload = pktBuffer+headerSize;
                        for(int i=0; i<msgSize/2; ++i)
                            addrs[i] = payload[i*2] << 8 | payload[i*2 + 1];
                        header->status = device->ReadRegisters(addrs.data(), data.data(), addrs.size());

                        for (size_t i = 0; i < addrs.size(); ++i)
                        {
                            payload[i*2] = data[i] >> 8;
                            payload[i*2 + 1] = data[i] & 0xFF;
                        }
                        break;
                    }
                    case ConnectionRemote::NETWORK_I2C_WR:
                    {
                        std::string temp;
                        temp.resize(header->payloadLen);
                        const char* payload = (const char*)(pktBuffer+headerSize);
                        temp.replace(0, header->payloadLen, payload);
                        header->status = device->WriteI2C(header->periphID, temp);
                        break;
                    }
                    case ConnectionRemote::NETWORK_I2C_RD:
                    {
                        char* payload = (char*)(pktBuffer+headerSize);
                        uint16_t numBytes = (payload[0] << 8) | payload[1];
                        std::string temp;
                        temp.resize(numBytes);
                        header->status = device->ReadI2C(header->periphID, numBytes, temp);
                        memcpy(payload, temp.data(), numBytes);
                        break;
                    }
                }
            }

            int bsent = 0;
            printf("Sending response:\n");
            while(bsent < headerSize+header->payloadLen)
            {
                int r = send(clientFd, pktBuffer+bsent, headerSize+header->payloadLen-bsent, 0);
                if(r < 0)
                {
                    lime::log(LOG_LEVEL_ERROR, "RemoteControl send with error: %d\n", r);
                    connected = false;
                    break;
                }
                printf("Send: %i\n", r);
                bsent += r;
            }
        }
        if (clientFd >= 0)
        {
#ifndef __unix__
            //shutdown(socketFd, SD_BOTH);
            closesocket(clientFd);
#else
            shutdown(clientFd, SHUT_RDWR);
            close(clientFd);
#endif
        }
    }
    delete[] pktBuffer;
}

}
