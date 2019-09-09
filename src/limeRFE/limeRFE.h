/**
* @file limeRFE/limeRFE.h
*
* @brief LimeRFE API functions
*
* Copyright (C) 2016 Lime Microsystems
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __limeRFE__
#define __limeRFE__

#include "LimeSuite.h"

///LimeRFE I2C address
#define RFE_I2C_ADDRESS       0x51

///LimeRFE Channel IDs
#define RFE_CID_WB_1000 0
#define RFE_CID_WB_4000 1
#define RFE_CID_HAM_0030 2
#define RFE_CID_HAM_0145 3
#define RFE_CID_HAM_0435 4
#define RFE_CID_HAM_1280 5
#define RFE_CID_HAM_2400 6
#define RFE_CID_HAM_3500 7
#define RFE_CID_CELL_BAND01 8
#define RFE_CID_CELL_BAND02 9
#define RFE_CID_CELL_BAND03 10
#define RFE_CID_CELL_BAND07 11
#define RFE_CID_CELL_BAND38 12
#define RFE_CID_COUNT 13

///LimeRFE Ports
#define RFE_PORT_1	1	///<Connector J3 - 'TX/RX'
#define RFE_PORT_2	2	///<Connector J4 - 'TX'
#define RFE_PORT_3	3	///<Connector J5 - '30 MHz TX/RX'

///LimeRFE convenience constants for notch on/off control
#define RFE_NOTCH_OFF 0
#define RFE_NOTCH_ON 1

///LimeRFE Modes
#define RFE_MODE_RX 0     ///<RX - Enabled; TX - Disabled
#define RFE_MODE_TX 1     ///<RX - Disabled; TX - Enabled
#define RFE_MODE_NONE 2   ///<RX - Disabled; TX - Disabled
#define RFE_MODE_TXRX 3   ///<RX - Enabled; TX - Enabled

///LimeRFE ADC constants
#define RFE_ADC1 0        ///<ADC #1, this ADC value is proportional to output power in dB.
#define RFE_ADC2 1        ///<ADC #2, this ADC value is proportional to reflection coefficient in dB.
#define RFE_ADC_VREF 5.0  ///<ADC referent voltage
#define RFE_ADC_BITS 10   ///<ADC resolution

///LimeRFE error codes
#define RFE_SUCCESS	0
#define RFE_ERROR_COMM_SYNC -4	///<Error synchronizing communication
#define RFE_ERROR_GPIO_PIN	-3	///<Non-configurable GPIO pin specified. Only pins 4 and 5 are configurable.
#define RFE_ERROR_CONF_FILE	-2	///<Problem with .ini configuration file
#define RFE_ERROR_COMM	-1	///<Communication error
#define RFE_ERROR_TX_CONN	1	///<Wrong TX connector - not possible to route TX of the selecrted channel to the specified port
#define RFE_ERROR_RX_CONN	2	///<Wrong RX connector - not possible to route RX of the selecrted channel to the specified port
#define RFE_ERROR_RXTX_SAME_CONN	3	///<Mode TXRX not allowed - when the same port is selected for RX and TX, it is not allowed to use mode RX & TX
#define RFE_ERROR_CELL_WRONG_MODE	4	///<Wrong mode for cellular channel - Cellular FDD bands (1, 2, 3, and 7) are only allowed mode RX & TX, while TDD band 38 is allowed only RX or TX mode
#define RFE_ERROR_CELL_TX_NOT_EQUAL_RX	5	///<Cellular channels must be the same both for RX and TX

///LimeRFE configurable GPIO pins
#define RFE_GPIO4 4
#define RFE_GPIO5 5

///LimeRFE configurable GPIO pins direction
#define RFE_GPIO_DIR_OUT	0	///<Output
#define RFE_GPIO_DIR_IN		1	///<Input

///LimeRFE SWR subsystem enable
#define RFE_SWR_DISABLE	0	///<Disable
#define RFE_SWR_ENABLE	1	///<Enable

///LimeRFE SWR subsystem source
#define RFE_SWR_SRC_EXT		0	///<External - SWR signals are supplied to the connectors J18 for forward and J17 for forward signal. To be supplied from the external coupler.
#define RFE_SWR_SRC_CELL	1	///<Cellular - Power Meter signal is provided internally from the cellular TX amplifier outputs.

///LimeRFE board configuration parameters
typedef struct
{
	char channelIDRX;	///<RX channel ID (convenience constants defined in limeRFE.h).For example constant RFE_CID_HAM_0145 identifies 2m(144 - 146 MHz) HAM channel.
	char channelIDTX;	///<TX channel ID (convenience constants defined in limeRFE.h).For example constant RFE_CID_HAM_0145 identifies 2m(144 - 146 MHz) HAM channel.If - 1 then the same channel as for RX is used.
	char selPortRX;	///<RX port (convenience constants defined in limeRFE.h).
	char selPortTX;	///<TX port (convenience constants defined in limeRFE.h).
	char mode;	///<Operation mode (defined in limeRFE.h). Not all modes all applicable to all congfigurations.HAM channels using same port for RX and TX are not allowed RFE_MODE_TXRX mode. Cellular FDD bands 1, 2, 3, and 7 are always in RFE_MODE_TXRX mode.Cellular TDD band 38 can not be in RFE_MODE_TXRX.
	char notchOnOff;	///<Specifies whether the notch filter is applied or not (convenience constants defined in limeRFE.h).
	char attValue;	///<Specifies the attenuation in the RX path. Attenuation [dB] = 2 * attenuation.
	char enableSWR;	///<Enable SWR subsystem. (convenience constants defined in limeRFE.h).
	char sourceSWR;	///<SWR subsystem source. (convenience constants defined in limeRFE.h).
} rfe_boardState;

#ifdef __cplusplus
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__
#   define CALL_CONV __cdecl
#   ifdef __GNUC__
#       define API_EXPORT __attribute__ ((dllexport))
#   else
#       define API_EXPORT __declspec(dllexport)
#   endif
#elif defined _DOXYGEN_ONLY_
	/** Marks an API routine to be made visible to the dynamic loader.
	*  This is OS and/or compiler-specific. */
#   define API_EXPORT
	/** Specifies calling convention, if necessary.
	*  This is OS and/or compiler-specific. */
#   define CALL_CONV
#else
#   define API_EXPORT __attribute__ ((visibility ("default")))
#   define CALL_CONV
#endif

/****************************************************************************
*   LimeRFE API Functions
*****************************************************************************/
/**
* @defgroup RFE_API    LimeRFE API Functions
*
* @{
*/
/**
*This functions opens port to LimeRFE in case of USB communication.
*In case of I2C communication via LimeSDR, this function is not needed.
*
* @param serialport  Serial port name, e.g. �COM3�
* @param baudrate    Baudrate, e.g. 9600. Should match the value in firmware, by default 9600.
*
* @return            Positive number on success, (-1) on failure
*/
API_EXPORT int CALL_CONV RFE_Open(const char* serialport, int baudrate);

/**
*This function closes the port previously opened with RFE_Open.
*
* @param fd          Port file handle previously obtained from invoking RFE_Open.
*
* @return            None
*/
API_EXPORT void CALL_CONV RFE_Close(int fd);

/**
*This function gets the firmware and hardware version, as well as 2 status bytes (reserved for future use).
*
* @param dev         LimeSDR device obtained by invoking LMS_Open. In case of direct USB communication this value should be NULL.
* @param fd          Port file handle previously obtained from invoking RFE_Open. Used only in case of direct USB communication.
* @param cinfo       Board info: cinfo[0] - Firmware version; cinfo[1] - Hardware version; cinfo[2] - Status (reserved for future use); cinfo[3] - Status (reserved for future use)
*
* @return             0 on success, other on failure (see LimeRFE error codes)
*/
API_EXPORT int CALL_CONV RFE_GetInfo(lms_device_t *dev, int fd, unsigned char* cinfo);

/**
*This function loads LimeRFE configuration from an .ini file, and configures the board accordingly.
*
* @param dev         LimeSDR device obtained by invoking LMS_Open. In case of direct USB communication this value should be NULL.
* @param fd          Port file handle previously obtained from invoking RFE_Open. Used only in case of direct USB communication.
* @param filename    Full path to .ini configuration file
*
* @return            0 on success, other on failure (see LimeRFE error codes)
*/
API_EXPORT int CALL_CONV RFE_LoadConfig(lms_device_t *dev, int fd, const char *filename);

/**
*This function Resets the board.
*
* @param dev         LimeSDR device obtained by invoking LMS_Open. In case of direct USB communication this value should be NULL.
* @param fd          Port file handle previously obtained from invoking RFE_Open. Used only in case of direct USB communication.
*
* @return            0 on success, other on failure (see LimeRFE error codes)
*/
API_EXPORT int CALL_CONV RFE_Reset(lms_device_t *dev, int fd);

/**
*This function configures the LimeRFE board.
*
* @param dev           LimeSDR device obtained by invoking LMS_Open. In case of direct USB communication this value should be NULL.
* @param fd            Port file handle previously obtained from invoking RFE_Open. Used only in case of direct USB communication.
* @param channelIDRX   RX channel to be acitvated (convenience constants defined in limeRFE.h). For example constant RFE_CID_HAM_0145 identifies 2m (144 - 146 MHz) HAM channel.
* @param channelIDTX   TX channel to be acitvated (convenience constants defined in limeRFE.h). For example constant RFE_CID_HAM_0145 identifies 2m (144 - 146 MHz) HAM channel. If -1 then the same channel as for RX is used.
* @param portRX        RX port (convenience constants defined in limeRFE.h).
* @param portTX        TX port (convenience constants defined in limeRFE.h).
* @param mode          Operation mode (defined in limeRFE.h). Not all modes all applicable to all congfigurations. HAM channels using same port for RX and TX are not allowed RFE_MODE_TXRX mode. Cellular FDD bands 1, 2, 3, and 7 are always in RFE_MODE_TXRX mode. Cellular TDD band 38 can not be in RFE_MODE_TXRX.
* @param notch         Specifies whether the notch filter is applied or not (convenience constants defined in limeRFE.h).
* @param attenuation   Specifies the attenuation in the RX path. Attenuation [dB] = 2 * attenuation.
* @param enableSWR     Enable SWR subsystem. (convenience constants defined in limeRFE.h).
* @param sourceSWR     SWR subsystem source. (convenience constants defined in limeRFE.h).
*
* @return              0 on success, other on failure (see LimeRFE error codes)
*/
API_EXPORT int CALL_CONV RFE_Configure(lms_device_t *dev, char fd, char channelIDRX, char channelIDTX, char portRX, char portTX, char mode, char notch, char attenuation, char enableSWR, char sourceSWR);

/**
*This function configures the LimeRFE board. It's functionality is identical to RFE_Configure, with different arguments.
*
* @param dev           LimeSDR device obtained by invoking LMS_Open. In case of direct USB communication this value should be NULL.
* @param fd            Port file handle previously obtained from invoking RFE_Open. Used only in case of direct USB communication.
* @param state         Structure containing configuration parameters.
*
* @return              0 on success, other on failure (see LimeRFE error codes)
*/
API_EXPORT int RFE_ConfigureState(lms_device_t *dev, int fd, rfe_boardState state);

/**
*This function sets the LimeRFE mode (receive, transmit, both, or none)
*
* @param dev           LimeSDR device obtained by invoking LMS_Open. In case of direct USB communication this value should be NULL.
* @param fd            Port file handle previously obtained from invoking RFE_Open. Used only in case of direct USB communication.
* @param mode          Operation mode (convenience constants defined in limeRFE.h). Not all modes all applicable to all congfigurations. HAM channels using same port for RX and TX are not allowed RFE_MODE_TXRX mode. Cellular FDD bands 1, 2, 3, and 7 are always in RFE_MODE_TXRX mode. Cellular TDD band 38 can not be in RFE_MODE_TXRX.
*
* @return              0 on success, other on failure (see LimeRFE error codes)
*/
API_EXPORT int CALL_CONV RFE_Mode(lms_device_t *dev, int fd, int mode);

/**
*This function reads the value of the speficied ADC.
*
* @param dev           LimeSDR device obtained by invoking LMS_Open. In case of direct USB communication this value should be NULL.
* @param fd            Port file handle previously obtained from invoking RFE_Open. Used only in case of direct USB communication.
* @param adcID         Specifies which ADC is to be read (convenience constants defined in limeRFE.h).
* @param value         ADC value.
*
* @return              0 on success, other on failure (see LimeRFE error codes)
*/
API_EXPORT int CALL_CONV RFE_ReadADC(lms_device_t *dev, int fd, int adcID, int* value);

/**
*This function configures GPIO pin. Only pins 4 and 5 are configurable.
*
* @param dev           LimeSDR device obtained by invoking LMS_Open. In case of direct USB communication this value should be NULL.
* @param fd            Port file handle previously obtained from invoking RFE_Open. Used only in case of direct USB communication.
* @param gpioNum       GPIO pin number. Only pins 4 and 5 are configurable.
* @param direction     GPIO pin direction (convenience constants defined in limeRFE.h). 0 - Output; 1 - Input.
*
* @return              0 on success, other on failure (see LimeRFE error codes)
*/
API_EXPORT int CALL_CONV RFE_ConfGPIO(lms_device_t *dev, int fd, int gpioNum, int direction);

/**
*This function sets the GPIO pin value. GPIO pin should have been previously configured as output using RFE_ConfGPIO function.
*
* @param dev           LimeSDR device obtained by invoking LMS_Open. In case of direct USB communication this value should be NULL.
* @param fd            Port file handle previously obtained from invoking RFE_Open. Used only in case of direct USB communication.
* @param gpioNum       GPIO pin number. Only pins 4 and 5 are configurable.
* @param val           GPIO pin value.
*
* @return              0 on success, other on failure (see LimeRFE error codes)
*/
API_EXPORT int CALL_CONV RFE_SetGPIO(lms_device_t *dev, int fd, int gpioNum, int val);

/**
*This function reads the GPIO pin value. GPIO pin should have been previously configured as input using RFE_ConfGPIO function.
*
* @param dev           LimeSDR device obtained by invoking LMS_Open. In case of direct USB communication this value should be NULL.
* @param fd            Port file handle previously obtained from invoking RFE_Open. Used only in case of direct USB communication.
* @param gpioNum       GPIO pin number. Only pins 4 and 5 are configurable.
* @param val           GPIO pin value.
*
* @return              0 on success, other on failure (see LimeRFE error codes)
*/
API_EXPORT int CALL_CONV RFE_GetGPIO(lms_device_t *dev, int fd, int gpioNum, int * val);

/** @} (End RFE_API) */

#ifdef __cplusplus
} //extern "C"
#endif

#endif // __limeRFE__