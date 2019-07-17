/**
    @file   gpio_example.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  GPIO example
 */
#include <lime/LimeSuite.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <math.h>
#include <cstring>
//#include<conio.h>

#define CMD_CONFIGURE     0x01
#define CMD_READ_GPIO     0x02
#define CMD_WRITE_GPIO    0x03

#define BUFFER_SIZE 16

#define DIROUT	0
#define DIRIN	1

//Device structure, should be initialize to NULL
lms_device_t* device = NULL;

int openPort(char* portName);
void closePort(int fd);

int error()
{
    if (device != NULL)
        LMS_Close(device);
    exit(-1);
}

void print_gpio(int gpio_val)
{
    for (int i = 0; i < 8; i++)
    {
        bool set = gpio_val&(1<<i);
        std::cout << "GPIO" << i <<": " << (set ? "High" : "Low") << std::endl;
    }
}

int main(int argc, char** argv)
{

	if (argc != 3)
	{
		printf("Error: Wrong number of parameters\n");
		printf("Usage: limeRFE_GPIO_test <Arduino COM port> <LimeRFE COM port>\n");
		exit(1);
	}

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

/*******************************************************
* GPIO0 - GPIO3
********************************************************/
	int fd = openPort(argv[1]);
	if (fd == -1) {
		std::cout << "Error openning port. Exiting..." << std::endl;
		exit(1);
	}

	std::cout << "-------------------------------------------------------------------------" << std::endl;
	std::cout << "GPIO test: Arduino <-> LimeSDR" << std::endl;
	std::cout << "-------------------------------------------------------------------------" << std::endl;

	std::cout << "Connect LimeSDR & LimeRFE using 10-pin GPIO headers" << std::endl;
	std::cout << "Connect LimeRFE & Arduino using custom cable (J13)" << std::endl;
	std::cout << "Press [Enter] to continue . . .";
	std::cin.get();
	std::cout << std::endl;

	//change GPIO pins direction using LMS_GPIODirWrite()
	uint8_t gpio_dir = 0x3F; //set bits 0,1,2,3,4 and 5
	if (LMS_GPIODirWrite(device, &gpio_dir, 1) != 0) //1 byte buffer is enough to configure 8 GPIO pins on LimeSDR-USB
		error();

	unsigned char buf[BUFFER_SIZE];
	memset(buf, 0, BUFFER_SIZE);

	bool passed = true;

	for (int i = 0; i < 4; i++) {
		gpio_val = 1 << i;
		if (LMS_GPIOWrite(device, &gpio_val, 1) != 0) //1 byte buffer is enough to set 8 GPIO pins on LimeSDR-USB
			error();

		buf[0] = CMD_CONFIGURE;
		buf[1] = DIRIN;
		LIMERFE_Cmd(1, NULL, 0, fd, buf);

		buf[0] = CMD_READ_GPIO;

		LIMERFE_Cmd(1, NULL, 0, fd, buf);

//		printf("GPIO0 = %d\n", buf[0]); printf("GPIO1 = %d\n", buf[1]); printf("GPIO2 = %d\n", buf[2]); printf("GPIO3 = %d\n", buf[3]);

		unsigned char gpio_val_set = 0;

		for (int j = 0; j < 4; j++) {
			gpio_val_set += buf[j] * pow(2, j);
		}

		printf("gpio_val = 0x%2x; gpio_val_set = 0x%2x\n", gpio_val, gpio_val_set);
		if (gpio_val != gpio_val_set) {
			passed = false;
			break;
		}
	}

	if (passed) {
		std::cout << "----------------------------------------------" << std::endl;
		std::cout << "GPIO test: LimeSDR -> Arduino passed" << std::endl;
		std::cout << "----------------------------------------------" << std::endl;
	}
	else {
		std::cout << "*****************************************************" << std::endl;
		std::cout << "ERROR: GPIO test: LimeSDR -> Arduino FAILED" << std::endl;
		std::cout << "*****************************************************" << std::endl;
	}

	std::cout << "Change SW2 switches to ON" << std::endl;
	std::cout << "Press [Enter] to continue . . .";
	std::cin.get();

	//change GPIO pins direction using LMS_GPIODirWrite()
	//All inputs
	gpio_dir = 0x00;
	if (LMS_GPIODirWrite(device, &gpio_dir, 1) != 0) //1 byte buffer is enough to configure 8 GPIO pins on LimeSDR-USB
		error();

	buf[0] = CMD_CONFIGURE;
	buf[1] = DIROUT;
	LIMERFE_Cmd(1, NULL, 0, fd, buf);

	passed = true;

	for (int i = 0; i < 4; i++) {

		unsigned char gpio_val_set = (1 << i);

		buf[0] = CMD_WRITE_GPIO;
		for (int j = 0; j < 4; j++) {
			buf[j + 1] = int(i == j);
		}
		LIMERFE_Cmd(1, NULL, 0, fd, buf);

//		printf("GPIO0 = %d\n", buf[1]); printf("GPIO1 = %d\n", buf[2]); printf("GPIO2 = %d\n", buf[3]); printf("GPIO3 = %d\n", buf[4]);

		if (LMS_GPIORead(device, &gpio_val, 1) != 0)
			error();
//		print_gpio(gpio_val);
		gpio_val = gpio_val & 0xF; //Only consider bits 0 - 3 !!!

		printf("gpio_val = 0x%2x; gpio_val_set = 0x%2x\n", gpio_val, gpio_val_set);
		if (gpio_val != gpio_val_set) {
			passed = false;
			break;
		}
	}

	if (passed) {
		std::cout << "----------------------------------------------" << std::endl;
		std::cout << "GPIO test: Arduino -> LimeSDR passed" << std::endl;
		std::cout << "----------------------------------------------" << std::endl;
	}
	else {
		std::cout << "*****************************************************" << std::endl;
		std::cout << "ERROR: GPIO test: Arduino -> LimeSDR FAILED" << std::endl;
		std::cout << "*****************************************************" << std::endl;
	}

	std::cout << "Don't forget to return SW2 to OFF (default)" << std::endl;

	closePort(fd);

/*******************************************************
* GPIO4 & GPIO5
********************************************************/

	std::cout << "-------------------------------------------------------------------------" << std::endl;
	std::cout << "GPIO test: Arduino <-> LimeRFE" << std::endl;
	std::cout << "-------------------------------------------------------------------------" << std::endl;

	int fd_limeRFE = openPort(argv[2]);
	if (fd_limeRFE == -1) {
		std::cout << "Error openning port. Exiting..." << std::endl;
		exit(1);
	}

	std::cout << std::endl;
	std::cout << "Put jumper on J22 on pins 2 and 3" << std::endl;
	std::cout << "Press [Enter] to continue . . .";
	std::cin.get();
	std::cout << std::endl;

	//change GPIO pins direction using LMS_GPIODirWrite()
	gpio_dir = 0x3F; //set bits 0,1,2,3,4 and 5
	if (LMS_GPIODirWrite(device, &gpio_dir, 1) != 0) //1 byte buffer is enough to configure 8 GPIO pins on LimeSDR-USB
		error();

	//Configure GPIO 4 & 5 as inputs (direction = 1)
	LIMERFE_ConfGPIO45(1, NULL, 0, fd_limeRFE, 4, 1);
	LIMERFE_ConfGPIO45(1, NULL, 0, fd_limeRFE, 5, 1);

	passed = true;

	for (int i = 4; i < 6; i++) {
		gpio_val = 1 << i;
		if (LMS_GPIOWrite(device, &gpio_val, 1) != 0) //1 byte buffer is enough to set 8 GPIO pins on LimeSDR-USB
			error();

		unsigned char gpio_val_set = 0;

		int GPIO4, GPIO5;
		LIMERFE_GetGPIO45(1, NULL, 0, fd_limeRFE, 4, &GPIO4);
		LIMERFE_GetGPIO45(1, NULL, 0, fd_limeRFE, 5, &GPIO5);

//		printf("GPIO4 = %d\n", GPIO4); printf("GPIO5 = %d\n", GPIO5);

		gpio_val_set += GPIO4 * pow(2, 4);
		gpio_val_set += GPIO5 * pow(2, 5);

		printf("gpio_val = 0x%2x; gpio_val_set = 0x%2x\n", gpio_val, gpio_val_set);
		if (gpio_val != gpio_val_set) {
			passed = false;
			break;
		}
	}

	if (passed) {
		std::cout << "----------------------------------------------" << std::endl;
		std::cout << "GPIO test: LimeSDR -> LimeRFE passed" << std::endl;
		std::cout << "----------------------------------------------" << std::endl;
	}
	else {
		std::cout << "*****************************************************" << std::endl;
		std::cout << "ERROR: GPIO test: LimeSDR -> LimeRFE FAILED" << std::endl;
		std::cout << "*****************************************************" << std::endl;
	}

	//change GPIO pins direction using LMS_GPIODirWrite()
	//All inputs
	gpio_dir = 0x00;
	if (LMS_GPIODirWrite(device, &gpio_dir, 1) != 0) //1 byte buffer is enough to configure 8 GPIO pins on LimeSDR-USB
		error();

	//Configure GPIO 4 & 5 as outputs (direction = 0)
	LIMERFE_ConfGPIO45(1, NULL, 0, fd_limeRFE, 4, 0);
	LIMERFE_ConfGPIO45(1, NULL, 0, fd_limeRFE, 5, 0);

	passed = true;

	for (int i = 4; i < 6; i++) {

		unsigned char gpio_val_set = (1 << i);

		int GPIO4, GPIO5;

		GPIO4 = int(i == 4);
		GPIO5 = int(i == 5);

		LIMERFE_SetGPIO45(1, NULL, 0, fd_limeRFE, 4, GPIO4);
		LIMERFE_SetGPIO45(1, NULL, 0, fd_limeRFE, 5, GPIO5);

		//		printf("GPIO0 = %d\n", buf[1]); printf("GPIO1 = %d\n", buf[2]); printf("GPIO2 = %d\n", buf[3]); printf("GPIO3 = %d\n", buf[4]);

		if (LMS_GPIORead(device, &gpio_val, 1) != 0)
			error();
		//		print_gpio(gpio_val);
		gpio_val = gpio_val & 0x30; //Only consider bits 4 - 5 !!!

		printf("gpio_val = 0x%2x; gpio_val_set = 0x%2x\n", gpio_val, gpio_val_set);
		if (gpio_val != gpio_val_set) {
			passed = false;
			break;
		}
	}

	if (passed) {
		std::cout << "----------------------------------------------" << std::endl;
		std::cout << "GPIO test: LimeRFE -> LimeSDR passed" << std::endl;
		std::cout << "----------------------------------------------" << std::endl;
	}
	else {
		std::cout << "*****************************************************" << std::endl;
		std::cout << "ERROR: GPIO test: LimeRFE -> LimeSDR FAILED" << std::endl;
		std::cout << "*****************************************************" << std::endl;
	}

	std::cout << "Don't forget to return SW2 to OFF (default)" << std::endl;

	closePort(fd_limeRFE);

/*******************************************************
* Relay Drivers
********************************************************/

	std::cout << "-------------------------------------------------------------------------" << std::endl;
	std::cout << "GPIO test: Relay Drivers" << std::endl;
	std::cout << "-------------------------------------------------------------------------" << std::endl;

	std::cout << "Change SW2 switches to OFF" << std::endl;
	std::cout << "Press [Enter] to continue . . .";
	std::cin.get();

	std::cout << "Connect 5V from Arduino (or any other voltage to VCOM)" << std::endl;
	std::cout << "Pins RELAY_0 to RELAY_3 will be set to high, one by one" << std::endl;
	std::cout << "Press [Enter] to continue . . .";
	std::cin.get();
	std::cout << std::endl;

	//change GPIO pins direction using LMS_GPIODirWrite()
	gpio_dir = 0x3F; //set bits 0,1,2,3,4 and 5
	if (LMS_GPIODirWrite(device, &gpio_dir, 1) != 0) //1 byte buffer is enough to configure 8 GPIO pins on LimeSDR-USB
		error();

	buf[0] = CMD_CONFIGURE;
	buf[1] = DIRIN;
	LIMERFE_Cmd(1, NULL, 0, fd, buf);

	for (int i = 0; i < 4; i++) {
		gpio_val = 1 << i;
		if (LMS_GPIOWrite(device, &gpio_val, 1) != 0) //1 byte buffer is enough to set 8 GPIO pins on LimeSDR-USB
			error();

		printf("gpio_val = 0x%2x;\n", gpio_val);
		std::cout << "Press [Enter] to continue . . .";
		std::cin.get();
	}
    return 0;
}

#define ARDUINO_HELLO_ATTEMPTS_EXCEEDED -10

int openPort(char* portName) {
	int baudrate = 9600;

	int fd = LIMERFE_Open(portName, baudrate);

	//for (int i = 0; i < 5; i++) {
	//AddMssg(".");
	//Sleep(1000);

	if (fd == ARDUINO_HELLO_ATTEMPTS_EXCEEDED) {
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
	return fd;
}

void closePort(int fd) {
	LIMERFE_Close(fd);
//	std::cout << "Port closed" << std::endl;
}


