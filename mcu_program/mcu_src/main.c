#include "LMS7002_REGx51.h"
#include "spi.h"	  
#include "lms7002m_calibrations.h"
#include "LMS7002M_parameters_compact.h"
#include "lms7002m_controls.h"

bit runProcedure = false;
uint8_t currentInstruction;
uint8_t lastInstruction;
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
void UpdateReferenceClock()
{	
	RefClk = inputRegs[0]; //integer part MHz
	RefClk += (((uint16_t)inputRegs[1] << 8) | inputRegs[2]) / 1000.0; //fractional part MHz
	RefClk *= 1e6;
	P1 = MCU_IDLE;
}

void UpdateBW()
{	
	bandwidthRF = inputRegs[0]; //integer part MHz
	bandwidthRF += (((uint16_t)inputRegs[1] << 8) | inputRegs[2]) / 1000.0; //fractional part MHz
	bandwidthRF *= 1e6;
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
	lastInstruction = P0;
	while(1) 
	{			
		if(runProcedure)
		{
			switch(currentInstruction)
			{
			case 0:
				P1 = MCU_IDLE;
				break;
			case 1: //CalibrateTx
				P1 = MCU_IDLE | CalibrateTx();
				break;
			case 2: //CalibrateRx
                P1 = MCU_IDLE | CalibrateRx();
				break;
			case 3: 
				UpdateBW();
				break;
			case 4: //update ref clk
				UpdateReferenceClock();
				break;
			case 255: //return program ID
				P1 = 0x02;
				break;
			default:
				break;
			}
			lastInstruction = currentInstruction; 
			runProcedure = false;
		}
	}
}

