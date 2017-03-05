#include "LMS7002_REGx51.h"
#include "spi.h"	  
#include "lms7002m_calibrations.h"
#include "lms7002m_filters.h"
#include "LMS7002M_parameters_compact.h"
#include "lms7002m_controls.h"

#include "typedefs.h"

bool runProcedure = false;
uint8_t currentInstruction;
extern float_type RefClk;
extern float_type bandwidthRF;
#define MCU_PARAMETER_ADDRESS 0x002D //register used to pass parameter values to MCU

#define INPUT_COUNT 3
uint8_t inputRegs[INPUT_COUNT];

enum
{
	MCU_WORKING = 0xFF,
	MCU_IDLE = 0x00,
};

/**	@brief Reads reference clock from LMS register
*/
void UpdateFreq(bool refClk)
{
	const float freq = 1e6*(inputRegs[0] + ((((uint16_t)inputRegs[1] << 8) | inputRegs[2]) / 1000.0)); //integer part MHz
	if(refClk)
		RefClk = freq;
	else
		bandwidthRF = freq;
	P1 = MCU_IDLE;
}

void ext2_int() interrupt 7
{
	uint8_t i;
	P1 = MCU_WORKING;
	for(i=INPUT_COUNT-1; i>0; --i)
		inputRegs[i] = inputRegs[i-1];
	inputRegs[0] = P0;
	P1 = MCU_IDLE;
}

void ext3_int() interrupt 8
{
	P1 = MCU_WORKING;
	currentInstruction = P0;
	runProcedure = true;
}

const uint16_t proxyRegAddr = 0x002D;
const uint16_t proxyWrValue = 0x020C;
const uint16_t proxyRdValue = 0x040B;
            
uint8_t ProxyWrite()
{
    uint16_t addr;
    uint16_t wrValue;
    P1 = MCU_WORKING;
    slowSPI = 1;
    addr = SPI_read(proxyRegAddr);
    wrValue = SPI_read(proxyWrValue);
    SPI_write(addr, wrValue);
    slowSPI = 0;
    return MCU_IDLE;
}

uint8_t ProxyRead()
{
    uint16_t addr;
    uint16_t rdValue;
    P1 = MCU_WORKING;
    slowSPI = 1;
    addr = SPI_read(proxyRegAddr);
    rdValue = SPI_read(addr);
    SPI_write(proxyRdValue, rdValue);
    slowSPI = 0;
    return MCU_IDLE;
}

/*
	P1[7] : 0-MCU idle, 1-MCU_working
	P1[6:0] : return status (while working = 0x3F)
*/
void main()  //main routine
{
	SP=0xD0; // Set stack pointer
	DIR0=0x00; // ;DIR0 - Configure P0 as all inputs
  	DIR1=0xFF;  // ;DIR1 - Configure P1 as all outputs
	P1 = MCU_IDLE; 
  	DIR2=0x07;  // ;DIR2 -  Configure P2_3 is input
	IEN1=0xFF;//0x04;  //EX2=1 enable external interrupt 2
	IEN0=0x80;

	ucSCLK=0; //repairs external SPI
	ucSEN=1;//

	//P1 returns MCU status
	while(1) 
	{			
		if(runProcedure)
		{
			switch(currentInstruction)
			{
			//case 0:
			//	P1 = MCU_IDLE;
			//	break;
			case 1: //CalibrateTx
				P1 = MCU_IDLE | CalibrateTx();
				break;
			case 2: //CalibrateRx
                P1 = MCU_IDLE | CalibrateRx();
				break;
			case 3: 
				UpdateFreq(0);
				//UpdateBW();
				break;
			case 4: //update ref clk
				//UpdateReferenceClock();
				UpdateFreq(1);
				break;
			case 5:
				P1 = TuneRxFilter(bandwidthRF);
				break;	
            case 6:
				P1 = TuneTxFilter(bandwidthRF);
				break;	
            case 7:
                P1 = ProxyWrite();
				break;
            case 8:
                P1 = ProxyRead();
				break;
			case 255: //return program ID
				P1 = 0x03;
				break;
			
			}
			runProcedure = false;
		}
	}
}

