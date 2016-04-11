/**
    @file ConnectionNovenaRF7.cpp
    @author Lime Microsystems
    @brief NovenaRF connection over SPI-DEV linux.
*/

#include "ConnectionNovenaRF7.h"
#include "ErrorReporting.h"
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
#include <iostream>

using namespace std;
using namespace lime;

const int ConnectionNovenaRF7::cSPI_BUF_SIZE = 128;
const int ConnectionNovenaRF7::cSPI_SPEED_HZ = 2000000;

ConnectionNovenaRF7::ConnectionNovenaRF7(void)
{
    fd = -1;
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

ConnectionNovenaRF7::~ConnectionNovenaRF7(void)
{
    this->Close();
}

/** @brief Opens connection to first found chip
    @return 0-success
*/
int ConnectionNovenaRF7::Open(const char *spiDevPath)
{
#ifdef __unix__
	fd = open(spiDevPath, O_RDWR | O_SYNC);
	if (fd < 0)
	{
        //MessageLog::getInstance()->write("SPI PORT: device not found\n", LOG_ERROR);
		return ReportError(errno, "SPI PORT %s: device not found", spiDevPath);
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
    return 0;
#else
    return ReportError("spidev not supported on this OS");
#endif
}

/** @brief Closes connection to chip
*/
void ConnectionNovenaRF7::Close()
{
#ifdef __unix__
    close(fd);
    fd = -1;
#endif
}

/** @brief Returns whether chip is connected
    @return chip is connected
*/
bool ConnectionNovenaRF7::IsOpen()
{
    return (fd >= 0);
}

/** @brief Sends data through SPI port
    @param buffer data buffer to send
    @param length size of data buffer
    @param timeout_ms timeout limit for operation in milliseconds
    @return Number of bytes sent
*/
int ConnectionNovenaRF7::Write(const unsigned char *buffer, int length, int timeout_ms)
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
        bytesWritten += toWrite;
    }
    m_SEN << 1;
    m_SEN.flush();
    return bytesWritten;
#else
    return 0;
#endif
}

/** @brief Reads data from SPI port
    @param buffer pointer to data buffer for receiving
    @param length number of bytes to read
    @param timeout_ms timeout limit for operation in milliseconds
    @return Number of bytes received
*/
int ConnectionNovenaRF7::Read(unsigned char *buffer, int length, int timeout_ms)
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

void ConnectionNovenaRF7::UpdateExternalBandSelect(const size_t channel, const int trfBand, const int rfePath)
{
    //TODO...
    printf("Fill in the UpdateExternalBandSelect() function for NovenaRF7!\n");
    /*
    //in case of Novena board, need to update GPIO
    if(controlPort->GetInfo().device == LMS_DEV_NOVENA)
    {
        uint16_t regValue = SPI_read(0x0706) & 0xFFF8;
        //lms_gpio2 - tx output selection:
        //		0 - TX1_A and TX1_B (Band 1),
        //		1 - TX2_A and TX2_B (Band 2)
        regValue |= Get_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF)) << 2; //gpio2
        //RX active paths
        //lms_gpio0 | lms_gpio1      	RX_A		RX_B
        //  0 			0       =>  	no active path
        //  1   		0 		=>	LNAW_A  	LNAW_B
        //  0			1		=>	LNAH_A  	LNAH_B
        //  1			1		=>	LNAL_A 	 	LNAL_B
        switch(Get_SPI_Reg_bits(LMS7param(SEL_PATH_RFE)))
        {
            //set gpio1:gpio0
            case 0: regValue |= 0x0; break;
            case 1: regValue |= 0x2; break;
            case 2: regValue |= 0x3; break;
            case 3: regValue |= 0x1; break;
        }
        SPI_write(0x0706, regValue);
    }
    */
}
