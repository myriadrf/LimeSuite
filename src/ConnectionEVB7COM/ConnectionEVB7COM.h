/**
    @file ConnectionEVB7COM.h
    @author Lime Microsystems
    @brief Implementation of EVB7 connection of serial COM port.
*/

#pragma once
#include <ConnectionRegistry.h>
#include <IConnection.h>
#include <vector>
#include <string>

class ConnectionEVB7COM : public IConnection
{
public:
    ConnectionEVB7COM(const char *comName, int baudrate);

    ~ConnectionEVB7COM(void);

    bool IsOpen(void);

    OperationStatus DeviceReset(void);

    OperationStatus TransactSPI(const int index, const uint32_t *writeData, uint32_t *readData, const size_t size);

    //! JB TODO remove old interfaces
    DeviceStatus Open(){}
    DeviceStatus Open(unsigned i){}
    int GetOpenedIndex(){}

    std::vector<std::string> GetDeviceNames(){}
    int RefreshDeviceList(){}
    void ClearComm(){}
    //! -------------------------------------------------------------

private:

    DeviceStatus Open(const char *comName, int baudrate);

    void Close(void);

    int Write(const unsigned char *buffer, int length, int timeout_ms = 0);
    int Read(unsigned char *buffer, int length, int timeout_ms = 0);

    #ifndef __unix__
        HANDLE hComm;
        COMMTIMEOUTS m_ctmoNew;
        COMMTIMEOUTS m_ctmoOld;
        OVERLAPPED m_osROverlap;
        OVERLAPPED m_osWOverlap;
        DCB m_dcbCommPort;
    #else
        int hComm; //com port file descriptor
    #endif
};

class ConnectionEVB7COMEntry : public ConnectionRegistryEntry
{
public:
    ConnectionEVB7COMEntry(void);

    std::vector<ConnectionHandle> enumerate(const ConnectionHandle &hint);

    IConnection *make(const ConnectionHandle &handle);

private:
    std::vector<std::string> FindAllComPorts();
    std::vector<std::string> FilterDeviceList(const std::vector<std::string> &);
};
