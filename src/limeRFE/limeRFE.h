#ifndef __limeRFE__
#define __limeRFE__

#include "limeRFE_constants.h"
#include "limeRFE_gui.h"
//#include "lime/LimeSuite.h"

//#include "INI.h"
#include <iostream>
#include <fstream>
//using namespace std;
//#include <string.h>

#include <stdio.h>

#include <stdlib.h>
//#include <fcntl.h>    // File control definitions
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <vector>

//#include <math.h>

/*
#ifdef _MSC_VER
#include <tchar.h>

#define O_NOCTTY 0
#define	IXANY		0x00000800	// any char will restart after stop
extern "C" {
	//	#include "termiWin/termiWin.h"
#define TERMIWIN_DONOTREDEFINE
#include "termiWin.h"
}
#endif // WIN

#ifdef __unix__
#include <unistd.h>
#include <termios.h>  // POSIX terminal control definitions
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
};
*/
class limeRFE : public limeRFE_view
{
	protected:
		lms_device_t *lmsControl;

		// Handlers for AppFrame events.
		void OnbtnOpenPort(wxCommandEvent& event);
		void OnbtnClosePort(wxCommandEvent& event);
		void AddMssg(const char* mssg);
		void ReadPorts();
		void OnbtnRefreshPorts(wxCommandEvent& event);
		void OnbtnReset(wxCommandEvent& event);
		void OnbtnOpen(wxCommandEvent& event);
		void OnbtnSave(wxCommandEvent& event);
		boardState GUI2State();
		void State2GUI(boardState state);
		void OnbtnBoard2GUI(wxCommandEvent& event);

		void OncbI2CMaster(wxCommandEvent& event);

		void OncType(wxCommandEvent& event);
		void SetChannelsChoices();

		void OncChannel(wxCommandEvent& event);
		void SetConfigurationOptions();
		int GetChannelID();
		int GetChannelIndexes(int channelID, int* typeIndex, int* channelIndex);

		int lastSelection[3] = { 0, 0, 0 };

		int lastTX2TXRXSelection = 0;

		void OncTX2TXRX(wxCommandEvent& event);

		void OntbtnTXRX(wxCommandEvent& event);
		void SetModeLabel();

		void OnbtnConfigure(wxCommandEvent& event);

		bool configured;
		int configuredChannelID;
		int configredTX2TXRX;
		int configuredNotch;
		int configuredAttenuation;

		int activeMode;

		void UpdateRFEForm();

		void OncbNotch(wxCommandEvent& event);
		void OncAttenuation(wxCommandEvent& event);

		void OnbtnReadADC(wxCommandEvent& event);

		void OntbtnTXRXEN(wxCommandEvent& event);

		void OnrbI2CrbUSB(wxCommandEvent& event);

		void OnrbSWRsource(wxCommandEvent& event);

		void OnbtnCalibrate(wxCommandEvent& event);

		int CalculatePowerAndGamma(int adc1, int adc2, double * pin_dBm, double * gamma_dB);

		unsigned char GetI2CAddress();
		int GetCommType();

		int fd; //Port handle

		double powerCellCalCorr;
		double powerCalCorr;
		double gammaCalCorr;

	public:
		/** Constructor */
		limeRFE( wxWindow* parent );
	//// end generated class members
        limeRFE(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString &title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long styles = 0);
        void Initialize(lms_device_t* lms);
};


#endif // __limeRFE__
