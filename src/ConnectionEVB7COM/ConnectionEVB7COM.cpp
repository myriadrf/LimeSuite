/**
    @file ConnectionEVB7COM.cpp
    @author Lime Microsystems
    @brief Implementation of EVB7 connection of serial COM port.
*/

#include "ConnectionEVB7COM.h"
#include <stdexcept>

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
#include <iostream>
#include <stdio.h>
#endif // LINUX

const int COM_RETRY_INTERVAL = 20; //ms
const int COM_TOTAL_TIMEOUT = 300; //ms

ConnectionEVB7COM::ConnectionEVB7COM(const char *comName, int baudrate)
{
#ifndef __unix__
    hComm = INVALID_HANDLE_VALUE;
#else
    hComm = -1;
#endif

    if (this->Open(comName, baudrate) != SUCCESS)
    {
        throw std::runtime_error("Failed to open " + std::string(comName));
    }
}

ConnectionEVB7COM::~ConnectionEVB7COM(void)
{
#ifndef __unix__
    if (hComm != INVALID_HANDLE_VALUE)
    {
        SetCommTimeouts(hComm, &m_ctmoOld);
        CloseHandle(hComm);
    }
    hComm = INVALID_HANDLE_VALUE;
#else
    if( hComm >= 0)
    {
        close(hComm);
    }
    hComm = -1;
#endif
}

IConnection::DeviceStatus ConnectionEVB7COM::Open(const char *comName, int baudrate)
{

	if (strlen(comName) == 0)
		return FAILURE;

	DeviceStatus errorCode = SUCCESS;

#ifndef __unix__
	// Initialize Overlap structures
	m_osROverlap.Internal = 0;
	m_osROverlap.InternalHigh = 0;
	m_osROverlap.Offset = 0;
	m_osROverlap.OffsetHigh = 0;
	m_osROverlap.hEvent = CreateEvent(NULL, false, false, NULL);

	m_osWOverlap.Internal = 0;
	m_osWOverlap.InternalHigh = 0;
	m_osWOverlap.Offset = 0;
	m_osWOverlap.OffsetHigh = 0;
	m_osWOverlap.hEvent = CreateEvent(NULL, false, false, NULL);

	// Initialize DSB structure
	memset(&m_dcbCommPort, 0, sizeof(m_dcbCommPort));

	m_dcbCommPort.BaudRate = comBaudrate;
	m_dcbCommPort.fBinary = 1;
	m_dcbCommPort.fParity = 0;
	m_dcbCommPort.fOutxCtsFlow = 0;
	m_dcbCommPort.fOutxDsrFlow = 0;
	m_dcbCommPort.fDtrControl = 0;
	m_dcbCommPort.fDsrSensitivity = 0;
	m_dcbCommPort.fTXContinueOnXoff = 0;
	m_dcbCommPort.fOutX = 0;
	m_dcbCommPort.fInX = 0;
	m_dcbCommPort.fErrorChar = 0;
	m_dcbCommPort.fNull = 0;
	m_dcbCommPort.fRtsControl = 0;
	m_dcbCommPort.fAbortOnError = 0;
	m_dcbCommPort.fDummy2 = 0;
	// m_dcbCommPort.wReserved = 0;
	m_dcbCommPort.XonLim = 512;
	m_dcbCommPort.XoffLim = 512;
	m_dcbCommPort.ByteSize = 8;
	m_dcbCommPort.Parity = 0;
	m_dcbCommPort.StopBits = 0;
	//m_dcbCommPort.StopBits = 1;
	m_dcbCommPort.XonChar = 17;
	m_dcbCommPort.XoffChar = 19;
	m_dcbCommPort.ErrorChar = 0;
	m_dcbCommPort.EofChar = 26;
	m_dcbCommPort.EvtChar = 0;
	m_dcbCommPort.wReserved1 = 0;
	m_dcbCommPort.DCBlength = sizeof(DCB);

	// Initialize Timeout's
	m_ctmoNew.ReadIntervalTimeout = 50;
	m_ctmoNew.ReadTotalTimeoutMultiplier = 0;
	m_ctmoNew.ReadTotalTimeoutConstant = 100; // 1;
	m_ctmoNew.WriteTotalTimeoutMultiplier = 0;
	m_ctmoNew.WriteTotalTimeoutConstant = 100;

	// Open COM port
	string stmp;
	stmp = "\\\\.\\";
	stmp.append(comName);
	hComm = CreateFileA(stmp.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hComm != INVALID_HANDLE_VALUE)
	{
		// Set Events
		if (!SetCommMask(hComm, 0))
			errorCode = FAILURE;

		// Set Timeouts
		GetCommTimeouts(hComm, &m_ctmoOld);
		if (!SetCommTimeouts(hComm, &m_ctmoNew))
			errorCode = FAILURE;

		// Set DCB
		if (!SetCommState(hComm, &m_dcbCommPort))
			errorCode = FAILURE;
	}
	else
	{
		errorCode = FAILURE;
	};

	// Check the results
	if (errorCode != 0)
	{
		//unsigned long err = GetLastError();
		CloseHandle(hComm);
		hComm = INVALID_HANDLE_VALUE;
		return errorCode;
	}
	else
	{
		PurgeComm(hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);
		return SUCCESS;
	}
#else
    hComm = open(comName, O_RDWR | O_NOCTTY | O_SYNC);
    if(hComm < 0)
    {
//        printf("%s",strerror(errno));
//        MessageLog::getInstance()->write("Connection manager: failed opening COM port\n", LOG_ERROR);
        return FAILURE;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if( tcgetattr(hComm, &tty) != 0)
    {
//        MessageLog::getInstance()->write("Connection Manager: error from tcgetattr\n", LOG_ERROR);
        return FAILURE;
    }
    int speed = B9600;
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0; // read non blocking
    tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);

    if(tcsetattr(hComm, TCSANOW, &tty) != 0)
    {
//        MessageLog::getInstance()->write("Connection manager: error from tcsetattr\n", LOG_ERROR);
        return FAILURE;
    }
#endif
    return SUCCESS;
}
