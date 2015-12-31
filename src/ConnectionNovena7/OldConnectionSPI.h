/**
@file   ConnectionSPI.h
@author Lime Microsystems (www.limemicro.com)
@brief  Class for data communications through SPI port
*/

#ifndef CONNECTION_SPI_PORT_H
#define CONNECTION_SPI_PORT_H

#include "IConnection.h"
#include <fstream>
#include <string>
#include <vector>

class ConnectionSPI : public IConnection
{
public:
    static const int cSPI_BUF_SIZE;
    static const int cSPI_SPEED_HZ;

	ConnectionSPI();
	~ConnectionSPI();

	DeviceStatus Open();
	DeviceStatus Open(unsigned i);
	void Close();
	bool IsOpen();
	int GetOpenedIndex();

	int Write(const unsigned char *buffer, int length, int timeout_ms = 0);
	int Read(unsigned char *buffer, int length, int timeout_ms = 0);

	std::vector<std::string> GetDeviceNames();
	int RefreshDeviceList();
protected:
    std::vector<std::string> m_deviceNames;
	std::vector<unsigned char> rxbuf;
    int fd;
    std::fstream m_SEN;
};
#endif
