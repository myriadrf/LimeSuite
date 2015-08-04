/**
    @file   ConnectionSPI.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  Implementation of communications through SPI port
*/

#include "ConnectionSPI.h"

#include "string.h"
#ifdef __unix__
#include <fstream>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <linux/i2c-dev.h>
#endif // LINUX

#include <iostream>
#include <sstream>

const int ConnectionSPI::cSPI_BUF_SIZE = 128;
const int ConnectionSPI::cSPI_SPEED_HZ = 2000000;

/** @brief Tries to read EEPROM for Novena board signature
    @return true if Novena board
*/
bool IsNovenaBoard()
{
#ifdef __unix__
    char data[8];
    int count = 6;
    memset(data, 0, 8);
    int addr = 0;
    struct i2c_rdwr_ioctl_data session;
    struct i2c_msg messages[2];
    char set_addr_buf[2];
    memset(set_addr_buf, 0, sizeof(set_addr_buf));
    memset(data, 0, count);
    set_addr_buf[0] = addr>>8;
    set_addr_buf[1] = addr;
    messages[0].addr = 0xac>>1;
    messages[0].flags = 0;
    messages[0].len = sizeof(set_addr_buf);
    messages[0].buf = set_addr_buf;
    messages[1].addr = 0xac>>1;
    messages[1].flags = I2C_M_RD;
    messages[1].len = count;
    messages[1].buf = data;
    session.msgs = messages;
    session.nmsgs = 2;

    bool isNovena = false;

    int fd = open("/dev/i2c-2", O_RDWR);
    if(fd > 0)
    {
        if(ioctl(fd, I2C_RDWR, &session) < 0)
        {
            perror("Unable to communicate with i2c device");
            isNovena = false;
        }
        if(strcmp("Novena", data) == 0)
            isNovena = true;
    }
    close(fd);
    return isNovena;
#else
    return false;
#endif
}

/** @brief Initializes SPI port and exports needed GPIO
*/
ConnectionSPI::ConnectionSPI()
{
    fd = -1;
    m_connectionType = SPI_PORT;
    std::fstream gpio;
    //export SEN pin
    gpio.open("/sys/class/gpio/export", ios::out);
    if(gpio.good())
    {
        gpio << 122;
        gpio.flush();
        gpio.close();
        gpio.open("/sys/class/gpio/gpio122/direction", ios::out);
        gpio << "out";
        gpio.flush();
        gpio.close();
        m_SEN.open("/sys/class/gpio/gpio122/value", ios::out);
        m_SEN<< 1;
        m_SEN.flush();
        cout << "GPIO122: set to 1" << endl;
    }
}

ConnectionSPI::~ConnectionSPI()
{
    Close();
    m_SEN.close();
}

/** @brief Opens connection to first found chip
    @return 0-success
*/
IConnection::DeviceStatus ConnectionSPI::Open()
{
	Close();
#ifdef __unix__
	fd = open("/dev/spidev2.0", O_RDWR | O_SYNC);
	if (fd < 0)
	{
        //MessageLog::getInstance()->write("SPI PORT: device not found\n", LOG_ERROR);
		return IConnection::FAILURE;
	}
	int mode = SPI_MODE_0;
	int ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		printf("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		printf("can't get spi mode");

	/*
	 * bits per word
	 */
    int bits = 8;
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		printf("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		printf("can't get bits per word");

	/*
	 * max speed hz
	 */
	int speed = cSPI_SPEED_HZ;
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't get max speed hz");

	printf("spi mode: 0x%x\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
    return IConnection::SUCCESS;
#else
    return IConnection::FAILURE;
#endif
}

/** @brief Opens connection to selected chip
    @param index chip index in device list
    @return 0-success
*/
IConnection::DeviceStatus ConnectionSPI::Open(unsigned index)
{
    return Open();
}

/** @brief Closes connection to chip
*/
void ConnectionSPI::Close()
{
#ifdef __unix__
    close(fd);
    fd = -1;
#endif
}

/** @brief Returns whether chip is connected
    @return chip is connected
*/
bool ConnectionSPI::IsOpen()
{
    return (fd >= 0);
}

/** @brief Sends data through COM port
    @param buffer data buffer to send
    @param length size of data buffer
    @param timeout_ms timeout limit for operation in milliseconds
    @return Number of bytes sent
*/
int ConnectionSPI::Write(const unsigned char *buffer, int length, int timeout_ms)
{
    #ifdef __unix__
    if(fd < 0)
        return 0;
    m_SEN << 0;
    m_SEN.flush();
    unsigned long bytesWritten = 0;
    rxbuf.clear();
    int bytesReceived = 0;
    char rxbytes[2];
    while(bytesWritten < length)
    {
        int toWrite = length-bytesWritten > cSPI_BUF_SIZE ? cSPI_BUF_SIZE : length-bytesWritten;
        for(int i=0; i<toWrite; i+=2)
        {
            if(buffer[i] < 0x80) //reading
            {
                write(fd, &buffer[i+bytesWritten], 2);
                bytesReceived += read(fd, rxbytes, 2);
                rxbuf.push_back(rxbytes[0]);
                rxbuf.push_back(rxbytes[1]);
            }
            else //writing
            {
                write(fd, &buffer[i+bytesWritten], 4);
                i+=2; //data bytes have been written
            }
        }
//        stringstream ss;
//        ss << "write(" << toWrite << "): ";
//        for(int i=0; i<toWrite; ++i)
//        {
//            char ctemp[16];
//            sprintf(ctemp, "%02X", buffer[bytesWritten+i]);
//            ss << ctemp << " ";
//        }
//        ss << endl;
//        if(bytesReceived > 0)
//        {
//            ss << " re443ad(" << toWrite << "): ";
//            for(int i=0; i<toWrite; ++i)
//            {
//                char ctemp[16];
//                sprintf(ctemp, "%02X", rxbuf[bytesWritten+i]);
//                ss << ctemp << " ";
//            }
//            ss << endl;
//        }
//        cout << ss.str() << endl;
//        MessageLog::getInstance()->write(ss.str(), LOG_DATA);

        bytesWritten += toWrite;
    }
    m_SEN << 1;
    m_SEN.flush();
    return bytesWritten;
#else
    return 0;
#endif
}

/** @brief Reads data from COM port
    @param buffer pointer to data buffer for receiving
    @param length number of bytes to read
    @param timeout_ms timeout limit for operation in milliseconds
    @return Number of bytes received
*/
int ConnectionSPI::Read(unsigned char *buffer, int length, int timeout_ms)
{
#ifdef __unix__
    if(fd < 0)
        return 0;
    //because transfer is done in full duplex, function returns data from last transfer
    int tocpy = length > rxbuf.size() ? rxbuf.size() : length;
    memcpy(buffer, &rxbuf[0], tocpy);
    rxbuf.clear();
    return tocpy;
#else
    return 0;
#endif
}

/** @brief Finds all chips connected to com ports
    @return number of devices found
*/
int ConnectionSPI::RefreshDeviceList()
{
    m_deviceNames.clear();
#ifdef __unix__
    int spidev = open("/dev/spidev2.0", O_RDWR);
	if (spidev >= 0)
	{
	    if(IsNovenaBoard() == true)
            m_deviceNames.push_back("SPI (Novena)");
        else
            m_deviceNames.push_back("SPI");
	}
    close(spidev);
#endif
    return m_deviceNames.size();
}

/** @brief Returns found devices names
    @return vector of device names
*/
vector<string> ConnectionSPI::GetDeviceNames()
{
    return m_deviceNames;
}

int ConnectionSPI::GetOpenedIndex()
{
    return 0;
}
