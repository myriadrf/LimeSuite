#include "LMS7002_REGx51.h"
#include "spi.h"
#include "utilities.h"

#include "typedefs.h"

bool runProcedure = false;
uint8_t currentInstruction;

#define MCU_PARAMETER_ADDRESS 0x002D //register used to pass parameter values to MCU

#define INPUT_COUNT 3
uint8_t inputRegs[INPUT_COUNT];

enum
{
    MCU_WORKING = 0xFF,
    MCU_IDLE = 0x00,
};

void ext2_int() interrupt 7
{
    uint8_t i;
    P1 = MCU_WORKING;
    for(i=INPUT_COUNT-1; i>0; --i)
        inputRegs[i] = inputRegs[i-1];
    inputRegs[0] = P0;
    P1 = MCU_IDLE;
}

bool gStopWork;
void ext3_int() interrupt 8
{
    P1 = MCU_WORKING;
    currentInstruction = P0;
    if(P0 == 0)
        gStopWork = true;
    else
        gStopWork = false;
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
    ucSEN=1;

    //P1 returns MCU status
    while(1)
    {
        if(runProcedure)
        {
            switch(currentInstruction)
            {
            case 0:
                P1 = MCU_IDLE;
                break;
            case 254: //AGC
                P1 = MCU_IDLE;
                RunAGC();
                //P1 = MCU_IDLE;
            case 255: //return program ID
                P1 = 0x04;
                break;
            }
            runProcedure = false;
            gStopWork = false;
        }
    }
}

