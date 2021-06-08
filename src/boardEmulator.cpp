#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <termios.h>
#include <map>
#include <ctime>
#include <LMS64CProtocol.h>
#include <LMSBoards.h>

using namespace std;
using namespace lime;

bool stopApplication = false;

int ProcessLMS64C(const uint8_t *input, uint8_t *output);

void ApplicationStopHandler(int s)
{
	printf("Caught signal %d, stopping application\n",s);
	stopApplication = true;
}

int main(int argc, char** argv)
{
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = ApplicationStopHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	int masterFd = posix_openpt(O_RDWR);
	if(masterFd < 0)
	{
		printf("failed to open ptmx\n");
		return -1;
	}
	grantpt(masterFd);
	unlockpt(masterFd);
	printf("slave: %s\n", ptsname(masterFd));

	//create soft link to emulator
	char linkCommand[256];
	printf("Creating symbolik link /dev/ttyACM_LMS7emulator\n");
	sprintf(linkCommand, "sudo ln -s %s /dev/ttyACM_LMS7emulator", ptsname(masterFd));
	system(linkCommand);

	cout << "LMS7 board emulator started" << endl;

	const int bufSize = 64;
	vector<uint8_t> inputBuf;

	uint8_t outputBuf[64];

	struct termios term_settings;
	tcgetattr(masterFd, &term_settings);
	// Set raw mode on the slave side of the PTY
	cfmakeraw (&term_settings);
	tcsetattr (masterFd, TCSANOW, &term_settings);

	while(not stopApplication)
	{
		char tempbuf[128];
		int bwrite = 0;
		int bread = read(masterFd, tempbuf, bufSize);
		if(bread < 0)
		{
			int errsv = errno;
			printf("%s\n", strerror(errsv));
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			continue;
		}

		time_t rawtime;
		struct tm * timeinfo;
		char timeBuf[80];
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(timeBuf, 80, "[%H:%M:%S] ", timeinfo);

		printf("%s Received: %i\n", timeBuf, bread);
		for(int i=0; i<bread; ++i)
		{
			inputBuf.push_back(tempbuf[i]);
			if(inputBuf.size() == 64)
			{
				ProcessLMS64C(inputBuf.data(), outputBuf);
				bwrite = write(masterFd, outputBuf, bufSize);
				if(bwrite < 0)
				{
					int errsv = errno;
					printf("error writing data %s\n", strerror(errsv));
				}
				time(&rawtime);
				timeinfo = localtime(&rawtime);
				strftime(timeBuf, 80, "[%H:%M:%S] ", timeinfo);
				printf("%s Transmitted: %i\n", timeBuf, bwrite);
				inputBuf.clear();
			}
		}
	}
	close(masterFd);
	printf("Removing symbolic link /dev/ttyACM_LMS7emulator\n");
	sprintf(linkCommand, "sudo rm /dev/ttyACM_LMS7emulator");
	system(linkCommand);
	return 0;
}


//LMS7002 registers
map<uint16_t, uint16_t> channelA;
map<uint16_t, uint16_t> channelB;

//FPGA registers
map<uint16_t, uint16_t> fpgaRegisters;

void WriteRegister(const uint16_t addr, const uint16_t data)
{
	if((channelA[0x0020] & 0x1) != 0 || addr < 0x0100) //A channel
		channelA[addr] = data;
	if((channelA[0x0020] & 0x2) != 0 && addr >= 0x0100) //B channel
		channelB[addr] = data;
}
uint16_t ReadRegister(const uint16_t addr)
{
	uint16_t retValue = 0;
	if((channelA[0x0020] & 0x1) != 0 || addr < 0x0100) //A channel
		retValue |= channelA[addr];
	if((channelA[0x0020] & 0x2) != 0 && addr >= 0x0100) //B channel
		retValue |= channelB[addr];
	return retValue;
}

int ProcessLMS64C(const uint8_t *input, uint8_t *output)
{
	printf("Got cmd: %i\n", input[0]);
	const int hs = 8; //header size
	const int bufSize = 64;
	memset(output, 0, bufSize);
	switch(input[0])
	{
	case CMD_GET_INFO:
		memcpy(output, input, hs);
		output[1] = STATUS_COMPLETED_CMD;
		output[hs+0] = 255; //firmware
		output[hs+1] = LMS_DEV_EVB7V2; //device
		output[hs+2] = 255; //protocol
		output[hs+3] = 255; //hardware
		output[hs+4] = EXP_BOARD_UNSUPPORTED; //expansion board
		break;
	case CMD_LMS7002_WR:{
		memcpy(output, input, bufSize);
		output[1] = STATUS_COMPLETED_CMD;
        int blockCount = input[2];
        for(int i=0; i<blockCount; ++i)
		{
			int bufPos = hs+i*4;
			int addr = (input[bufPos]<<8) | input[bufPos+1];
			addr = addr & 0x7FFF;
			int data = (input[bufPos+2]<<8) | input[bufPos+3];
			WriteRegister(addr, data);
		}
		break;}
	case CMD_LMS7002_RD:{
		memcpy(output, input, hs);
		output[1] = STATUS_COMPLETED_CMD;
        int blockCount = input[2];
        for(int i=0; i<blockCount; ++i)
		{
			int bufPos = hs+i*2;
			int addr = (input[bufPos]<<8) | input[bufPos+1];
			addr = addr & 0x7FFF;
			int data = ReadRegister(addr);
			int outBufPos = hs+i*4;
			output[outBufPos] = input[bufPos];
			output[outBufPos+1] = input[bufPos+1];
			output[outBufPos+2] = data >> 8;
			output[outBufPos+3] = data & 0xFF;
		}
		break;}
	case CMD_BRDSPI_WR:{
		memcpy(output, input, bufSize);
		output[1] = STATUS_COMPLETED_CMD;
        int blockCount = input[2];
        for(int i=0; i<blockCount; ++i)
		{
			int bufPos = hs+i*4;
			int addr = (input[bufPos]<<8) | input[bufPos+1];
			addr = addr & 0x7FFF;
			int data = (input[bufPos+2]<<8) | input[bufPos+3];
			fpgaRegisters[addr] = data;
		}
		break;}
	case CMD_BRDSPI_RD:{
		memcpy(output, input, hs);
		output[1] = STATUS_COMPLETED_CMD;
        int blockCount = input[2];
        for(int i=0; i<blockCount; ++i)
		{
			int bufPos = hs+i*2;
			int addr = (input[bufPos]<<8) | input[bufPos+1];
			addr = addr & 0x7FFF;
			int data = fpgaRegisters[addr];
			int outBufPos = hs+i*4;
			output[outBufPos] = input[bufPos];
			output[outBufPos+1] = input[bufPos+1];
			output[outBufPos+2] = data >> 8;
			output[outBufPos+3] = data & 0xFF;
		}
		break;}
	case CMD_ALTERA_FPGA_GW_WR:
	case CMD_ALTERA_FPGA_GW_RD:
	case CMD_MEMORY_WR:
		memcpy(output, input, hs);
		output[1] = STATUS_COMPLETED_CMD;
		break;
	default:
		memcpy(output, input, hs);
		output[1] = STATUS_UNKNOWN_CMD;
	}
	return 0;
}
