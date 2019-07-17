/**
@file   limeRFE_I2C_example.cpp
@author Lime Microsystems (www.limemicro.com)
@brief  LimeRFE example with I2C communication via LimeSDR
*/
#include <lime/LimeSuite.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <stdio.h>

#include "limeRFE_constants.h"

int openPort(char* portName);
void closePort(int fd);

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("Error: Wrong number of parameters\n");
		printf("Usage: limeRFE_USB_test <LimeRFE COM port>\n");
		printf("Example: limeRFE_USB_test COM3\n");
		exit(1);
	}

	//Open port
	int fd = LIMERFE_Open(argv[1], 9600);

	if (fd == LIMERFE_HELLO_ATTEMPTS_EXCEEDED) {
		std::cout << "Error synchronizing board" << std::endl;
		return -1;
	}
	if (fd == -1) {
		std::cout << "Error initializing serial port" << std::endl;
		return -1;
	}
	else {
		char msg[200];
		sprintf(msg, "Port opened; fd = %d", fd);
		std::cout << msg << std::endl;
	}

	//Configure LimeRFE to use channel HAM 2m channel in receive mode.
	//Transmit output is routed to TX output. Notch is off. Attenuation is 0.
	LIMERFE_Configure(LIMERFE_USB, NULL, 0, fd, LIMERFE_CID_HAM_0145, LIMERFE_MODE_RX, LIMERFE_TX2TXRX_INDEX_TX, LIMERFE_NOTCH_VALUE_OFF, 0);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//Change mode to transmit
	LIMERFE_Mode(LIMERFE_USB, NULL, 0, fd, LIMERFE_MODE_TX);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//Reset LimeRFE
	LIMERFE_Reset(LIMERFE_USB, NULL, 0, fd);

	//Close port
	LIMERFE_Close(fd);

	return 0;
}