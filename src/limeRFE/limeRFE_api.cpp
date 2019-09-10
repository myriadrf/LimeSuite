#include "limeRFE_constants.h"
#include "lms7_device.h"
#include "RFE_Device.h"
#include "Logger.h"

#define SERIAL_BAUDRATE 9600

/****************************************************************************
*
*   LimeRFE API Functions
*
*****************************************************************************/
extern "C" API_EXPORT rfe_dev_t* CALL_CONV RFE_Open(const char* serialport, lms_device_t *dev) {
    if (dev == nullptr && serialport == nullptr)
        return nullptr;
    
    int fd = -1;

    if (serialport != nullptr)
    {
        fd = serialport_init(serialport, SERIAL_BAUDRATE);
        if (fd == -1)
            return nullptr;

        int result = Cmd_Hello(fd);
        if (result == 1) {
            return nullptr;
        }
    }

    return new RFE_Device(dev, fd);
}

extern "C" API_EXPORT void CALL_CONV RFE_Close(rfe_dev_t* rfe) {
    if (!rfe)
        return;
    auto* dev = static_cast<RFE_Device*>(rfe);
    if (dev->portFd)
	serialport_close(dev->portFd);
    delete dev;
}

extern "C" API_EXPORT int CALL_CONV RFE_GetInfo(rfe_dev_t* rfe, unsigned char* cinfo) {
	boardInfo info;
	int result = 0;
        if (!rfe)
            return -1;
        auto* dev = static_cast<RFE_Device*>(rfe);
	result = Cmd_GetInfo(dev->sdrDevice, dev->portFd, &info);
	cinfo[0] = info.fw_ver;
	cinfo[1] = info.hw_ver;
	cinfo[2] = info.status1;
	cinfo[3] = info.status2;

	return result;
}

extern "C" API_EXPORT int RFE_LoadConfig(rfe_dev_t* rfe, const char *filename) {

	int result = 0;
        if (!rfe)
            return -1;
        auto* dev = static_cast<RFE_Device*>(rfe);

	result = Cmd_LoadConfig(dev->sdrDevice, dev->portFd, filename);

        if (result == 0)
            dev->UpdateState();

	return result;
}

extern "C" API_EXPORT int RFE_Reset(rfe_dev_t* rfe) {
	int result = 0;

        if (!rfe)
            return -1;
        auto* dev = static_cast<RFE_Device*>(rfe);
	result = Cmd_Reset(dev->sdrDevice, dev->portFd);

        if (result == 0)
            dev->UpdateState();

	return result;
}

extern "C" API_EXPORT int RFE_Configure(rfe_dev_t* rfe, char channelIDRX, char channelIDTX, char portRX, char portTX, char mode, char notch, char attenuation, char enableSWR, char sourceSWR) {
	rfe_boardState state = {channelIDRX, channelIDTX, portRX, portTX, mode, notch, attenuation, enableSWR, sourceSWR};
        return RFE_ConfigureState(rfe, state);
}

extern "C" API_EXPORT int RFE_ConfigureState(rfe_dev_t* rfe, rfe_boardState state) {
	int result = 0;

        if (!rfe)
            return -1;
        auto* dev = static_cast<RFE_Device*>(rfe);

        dev->AutoFreq(state);

	result = Cmd_Configure(dev->sdrDevice, dev->portFd, state.channelIDRX, state.channelIDTX, state.selPortRX, state.selPortTX, state.mode, state.notchOnOff, state.attValue, state.enableSWR, state.sourceSWR);

        if (result == 0)
            dev->UpdateState(state);

	return result;
}

extern "C" API_EXPORT int RFE_Mode(rfe_dev_t* rfe, int mode) {
	int result = 0;

        if (!rfe)
            return -1;
        auto* dev = static_cast<RFE_Device*>(rfe);

	result = Cmd_Mode(dev->sdrDevice, dev->portFd, mode);

        if (result == 0)
            dev->UpdateState(mode);
	return result;
}

extern "C" API_EXPORT int RFE_ReadADC(rfe_dev_t* rfe, int adcID, int* value) {
        if (!rfe)
            return -1;
        auto* dev = static_cast<RFE_Device*>(rfe);

	return Cmd_ReadADC(dev->sdrDevice, dev->portFd, adcID, value);
}

extern "C" API_EXPORT int RFE_ConfGPIO(rfe_dev_t* rfe, int gpioNum, int direction) {
        if (!rfe)
            return -1;
        auto* dev = static_cast<RFE_Device*>(rfe);
	return Cmd_ConfGPIO(dev->sdrDevice, dev->portFd, gpioNum, direction);
}

extern "C" API_EXPORT int RFE_SetGPIO(rfe_dev_t* rfe, int gpioNum, int val) {
        if (!rfe)
            return -1;
        auto* dev = static_cast<RFE_Device*>(rfe);
	return Cmd_SetGPIO(dev->sdrDevice, dev->portFd, gpioNum, val);
}

extern "C" API_EXPORT int RFE_GetGPIO(rfe_dev_t* rfe, int gpioNum, int * val) {
        if (!rfe)
            return -1;
        auto* dev = static_cast<RFE_Device*>(rfe);
	return Cmd_GetGPIO(dev->sdrDevice, dev->portFd, gpioNum, val);
}

API_EXPORT int CALL_CONV RFE_AssignSDRChannels(rfe_dev_t* rfe, int rx, int tx)
{
    if (!rfe)
        return -1;
    auto* dev = static_cast<RFE_Device*>(rfe);
    dev->SetChannels(rx, tx);
}
