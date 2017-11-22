/*
 * File:   LimeSDR_mini.cpp
 * Author: Ignas J
 *
 * Created on September 18, 2016
 */
#include "LimeSDR_mini.h"
#include "Logger.h"

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
        {0x002D, 0x0641}, {0x0086, 0x4101}, {0x0087, 0x5555}, {0x0088, 0x03F0},
        {0x0089, 0x1078}, {0x008B, 0x338E}, {0x008C, 0x267B}, {0x0092, 0xFFFF},
	{0x0093, 0x03FF}, {0x00A6, 0x0001}, {0x00A9, 0x8000}, {0x00AC, 0x2000},
        {0x0108, 0x318C}, {0x010C, 0x8865}, {0x010E, 0x0000}, {0x0110, 0x2B14},
        {0x0113, 0x03C2}, {0x011C, 0xA941}, {0x011D, 0x0000}, {0x011E, 0x0740},
        {0x0121, 0x3650}, {0x0122, 0x033F}, {0x0123, 0x267B}, {0x0200, 0x00E1},
        {0x0208, 0x0170}, {0x020B, 0x4000}, {0x020C, 0x8000}, {0x0400, 0x8081},
        {0x040B, 0x1020}, {0x040C, 0x00F8}
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
    
    if (SetTxFrequency(0,1250e6)!=0)
        return -1;
    if (SetRxFrequency(0,1200e6)!=0)
        return -1;
    if (SetRate(10e6,2)!=0)
        return -1;

    return 0;
}

unsigned LMS7_LimeSDR_mini::GetNumChannels(const bool tx) const
{
    return 1;
};

int LMS7_LimeSDR_mini::SetRxFrequency(size_t chan, double f_Hz)
{
    lime::LMS7002M* lms = lms_list[0];
    rx_channels[0].freq = f_Hz;
     
    if (f_Hz < 30e6)
    {
        if (lms->SetFrequencySX(false, 30e6) != 0)
            return -1;
        rx_channels[0].cF_offset_nco = 30e6-f_Hz;
        if (SetRate(false,GetRate(false,0),2)!=0)
            return -1;
        return 0;
    }

    if (rx_channels[0].cF_offset_nco != 0)
        SetNCO(false,0,-1,true);
    rx_channels[0].cF_offset_nco = 0;
    if (lms->SetFrequencySX(false, f_Hz) != 0)
        return -1;
    return 0;
}

int LMS7_LimeSDR_mini::SetTxFrequency(size_t chan, double f_Hz)
{
    lime::LMS7002M* lms = lms_list[0];
    tx_channels[0].freq = f_Hz;
    
    if (f_Hz < 30e6)
    {
        if (lms->SetFrequencySX(true, 30e6) != 0)
            return -1;
        tx_channels[0].cF_offset_nco = 30e6-f_Hz;
        if (SetRate(true,GetRate(true,0),2)!=0)
            return -1;
        return 0;
    }

    if (tx_channels[0].cF_offset_nco != 0)
        SetNCO(true,0,-1,false);
    tx_channels[0].cF_offset_nco = 0;
    if (lms->SetFrequencySX(true, f_Hz) != 0)
        return -1;
    return 0;
}

std::vector<std::string> LMS7_LimeSDR_mini::GetPathNames(bool dir_tx, size_t chan) const
{
    if (dir_tx)
        return {"NONE", "TX_PATH1", "TX_PATH2"};
    else
        return {"NONE", "LNA_H", "LNA_L(NC)", "LNA_W"};
}

int LMS7_LimeSDR_mini::SetPath(bool tx, size_t chan, size_t path)
{
    lime::LMS7002M* lms = lms_list[0];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan%2) + 1, true) != 0)
        return -1;
    if (tx==false)
    {
        if ((lms->Modify_SPI_Reg_bits(LMS7param(SEL_PATH_RFE),path,true)!=0)
        || (lms->Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_L_RFE), path != 2, true) != 0)
        || (lms->Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_W_RFE), path != 3, true) != 0))
            return -1;
        if (path==LMS_PATH_LNAW)
        {     
            uint16_t value;
            connection->ReadRegister(0x17,value);
            value &= ~(1<<8);
            value |= 1<<9;
            connection->WriteRegister(0x17, value);
        }
        else if (path==LMS_PATH_LNAH)
        {
            uint16_t value;
            connection->ReadRegister(0x17,value);
            value &= ~(1<<9);
            value |= 1<<8;
            connection->WriteRegister(0x17, value);
        }       
        else if (LMS_PATH_LNAL)
            lime::warning("LNAL has no connection to RF ports");
    }
    else
    {
        if ((lms->Modify_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF), path == LMS_PATH_TX1, true) != 0)
        || (lms->Modify_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF), path == LMS_PATH_TX2, true) != 0))
            return -1;
        if (path==LMS_PATH_TX1)
        {     
            uint16_t value;
            connection->ReadRegister(0x17,value);
            value &= ~(1<<13);
            value |= 1<<12;
            connection->WriteRegister(0x17, value);
        }
        else if (path==LMS_PATH_TX2)
        {
            uint16_t value;
            connection->ReadRegister(0x17,value);
            value |= 1<<13;
            connection->WriteRegister(0x17, value);
        }   
    }
    return 0;
}

lms_range_t LMS7_LimeSDR_mini::GetRxPathBand(size_t path, size_t chan) const
{
  lms_range_t ret;
  ret.step = 1;
  switch (path)
  {
      case LMS_PATH_LNAH:
            ret.max = 3.8e9;
            ret.min = 2e9;
            break;
      case LMS_PATH_LNAW:
            ret.max = 2e9;
            ret.min = 30e6;
            break;
      default:
            ret.max = 0;
            ret.min = 0;
            ret.step = 0;
            break;
  }
  return ret;
}



