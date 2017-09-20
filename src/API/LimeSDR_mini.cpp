/*
 * File:   LimeSDR_mini.cpp
 * Author: Ignas J
 *
 * Created on September 18, 2016
 */
#include "LimeSDR_mini.h"

LMS7_LimeSDR_mini::LMS7_LimeSDR_mini(LMS7_Device *obj) : LMS7_Device(obj)
{

}

int LMS7_LimeSDR_mini::Init()
{
    struct regVal
    {
        uint16_t adr;
        uint16_t val;
     };

    const std::vector<regVal> initVals = {
        {0x0022, 0x07FF}, {0x0023, 0x5550}, {0x002B, 0x0038}, {0x002C, 0x0000},
        {0x002D, 0x0641}, {0x0086, 0x4101}, {0x0087, 0x5555}, {0x0088, 0x0525},
        {0x0089, 0x1078}, {0x008B, 0x1F8C}, {0x008C, 0x267B}, {0x00A6, 0x0001},
	{0x0092, 0xFFFF}, {0x0093, 0x03FF}, {0x00A9, 0x8000}, {0x00AC, 0x2000},
	{0x0108, 0x3026}, {0x010C, 0x8865}, {0x010E, 0x0000}, {0x0110, 0x0BFF},
	{0x0113, 0x03C2}, {0x011C, 0xA941}, {0x011D, 0x0000}, {0x011E, 0x0984}, 
	{0x0120, 0xB9FF}, {0x0121, 0x3650}, {0x0122, 0x033F}, {0x0123, 0x267B}, 
	{0x0200, 0x00E1}, {0x0208, 0x0170}, {0x020B, 0x4000}, {0x020C, 0x8000}, 
	{0x0400, 0x8081}, {0x040B, 0x1020}, {0x040C, 0x00F8}
    };

    lime::LMS7002M* lms = lms_list[0];
    if (lms->ResetChip() != 0)
        return -1;

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    for (auto i : initVals)
        lms->SPI_write(i.adr, i.val);

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    lms->EnableChannel(false, false);
    lms->EnableChannel(true, false);

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);

    if (lms->UploadAll()!=0)
        return -1;
    
    if (SetTxFrequency(0,1200e6)!=0)
        return -1;
    if (SetRxFrequency(0,1200e6)!=0)
        return -1;
    if (SetRate(10e6,2)!=0)
        return -1;

    return 0;
}

size_t LMS7_LimeSDR_mini::GetNumChannels(const bool tx) const
{
    return 1;
};



