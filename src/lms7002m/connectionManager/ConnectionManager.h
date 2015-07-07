/**
    @file ConnectionManager.h
    @author Lime Microsystems (www.limemicro.com)
    @brief Class for managing connection to devices
*/

#ifndef LMS_CONNECTION_MANAGER_H
#define LMS_CONNECTION_MANAGER_H

#include "IConnection.h"
#include <map>

class ConnectionManager
{
public:
    struct DeviceInfo
    {
        std::string name;
        IConnection::eConnectionType port;
        int portIndex;
    };

    ConnectionManager();
    ~ConnectionManager();
    bool IsOpen();
    bool Open();
    int Open(unsigned i);
    void Close();
    int RefreshDeviceList();
    int GetOpenedIndex();
    std::vector<std::string> GetDeviceList(){return mDeviceList;};

    int Write(const unsigned char *buffer, int length, int timeout_ms = 0);
	int Read(unsigned char *buffer, int length, int timeout_ms = 0);

	int WriteStream(const char *buffer, int length);
	int ReadStream(char *buffer, int length, unsigned int timeout_ms);

    int BeginDataReading(char *buffer, long length);
    int WaitForReading(int contextHandle, unsigned int timeout_ms);
    int FinishDataReading(char *buffer, long &length, int contextHandle);
    void AbortReading();

    int BeginDataSending(const char *buffer, long length);
    int WaitForSending(int contextHandle, unsigned int timeout_ms);
    int FinishDataSending(const char *buffer, long &length, int contextHandle);
    void AbortSending();

protected:
    bool mLogData;
    /// Port used for communication.
	IConnection *activeControlPort;
    std::vector<DeviceInfo> mDevices;
    std::vector<std::string> mDeviceList;
    int mOpenedDevice;
    std::map<IConnection::eConnectionType, IConnection*> m_connections;
};

#endif // LMS_CONNECTION_MANAGER_H

