/* termiWin.h
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

#ifndef TERMIWIN_H_
#define TERMIWIN_H_

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

/*Redefining functions from winsock to termiWin. This is very important since winsock2 defines functions such as close as closesocket we have to redefine it*/

#ifndef TERMIWIN_DONOTREDEFINE
#define read readFromSerial
#define write writeToSerial
#define select selectSerial
#define open openSerial
#define close closeSerial
#endif

//Serial options - Linux -> Windows

/*setAttr flags - ~ in front of flags -> deny them*/

//iFlag

#define INPCK		0x00004000 	/*If this bit is set, input parity checking is enabled. If it is not set, no checking at all is done for parity errors on input; the characters are simply passed through to the application.*/
#define IGNPAR	0x00001000	/*If this bit is set, any byte with a framing or parity error is ignored. This is only useful if INPCK is also set.*/
#define PARMRK	0x00040000	/*If this bit is set, input bytes with parity or framing errors are marked when passed to the program. This bit is meaningful only when INPCK is set and IGNPAR is not set.*/
#define ISTRIP	0x00008000	/*If this bit is set, valid input bytes are stripped to seven bits; otherwise, all eight bits are available for programs to read. */
#define IGNBRK	0x00000400	/*If this bit is set, break conditions are ignored. */
#define BRKINT	0x00000100	/*If this bit is set and IGNBRK is not set, a break condition clears the terminal input and output queues and raises a SIGINT signal for the foreground process group associated with the terminal. */
#define IGNCR		0x00000800	/*If this bit is set, carriage return characters ('\r') are discarded on input. Discarding carriage return may be useful on terminals that send both carriage return and linefeed when you type the RET key. */
#define ICRNL		0x00000200	/*If this bit is set and IGNCR is not set, carriage return characters ('\r') received as input are passed to the application as newline characters ('\n').*/
#define INLCR		0x00002000	/*If this bit is set, newline characters ('\n') received as input are passed to the application as carriage return characters ('\r').*/
#define IXOFF		0x00010000	/*If this bit is set, start/stop control on input is enabled. In other words, the computer sends STOP and START characters as necessary to prevent input from coming in faster than programs are reading it. The idea is that the actual terminal hardware that is generating the input data responds to a STOP character by suspending transmission, and to a START character by resuming transmission.*/
#define IXON		0x00020000	/*If this bit is set, start/stop control on output is enabled. In other words, if the computer receives a STOP character, it suspends output until a START character is received. In this case, the STOP and START characters are never passed to the application program. If this bit is not set, then START and STOP can be read as ordinary characters.*/

//lFlag

#define ICANON	0x00001000	/*This bit, if set, enables canonical input processing mode. Otherwise, input is processed in noncanonical mode. */
#define ECHO		0x00000100	/*If this bit is set, echoing of input characters back to the terminal is enabled.*/
#define ECHOE		0x00000200	/*If this bit is set, echoing indicates erasure of input with the ERASE character by erasing the last character in the current line from the screen. Otherwise, the character erased is re-echoed to show what has happened (suitable for a printing terminal). */
#define ECHOK		0x00000400	/*This bit enables special display of the KILL character by moving to a new line after echoing the KILL character normally. The behavior of ECHOKE (below) is nicer to look at.*/
#define ECHONL	0x00000800	/*If this bit is set and the ICANON bit is also set, then the newline ('\n') character is echoed even if the ECHO bit is not set. */
#define ISIG		0x00004000	/*This bit controls whether the INTR, QUIT, and SUSP characters are recognized. The functions associated with these characters are performed if and only if this bit is set. Being in canonical or noncanonical input mode has no effect on the interpretation of these characters. */
#define IEXTEN	0x00002000	/*On BSD systems and GNU/Linux and GNU/Hurd systems, it enables the LNEXT and DISCARD characters.*/
#define NOFLSH	0x00008000	/*Normally, the INTR, QUIT, and SUSP characters cause input and output queues for the terminal to be cleared. If this bit is set, the queues are not cleared. */
#define TOSTOP	0x00010000	/*If this bit is set and the system supports job control, then SIGTTOU signals are generated by background processes that attempt to write to the terminal.*/

//cFlag

#define CSTOPB	0x00001000	/*If this bit is set, two stop bits are used. Otherwise, only one stop bit is used. */
#define PARENB	0x00004000	/*If this bit is set, generation and detection of a parity bit are enabled*/
#define PARODD	0x00008000	/*This bit is only useful if PARENB is set. If PARODD is set, odd parity is used, otherwise even parity is used. */
#define CSIZE		0x00000c00	/*This is a mask for the number of bits per character. */
#define CS5			0x00000000	/*This specifies five bits per byte. */
#define CS6			0x00000400	/*This specifies six bits per byte. */
#define CS7			0x00000800	/*This specifies seven bits per byte. */
#define CS8			0x00000c00	/*This specifies eight bits per byte. */
#define CLOCAL 	0x00000000 	/*Ignore modem control lines -> ignore data carrier detected - not implementable in windows*/
#define CREAD 	0x00000000 /*Enable receiver - if is not set no character will be received*/

//oFlag

#define OPOST	0x00000100 	/*If this bit is set, output data is processed in some unspecified way so that it is displayed appropriately on the terminal device. This typically includes mapping newline characters ('\n') onto carriage return and linefeed pairs. */

//cc

#define VEOF 0
#define VEOL 1
#define VERASE 2
#define VINTR 3
#define VKILL 4
#define VMIN 5 /*If set to 0, serial communication is NOT-BLOCKING, otherwise is BLOCKING*/
#define VQUIT 6
#define VSTART 7
#define VSTOP 8
#define VSUSP 9
#define VTIME 10

//END OF setAttr flags

/*Controls*/
#define TIOMBIC DTR_CONTROL_DISABLE
#define TIOMBIS DTR_CONTROL_ENABLE
#define CRTSCTS RTS_CONTROL_ENABLE

/*Others*/
#define NCCS 11

//Baud speed
#define B110 CBR_110
#define B300 CBR_300
#define B600 CBR_600
#define B1200 CBR_2400
#define B2400 CBR_2400
#define B4800 CBR_4800
#define B9600 CBR_9600
#define B19200 CBR_19200
#define B38400 CBR_38400
#define B57600 CBR_57600
#define B115200 CBR_115200

/*Attributes optional_actions*/
#define TCSANOW 0
#define TCSADRAIN 1
#define TCSAFLUSH 2

//typdef
typedef unsigned tcflag_t; /*This is an unsigned integer type used to represent the various bit masks for terminal flags.*/
typedef unsigned cc_t; /*This is an unsigned integer type used to represent characters associated with various terminal control functions.*/
typedef unsigned speed_t; /*used for terminal baud rates*/

typedef struct termios
{

	tcflag_t c_iflag; /*input modes*/
	tcflag_t c_oflag; /*output modes*/
	tcflag_t c_cflag; /*control modes*/
	tcflag_t c_lflag; /*local modes*/
	cc_t c_cc[NCCS]; /*special character*/

} termios;

//Serial configuration functions

int tcgetattr(HANDLE hComm, struct termios *termios_p);
int tcsetattr(HANDLE hComm, int optional_actions, const struct termios *termios_p);
int cfsetispeed(struct termios *termios_p, speed_t speed);
int cfsetospeed(struct termios *termios_p, speed_t speed);

int readFromSerial(HANDLE hComm, char* buffer, int count);
int writeToSerial(HANDLE hComm, char* buffer, int count);
HANDLE openSerial(char* portname, int opt);
int closeSerial(HANDLE hComm);

//Function to disable DTR, because in Windows each time the port opens the Arduino is reset
int setDTR(int val);

#endif

#ifndef _WIN32
#pragma message("-Warning: termiWin requires a Windows system!")
#endif

#endif //TERMIWIN_H_
