/* termiWin.c
*
* 	Copyright (C) 2017 Christian Visintin - christian.visintin1997@gmail.com
*
* 	This file is part of "termiWin: a termios porting for Windows"
*
*   termiWin is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   termiWin is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with termiWin.  If not, see <http://www.gnu.org/licenses/>.
*
*   Modified by Lime Microsystems (www.limemicro.com)
*
*/

#include "TermiWin.h"
#include <fcntl.h>
#include <stdlib.h>

DCB SerialParams = { 0 }; //Initializing DCB structure
COMMTIMEOUTS timeouts = { 0 }; //Initializing COMMTIMEOUTS structure

//LOCAL functions

/*nbyte 0->7*/
int getByte(tcflag_t flag, int nbyte, int nibble) {

	int byte;
	if (nibble == 1)
		byte = (flag >> (8 * (nbyte)) & 0x0f);
	else
		byte = (flag >> (8 * (nbyte)) & 0xf0);
	return byte;
}

//INPUT FUNCTIONS

int getIXOptions(tcflag_t flag) {

#define i_IXOFF 0x01
#define i_IXON 0x02
#define i_IXOFF_IXON 0x03
#define i_PARMRK 0x04
#define i_PARMRK_IXOFF 0x05
#define i_PARMRK_IXON 0x06
#define i_PARMRK_IXON_IXOFF 0x07

	int byte = getByte(flag, 1, 1);

	return byte;
}

//LOCALOPT FUNCTIONS

int getEchoOptions(tcflag_t flag) {

#define l_NOECHO 0x00
#define l_ECHO 0x01
#define l_ECHO_ECHOE 0x03
#define l_ECHO_ECHOK 0x05
#define l_ECHO_ECHONL 0x09
#define l_ECHO_ECHOE_ECHOK 0x07
#define l_ECHO_ECHOE_ECHONL 0x0b
#define l_ECHO_ECHOE_ECHOK_ECHONL 0x0f
#define l_ECHO_ECHOK_ECHONL 0x0d
#define l_ECHOE 0x02
#define l_ECHOE_ECHOK 0x06
#define l_ECHOE_ECHONL 0x0a
#define l_ECHOE_ECHOK_ECHONL 0x0e
#define l_ECHOK 0x04
#define l_ECHOK_ECHONL 0x0c
#define l_ECHONL 0x08

	int byte = getByte(flag, 1, 1);
	return byte;
}

int getLocalOptions(tcflag_t flag) {

#define l_ICANON 0x10
#define l_ICANON_ISIG 0x50
#define l_ICANON_IEXTEN 0x30
#define l_ICANON_NOFLSH 0x90
#define l_ICANON_ISIG_IEXTEN 0x70
#define l_ICANON_ISIG_NOFLSH 0xd0
#define l_ICANON_IEXTEN_NOFLSH 0xb0
#define l_ICANON_ISIG_IEXTEN_NOFLSH 0xf0
#define l_ISIG 0x40
#define l_ISIG_IEXTEN 0x60
#define l_ISIG_NOFLSH 0xc0
#define l_ISIG_IEXTEN_NOFLSH 0xe0
#define l_IEXTEN 0x20
#define l_IEXTEN_NOFLSH 0xa0
#define l_NOFLSH 0x80

	int byte = getByte(flag, 1, 0);
	return byte;
}

int getToStop(tcflag_t flag) {

#define l_TOSTOP 0x01

	int byte = getByte(flag, 1, 1);
	return byte;
}

//CONTROLOPT FUNCTIONS

int getCharSet(tcflag_t flag) {

	//FLAG IS MADE UP OF 8 BYTES, A FLAG IS MADE UP OF A NIBBLE -> 4 BITS, WE NEED TO EXTRACT THE SECOND NIBBLE (1st) FROM THE FIFTH BYTE (6th).
	int byte = getByte(flag, 1, 1);

	switch (byte) {

	case 0X0:
		return CS5;
		break;

	case 0X4:
		return CS6;
		break;

	case 0X8:
		return CS7;
		break;

	case 0Xc:
		return CS8;
		break;

	default:
		return CS8;
		break;
	}
}

int getControlOptions(tcflag_t flag) {

#define c_ALL_ENABLED 0xd0
#define c_PAREVEN_CSTOPB 0x50
#define c_PAREVEN_NOCSTOPB 0x40
#define c_PARODD_NOCSTOPB 0xc0
#define c_NOPARENB_CSTOPB 0x10
#define c_ALL_DISABLED 0x00

	int byte = getByte(flag, 1, 0);
	return byte;
}

//LIBFUNCTIONS

int tcgetattr(HANDLE hComm, struct termios* termios_p) {

	int ret = 0;

	ret = GetCommState(hComm, &SerialParams);

	return 0;
}

int tcsetattr(HANDLE hComm, int optional_actions, const struct termios* termios_p) {

	int ret = 0;

	//Store flags into local variables
	tcflag_t iflag = termios_p->c_iflag;
	tcflag_t lflag = termios_p->c_lflag;
	tcflag_t cflag = termios_p->c_cflag;
	tcflag_t oflag = termios_p->c_oflag;

	/*****************************************
		iflag
		*****************************************/

	int IX = getIXOptions(iflag);

	if ((IX == i_IXOFF_IXON) || (IX == i_PARMRK_IXON_IXOFF)) {

		SerialParams.fOutX = TRUE;
		SerialParams.fInX = TRUE;
		SerialParams.fTXContinueOnXoff = TRUE;
	}

	/*****************************************
		lflag
		*****************************************/

	int EchoOpt = getEchoOptions(lflag);
	int l_opt = getLocalOptions(lflag);
	int tostop = getToStop(lflag);

	//Missing parameters...

	/*****************************************
		cflag
		*****************************************/

	int CharSet = getCharSet(cflag);
	int c_opt = getControlOptions(cflag);

	switch (CharSet) {

	case CS5:
		SerialParams.ByteSize = 5;
		break;

	case CS6:
		SerialParams.ByteSize = 6;
		break;

	case CS7:
		SerialParams.ByteSize = 7;
		break;

	case CS8:
		SerialParams.ByteSize = 8;
		break;
	}

	switch (c_opt) {

	case c_ALL_ENABLED:
		SerialParams.Parity = ODDPARITY;
		SerialParams.StopBits = TWOSTOPBITS;
		break;

	case c_ALL_DISABLED:
		SerialParams.Parity = NOPARITY;
		SerialParams.StopBits = ONESTOPBIT;
		break;

	case c_PAREVEN_CSTOPB:
		SerialParams.Parity = EVENPARITY;
		SerialParams.StopBits = TWOSTOPBITS;
		break;

	case c_PAREVEN_NOCSTOPB:
		SerialParams.Parity = EVENPARITY;
		SerialParams.StopBits = ONESTOPBIT;
		break;

	case c_PARODD_NOCSTOPB:
		SerialParams.Parity = ODDPARITY;
		SerialParams.StopBits = ONESTOPBIT;
		break;

	case c_NOPARENB_CSTOPB:
		SerialParams.Parity = NOPARITY;
		SerialParams.StopBits = TWOSTOPBITS;
		break;
	}

	/*****************************************
		oflag
		*****************************************/

		/*int OP;
		if(oflag == OPOST)
		else ...*/
		//Missing parameters...

		/*****************************************
			special characters
			*****************************************/

	if (termios_p->c_cc[VEOF] != 0) SerialParams.EofChar = (char)termios_p->c_cc[VEOF];
	if (termios_p->c_cc[VINTR] != 0) SerialParams.EvtChar = (char)termios_p->c_cc[VINTR];

	if (termios_p->c_cc[VMIN] == 1) { //Blocking

		timeouts.ReadIntervalTimeout = 0;         // in milliseconds
		timeouts.ReadTotalTimeoutConstant = 0;    // in milliseconds
		timeouts.ReadTotalTimeoutMultiplier = 0;  // in milliseconds
		timeouts.WriteTotalTimeoutConstant = 0;   // in milliseconds
		timeouts.WriteTotalTimeoutMultiplier = 0; // in milliseconds

	}

	else { //Non blocking

		timeouts.ReadIntervalTimeout = termios_p->c_cc[VTIME] * 100;         // in milliseconds
		timeouts.ReadTotalTimeoutConstant = termios_p->c_cc[VTIME] * 100;    // in milliseconds
		timeouts.ReadTotalTimeoutMultiplier = termios_p->c_cc[VTIME] * 100;  // in milliseconds
		timeouts.WriteTotalTimeoutConstant = termios_p->c_cc[VTIME] * 100;   // in milliseconds
		timeouts.WriteTotalTimeoutMultiplier = termios_p->c_cc[VTIME] * 100; // in milliseconds
	}

	SetCommTimeouts(hComm, &timeouts);

	/*****************************************
		EOF
		*****************************************/

	ret = SetCommState(hComm, &SerialParams);
	if (ret != 0)
		return 0;
	else
		return -1;
}

int cfsetispeed(struct termios* termios_p, speed_t speed) {
	SerialParams.BaudRate = speed;
	return 0;
}

int cfsetospeed(struct termios* termios_p, speed_t speed) {
	SerialParams.BaudRate = speed;
	return 0;
}

int readFromSerial(HANDLE hComm, char* buffer, int count) {
	int rc = 0;
	int ret;

	DWORD rc_dw = 0;
	ret = ReadFile(hComm, buffer, count, &rc_dw, NULL);
	rc = rc_dw;

	if (ret == 0)
		return -1;
	else
		return rc;
}

int writeToSerial(HANDLE hComm, char* buffer, int count) {
	int rc = 0;
	int ret;

	DWORD rc_dw = 0;
	ret = WriteFile(hComm, buffer, count, &rc_dw, NULL);
	rc = rc_dw;

	if (ret == 0)
		return -1;
	else
		return rc;
}

HANDLE openSerial(char* portname, int opt) {
	HANDLE hComm;
	char* port;

	if (strlen(portname) < 4) return -1;

	//COMxx
	if (strlen(portname) > 4) {
		port = (char*)calloc(1, sizeof(char) * strlen("\\\\.\\COM10") + 1);
		strncat(port, "\\\\.\\", strlen("\\\\.\\"));
	}
	//COMx
	else {
		port = (char*)calloc(1, sizeof(char) * 5);
	}
	strncat(port, portname, strlen(portname));

	wchar_t wport[20];
	mbstowcs(wport, port, strlen(port) + 1);//Plus null

	switch (opt) {

	case O_RDWR:
		hComm = CreateFile(wport, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		break;

	case O_RDONLY:
		hComm = CreateFile(wport, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		break;

	case O_WRONLY:
		hComm = CreateFile(wport, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		break;
	}

	return hComm;
}

int closeSerial(HANDLE hComm) {
	int ret = CloseHandle(hComm);
	if (ret != 0)
		return 0;
	else
		return -1;
}

//Function to disable DTR, because in Windows each time the port opens the Arduino is reset
int setDTR(int val) {
	SerialParams.fDtrControl = val;
	return 0;
}
