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

	uint8_t gpio_val = 0;

	//Configure LimeRFE to use channel HAM 2m channel in receive mode.
	//Transmit output is routed to TX output. Notch is off. Attenuation is 0.
	LIMERFE_Configure(LIMERFE_I2C, device, LIMERFE_I2C_ADDRESS, 0, LIMERFE_CID_HAM_0145, LIMERFE_MODE_RX, LIMERFE_TX2TXRX_INDEX_TX, LIMERFE_NOTCH_VALUE_OFF, 0);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//Change mode to transmit
	LIMERFE_Mode(LIMERFE_I2C, device, LIMERFE_I2C_ADDRESS, 0, LIMERFE_MODE_TX);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//Reset LimeRFE
	LIMERFE_Reset(LIMERFE_I2C, device, LIMERFE_I2C_ADDRESS, 0);

	return 0;
}