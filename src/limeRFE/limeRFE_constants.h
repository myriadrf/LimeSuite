#ifndef __limeRFE_constants__
#define __limeRFE_constants__

#include <fcntl.h>    // File control definitions
#include "lime/LimeSuite.h"
using namespace std;
#include <string.h>
#include <math.h>

#ifdef _MSC_VER
#include <tchar.h>

#define O_NOCTTY 0
#define	IXANY		0x00000800	/* any char will restart after stop */
extern "C" {
	//	#include "termiWin/termiWin.h"
#define TERMIWIN_DONOTREDEFINE
#include "termiWin.h"
}
#endif // WIN

#ifdef __unix__
#include <unistd.h>
#include <termios.h>  /* POSIX terminal control definitions */
#include <sys/ioctl.h>
#include <getopt.h>

//tchar.h
typedef char TCHAR;

#endif // LINUX

#define LIMERFE_I2C 0
#define LIMERFE_USB 1 

#define LIMERFE_BUFFER_SIZE 16

//test
#define LIMERFE_CMD_LED_ONOFF     0xFF

#define GPIO_SCL 6
#define GPIO_SDA 7

#define LIMERFE_I2C_FSCL 100E3 //Approx. SCL frequency - ???

#define LIMERFE_I2C_ADDRESS       0x51

#define LIMERFE_CMD_HELLO         0x00

// CTRL
#define LIMERFE_CMD_MODE                    0xd1  
#define LIMERFE_CMD_CONFIG                  0xd2  // CMD + CHANNELID + RXTX + NOTCH + ATTENUATION + MODE
#define LIMERFE_CMD_MODE_FULL               0xd3  // CMD + MODE
#define LIMERFE_CMD_CONFIG_FULL             0xd4  // CMD + chain bytes + MCU STATE byte

#define LIMERFE_CMD_READ_ADC1               0xa1  // returns ADC1 value
#define LIMERFE_CMD_READ_ADC2               0xa2  // returns ADC2 value
#define LIMERFE_CMD_READ_TEMP               0xa3  // returns temperature

#define LIMERFE_CMD_CONFGPIO45              0xb1  // sets the direction of the GPIO4 or GPIO5 pin
#define LIMERFE_CMD_SETGPIO45               0xb2  // sets the value on GPIO4 or GPIO5 pin
#define LIMERFE_CMD_GETGPIO45               0xb3  // gets the value from GPIO4 or GPIO5 pin

// General CTRL
#define LIMERFE_CMD_GET_INFO                0xe1
#define LIMERFE_CMD_RESET                   0xe2
#define LIMERFE_CMD_GET_CONFIG              0xe3
#define LIMERFE_CMD_GET_CONFIG_FULL         0xe4
#define LIMERFE_CMD_I2C_MASTER              0xe5

#define LIMERFE_DISABLE 0
#define LIMERFE_ENABLE  1

#define LIMERFE_OFF 0
#define LIMERFE_ON  1

#define LIMERFE_MAX_HELLO_ATTEMPTS 50

#define LIMERFE_HELLO_ATTEMPTS_EXCEEDED -10

#define LIMERFE_TIME_BETWEEN_HELLO_MS 200

#define LIMERFE_TYPE_INDEX_WB 0
#define LIMERFE_TYPE_INDEX_HAM 1
#define LIMERFE_TYPE_INDEX_CELL 2

#define LIMERFE_CHANNEL_INDEX_WB_1000 0
#define LIMERFE_CHANNEL_INDEX_WB_4000 1
#define LIMERFE_CHANNEL_INDEX_WB_COUNT 2

#define LIMERFE_CHANNEL_INDEX_HAM_0030 0
#define LIMERFE_CHANNEL_INDEX_HAM_0145 1
#define LIMERFE_CHANNEL_INDEX_HAM_0435 2
#define LIMERFE_CHANNEL_INDEX_HAM_1280 3
#define LIMERFE_CHANNEL_INDEX_HAM_2400 4
#define LIMERFE_CHANNEL_INDEX_HAM_3500 5
#define LIMERFE_CHANNEL_INDEX_HAM_COUNT 6

#define LIMERFE_CHANNEL_INDEX_CELL_BAND01 0
#define LIMERFE_CHANNEL_INDEX_CELL_BAND02 1
#define LIMERFE_CHANNEL_INDEX_CELL_BAND03 2
#define LIMERFE_CHANNEL_INDEX_CELL_BAND07 3
#define LIMERFE_CHANNEL_INDEX_CELL_BAND38 4
#define LIMERFE_CHANNEL_INDEX_CELL_COUNT 5

#define LIMERFE_CID_WB_1000 0
#define LIMERFE_CID_WB_4000 1
#define LIMERFE_CID_HAM_0030 2
#define LIMERFE_CID_HAM_0145 3
#define LIMERFE_CID_HAM_0435 4
#define LIMERFE_CID_HAM_1280 5
#define LIMERFE_CID_HAM_2400 6
#define LIMERFE_CID_HAM_3500 7
#define LIMERFE_CID_CELL_BAND01 8
#define LIMERFE_CID_CELL_BAND02 9
#define LIMERFE_CID_CELL_BAND03 10
#define LIMERFE_CID_CELL_BAND07 11
#define LIMERFE_CID_CELL_BAND38 12
#define LIMERFE_CID_COUNT 13

#define LIMERFE_TX2TXRX_INDEX_TX 0
#define LIMERFE_TX2TXRX_INDEX_TXRX 1
#define LIMERFE_TX2TXRX_INDEX_COUNT 2
#define LIMERFE_TX2TXRX_INDEX_DEFAULT 1

#define LIMERFE_TXRX_VALUE_RX 0
#define LIMERFE_TXRX_VALUE_TX 1

#define LIMERFE_NOTCH_VALUE_OFF 0
#define LIMERFE_NOTCH_VALUE_ON 1
#define LIMERFE_NOTCH_DEFAULT 0

#define LIMERFE_NOTCH_BIT_OFF 1
#define LIMERFE_NOTCH_BIT_ON 0

#define LIMERFE_NOTCH_BYTE 8
#define LIMERFE_NOTCH_BIT 0
#define LIMERFE_ATTEN_BYTE 12
#define LIMERFE_ATTEN_BIT 0 //LSB bit - Attenuation is 3-bit value
#define LIMERFE_TX2TXRX_BYTE 11
#define LIMERFE_TX2TXRX_BIT 5

#define LIMERFE_MODE_RX 0
#define LIMERFE_MODE_TX 1
#define LIMERFE_MODE_NONE 2
#define LIMERFE_MODE_TXRX 3

#define LIMERFE_MCU_BYTE_PA_EN_BIT 0
#define LIMERFE_MCU_BYTE_LNA_EN_BIT 1
#define LIMERFE_MCU_BYTE_TXRX0_BIT 2
#define LIMERFE_MCU_BYTE_TXRX1_BIT 3
#define LIMERFE_MCU_BYTE_RELAY_BIT 4

#define LIMERFE_ADC1 0
#define LIMERFE_ADC2 1
#define LIMERFE_ADC_VREF	5.0

#define LIMERFE_COM_SUCCESS	0
#define LIMERFE_COM_ERROR	1

typedef struct
{
	unsigned char status1;
	unsigned char status2;
	unsigned char fw_ver;
	unsigned char hw_ver;
} boardInfo;

struct boardState
{
	unsigned char channelID;
	unsigned char attValue;
	unsigned char notchOnOff;
	unsigned char selTX2TXRX;
	unsigned char mode;
	unsigned char i2Caddress;
	double powerCellCorr;
	double powerCorr;
	double gammaCorr;
};


#if __cplusplus
extern "C" {
#endif

	void write_buffer_fd(int fd, unsigned char* c, int size);
	int read_buffer_fd(int fd, unsigned char * data, int size);
	void write_buffer(int commType, lms_device_t *dev, int i2Caddress, int fd, unsigned char* data, int size);
	int read_buffer(int commType, lms_device_t *dev, int i2Caddress, int fd, unsigned char * data, int size);

	// declarations go here

	int my_read(int fd, char* buffer, int count);
	int my_write(int fd, char* buffer, int count);
	int my_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);
	int my_open(char* portname, int opt);
	int my_close(int fd);
	int serialport_write(int fd, const char* str, int len);
	int serialport_read(int fd, char* buff, int len);

	int serialport_init(const char* serialport, int baud);
	void serialport_close(int fd);
//	boardInfo Cmd_GetInfo(int fd);
	boardInfo Cmd_GetInfo(int commType, lms_device_t *dev, int i2Caddress, int fd);
	int Cmd_NotchOnOff(int fd, int onOff);
	int Cmd_AttenSet(int fd, int value);
	int Cmd_FilterSet(int fd, int value);
	int Cmd_TXRXSet(int fd, int txRx);
	int Cmd_TX2TXOutSet(int fd, int tx2txOut);
	int Cmd_PredriverEnable(int fd, int enable);
//	int Cmd_GetConfigFull(int fd, boardState *state);
	int Cmd_GetConfigFull(int commType, lms_device_t *dev, int i2Caddress, int fd, boardState *state);

	int Cmd_I2C_Master(int fd, int isMaster);

	int Cmd_Hello(int fd);

	int ReadConfig(const char *filename, boardState *state);
	int SaveConfig(const char *filename, boardState state);
//	int Cmd_LoadConfig(int fd, const char *filename);
	int Cmd_LoadConfig(int commType, lms_device_t *dev, int i2Caddress, int fd, const char *filename);
//	int Cmd_Reset(int fd);
	int Cmd_Reset(int commType, lms_device_t *dev, int i2Caddress, int fd);

//	int Cmd_ConfigureFull(int fd, int channelID, int selTX2TXRX = 0, int notch = 0, int attenuation = 0);
//	int Cmd_ConfigureFull(int commType, lms_device_t *dev, int i2Caddress, int fd, int channelID, int selTX2TXRX = 0, int notch = 0, int attenuation = 0);
	int Cmd_ConfigureFull(int commType, lms_device_t *dev, int i2Caddress, int fd, int channelID, int mode, int selTX2TXRX = 0, int notch = 0, int attenuation = 0);
	int Cmd_Configure(int commType, lms_device_t *dev, int i2Caddress, int fd, int channelID, int mode, int selTX2TXRX = 0, int notch = 0, int attenuation = 0);

//	int Cmd_ModeFull(int fd, int channelID, int mode);
	int Cmd_ModeFull(int commType, lms_device_t *dev, int i2Caddress, int fd, int channelID, int mode);
	int Cmd_Mode(int commType, lms_device_t *dev, int i2Caddress, int fd, int mode);

	void getDefaultConfigurations(unsigned char** bytes);
	void getDefaultConfiguration(int channelID, unsigned char* bytes);

	int getDefaultMode(int channelID, int mode, unsigned char* mcu_byte);

//	int Cmd_ReadADC(int fd, int adcID, int* value);
	int Cmd_ReadADC(int commType, lms_device_t *dev, int i2Caddress, int fd, int adcID, int* value);

	int Cmd_LEDonOff(int commType, lms_device_t *dev, int i2Caddress, int fd, int onOff);

	int Cmd_Cmd(int commType, lms_device_t *dev, int i2Caddress, int fd, unsigned char* buf);

	int Cmd_ConfGPIO45(int commType, lms_device_t *dev, int i2Caddress, int fd, int gpioNum, int direction);
	int Cmd_SetGPIO45(int commType, lms_device_t *dev, int i2Caddress, int fd, int gpioNum, int val);
	int Cmd_GetGPIO45(int commType, lms_device_t *dev, int i2Caddress, int fd, int gpioNum, int * val);

/************************************************************************
* I2C Functions
*************************************************************************/
	int i2c_error();
	void mySleep(double sleepms);
	void i2c_dly(void);
	void i2c_setVal(int bitGPIO, int value);
	int i2c_getVal(int bitGPIO);
	void i2c_start(void);
	void i2c_stop(void);
	unsigned char i2c_rx(char ack);
	int i2c_tx(unsigned char d);
	void i2c_write_buffer(lms_device_t* lms, unsigned char addressI2C, unsigned char* c, int size);
//	void i2c_read_buffer(unsigned char addressI2C, unsigned char* c, int size);
	int i2c_read_buffer(lms_device_t* lms, unsigned char addressI2C, unsigned char* c, int size);
	int I2C_Cmd_LEDonOff(lms_device_t* lms, unsigned char addressI2C, int onOff);

#if __cplusplus
}
#endif


#endif // __limeRFE_constants__
