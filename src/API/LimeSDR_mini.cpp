/*
 * File:   LimeSDR_mini.cpp
 * Author: Ignas J
 *
 * Created on September 18, 2016
 */
#include "LimeSDR_mini.h"
#include "lime/LimeSuite.h"
#include "Logger.h"
#include "FPGA_Mini.h"
#include "device_constants.h"

namespace lime
{

LMS7_LimeSDR_mini::LMS7_LimeSDR_mini(lime::IConnection* conn, lime::LMS7_Device *obj) : lime::LMS7_Device(obj) 
{
    fpga = new lime::FPGA_Mini();
    while (obj && lms_list.size() > 1)
    {
        delete lms_list.back();
        lms_list.pop_back();
    }
    fpga->SetConnection(conn);
    double refClk = fpga->DetectRefClk();
    this->lms_list[0]->SetConnection(conn);
    mStreamers.push_back(new lime::Streamer(fpga,lms_list[0],0));
    lms_list[0]->SetReferenceClk_SX(false, refClk);
    connection = conn;
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
        {0x0089, 0x1078}, {0x008B, 0x3E00}, {0x008C, 0x267B}, {0x0092, 0xFFFF},
	{0x0093, 0x03FF}, {0x00A6, 0x0001}, {0x00A7, 0x3865}, {0x00A9, 0x8000},
        {0x00AC, 0x2000}, {0x0105, 0x0011}, {0x0108, 0x118C}, {0x0109, 0x6100},
        {0x010A, 0x1F4C}, {0x010B, 0x0001}, {0x010C, 0x8865}, {0x010E, 0x0000},
        {0x010F, 0x3142}, {0x0110, 0x2B14}, {0x0111, 0x0000}, {0x0112, 0x942E},
        {0x0113, 0x03C2}, {0x0114, 0x00D0}, {0x0117, 0x1230}, {0x0119, 0x18D2},
        {0x011C, 0x8941}, {0x011D, 0x0000}, {0x011E, 0x0740}, {0x0120, 0xE6B4},
        {0x0121, 0x3650}, {0x0123, 0x000F}, {0x0200, 0x00E1}, {0x0208, 0x017B},
        {0x020B, 0x4000}, {0x020C, 0x8000}, {0x0400, 0x8081}, {0x0404, 0x0006},
        {0x040B, 0x1020}, {0x040C, 0x00FB}
    };

    lime::LMS7002M* lms = lms_list[0];
    if (lms->ResetChip() != 0)
        return -1;

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    for (auto i : initVals)
        lms->SPI_write(i.adr, i.val);

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    lms->SPI_write(0x0123, 0x000F);  //SXT
    lms->SPI_write(0x0120, 0xE6B4);  //SXT
    lms->SPI_write(0x011C, 0x8941);  //SXT
    lms->EnableChannel(false, false);
    lms->EnableChannel(true, false);

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    
    if (SetFrequency(true,0,1250e6)!=0)
        return -1;
    if (SetFrequency(false,0,1200e6)!=0)
        return -1;
    if (SetRate(10e6, 1)!=0)
        return -1;

    return 0;
}

unsigned LMS7_LimeSDR_mini::GetNumChannels(const bool tx) const
{
    return 1;
};

int LMS7_LimeSDR_mini::SetFrequency(bool isTx, unsigned chan, double f_Hz)
{
    lime::LMS7002M* lms = lms_list[0];
    
    ChannelInfo& channel = isTx ? tx_channels[0] : rx_channels[0];
    channel.freq = f_Hz;
       
    auto setTDD = [=](double center)->int
    {
        ChannelInfo& other = isTx ? rx_channels[0] : tx_channels[0];    
        bool tdd =  fabs(other.freq+other.cF_offset_nco-center) > 0.1 ? false : true;    
        lms->EnableSXTDD(tdd);
        if (isTx || (!tdd))
            if (lms->SetFrequencySX(isTx, center) != 0)
                return -1;
        return 0;
    };
     
    if (f_Hz < 30e6)
    {
        if (setTDD(30e6) != 0)
            return -1;
        channel.cF_offset_nco = 30e6-f_Hz;
        if (SetRate(isTx,GetRate(isTx,0),2)!=0)
            return -1;
        return 0;
    }

    if (channel.cF_offset_nco != 0)
        SetNCOFreq(isTx, 0, -1, 0.0);
    channel.cF_offset_nco = 0;
    if (setTDD(f_Hz) != 0)
        return -1;
    return 0;
}

std::vector<std::string> LMS7_LimeSDR_mini::GetPathNames(bool dir_tx, unsigned chan) const
{
    if (dir_tx)
        return {"NONE", "TX_PATH1", "TX_PATH2"};
    else
        return {"NONE", "LNA_H", "LNA_L(NC)", "LNA_W"};
}

int LMS7_LimeSDR_mini::SetPath(bool tx, unsigned chan, unsigned path)
{
    lime::LMS7002M* lms = lms_list[0];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan%2) + 1) != 0)
        return -1;
    if (tx==false)
    {
        if ((lms->Modify_SPI_Reg_bits(LMS7param(SEL_PATH_RFE),path)!=0)
        || (lms->Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_L_RFE), path != 2) != 0)
        || (lms->Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_W_RFE), path != 3) != 0))
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
        if ((lms->Modify_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF), path == LMS_PATH_TX1) != 0)
        || (lms->Modify_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF), path == LMS_PATH_TX2) != 0))
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

LMS7_Device::Range LMS7_LimeSDR_mini::GetRxPathBand(unsigned path, unsigned chan) const
{
  switch (path)
  {
      case LMS_PATH_LNAH: return Range(2e9, 3.8e9);
      case LMS_PATH_LNAW: return Range(30e6, 2e9);
      default: return Range();
  }
}

std::vector<std::string> LMS7_LimeSDR_mini::GetProgramModes() const
{
    return {program_mode::autoUpdate,
            program_mode::fpgaFlash, program_mode::fpgaReset,
            program_mode::mcuRAM, program_mode::mcuEEPROM, program_mode::mcuReset};
}


}//namespace lime


