#ifndef __limeRFE_constants__
#define __limeRFE_constants__

#include "limeRFE.h"

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

#define RFE_I2C 0
#define RFE_USB 1

#define RFE_BUFFER_SIZE 16
#define RFE_BUFFER_SIZE_MODE 2

//test
#define RFE_CMD_LED_ONOFF     0xFF

#define GPIO_SCL 6
#define GPIO_SDA 7

#define RFE_I2C_FSCL 100E3 //Approx. SCL frequency - ???

#define RFE_CMD_HELLO         0x00

// CTRL
#define RFE_CMD_MODE                    0xd1
#define RFE_CMD_CONFIG                  0xd2
#define RFE_CMD_MODE_FULL               0xd3
#define RFE_CMD_CONFIG_FULL             0xd4

#define RFE_CMD_READ_ADC1               0xa1
#define RFE_CMD_READ_ADC2               0xa2
#define RFE_CMD_READ_TEMP               0xa3

#define RFE_CMD_CONFGPIO45              0xb1
#define RFE_CMD_SETGPIO45               0xb2
#define RFE_CMD_GETGPIO45               0xb3

// General CTRL
#define RFE_CMD_GET_INFO                0xe1
#define RFE_CMD_RESET                   0xe2
#define RFE_CMD_GET_CONFIG              0xe3
#define RFE_CMD_GET_CONFIG_FULL         0xe4
#define RFE_CMD_I2C_MASTER              0xe5

#define RFE_DISABLE 0
#define RFE_ENABLE  1

#define RFE_OFF 0
#define RFE_ON  1

#define RFE_MAX_HELLO_ATTEMPTS 50

#define RFE_TIME_BETWEEN_HELLO_MS 200

#define RFE_TYPE_INDEX_WB 0
#define RFE_TYPE_INDEX_HAM 1
#define RFE_TYPE_INDEX_CELL 2
#define RFE_TYPE_INDEX_COUNT 3

#define RFE_CHANNEL_INDEX_WB_1000 0
#define RFE_CHANNEL_INDEX_WB_4000 1
#define RFE_CHANNEL_INDEX_WB_COUNT 2

#define RFE_CHANNEL_INDEX_HAM_0030 0
#define RFE_CHANNEL_INDEX_HAM_0145 1
#define RFE_CHANNEL_INDEX_HAM_0435 2
#define RFE_CHANNEL_INDEX_HAM_1280 3
#define RFE_CHANNEL_INDEX_HAM_2400 4
#define RFE_CHANNEL_INDEX_HAM_3500 5
#define RFE_CHANNEL_INDEX_HAM_COUNT 6

#define RFE_CHANNEL_INDEX_CELL_BAND01 0
#define RFE_CHANNEL_INDEX_CELL_BAND02 1
#define RFE_CHANNEL_INDEX_CELL_BAND03 2
#define RFE_CHANNEL_INDEX_CELL_BAND07 3
#define RFE_CHANNEL_INDEX_CELL_BAND38 4
#define RFE_CHANNEL_INDEX_CELL_COUNT 5

#define RFE_PORT_1_NAME	"TX/RX (J3)"		// J3 - TX/RX
#define RFE_PORT_2_NAME	"TX (J4)"			// J4 - TX
#define RFE_PORT_3_NAME	"30 MHz TX/RX (J5)"	// J5 - 30 MHz TX/RX

#define RFE_TXRX_VALUE_RX 0
#define RFE_TXRX_VALUE_TX 1

#define RFE_NOTCH_DEFAULT 0

#define RFE_NOTCH_BIT_OFF 1
#define RFE_NOTCH_BIT_ON 0

#define RFE_NOTCH_BYTE 8
#define RFE_NOTCH_BIT 0
#define RFE_ATTEN_BYTE 12
#define RFE_ATTEN_BIT 0 //LSB bit - Attenuation is 3-bit value
#define RFE_PORTTX_BYTE 11
#define RFE_PORTTX_BIT 5

#define RFE_MODE_RX 0
#define RFE_MODE_TX 1
#define RFE_MODE_NONE 2
#define RFE_MODE_TXRX 3

#define RFE_MCU_BYTE_PA_EN_BIT 0
#define RFE_MCU_BYTE_LNA_EN_BIT 1
#define RFE_MCU_BYTE_TXRX0_BIT 2
#define RFE_MCU_BYTE_TXRX1_BIT 3
#define RFE_MCU_BYTE_RELAY_BIT 4

#define RFE_CHANNEL_RX 0
#define RFE_CHANNEL_TX 1

typedef struct
{
	unsigned char status1;
	unsigned char status2;
	unsigned char fw_ver;
	unsigned char hw_ver;
} boardInfo;

struct guiState
{
	double powerCellCorr;
	double powerCorr;
	double rlCorr;
};

#if __cplusplus
extern "C" {
#endif

	int write_buffer_fd(int fd, unsigned char* c, int size);
	int read_buffer_fd(int fd, unsigned char * data, int size);
	int write_buffer(lms_device_t *dev, int fd, unsigned char* data, int size);
	int read_buffer(lms_device_t *dev, int fd, unsigned char * data, int size);
	int my_read(int fd, char* buffer, int count);
	int my_write(int fd, char* buffer, int count);
	int my_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);
	int my_open(char* portname, int opt);
	int my_close(int fd);
	int serialport_write(int fd, const char* str, int len);
	int serialport_read(int fd, char* buff, int len);
	int serialport_init(const char* serialport, int baud);
	void serialport_close(int fd);
	int Cmd_GetInfo(lms_device_t *dev, int fd, boardInfo* info);
	int Cmd_GetConfigFull(lms_device_t *dev, int fd, rfe_boardState *state);
	int Cmd_GetConfig(lms_device_t *dev, int fd, rfe_boardState *state);
	int Cmd_Hello(int fd);
	int ReadConfig(const char *filename, rfe_boardState *stateBoard, guiState *stateGUI);
	int SaveConfig(const char *filename, rfe_boardState state, guiState stateGUI);
	int Cmd_LoadConfig(lms_device_t *dev, int fd, const char *filename);
	int Cmd_Reset(lms_device_t *dev, int fd);
	int Cmd_ConfigureFull(lms_device_t *dev, int fd, int channelID, int mode, int selPortTX = 0, int notch = 0, int attenuation = 0);
        int Cmd_ConfigureState(lms_device_t* dev, int fd, rfe_boardState state);
	int Cmd_Configure(lms_device_t *dev, int fd, int channelIDRX, int channelIDTX = -1, int selPortRX = 0, int selPortTX = 0, int mode = 0, int notch = 0, int attenuation = 0, int enableSWR = 0, int sourceSWR = 0);
	int Cmd_ModeFull(lms_device_t *dev, int fd, int channelID, int mode);
	int Cmd_Mode(lms_device_t *dev, int fd, int mode);

	void getDefaultConfigurations(unsigned char** bytes);
	void getDefaultConfiguration(int channelID, unsigned char* bytes);
	int getDefaultMode(int channelID, int mode, unsigned char* mcu_byte);

	int Cmd_ReadADC(lms_device_t *dev, int fd, int adcID, int* value);
	int Cmd_Cmd(lms_device_t *dev, int fd, unsigned char* buf);
	int Cmd_ConfGPIO(lms_device_t *dev, int fd, int gpioNum, int direction);
	int Cmd_SetGPIO(lms_device_t *dev, int fd, int gpioNum, int val);
	int Cmd_GetGPIO(lms_device_t *dev, int fd, int gpioNum, int * val);

/************************************************************************
* I2C Functions
*************************************************************************/
	void mySleep(double sleepms);
	void i2c_dly(void);
	int i2c_setVal(lms_device_t* lms, int bitGPIO, int value);
	int i2c_getVal(lms_device_t* lms, int bitGPIO);
	int i2c_start(lms_device_t* lms);
	int i2c_stop(lms_device_t* lms);
	unsigned char i2c_rx(lms_device_t* lms, char ack);
	int i2c_tx(lms_device_t* lms, unsigned char d);
	int i2c_write_buffer(lms_device_t* lms, unsigned char* c, int size);
	int i2c_read_buffer(lms_device_t* lms, unsigned char* c, int size);

#if __cplusplus
}
#endif

#endif // __limeRFE_constants__
