/**
@file   ConnectionCOM.h
@author Lime Microsystems (www.limemicro.com)
@brief  Class for data communications through COM port
*/

#ifndef CONNECTION_COM_PORT_H
#define CONNECTION_COM_PORT_H

#ifndef __unix__
	#include "windows.h"
#endif

#include "IConnection.h"

class ConnectionCOM : public IConnection
{
public:
    static const int COM_BUFFER_LENGTH = 1024; //max buffer size for data

	ConnectionCOM();
	~ConnectionCOM();

	DeviceStatus Open();
	DeviceStatus Open(unsigned i);
	void Close();
	bool IsOpen();
	int GetOpenedIndex();

	int Write(const unsigned char *buffer, int length, int timeout_ms = 0);
	int Read(unsigned char *buffer, int length, int timeout_ms = 0);

	std::vector<std::string> GetDeviceNames();
	int RefreshDeviceList();
	void ClearComm();
private:
	void FindAllComPorts();
	DeviceStatus Open(const char *comName, int baudrate);

	bool TestConnectivity();

	std::string comPortName;
	int comBaudrate;
	bool connected;

	int currentDeviceIndex;
	std::vector<std::string> comPortList;
	std::vector<std::string> m_deviceNames;

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
#endif
