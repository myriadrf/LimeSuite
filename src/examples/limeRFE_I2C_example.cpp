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

#include "limeRFE.h"

//Device structure, should be initialize to NULL
lms_device_t* device = NULL;

int error()
{
    if (device != NULL)
        LMS_Close(device);
    exit(-1);
}

int main(int argc, char** argv)
{
	//Find devices
	int n;
	lms_info_str_t list[8]; //should be large enough to hold all detected devices
	if ((n = LMS_GetDeviceList(list)) < 0) //NULL can be passed to only get number of devices
		error();

	std::cout << "Devices found: " << n << std::endl; //print number of devices
	if (n < 1)
		return -1;

	//open the first device
	if (LMS_Open(&device, list[0], NULL))
		error();

	unsigned char cinfo[4];
	RFE_GetInfo(device, 0, cinfo);

	printf("LimeRFE Firmware version: %d\n", cinfo[0]);
	printf("LimeRFE Hardware version: 0x%x\n", cinfo[1]);

	//Configure LimeRFE to use channel HAM 2m channel in receive mode.
	//Transmit output is routed to TX/RX output. Notch is off. Attenuation is 0.
	RFE_Configure(device, 0, RFE_CID_HAM_0145, RFE_CID_HAM_0145, RFE_PORT_1, RFE_PORT_1, RFE_MODE_RX, RFE_NOTCH_OFF, 0);
	//or simpler (by using default arguments)
	//RFE_Configure(device, 0, RFE_CID_HAM_0145);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//Change mode to transmit
	RFE_Mode(device, 0, RFE_MODE_TX);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//Reset LimeRFE
	RFE_Reset(device, 0);

	return 0;
}