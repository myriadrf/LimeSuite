/**
@file	lms7002_defines.h
@author Lime Microsystems
@brief	enumerations of available LMS7002M commands and statuses
*/
#pragma once

const int LMS_RST_DEACTIVATE = 0;
const int LMS_RST_ACTIVATE = 1;
const int LMS_RST_PULSE = 2;

enum eLMS_DEV
{
	LMS_DEV_UNKNOWN,
	LMS_DEV_EVB6,
	LMS_DEV_DIGIGREEN,
	LMS_DEV_DIGIRED,
	LMS_DEV_EVB7,
	LMS_DEV_ZIPPER,
	LMS_DEV_SOCKETBOARD,
	LMS_DEV_EVB7V2,
	LMS_DEV_STREAM,
	LMS_DEV_NOVENA,
	LMS_DEV_DATASPARK,
	LMS_DEV_RFSPARK,
	LMS_DEV_LMS6002USB,
	LMS_DEV_RFESPARK,
	LMS_DEV_SODERA,
	
	LMS_DEV_COUNT
};

const char LMS_DEV_NAMES[][80] = 
{
	"UNKNOWN",
	"EVB6",
	"DigiGreen",
	"DigiRed",
	"EVB7",
	"ZIPPER",
	"Socket Board",
	"EVB7_v2",
	"Stream",
	"Novena",
	"DataSpark",
	"RF-Spark",
	"LMS6002-USB Stick",
	"RF-ESpark",
	"SoDeRa",
};

static const char* GetDeviceName(const eLMS_DEV device)
{
    if (LMS_DEV_UNKNOWN < device && device < LMS_DEV_COUNT)
        return LMS_DEV_NAMES[device];
    else
        return LMS_DEV_NAMES[LMS_DEV_UNKNOWN];
}

enum eEXP_BOARD 
{
	EXP_BOARD_UNKNOWN,
	EXP_BOARD_UNSUPPORTED,
	EXP_BOARD_NO,
	EXP_BOARD_MYRIAD1,
	EXP_BOARD_MYRIAD2,
	EXP_BOARD_MYRIAD_NOVENA,
	EXP_BOARD_HPM1000,
	EXP_BOARD_MYRIAD7,
	EXP_BOARD_HPM7,
	
	EXP_BOARD_COUNT
};

const char EXP_BOARD_NAMES[][80] = 
{
	"UNKNOWN", 
	"UNSUPPORTED", 
	"NOT AVAILABLE",
	"Myriad1",
	"Myriad2",
	"Novena",
	"HPM1000",
	"Myriad7",
	"HMP7"
};

static const char* GetExpansionBoardName(const eEXP_BOARD board)
{
    if (EXP_BOARD_UNKNOWN < board && board < EXP_BOARD_COUNT)
        return EXP_BOARD_NAMES[board];
    else
        return EXP_BOARD_NAMES[EXP_BOARD_UNKNOWN];
}
			
struct LMS64C_Info
{
	unsigned char firmware;
    unsigned char deviceType;
    unsigned char protocol;
    unsigned char hardware;
    unsigned char expansionBoard;
};

enum eCMD_LMS
{
    CMD_GET_INFO = 0x00,
    CMD_LMS6002_RST = 0x10,
    ///Writes data to SI5356 synthesizer via I2C
    CMD_SI5356_WR = 0x11,
    ///Reads data from SI5356 synthesizer via I2C
    CMD_SI5356_RD = 0x12,
    ///Writes data to SI5351 synthesizer via I2C
    CMD_SI5351_WR = 0x13,
    ///Reads data from SI5351 synthesizer via I2C
    CMD_SI5351_RD = 0x14,
    ///PanelBus DVI (HDMI) Transmitter control
    CMD_TFP410_WR = 0x15,
    ///PanelBus DVI (HDMI) Transmitter control
    CMD_TFP410_RD = 0x16,
    ///Sets new LMS7002M chip’s RESET pin level (0, 1, pulse)
    CMD_LMS7002_RST = 0x20,
    ///Writes data to LMS7002M chip via SPI
    CMD_LMS7002_WR = 0x21,
    ///Reads data from LMS7002M chip via SPI
    CMD_LMS7002_RD = 0x22,
    ///Writes data to LMS6002 chip via SPI
    CMD_LMS6002_WR = 0x23,
    ///Reads data from LMS6002 chip via SPI
    CMD_LMS6002_RD = 0x24,

    CMD_LMS_LNA = 0x2A,
    CMD_LMS_PA = 0x2B,

    CMD_PROG_MCU = 0x2C,
    ///Writes data to ADF4002 chip via SPI
    CMD_ADF4002_WR = 0x31,

    CMD_USB_FIFO_RST = 0x40,
    CMD_PE636040_WR = 0x41,
	CMD_PE636040_RD = 0x42,

	CMD_GPIO_WR = 0x51,
	CMD_GPIO_RD = 0x52,

	CMD_ALTERA_FPGA_GW_WR = 0x53,
	CMD_ALTERA_FPGA_GW_RD = 0x54,

	CMD_BRDSPI_WR = 0x55,//16 bit spi for stream, dataspark control
    CMD_BRDSPI_RD = 0x56,//16 bit spi for stream, dataspark control
	CMD_BRDSPI8_WR = 0x57, //8 + 8 bit spi for stream, dataspark control
	CMD_BRDSPI8_RD = 0x58, //8 + 8 bit spi for stream, dataspark control
	
	CMD_BRDCONF_WR = 0x5D, //write config data to board
	CMD_BRDCONF_RD = 0x5E, //read config data from board
	
	CMD_ANALOG_VAL_WR = 0x61, //write analog value
	CMD_ANALOG_VAL_RD = 0x62, //read analog value

	CMD_MYRIAD_RST = 0x80,
	CMD_MYRIAD_WR = 0x81,
	CMD_MYRIAD_RD = 0x82,
	CMD_MYRIAD_PROG = 0x8C
};

enum eCMD_STATUS
{
    STATUS_UNDEFINED,
    STATUS_COMPLETED_CMD,
    STATUS_UNKNOWN_CMD,
    STATUS_BUSY_CMD,
    STATUS_MANY_BLOCKS_CMD,
    STATUS_ERROR_CMD,
    STATUS_WRONG_ORDER_CMD,
	STATUS_RESOURCE_DENIED_CMD,
    STATUS_COUNT
};

static const char  status_text[][32]=
{
    "Undefined/Failure",
    "Completed",
    "Unknown command",
    "Busy",
    "Too many blocks",
    "Error",
    "Wrong order",
    "Resource denied"
};

static const char* status2string(const int status)
{
    if(status >= 0 && status < STATUS_COUNT)
        return status_text[status];
    else
        return "Unknown status";
}

enum eADC_UNITS
{
	RAW,
	VOLTAGE,
	CURRENT,
	RESISTANCE,
	POWER,
	TEMPERATURE,
	ADC_UNITS_COUNT
};

static const char  adc_units_text[][8] = {"", "V", "A", "Ohm", "W", "C"};

static const char* adcUnits2string(const unsigned units)
{
	if (units < ADC_UNITS_COUNT)
		return adc_units_text[units];
	else
		return " unknown";
}
