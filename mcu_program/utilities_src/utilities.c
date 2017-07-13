#include "utilities.h"
#include "LMS7002M_parameters_compact.h"
#include "spi.h"

//db_gain, lna setting, pga setting offset
//MUST have gain values in 1 increments starting from 0
static ROM const int8_t agcTable[] =
{
0,15,0,
1,15,-1,
2,15,-2,
3,15,-3,
4,15,-4,
5,15,-5,
6,15,-6,
7,14,-6,
8,13,-6,
9,12,-6,
10,11,-6,
11,10,-6,
12,9,-6,
13,8,-4,
14,8,-5,
15,8,-6,
16,7,-3,
17,7,-4,
18,7,-5,
19,6,-3,
20,6,-4,
21,6,-5,
22,5,-3,
23,5,-4,
24,5,-5,
25,4,-3,
26,4,-4,
27,4,-5,
28,3,-1,
29,3,-2,
30,3,-3,
31,2,-1,
32,2,-2,
33,2,-3,
34,2,-4,
35,2,-5,
36,2,-6,
37,2,-7,
38,2,-8,
39,2,-9,
40,1,-5,
41,1,-6,
42,1,-7,
43,1,-8,
44,1,-9,
45,1,-10,
46,1,-11,
47,1,-12,
48,1,-13,
49,1,-14,
};
#define AGC_TABLE_ROW_CNT sizeof(agcTable)/sizeof(agcTable[0])
#define AGC_TABLE_COL_CNT 3

const uint8_t rssiFloor = 120;
const uint8_t rssiCeil = 203;
const float rssiStep = 2.75;
uint8_t pgaCeil = 12;
uint8_t tiaGain = 2;

extern bool gStopWork; //global variable to interupt infinite cycles

void RunAGC()
{
    uint8_t pga, lna, c_ctl_pga, needGain_dB, tableArrayOffset;
    int16_t rssiDiff;
    pgaCeil = SPI_read(0x002D);

    while(!gStopWork)
    {
        rssiDiff = Get_SPI_Reg_bits(RSSI_RSSI1_VAL)-rssiFloor;
        if(rssiDiff <= 0)
            tableArrayOffset = 0;
        else
        {
            needGain_dB = (rssiDiff/rssiStep+0.5);
            tableArrayOffset = needGain_dB*AGC_TABLE_COL_CNT;
            if(tableArrayOffset > AGC_TABLE_ROW_CNT/AGC_TABLE_COL_CNT)
                tableArrayOffset = (AGC_TABLE_ROW_CNT-1)*AGC_TABLE_COL_CNT; //last entry, max gain
        }

        lna = agcTable[tableArrayOffset+1];
        pga = agcTable[tableArrayOffset+2]+pgaCeil;
        if(pga < 0)
            pga = 0;

        if(pga < 8)
            c_ctl_pga = 3;
        else if(pga < 13)
            c_ctl_pga = 2;
        else if(pga < 21)
            c_ctl_pga = 1;
        else
            c_ctl_pga = 0;

        SPI_write(0x0126, (c_ctl_pga << 11) | (pga << 6) | (lna << 2) | tiaGain);
    }
}
