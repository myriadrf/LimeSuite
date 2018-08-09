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
    if (refClk < 0)
    {
    	lime::error("Failed to detect reference clock");
    	refClk = 40e6;
    }
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

    const std::vector<regVal> initVals_1v0 = {
        {0x0022, 0x0FFF}, {0x0023, 0x5550}, {0x002B, 0x0038}, {0x002C, 0x0000},
        {0x002D, 0x0641}, {0x0086, 0x4101}, {0x0087, 0x5555}, {0x0088, 0x03F0},
        {0x0089, 0x1078}, {0x008B, 0x2100}, {0x008C, 0x267B}, {0x0092, 0xFFFF},
	{0x0093, 0x03FF}, {0x00A1, 0x656A}, {0x00A6, 0x0001}, {0x00A9, 0x8000},
        {0x00AC, 0x2000}, {0x0105, 0x0011}, {0x0108, 0x118C}, {0x0109, 0x6100},
        {0x010A, 0x1F4C}, {0x010B, 0x0001}, {0x010C, 0x8865}, {0x010E, 0x0000},
        {0x010F, 0x3142}, {0x0110, 0x2B14}, {0x0111, 0x0000}, {0x0112, 0x942E},
        {0x0113, 0x03C2}, {0x0114, 0x00D0}, {0x0117, 0x1230}, {0x0119, 0x18D2},
        {0x011C, 0x8941}, {0x011D, 0x0000}, {0x011E, 0x0740}, {0x0120, 0xE6B4},
        {0x0121, 0x3650}, {0x0123, 0x000F}, {0x0200, 0x00E1}, {0x0208, 0x017B},
        {0x020B, 0x4000}, {0x020C, 0x8000}, {0x0400, 0x8081}, {0x0404, 0x0006},
        {0x040B, 0x1020}, {0x040C, 0x00FB}
    };

    const std::vector<regVal> initVals_1v2 = {
        {0x0022, 0x0FFF}, {0x0023, 0x5550}, {0x002B, 0x0038}, {0x002C, 0x0000},
        {0x002D, 0x0641}, {0x0086, 0x4101}, {0x0087, 0x5555}, {0x0088, 0x03F0},
        {0x0089, 0x1078}, {0x008B, 0x2100}, {0x008C, 0x267B}, {0x00A1, 0x656A},
        {0x00A6, 0x0009}, {0x00A7, 0x8A8A}, {0x00A9, 0x8000}, {0x00AC, 0x2000},
        {0x0105, 0x0011}, {0x0108, 0x118C}, {0x0109, 0x6100}, {0x010A, 0x1F4C},
        {0x010B, 0x0001}, {0x010C, 0x8865}, {0x010E, 0x0000}, {0x010F, 0x3142},
        {0x0110, 0x2B14}, {0x0111, 0x0000}, {0x0112, 0x942E}, {0x0113, 0x03C2},
        {0x0114, 0x00D0}, {0x0117, 0x1230}, {0x0119, 0x18D2}, {0x011C, 0x8941},
        {0x011D, 0x0000}, {0x011E, 0x0740}, {0x0120, 0xE6B4}, {0x0121, 0x3650},
        {0x0123, 0x000F}, {0x0200, 0x00E1}, {0x0208, 0x017B}, {0x020B, 0x4000},
        {0x020C, 0x8000}, {0x0400, 0x8081}, {0x0404, 0x0006}, {0x040B, 0x1020},
        {0x040C, 0x00FB}
    };

    int hw_version = fpga->ReadRegister(3);
    auto &initVals = hw_version >= 2 ? initVals_1v2 : initVals_1v0;

    lime::LMS7002M* lms = lms_list[0];
    if (lms->ResetChip() != 0)
        return -1;

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    for (auto i : initVals)
        lms->SPI_write(i.adr, i.val, true);

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
    if (SetRate(15.36e6, 1)!=0)
        return -1;

    return 0;
}

unsigned LMS7_LimeSDR_mini::GetNumChannels(const bool tx) const
{
    return 1;
}

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
        return {"NONE", "BAND1", "BAND2"};
    else
	return {"NONE", "LNAH", "LNAL_NC", "LNAW"};
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
            uint16_t value = fpga->ReadRegister(0x17);
            value &= ~(1<<8);
            value |= 1<<9;
            fpga->WriteRegister(0x17, value);
        }
        else if (path==LMS_PATH_LNAH)
        {
            uint16_t value = fpga->ReadRegister(0x17);
            value &= ~(1<<9);
            value |= 1<<8;
            fpga->WriteRegister(0x17, value);
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
            uint16_t value = fpga->ReadRegister(0x17);
            value &= ~(1<<13);
            value |= 1<<12;
            fpga->WriteRegister(0x17, value);
        }
        else if (path==LMS_PATH_TX2)
        {
            uint16_t value = fpga->ReadRegister(0x17);
            value &= ~(1<<12);
            value |= 1<<13;
            fpga->WriteRegister(0x17, value);
        }
    }
    return 0;
}

int LMS7_LimeSDR_mini::SetRate(double f_Hz, int oversample)
{
    lime::LMS7002M* lms = lms_list[0];

    if (oversample == 0)
        oversample = lime::cgenMax/(16*f_Hz);
    bool bypass = (oversample <= 1 && tx_channels[0].cF_offset_nco == 0.0 && rx_channels[0].cF_offset_nco == 0.0);

    if ((lms->Modify_SPI_Reg_bits(LMS7_LML1_SISODDR, 1)!=0)
        || (lms->Modify_SPI_Reg_bits(LMS7_LML2_SISODDR, 1)!=0)
        || (lms->Modify_SPI_Reg_bits(LMS7_CDSN_RXALML, !bypass)!=0))
            return -1;

    if (!bypass)
        return LMS7_Device::SetRate(f_Hz, oversample);

    tx_channels[0].sample_rate = f_Hz;
    rx_channels[0].sample_rate = f_Hz;

    if ((lms->SetFrequencyCGEN(f_Hz*4) != 0)
        || (lms->Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0) != 0)
        || (lms->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2) != 0)
        || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1) != 0)
        || (lms->SetInterfaceFrequency(lms->GetFrequencyCGEN(), 7, 7) != 0))
        return -1;

     double fpgaTxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx);
     double fpgaRxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx);
     if (fpga->SetInterfaceFreq(fpgaTxPLL, fpgaRxPLL, 0) != 0)
        return -1;
     return 0;
}


int LMS7_LimeSDR_mini::SetRate(bool tx, double f_Hz, unsigned oversample)
{
    return SetRate(f_Hz, oversample); //different rates for TX and Rx are not supported
}

LMS7_Device::Range LMS7_LimeSDR_mini::GetRxPathBand(unsigned path, unsigned chan) const
{
  switch (path)
  {
      case LMS_PATH_LNAH: return Range(2e9, 2.6e9);
      case LMS_PATH_LNAW: return Range(700e6, 900e6);
      default: return Range();
  }
}

LMS7_Device::Range LMS7_LimeSDR_mini::GetTxPathBand(unsigned path, unsigned chan) const
{
  switch (path)
  {
      case LMS_PATH_TX1: return Range(2e9, 2.6e9);
      case LMS_PATH_TX2: return Range(30e6, 1.9e9);
      default: return Range();
  }
}


std::vector<std::string> LMS7_LimeSDR_mini::GetProgramModes() const
{
    return {program_mode::autoUpdate,
            program_mode::fpgaFlash, program_mode::fpgaReset,
            program_mode::mcuRAM, program_mode::mcuEEPROM, program_mode::mcuReset};
}

LMS7_Device::Range LMS7_LimeSDR_mini::GetRateRange(bool /*dir*/, unsigned /*chan*/) const
{
    return Range(100e3, 30.72e6);
}

LMS7_Device::Range LMS7_LimeSDR_mini::GetFrequencyRange(bool tx) const
{
    return Range(10e6, 3.5e9);
}


}//namespace lime


