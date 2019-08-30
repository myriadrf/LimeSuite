#include "limeRFE_constants.h"

/****************************************************************************
*
*   LimeRFE API Functions
*
*****************************************************************************/
extern "C" API_EXPORT int CALL_CONV RFE_Open(const char* serialport, int baudrate) {
	int fd = 0;

	fd = serialport_init(serialport, baudrate);
	if (fd == -1)
		return fd;

	int result = Cmd_Hello(fd);
	if (result == 1) {
		fd = RFE_ERROR_COMM_SYNC;
	}

	return fd;
}

extern "C" API_EXPORT void CALL_CONV RFE_Close(int fd) {

	serialport_close(fd);

}

extern "C" API_EXPORT int CALL_CONV RFE_GetInfo(lms_device_t *dev, int fd, unsigned char* cinfo) {
	boardInfo info;
	int result = 0;
	result = Cmd_GetInfo(dev, fd, &info);
	cinfo[0] = info.fw_ver;
	cinfo[1] = info.hw_ver;
	cinfo[2] = info.status1;
	cinfo[3] = info.status2;

	return result;
}

extern "C" API_EXPORT int RFE_LoadConfig(lms_device_t *dev, int fd, const char *filename) {

	int result = 0;

	result = Cmd_LoadConfig(dev, fd, filename);

	return result;
}

extern "C" API_EXPORT int RFE_Reset(lms_device_t *dev, int fd) {
	int result = 0;

	result = Cmd_Reset(dev, fd);

	return result;
}

extern "C" API_EXPORT int RFE_Configure(lms_device_t *dev, int fd, int channelIDRX, int channelIDTX, int portRX, int portTX, int mode, int notch, int attenuation, int enableSWR, int sourceSWR) {
	int result = 0;

	result = Cmd_Configure(dev, fd, channelIDRX, channelIDTX, portRX, portTX, mode, notch, attenuation, enableSWR, sourceSWR);

	return result;
}

extern "C" API_EXPORT int RFE_ConfigureState(lms_device_t *dev, int fd, rfe_boardState state) {
	int result = 0;

	result = Cmd_Configure(dev, fd, state.channelIDRX, state.channelIDTX, state.selPortRX, state.selPortTX, state.mode, state.notchOnOff, state.attValue, state.enableSWR, state.sourceSWR);

	return result;
}

extern "C" API_EXPORT int RFE_Mode(lms_device_t *dev, int fd, int mode) {
	int result = 0;

	result = Cmd_Mode(dev, fd, mode);

	return result;
}

extern "C" API_EXPORT int RFE_ReadADC(lms_device_t *dev, int fd, int adcID, int* value) {
	int result = 0;

	result = Cmd_ReadADC(dev, fd, adcID, value);

	return result;
}

extern "C" API_EXPORT int RFE_ConfGPIO(lms_device_t *dev, int fd, int gpioNum, int direction) {
	int result = 0;

	result = Cmd_ConfGPIO(dev, fd, gpioNum, direction);

	return result;
}

extern "C" API_EXPORT int RFE_SetGPIO(lms_device_t *dev, int fd, int gpioNum, int val) {
	int result = 0;

	result = Cmd_SetGPIO(dev, fd, gpioNum, val);

	return result;
}

extern "C" API_EXPORT int RFE_GetGPIO(lms_device_t *dev, int fd, int gpioNum, int * val) {
	int result = 0;

	result = Cmd_GetGPIO(dev, fd, gpioNum, val);

	return result;
}