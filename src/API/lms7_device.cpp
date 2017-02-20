/*
 * File:   lms7_device.cpp
 * Author: ignas
 *
 * Created on March 9, 2016, 12:54 PM
 */

#include "lms7_device.h"
#include "GFIR/lms_gfir.h"
#include "IConnection.h"
#include <cmath>
#include "dataTypes.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include "ErrorReporting.h"
#include "MCU_BD.h"
#include "FPGA_common.h"
#include "LMS64CProtocol.h"
#include <assert.h>

static const size_t LMS_PATH_NONE = 0;
static const size_t LMS_PATH_HIGH = 1;
static const size_t LMS_PATH_LOW = 2;
static const size_t LMS_PATH_WIDE = 3;
static const size_t LMS_PATH_TX1 = 1;
static const size_t LMS_PATH_TX2 = 2;

const double LMS7_Device::LMS_CGEN_MAX = 640000000;

LMS7_Device::LMS7_Device() : LMS7002M(){

    tx_channels = new lms_channel_info[2];
    rx_channels = new lms_channel_info[2];
    tx_channels[0].cF_offset_nco = 0;
    rx_channels[0].cF_offset_nco = 0;
    tx_channels[1].cF_offset_nco = 0;
    rx_channels[1].cF_offset_nco = 0;
    tx_channels[0].sample_rate = 30e6;
    rx_channels[0].sample_rate = 30e6;
    tx_channels[1].sample_rate = 30e6;
    rx_channels[1].sample_rate = 30e6;
    EnableValuesCache(false);
}

int LMS7_Device::ConfigureRXLPF(bool enabled,int ch,float_type bandwidth)
{

    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -1;

    if (enabled)
    {
        if (Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB),0,true)!=0)
            return -1;
        Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB),0,true);
        Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB),0,true);

        if (bandwidth > 0)
        {
            if (TuneRxFilter(bandwidth)!=0)
                return -1;
        }
    }
    else
    {
        if (Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB),1,true)!=0)
            return -1;
        Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB),1,true);
        Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB),2,true);

    }

    return 0;
}


int LMS7_Device::ConfigureGFIR(bool enabled,bool tx, double bandwidth, size_t ch)
{
    double w,w2;
    int L;
    int div = 1;

    bandwidth /= 1e6;

    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -1;

    if (tx)
    {
        Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_TXTSP),enabled==false,true);
        Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_TXTSP),enabled==false,true);
        Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_TXTSP),enabled==false,true);

    }
    else
    {
        Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP),enabled==false,true);
        Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP),enabled==false,true);
        Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP),enabled==false,true);

    }

    if (bandwidth < 0)
        return 0;

    if (enabled)
    {

        double interface_MHz;
        int ratio;

        if (tx)
        {
          ratio = Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
          interface_MHz = GetReferenceClk_TSP(lime::LMS7002M::Tx)/1e6;
        }
        else
        {
          ratio =Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
          interface_MHz = GetReferenceClk_TSP(lime::LMS7002M::Rx)/1e6;
        }

        if (ratio == 7)
            interface_MHz /= 2;
        else
            div = (2<<(ratio));

        w = (bandwidth/2)/(interface_MHz/div);

        L = div > 8 ? 8 : div;
        div -= 1;

        w *=0.95;
        w2 = w*1.1;
        if (w2 > 0.495)
        {
         w2 = w*1.05;
         if (w2 > 0.495)
         {
             return 0; //Filter disabled
         }
        }
    }
    else return 0;

  double coef[120];
  double coef2[40];
  short gfir1[120];
  short gfir2[40];

  GenerateFilter(L*15, w, w2, 1.0, 0, coef);
  GenerateFilter(L*5, w, w2, 1.0, 0, coef2);

    int sample = 0;
    for(int i=0; i<15; i++)
    {
	for(int j=0; j<8; j++)
        {
            if( (j < L) && (sample < L*15) )
            {
                gfir1[i*8+j] = (coef[sample]*32767.0);
		sample++;
            }
            else
            {
		gfir1[i*8+j] = 0;
            }
	}
    }

    sample = 0;
    for(int i=0; i<5; i++)
    {
	for(int j=0; j<8; j++)
        {
            if( (j < L) && (sample < L*5) )
            {
                gfir2[i*8+j] = (coef2[sample]*32767.0);
		sample++;
            }
            else
            {
		gfir2[i*8+j] = 0;
            }
	}
    }

    L-=1;

    if (tx)
    {
      if (Modify_SPI_Reg_bits(LMS7param(GFIR1_L_TXTSP),L,true)!=0)
           return -1;
      Modify_SPI_Reg_bits(LMS7param(GFIR1_N_TXTSP),div,true);
      Modify_SPI_Reg_bits(LMS7param(GFIR2_L_TXTSP),L,true);
      Modify_SPI_Reg_bits(LMS7param(GFIR2_N_TXTSP),div,true);
      Modify_SPI_Reg_bits(LMS7param(GFIR3_L_TXTSP),L,true);
      Modify_SPI_Reg_bits(LMS7param(GFIR3_N_TXTSP),div,true);

    }
    else
    {
      if (Modify_SPI_Reg_bits(LMS7param(GFIR1_L_RXTSP),L,true)!=0)
          return -1;
      Modify_SPI_Reg_bits(LMS7param(GFIR1_N_RXTSP),div,true);
      Modify_SPI_Reg_bits(LMS7param(GFIR2_L_RXTSP),L,true);
      Modify_SPI_Reg_bits(LMS7param(GFIR2_N_RXTSP),div,true);
      Modify_SPI_Reg_bits(LMS7param(GFIR3_L_RXTSP),L,true);
      Modify_SPI_Reg_bits(LMS7param(GFIR3_N_RXTSP),div,true);

    }

    if ((SetGFIRCoefficients(tx,0,gfir2,40)!=0)
    ||(SetGFIRCoefficients(tx,1,gfir2,40)!=0)
    ||(SetGFIRCoefficients(tx,2,gfir1,120)!=0))
        return -1;

  return 0;
}


int LMS7_Device::ConfigureTXLPF(bool enabled,int ch,double bandwidth)
{
    if (ch == 1)
    {
        if (Modify_SPI_Reg_bits(LMS7param(MAC),2,true)!=0)
            return -1;
    }
    else
    {
       if (Modify_SPI_Reg_bits(LMS7param(MAC),1,true)!=0)
           return -1;
    }
    if (enabled)
    {

            if (Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB),0,true)!=0)
                return -1;
            Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB),0,true);
            Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB),0,true);

            if (bandwidth > 0)
            {
                if (TuneTxFilter(bandwidth)!=0)
                        return -1;
            }
    }
    else
    {
        Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB),1,true);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB),1,true);
        return Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB),1,true);
    }
    return 0;
}

LMS7_Device::~LMS7_Device()
{
    delete [] tx_channels;
    delete [] rx_channels;
}

int LMS7_Device::SetReferenceClock(const double refCLK_Hz)
{
    SetReferenceClk_SX(false, refCLK_Hz);
    SetReferenceClk_SX(true, refCLK_Hz);
    return 0;
}

size_t LMS7_Device::GetNumChannels(const bool tx) const
{
    return 2;
}

int LMS7_Device::SetRate(float_type f_Hz, int oversample)
{
   int decim = 0;
   float_type nco_f=0;
   for (size_t i = 0; i < GetNumChannels(false);i++)
   {
        if (rx_channels[i].cF_offset_nco > nco_f)
            nco_f = rx_channels[i].cF_offset_nco;
        if (tx_channels[i].cF_offset_nco > nco_f)
            nco_f = tx_channels[i].cF_offset_nco;
        tx_channels[i].sample_rate = f_Hz;
        rx_channels[i].sample_rate = f_Hz;
   }

   if (nco_f != 0)
   {
       int nco_over = 2+2*(nco_f-1)/f_Hz;
       oversample = oversample > nco_over ? oversample : nco_over;
       if (oversample > 32)
       {
           lime::ReportError(ERANGE, "Cannot achieve desired sample rate: rate too low");
           return -1;
       }
   }

   if (oversample > 1)
   {
       for (decim = 0; decim < 4; decim++)
       {
            if ( (1<<decim) >= (oversample+1)/2)
            {
             break;
            }
       }
   }
   else if (oversample == 0)
       decim = 0;
   else decim = 7;

   int ratio = oversample <= 2 ? 2 : (2<<decim);
   float_type cgen = f_Hz*4*ratio;
   if (cgen > LMS_CGEN_MAX)
   {
       lime::ReportError(ERANGE, "Cannot set desired sample rate. CGEN clock out of range");
       return -1;
   }
   if ((SetFrequencyCGEN(cgen)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN),0)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN),2)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(MAC),2,true)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), decim)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), decim)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(MAC),1,true)!=0)
   ||(SetInterfaceFrequency(GetFrequencyCGEN(),decim,decim)!=0))
           return -1;

    float_type fpgaTxPLL = GetReferenceClk_TSP(lime::LMS7002M::Tx) /
                            pow(2.0, Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP)));
    float_type fpgaRxPLL = GetReferenceClk_TSP(lime::LMS7002M::Rx) /
                            pow(2.0, Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP)));

    for (size_t i = 0; i < GetNumChannels(false);i++)
    {
        float_type freq[LMS_NCO_VAL_COUNT]={0};
        if (rx_channels[i].cF_offset_nco != 0)
        {
           freq[0] = rx_channels[i].cF_offset_nco;
           SetNCOFreq(false,i,freq,0);
           SetNCO(false,i,0,true);
        }

        if (tx_channels[i].cF_offset_nco != 0)
        {
           freq[0] = tx_channels[i].cF_offset_nco;
           SetNCOFreq(true,i,freq,0);
           SetNCO(true,i,0,true);
        }
    }

    return this->streamPort->UpdateExternalDataRate(0,fpgaTxPLL/2,fpgaRxPLL/2);
}

int LMS7_Device::SetRate(bool tx, float_type f_Hz, size_t oversample)
{
    float_type tx_clock;
    float_type rx_clock;
    float_type cgen;

    int decimation;
    int interpolation;
    size_t tmp;

   float_type nco_rx=0;
   float_type nco_tx=0;
   int min_int = 1;
   int min_dec = 1;
   bool retain_nco = false;

   for (size_t i = 0; i < GetNumChannels(false);i++)
   {
        if (rx_channels[i].cF_offset_nco > nco_rx)
            nco_rx = rx_channels[i].cF_offset_nco;
        if (tx_channels[i].cF_offset_nco > nco_tx)
            nco_tx = tx_channels[i].cF_offset_nco;
        if (tx)
            tx_channels[i].sample_rate = f_Hz;
        else
            rx_channels[i].sample_rate = f_Hz;
   }


   if (nco_rx != 0 || nco_tx != 0)
   {
       retain_nco = true;
       min_int = 2+2*(nco_tx-1)/tx_channels[0].sample_rate;
       min_dec = 2+2*(nco_rx-1)/rx_channels[0].sample_rate;
       unsigned int nco_over = tx ? min_int : min_dec;
       oversample = oversample > nco_over ? oversample : nco_over;
       if (oversample > 32)
       {
           lime::ReportError(ERANGE, "Cannot achieve desired sample rate: rate too low");
           return -1;
       }
   }

    if (oversample > 1)
    {
        for (tmp = 0; tmp < 4; tmp++)
        {
            if ( size_t(1<<tmp) >= (oversample+1)/2)
            {
                break;
            }
        }
    }
    else if (oversample == 0)
        tmp = 0;
    else tmp = 7;

    int ratio = oversample <= 2 ? 2 : (2<<tmp);

    if (tx)
    {
        interpolation = tmp;
        decimation = Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
        rx_clock = GetReferenceClk_TSP(lime::LMS7002M::Rx);
        tx_clock = f_Hz*ratio;
    }
    else
    {
        decimation = tmp;
        interpolation = Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
        tx_clock = GetReferenceClk_TSP(lime::LMS7002M::Tx);
        rx_clock = f_Hz * ratio;
    }

    int div_index = floor(log2(tx_clock/rx_clock)+0.5);

    while (div_index < -1)
    {
        if (tx)
        {
           if ((decimation > 0) && (min_dec <= (1<<decimation)))
           {
             decimation--;
             div_index++;
           }
           else if (interpolation < 4)
           {
             interpolation++;
             div_index++;
           }
           else
           {
               div_index = -1;
               break;
           }
        }
        else
        {
           if (interpolation < 4)
           {
             interpolation++;
             div_index++;
           }
           else if ((decimation > 0) && (min_dec <= (1<<decimation)))
           {
             decimation--;
             div_index++;
           }
           else
           {
               div_index = -1;
               break;
           }
        }
    }

    while (div_index > 5)
    {
        if (tx)
        {
           if (decimation < 4)
           {
             decimation++;
             div_index--;
           }
           else if ((interpolation > 0) && (min_int <= (1<<interpolation)))
           {
             interpolation--;
             div_index--;
           }
           else
           {
             div_index = 5;
             break;
           }
        }
        else
        {
           if ((interpolation > 0) && (min_int <= (1<<interpolation)))
           {
             interpolation--;
             div_index--;
           }
           else if (decimation < 4)
           {
             decimation++;
             div_index--;
           }
           else
           {
               div_index = 5;
               break;
           }
        }
    }

    if (min_int > (2<<interpolation) || min_dec > (2<<decimation))
    {
        lime::ReportError(ERANGE, "Unable to meet NCO oversampling requirements");
        return -1;
    }

    int clk_mux;
    int clk_div;

    switch (div_index)
    {
        case -1://2:1
                clk_mux = 0;
                clk_div = 3;
                break;
        case 0://1:1
                clk_mux = 0;
                clk_div = 2;
                break;
        case 1: //1:2
                clk_mux = 0;
                clk_div = 1;
                break;
        case 2://1:4
                clk_mux = 0;
                clk_div = 0;
                break;
        case 3: //1:8
                clk_mux = 1;
                clk_div = 1;
                break;
        case 4: //1:16
                clk_mux = 1;
                clk_div = 2;
                break;
        case 5: //1:32
                clk_mux = 1;
                clk_div = 3;
                break;
    }



    if (tx)
    {
        ratio = oversample <= 2 ? 2 : (2<<interpolation);
        cgen = f_Hz*ratio;
    }
    else
    {
        ratio = oversample <= 2 ? 2 : (2<<decimation);
        cgen = f_Hz * ratio * 4;
    }

    if ((tx && clk_mux == 0)||(tx == false && clk_mux == 1))
    {
        while (cgen*(1<<clk_div)>LMS_CGEN_MAX)
        {
            if (clk_div > 0)
            {
                clk_div--;
            }
            else
            {
               lime::ReportError(ERANGE, "Cannot set desired sample rate. CGEN clock out of range");
               return -1;
            }
        }
        cgen *= (1 << clk_div);
    }

    if (cgen > LMS_CGEN_MAX)
    {
        lime::ReportError(ERANGE, "Cannot set desired sample rate. CGEN clock out of range");
        return -1;
    }

   if ((SetFrequencyCGEN(cgen,retain_nco)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN),clk_mux)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN),clk_div)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(MAC),2,true)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), decimation)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), interpolation)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(MAC),1,true)!=0)
   ||(SetInterfaceFrequency(GetFrequencyCGEN(),interpolation,decimation)!=0))
           return -1;

    for (size_t i = 0; i < GetNumChannels(false);i++)
    {
        float_type freq[LMS_NCO_VAL_COUNT]={0};
        if (rx_channels[i].cF_offset_nco != 0)
        {
           freq[0] = rx_channels[i].cF_offset_nco;
           SetNCOFreq(false,i,freq,0);
           SetNCO(false,i,0,true);
        }

        if (tx_channels[i].cF_offset_nco != 0)
        {
           freq[0] = tx_channels[i].cF_offset_nco;
           SetNCOFreq(true,i,freq,0);
           SetNCO(true,i,0,true);
        }
    }

   return 0;
}


double LMS7_Device::GetRate(bool tx, size_t chan,float_type *rf_rate_Hz)
{

    double interface_Hz;
    int ratio;

    if (Modify_SPI_Reg_bits(LMS7param(MAC),chan+1,true)!=0)
        return -1;

    if (tx)
    {
        ratio = Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
        interface_Hz = GetReferenceClk_TSP(lime::LMS7002M::Tx);
    }
    else
    {
        ratio = Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
        interface_Hz = GetReferenceClk_TSP(lime::LMS7002M::Rx);
    }

    if (ratio == 7)
       interface_Hz /= 2;

    if (rf_rate_Hz)
        *rf_rate_Hz = interface_Hz;

    if (ratio != 7)
        interface_Hz /= 2*pow(2.0, ratio);

    return interface_Hz;
}

lms_range_t LMS7_Device::GetRxRateRange(const size_t chan) const
{
  lms_range_t ret;
  ret.max = 30000000;
  ret.min = 100000;
  ret.step = 1;
  return ret;
}

lms_range_t LMS7_Device::GetTxRateRange(size_t chan) const
{
  lms_range_t ret;
  ret.max = 30000000;
  ret.min = 100000;
  ret.step = 1;
  return ret;
}

std::vector<std::string> LMS7_Device::GetPathNames(bool dir_tx, size_t chan) const
{
    if (dir_tx)
    {
        return {"NONE", "TX_PATH1", "TX_PATH2"};
    }
    else
    {
        return {"NONE", "LNA_H", "LNA_L", "LNA_W"};
    }
}

int LMS7_Device::SetPath(bool tx, size_t chan, size_t path)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),chan+1,true)!=0)
        return -1;
    if (tx==false)
    {
        if ((Modify_SPI_Reg_bits(LMS7param(SEL_PATH_RFE),path,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_L_RFE),path!=2,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_W_RFE),path!=3,true)!=0))
            return -1;
    }
    else
    {
        if ((Modify_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF),path==LMS_PATH_TX1,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF),path==LMS_PATH_TX2,true)!=0))
            return -1;
    }
    return 0;
}

size_t LMS7_Device::GetPath(bool tx, size_t chan)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),chan+1,true)!=0)
        return -1;
    if (tx)
    {
        if (Get_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF),true))
            return LMS_PATH_TX2;
        else if (Get_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF),true))
            return LMS_PATH_TX1;
        else
            return 0;
    }

    return Get_SPI_Reg_bits(LMS7param(SEL_PATH_RFE),true);
}

lms_range_t LMS7_Device::GetRxPathBand(size_t path, size_t chan) const
{
  lms_range_t ret;
  ret.step = 1;
  if (GetConnection()->GetDeviceInfo().deviceName == lime::GetDeviceName(lime::LMS_DEV_ULIMESDR))
  {
      if (path == LMS_PATH_LOW)
      {
        ret.max = 3800000000;
        ret.min = 30000000;
      }
      else
      {
            ret.max = 0;
            ret.min = 0;
            ret.step = 0;
      }
  }
  else
  switch (path)
  {
      case LMS_PATH_HIGH:
            ret.max = 3800000000;
            ret.min = 2000000000;
            break;
      case LMS_PATH_WIDE:
            ret.max = 3800000000;
            ret.min = 700000000;
            break;
      case LMS_PATH_LOW:
            ret.max = 1100000000;
            ret.min = 30000000;
            break;
      default:
            ret.max = 0;
            ret.min = 0;
            ret.step = 0;
            break;
  }

  return ret;
}

lms_range_t LMS7_Device::GetTxPathBand(size_t path, size_t chan) const
{
  lms_range_t ret;

  ret.step = 1;
  if (GetConnection()->GetDeviceInfo().deviceName == lime::GetDeviceName(lime::LMS_DEV_ULIMESDR))
  {
      if (path == LMS_PATH_TX2)
      {
        ret.max = 3800000000;
        ret.min = 30000000;
      }
      else
      {
            ret.max = 0;
            ret.min = 0;
            ret.step = 0;
      }
  }
  else
  switch (path)
  {
      case LMS_PATH_TX2:
            ret.max = 3800000000;
            ret.min = 1500000000;
            break;
      case LMS_PATH_TX1:
            ret.max = 1500000000;
            ret.min = 30000000;
            break;
      default:
            ret.max = 0;
            ret.min = 0;
            ret.step = 0;
            break;
  }

  return ret;
}

int LMS7_Device::SetLPF_Fixed(bool tx,size_t chan, LPF_FixedBW bandwidth)
{
    if(!tx)
        return lime::ReportError(ENOTSUP, "LPF Fixed BW not supported for Rx");
    float bw = 5;
    switch(bandwidth)
    {
        case LPF_BW_5_MHz: bw = 5e6; break;
        case LPF_BW_10_MHz: bw = 10e6; break;
        case LPF_BW_15_MHz: bw = 15e6; break;
        case LPF_BW_20_MHz: bw = 20e6; break;
        default: bw = 5e6;
    }
    return TuneTxFilterFixed(bw);
}

int LMS7_Device::SetLPF(bool tx,size_t chan, bool filt, bool en, float_type bandwidth)
{

    if (filt)
    {
       if (tx)
       {
          if(en)
            tx_channels[chan].lpf_bw = bandwidth;

          return ConfigureTXLPF(en,chan,bandwidth)!=0;
       }
       else
       {
          if (en)
            rx_channels[chan].lpf_bw = bandwidth;

          return ConfigureRXLPF(en,chan,bandwidth);
       }
    }
    else
    {
       return ConfigureGFIR(en,tx,bandwidth,chan);
    }
}

float_type LMS7_Device::GetLPFBW(bool tx,size_t chan, bool filt)
{

    if (filt)
    {
       if (tx)
       {
           return tx_channels[chan].lpf_bw;
       }
       else
       {
            return rx_channels[chan].lpf_bw;
       }
    }
    else
    {
       return -1;
    }
}


lms_range_t LMS7_Device::GetLPFRange(bool tx, size_t chan, bool filt)
{
    lms_range_t ret;
    if (filt)
    {
        if (tx)
        {
            ret.max = 130000000;
            ret.min = 5000000;
        }
        else
        {
            ret.max = 130000000;
            ret.min = 1400000;
        }
    }
    else
    {
      ret.max = GetRate(false,chan)/1.03;
      ret.min = ret.max/30;
    }

    ret.step = 1;
    return ret;
}


int LMS7_Device::SetGFIRCoef(bool tx, size_t chan, lms_gfir_t filt, const float_type* coef,size_t count)
{
    short gfir[120];
    int L;
    int div = 1;
    int ret = 0;

    if (count > 120)
    {
        lime::ReportError(ERANGE, "Max number of coefficients for GFIR3 is 120 and for GFIR1(2) - 40");
        return -1;
    }
    else if (count > 40 && filt != LMS_GFIR3)
    {
        lime::ReportError(ERANGE, "Max number of coefficients for GFIR1(2) is 40");
        return -1;
    }

    if (Modify_SPI_Reg_bits(LMS7param(MAC),chan+1,true)!=0)
        return -1;

    double interface_Hz;
    int ratio;

    if (tx)
    {
      ratio = Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
      interface_Hz = GetReferenceClk_TSP(lime::LMS7002M::Tx);
    }
    else
    {
      ratio =Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
      interface_Hz = GetReferenceClk_TSP(lime::LMS7002M::Rx);
    }

    if (ratio == 7)
        interface_Hz /= 2;
    else
        div = (2<<(ratio));

    if ((div > 8) || (count == 120) || (count == 40 && filt != LMS_GFIR3))
        L = 8;
    else
        L = div;

    float_type max=0;
    for (int i=0; i< (filt==LMS_GFIR3 ? 120 : 40); i++)
        if (fabs(coef[i])>max)
            max=fabs(coef[i]);

    if (max < 1.0)
        max = 1.0f;
    else if (max > 100)
        max = 32767.0f;

    size_t sample = 0;
    for(int i=0; i< (filt==LMS_GFIR3 ? 15 : 5); i++)
    {
        for(int j=0; j<8; j++)
        {
            if( (j < L) && (sample < count) )
            {
                gfir[i*8+j] = (coef[sample]*32767.0/max);
                sample++;
            }
            else
            {
                gfir[i*8+j] = 0;
            }
        }
    }

    div -= 1;
    L = div > 7 ? 7 : div;

    if (tx)
    {
      if (filt ==LMS_GFIR1)
      {
          if ((Modify_SPI_Reg_bits(LMS7param(GFIR1_L_TXTSP),L,true)!=0)
          || (Modify_SPI_Reg_bits(LMS7param(GFIR1_N_TXTSP),div,true)!=0))
              return -1;
      }
      else if (filt ==LMS_GFIR2)
      {
          if ((Modify_SPI_Reg_bits(LMS7param(GFIR2_L_TXTSP),L,true)!=0)
          || (Modify_SPI_Reg_bits(LMS7param(GFIR2_N_TXTSP),div,true)!=0))
              return -1;
      }
      else if (filt ==LMS_GFIR3)
      {
          if ((Modify_SPI_Reg_bits(LMS7param(GFIR3_L_TXTSP),L,true)!=0)
          || (Modify_SPI_Reg_bits(LMS7param(GFIR3_N_TXTSP),div,true)!=0))
              return -1;
      }
    }
    else
    {
      if (filt ==LMS_GFIR1)
      {
        if ((Modify_SPI_Reg_bits(LMS7param(GFIR1_L_RXTSP),L,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(GFIR1_N_RXTSP),div,true)!=0))
            return -1;
      }
      else if (filt ==LMS_GFIR2)
      {
        if ((Modify_SPI_Reg_bits(LMS7param(GFIR2_L_RXTSP),L,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(GFIR2_N_RXTSP),div,true)!=0))
            return -1;
      }
      else if (filt ==LMS_GFIR3)
      {
        if ((Modify_SPI_Reg_bits(LMS7param(GFIR3_L_RXTSP),L,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(GFIR3_N_RXTSP),div,true)!=0))
            return -1;
      }
    }

   if (SetGFIRCoefficients(tx,filt,gfir,filt==LMS_GFIR3 ? 120 : 40)!=0)
       return -1;
   return ret;
}

int LMS7_Device::GetGFIRCoef(bool tx, size_t chan, lms_gfir_t filt, float_type* coef)
{
   if (Modify_SPI_Reg_bits(LMS7param(MAC),chan+1,true)!=0)
       return -1;
   int16_t coef16[120];

   if (GetGFIRCoefficients(tx,filt,coef16,filt==LMS_GFIR3 ? 120 : 40)!=0)
       return -1;
   if (coef != NULL)
   {
       for (int i = 0; i < (filt==LMS_GFIR3 ? 120 : 40) ; i++)
       {
           coef[i] = coef16[i];
           coef[i] /= (1<<15);
       }
   }
   return (filt==LMS_GFIR3) ? 120 : 40;
}

int LMS7_Device::SetGFIR(bool tx, size_t chan, lms_gfir_t filt, bool enabled)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),chan+1,true)!=0)
        return -1;
    if (tx)
    {
        if (filt ==LMS_GFIR1)
        {
            if (Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_TXTSP),enabled==false,true)!=0)
                return -1;
        }
        else if (filt == LMS_GFIR2)
        {
            if (Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_TXTSP),enabled==false,true)!=0)
                return -1;
        }
        else if (filt == LMS_GFIR3)
        {
            if (Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_TXTSP),enabled==false,true)!=0)
                return -1;
        }
    }
    else
    {
        if (filt ==LMS_GFIR1)
        {
            if (Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP),enabled==false,true)!=0)
                return -1;
        }
        else if (filt == LMS_GFIR2)
        {
            if (Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP),enabled==false,true)!=0)
                return -1;
        }
        else if (filt == LMS_GFIR3)
        {
            if (Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP),enabled==false,true)!=0)
                return -1;
        }
    }

   return 0;
}

int LMS7_Device::SetNormalizedGain(bool dir_tx, size_t chan,double gain)
{
    const int gain_total = 27 + 12 + 31;
    if (dir_tx)
        return SetGain(dir_tx,chan,63*gain+0.49);
    else
        return SetGain(dir_tx,chan,gain*gain_total+0.49);
}

int LMS7_Device::SetGain(bool dir_tx, size_t chan, unsigned gain)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),chan+1,true)!=0)
        return -1;

    if (dir_tx)
    {
        if (gain > 63)
            gain = 63;
        if (Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB),gain,true)!=0)
            return -1;
    }
    else
    {
        if (gain > 70)
            gain = 70;
        unsigned lna = 0, tia = 0, pga = 0;
        //LNA table
        const unsigned lnaTbl[71] = {
            1,  1,  1,  2,  2,  2,  3,  3,  3,  4,  4,  4,  5,  5,  5,  6,
            6,  6,  7,  7,  7,  8,  9,  10, 11, 11, 11, 11, 11, 11, 11, 11,
            11, 11, 11, 11, 11, 12, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14,
            14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
            14, 14, 14, 14, 14, 14, 14
        };
        //PGA table
        const unsigned pgaTbl[71] = {
            0,  1,  2,  0,  1,  2,  0,  1,  2,  0,  1,  2,  0,  1,  2,  0,
            1,  2,  0,  1,  2,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,
            8,  9,  10, 11, 12, 12, 12, 12, 4,  5,  6,  7,  8,  9,  10, 11,
            12, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
            25, 26, 27, 28, 29, 30, 31
        };

        //TIA table
        if (gain > 48)
            tia = 2;
        else if (gain > 39)
            tia = 1;

        lna = lnaTbl[gain];
        pga = pgaTbl[gain];

        int rcc_ctl_pga_rbb = (430*(pow(0.65,((double)pga/10)))-110.35)/20.4516+16;

        if ((Modify_SPI_Reg_bits(LMS7param(G_LNA_RFE),lna+1,true)!=0)
          ||(Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE),tia+1,true)!=0)
          ||(Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB),pga,true)!=0)
          ||(Modify_SPI_Reg_bits(LMS7param(RCC_CTL_PGA_RBB),rcc_ctl_pga_rbb,true)!=0))
            return -1;
    }
    return 0;
}

double LMS7_Device::GetNormalizedGain(bool dir_tx, size_t chan)
{
    const double gain_total = 27 + 12 + 31;
    double ret = GetGain(dir_tx, chan);
    if (dir_tx)
        return ret / 63.0;
    else
        return ret / gain_total;
}

int LMS7_Device::GetGain(bool dir_tx, size_t chan)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),chan+1,true)!=0)
        return -1;
    if (dir_tx)
    {
        int ret = Get_SPI_Reg_bits(LMS7param(CG_IAMP_TBB),true);
        return ret;
    }
    else
    {
        const int max_gain_lna = 27;
        const int max_gain_tia = 12;

        int pga = Get_SPI_Reg_bits(LMS7param(G_PGA_RBB),true);
        int ret = pga;

        int tia = Get_SPI_Reg_bits(LMS7param(G_TIA_RFE),true);
        if (tia == 3)
            ret +=max_gain_tia;
        else if (tia == 2)
            ret += max_gain_tia-3;

        int lna = Get_SPI_Reg_bits(LMS7param(G_LNA_RFE),true);

        if (lna > 8)
            ret += (max_gain_lna+lna-15);
        else if (lna > 1)
            ret += (lna-2)*3;

        return ret;
    }
    return 0;
}

int LMS7_Device::SetTestSignal(bool dir_tx, size_t chan, lms_testsig_t sig, int16_t dc_i, int16_t dc_q)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC), chan + 1, true) != 0)
        return -1;

    if (dir_tx == false)
    {
        if (Modify_SPI_Reg_bits(LMS7param(INSEL_RXTSP), sig != LMS_TESTSIG_NONE, true) != 0)
            return -1;

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV8F)
            Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1, true);
        else if (sig == LMS_TESTSIG_NCODIV4 || sig == LMS_TESTSIG_NCODIV4F)
            Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 2, true);

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV4)
            Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 0, true);
        else if (sig == LMS_TESTSIG_NCODIV8F || sig == LMS_TESTSIG_NCODIV4F)
            Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 1, true);

        return Modify_SPI_Reg_bits(LMS7param(TSGMODE_RXTSP), sig == LMS_TESTSIG_DC, true);
        ;
    }
    else
    {
        if (Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), sig != LMS_TESTSIG_NONE, true) != 0)
            return -1;

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV8F)
            Modify_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), 1, true);
        else if (sig == LMS_TESTSIG_NCODIV4 || sig == LMS_TESTSIG_NCODIV4F)
            Modify_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), 2, true);

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV4)
            Modify_SPI_Reg_bits(LMS7param(TSGFC_TXTSP), 0, true);
        else if (sig == LMS_TESTSIG_NCODIV8F || sig == LMS_TESTSIG_NCODIV4F)
            Modify_SPI_Reg_bits(LMS7param(TSGFC_TXTSP), 1, true);

        return Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), sig == LMS_TESTSIG_DC, true);
        ;
    }

    if (sig == LMS_TESTSIG_DC)
        return LoadDC_REG_IQ(dir_tx, dc_i, dc_q);

    return 0;
}

int LMS7_Device::GetTestSignal(bool dir_tx, size_t chan)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),chan+1,true)!=0)
        return -1;

    if (dir_tx)
    {
        if (Get_SPI_Reg_bits(LMS7param(INSEL_TXTSP),true)==0)
        {
            return LMS_TESTSIG_NONE;
        }
        else if (Get_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP),true)!=0)
        {
            return LMS_TESTSIG_DC;
        }
        else return Get_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), true) + 2 * Get_SPI_Reg_bits(LMS7param(TSGFC_TXTSP), true);
    }
    else
    {
        if (Get_SPI_Reg_bits(LMS7param(INSEL_RXTSP),true)==0)
        {
            return LMS_TESTSIG_NONE;
        }
        else if (Get_SPI_Reg_bits(LMS7param(TSGMODE_RXTSP),true)!=0)
        {
            return LMS_TESTSIG_DC;
        }
        else return Get_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), true) + 2 * Get_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), true);
    }
    return -1;
}


int LMS7_Device::SetNCOFreq(bool tx, size_t ch, const float_type *freq, float_type pho)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -1;

    float_type rf_rate;
    GetRate(tx,ch,&rf_rate);
    rf_rate /=2;
    for (size_t i = 0; i < LMS_NCO_VAL_COUNT; i++)
    {
        if (freq[i] < 0 || freq[i] > rf_rate)
        {
            lime::ReportError(ERANGE, "NCO frequency is negative or outside of RF bandwidth range");
            return -1;
        }
        if (SetNCOFrequency(tx,i,freq[i])!=0)
            return -1;
    }
    if (tx)
    {
        if ((Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP),0,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(SEL_TX),0,true)!=0)
            )
            return -1;
        if (Modify_SPI_Reg_bits(LMS7param(MODE_TX),0,true)!=0)
            return -1;
        tx_channels[ch].nco_pho = pho;
    }
    else
    {
        if ((Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP),0,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(SEL_RX),0,true)!=0)
            )
            return -1;
        if (Modify_SPI_Reg_bits(LMS7param(MODE_RX),0,true)!=0)
            return -1;
        rx_channels[ch].nco_pho = pho;
    }
    return SetNCOPhaseOffsetForMode0(tx,pho);
}

int LMS7_Device::SetNCO(bool tx,size_t ch,size_t ind,bool down)
{
    if ((!tx) && (Get_SPI_Reg_bits(LMS7_MASK, true) != 0))
        down = !down;
    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -1;
    if (ind >= LMS_NCO_VAL_COUNT)
    {
        if (tx)
        {
            if ((Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP),1,true)!=0)
            || (Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_TXTSP), 0, true)!=0))
                return -1;
        }
        else
        {
            if ((Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP),1,true)!=0)
            || (Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_RXTSP), 0, true)!=0))
                return -1;
        }
    }
    else
    {
        if (tx)
        {
            if ((Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP),0,true)!=0)
            || (Modify_SPI_Reg_bits(LMS7param(SEL_TX),ind,true)!=0)
            || (Modify_SPI_Reg_bits(LMS7param(CMIX_SC_TXTSP),down,true)!=0)
            || (Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_TXTSP), 1, true)!=0))
                return -1;
            ;
        }
        else
        {
            if ((Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP),0,true)!=0)
            || (Modify_SPI_Reg_bits(LMS7param(SEL_RX),ind,true)!=0)
            || (Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP),down,true)!=0)
            || (Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_RXTSP), 1, true)!=0))
                return -1;
        }
    }
    return 0;
}


int LMS7_Device::GetNCOFreq(bool tx, size_t ch, float_type *freq,float_type *pho)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -1;
    for (size_t i = 0; i < LMS_NCO_VAL_COUNT; i++)
    {
        freq[i] = GetNCOFrequency(tx,i,true);
    }
    *pho = tx ? tx_channels[ch].nco_pho : rx_channels[ch].nco_pho;
    return 0;
}

int LMS7_Device::SetNCOPhase(bool tx, size_t ch, const float_type *phase, float_type fcw)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -1;

    for (size_t i = 0; i < LMS_NCO_VAL_COUNT; i++)
    {
        if (SetNCOPhaseOffset(tx,i,phase[i])!=0)
            return -1;
    }

    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -1;

    if (tx)
    {
        if ((Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP),0,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(SEL_TX),0,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(CMIX_SC_TXTSP),0,true)!=0))
            return -1;
        if (Modify_SPI_Reg_bits(LMS7param(MODE_TX),1,true)!=0)
            return -1;
        tx_channels[ch].nco_pho = fcw;
    }
    else
    {
        if ((Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP),0,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(SEL_RX),0,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP),0,true)!=0))
            return -1;
        if (Modify_SPI_Reg_bits(LMS7param(MODE_RX),1,true)!=0)
            return -1;
        rx_channels[ch].nco_pho = fcw;
    }

    return SetNCOFrequency(tx,0,fcw);
}


int LMS7_Device::GetNCOPhase(bool tx, size_t ch, float_type *phase,float_type *fcw)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -1;
    for (size_t i = 0; i < LMS_NCO_VAL_COUNT; i++)
    {
        phase[i] = GetNCOPhaseOffset_Deg(tx,i);
    }
    *fcw = tx ? tx_channels[ch].nco_pho : rx_channels[ch].nco_pho;
    return 0;
}

size_t LMS7_Device::GetNCO(bool tx, size_t ch)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -2;
    if (tx)
    {
        if (Get_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP),true)!=0)
        {
            lime::ReportError(ENODEV, "NCO is disabled");
            return -1;
        }
        return Get_SPI_Reg_bits(LMS7param(SEL_TX),true);
    }

    if (Get_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP),true)!=0)
        return -1;
    return Get_SPI_Reg_bits(LMS7param(SEL_RX),true);
}

int LMS7_Device::SetRxFrequency(size_t chan, double f_Hz)
{
    if (f_Hz < 30e6)
    {
        if (SetFrequencySX(false,30e6)!=0)
            return -1;
        rx_channels[chan].cF_offset_nco = 30e6-f_Hz;
        if (SetRate(false,GetRate(true,chan),2)!=0)
            return -1;
    }
    else
    {
        if (rx_channels[chan].cF_offset_nco != 0)
            SetNCO(false,chan,~0,true);
        rx_channels[chan].cF_offset_nco = 0;
        if (SetFrequencySX(false,f_Hz)!=0)
            return -1;
    }
    return 0;
}


float_type LMS7_Device::GetTRXFrequency(bool tx, size_t chan)
{
   double offset = tx ? tx_channels[chan].cF_offset_nco : rx_channels[chan].cF_offset_nco;
   return GetFrequencySX(tx) - offset;
}

int LMS7_Device::SetTxFrequency(size_t chan, double f_Hz)
{
    if (f_Hz < 30e6)
    {
        if (SetFrequencySX(true,30e6)!=0)
            return -1;
        tx_channels[chan].cF_offset_nco = 30e6-f_Hz;
        if (SetRate(true,GetRate(true,chan),2)!=0)
            return -1;
    }
    else
    {
        if (tx_channels[chan].cF_offset_nco != 0)
            SetNCO(true,chan,~0,false);
        tx_channels[chan].cF_offset_nco = 0;
        if (SetFrequencySX(true,f_Hz)!=0)
            return -1;
    }
    return 0;
}


lms_range_t LMS7_Device::GetFrequencyRange(bool tx) const
{
  lms_range_t ret;
  ret.max = 3800000000;
  ret.min = 30000000;
  ret.step = 1;
  return ret;
}

int LMS7_Device::Init()
{
    if (ResetChip()!=0)
        return -1;
    //load initial settings to get samples
    if (UploadAll()!=0)
        return -1;
    SetActiveChannel(lime::LMS7002M::ChA);
    Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0);
    Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2);
    Modify_SPI_Reg_bits(LMS7param(LML1_MODE), 0);
    Modify_SPI_Reg_bits(LMS7param(LML2_MODE), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);

    SetActiveChannel(lime::LMS7002M::ChAB);
    Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(AGC_BYP_RXTSP), 1);
    return Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);
}

int LMS7_Device::ProgramFPGA(const char* data, size_t len, lms_target_t mode,lime::IConnection::ProgrammingCallback callback)
{
    if (mode > LMS_TARGET_BOOT)
    {
        lime::ReportError(ENOTSUP, "Unsupported target storage type");
        return -1;
    }
    //device FPGA(2)
    //mode to RAM(0), to FLASH (1)
    return streamPort->ProgramWrite(data,len,mode,2,callback);
}

int LMS7_Device::ProgramFPGA(std::string fname, lms_target_t mode,lime::IConnection::ProgrammingCallback callback)
{
    std::ifstream file(fname);
    int len;
    char* data;
    if (file.is_open())
    {
        file.seekg (0, file.end);
        len = file.tellg();
        file.seekg (0, file.beg);
        data = new char[len];
        file.read(data,len);
        file.close();
    }
    else
    {
        lime::ReportError(ENOENT, "Unable to open the specified file");
        return -1;
    }
    int ret = ProgramFPGA(data,len,mode,callback);
    delete [] data;
    return ret;
}

int LMS7_Device::ProgramHPM7(const char* data, size_t len, int mode,lime::IConnection::ProgrammingCallback callback)
{
    if (mode > LMS_TARGET_BOOT)
    {
        lime::ReportError(ENOTSUP, "Unsupported target storage type");
        return -1;
    }
    //device FPGA(2)
    //mode to RAM(0), to FLASH (1)
    return streamPort->ProgramWrite(data,len,mode,0,callback);
}

int LMS7_Device::ProgramUpdate(const bool download,lime::IConnection::ProgrammingCallback callback)
{
    return streamPort->ProgramUpdate(download,callback);
}

int LMS7_Device::ProgramHPM7(std::string fname, int mode,lime::IConnection::ProgrammingCallback callback)
{
    std::ifstream file(fname);
    int len;
    char* data;
    if (file.is_open())
    {
        file.seekg (0, file.end);
        len = file.tellg();
        file.seekg (0, file.beg);
        data = new char[len];
        file.read(data,len);
        file.close();
    }
    else
    {
        lime::ReportError(ENOENT, "Unable to open the specified file");
        return -1;
    }
    int ret = ProgramHPM7(data,len,mode,callback);
    delete [] data;
    return ret;
}

//TODO: fx3 needs restart
int LMS7_Device::ProgramFW(const char* data, size_t len, lms_target_t mode,lime::IConnection::ProgrammingCallback callback)
{

    if (mode == LMS_TARGET_FLASH)
        return streamPort->ProgramWrite(data,len,2,1,callback);
    else if (mode == LMS_TARGET_RAM)
        return streamPort->ProgramWrite(data,len,1,1,callback);
    else
        return streamPort->ProgramWrite(nullptr,0,0,1,callback);
}

int LMS7_Device::ProgramFW(std::string fname, lms_target_t mode,lime::IConnection::ProgrammingCallback callback)
{
    std::ifstream file(fname);
    int len;
    char* data;
    if (file.is_open())
    {
        file.seekg (0, file.end);
        len = file.tellg();
        file.seekg (0, file.beg);
        data = new char[len];
        file.read(data,len);
        file.close();
    }
    else
    {
        lime::ReportError(ENOENT, "Unable to open the specified file");
        return -1;
    }
    int ret = ProgramFW(data,len,mode,callback);
    delete [] data;
    return ret;
}

int LMS7_Device::ProgramMCU(const char* data, size_t len, lms_target_t target,lime::IConnection::ProgrammingCallback callback)
{
    lime::MCU_BD *mcu = this->GetMCUControls();
    lime::IConnection::MCU_PROG_MODE mode;
    uint8_t bin[lime::MCU_BD::cMaxFWSize];

    if(data != nullptr)
    {
        memcpy(bin,data,len>sizeof(bin) ? sizeof(bin) : len);
    }

    if (target == LMS_TARGET_RAM)
    {
        mode = lime::IConnection::MCU_PROG_MODE::SRAM;
    }
    else if (target == LMS_TARGET_FLASH)
    {
        mode = lime::IConnection::MCU_PROG_MODE::EEPROM_AND_SRAM;
    }
    else if (target == LMS_TARGET_BOOT)
    {
        mode = lime::IConnection::MCU_PROG_MODE::BOOT_SRAM_FROM_EEPROM;
    }
    else
    {
        lime::ReportError(ENOTSUP, "Unsupported target storage type");
        return -1;
    }

    mcu->callback = callback;
    mcu->Program_MCU(bin,mode);
    mcu->callback = nullptr;
    return 0;
}

int LMS7_Device::DACWrite(uint16_t val)
{
    uint8_t id=0;
    double dval= val;
    return streamPort->CustomParameterWrite(&id,&dval,1,NULL);
}

int LMS7_Device::DACRead()
{
    uint8_t id=0;
    double dval=0;
    int ret = streamPort->CustomParameterRead(&id,&dval,1,NULL);
    return ret >=0 ? dval : -1;
}

lms_dev_info_t* LMS7_Device::GetInfo()
{
    memset(&devInfo,0,sizeof(lms_dev_info_t));
    auto info = GetConnection()->GetDeviceInfo();
    strncpy(devInfo.deviceName,info.deviceName.c_str(),sizeof(devInfo.deviceName)-1);
    strncpy(devInfo.expansionName,info.expansionName.c_str(),sizeof(devInfo.expansionName)-1);
    strncpy(devInfo.firmwareVersion,info.firmwareVersion.c_str(),sizeof(devInfo.firmwareVersion)-1);
    strncpy(devInfo.hardwareVersion,info.hardwareVersion.c_str(),sizeof(devInfo.hardwareVersion)-1);
    strncpy(devInfo.protocolVersion,info.protocolVersion.c_str(),sizeof(devInfo.protocolVersion)-1);
    strncpy(devInfo.gatewareVersion,info.gatewareVersion.c_str(),sizeof(devInfo.gatewareVersion)-1);
    strncpy(devInfo.gatewareRevision,info.gatewareRevision.c_str(),sizeof(devInfo.gatewareRevision)-1);
    strncpy(devInfo.gatewareTargetBoard,info.gatewareTargetBoard.c_str(),sizeof(devInfo.gatewareTargetBoard)-1);
    info.boardSerialNumber = info.boardSerialNumber;
    devInfo.boardSerialNumber = info.boardSerialNumber;
    return &devInfo;
}









