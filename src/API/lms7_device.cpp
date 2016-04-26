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

static const int MAX_PACKETS_BATCH = 16;
static const int SAMPLES12_PACKET = 1360;
static const int SAMPLES16_PACKET = 1020;
static const size_t LMS_PATH_NONE = 0;
static const size_t LMS_PATH_LOW = 2;
static const size_t LMS_PATH_HIGH = 1;
static const size_t LMS_PATH_WIDE = 3;

const double LMS7_Device::LMS_CGEN_MAX = 640000000;

LMS7_Device::LMS7_Device() : LMS7002M(){
    
    tx_channels = new lms_channel_info[2];
    rx_channels = new lms_channel_info[2];
    tx_channels[0].enabled = true;
    rx_channels[0].enabled = true;
    tx_channels[1].enabled = false;
    rx_channels[1].enabled = false;
    rx_channels[0].half_duplex = false;
    rx_channels[1].half_duplex = false;
    
    channelsCount = 1;
   
    rx_packetsToBatch = 4;
    rx_buffersCount = 32; // must be power of 2
    rx_handles = nullptr;
    rx_buffers = nullptr;
    rx_running = false;
    
    tx_packetsToBatch = 4;
    tx_buffersCount = 32; // must be power of 2
    tx_handles = nullptr;
    tx_buffers = nullptr;
    tx_bufferUsed = nullptr; 
    tx_running = false;
    forced_mode = MODE_AUTO;
    sample_fmt = FMT_INT16;
    EnableValuesCache(true);
}

int LMS7_Device::ConfigureRXLPF(bool enabled,int ch,float_type bandwidth)
{
    bandwidth /= 1e6;
    if (ch == 0)
    {
        if (Modify_SPI_Reg_bits(LMS7param(MAC),1,true)!=0)
            return -1;
    }
    else
    {
        if (Modify_SPI_Reg_bits(LMS7param(MAC),2,true)!=0)
            return -1;
    }
    
    if (enabled)
    {
        if (bandwidth > 0)
        {
            if (TuneRxFilter(RX_TIA,bandwidth)!=0)
                return -1;
        }
        if (bandwidth < 20)
        {
            if (Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB),0,true)!=0)
                   return -1; 
            Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB),1,true);
            Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB),0,true);
                    
            
            if (bandwidth > 0)
            {
                if (TuneRxFilter(RX_LPF_LOWBAND, bandwidth)!=0)
                    return -1;
            }       
        }
        else
        {
            if (Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB),1,true)!=0)
                return -1;
            Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB),0,true);
            Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB),1,true);
                
            
            if (bandwidth > 0)
            {
                if (TuneRxFilter(lime::LMS7002M::RX_LPF_HIGHBAND, bandwidth*1.3)!=0)
                    return -1;
            }
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
    bandwidth /= 1e6;
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
        if (bandwidth < gLadder_higher_limit)
        {
            //low band chain
            if (bandwidth >= gRealpole_higher_limit)
            {
                if ((Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB),1,true)!=0)
                || (Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB),0,true)!=0)
                || (Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB),0,true)!=0)
                || (Modify_SPI_Reg_bits(LMS7param(BYPLADDER_TBB),0,true)!=0)
                || (TuneTxFilter(lime::LMS7002M::TX_REALPOLE,bandwidth > gRealpole_higher_limit ? gRealpole_higher_limit : bandwidth)!=0)
                || (TuneTxFilter(lime::LMS7002M::TX_LADDER,bandwidth )!=0))
                        return -1; 
            }
            else
            {
                if ((Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB),1,true)!=0)
                || (Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB),1,true)!=0)
                || (Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB),0,true)!=0)
                || (Modify_SPI_Reg_bits(LMS7param(BYPLADDER_TBB),1,true)!=0)
                || (TuneTxFilter(lime::LMS7002M::TX_REALPOLE,bandwidth)!=0))
                        return -1; 
            }
        }
        else
        {
            // high band
            if (bandwidth < gHighband_lower_limit)
                bandwidth = gHighband_lower_limit;
            if ((Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB),0,true)!=0)
            || (Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB),1,true)!=0)
            || (Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB),1,true)!=0)
            || (TuneTxFilter(lime::LMS7002M::TX_HIGHBAND,bandwidth)!=0))
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

int LMS7_Device::ConfigureSamplePositions() 
{
    if (Modify_SPI_Reg_bits(LMS7param(LML2_BQP),3,true)!=0)
        return -1;
    Modify_SPI_Reg_bits(LMS7param(LML2_BIP),2,true);
    Modify_SPI_Reg_bits(LMS7param(LML2_AQP),1,true);
    Modify_SPI_Reg_bits(LMS7param(LML2_AIP),0,true); 
    Modify_SPI_Reg_bits(LMS7param(LML1_BQP),3,true);
    Modify_SPI_Reg_bits(LMS7param(LML1_BIP),2,true);
    Modify_SPI_Reg_bits(LMS7param(LML1_AQP),1,true);
    Modify_SPI_Reg_bits(LMS7param(LML1_AIP),0,true);
        
    Modify_SPI_Reg_bits(LMS7param(LML1_S3S),2,true);
    Modify_SPI_Reg_bits(LMS7param(LML1_S2S),3,true);
    Modify_SPI_Reg_bits(LMS7param(LML1_S1S),0,true);
    Modify_SPI_Reg_bits(LMS7param(LML1_S0S),1,true);
    Modify_SPI_Reg_bits(LMS7param(LML2_S3S),2,true);
    Modify_SPI_Reg_bits(LMS7param(LML2_S2S),3,true);
    Modify_SPI_Reg_bits(LMS7param(LML2_S1S),0,true);
    Modify_SPI_Reg_bits(LMS7param(LML2_S0S),1,true);

   return 0;
}

LMS7_Device::~LMS7_Device() {
    StopRx();
    StopTx();
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
    
    this->controlPort->UpdateExternalDataRate(0,fpgaTxPLL/2,fpgaRxPLL/2);
   return 0;
}

int LMS7_Device::SetRate(bool tx, float_type f_Hz, size_t oversample)
{
    float_type tx_clock;
    float_type rx_clock;
    float_type cgen;
    
    int decimation;
    int interpolation;
    int tmp;
    
    if (oversample > 1)
    {
        for (tmp = 0; tmp < 4; tmp++)
        {
             if ( (1<<tmp) >= (oversample+1)/2)
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
        cgen = tx_clock;
    }
    else
    {
        decimation = tmp;
        interpolation = Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
        tx_clock = GetReferenceClk_TSP(lime::LMS7002M::Tx);
        rx_clock = f_Hz * ratio;
        cgen = rx_clock * 4;
    }

    int div_index = floor(log2(tx_clock/rx_clock)+0.5); 
    
    while (div_index < -1)
    {
        if (tx)
        {
           if (decimation > 0)
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
        else
        {
           if (interpolation < 4)
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
           else
           {
             div_index = 5;
             break;
           }
        }
        else
        {
           if (interpolation > 0)
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
    
   if ((SetFrequencyCGEN(cgen)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN),clk_mux)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN),clk_div)!=0) 
   ||(Modify_SPI_Reg_bits(LMS7param(MAC),2,true)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), decimation)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), interpolation)!=0)
   ||(Modify_SPI_Reg_bits(LMS7param(MAC),1,true)!=0)
   ||(SetInterfaceFrequency(GetFrequencyCGEN(),interpolation,decimation)!=0))
           return -1;
    
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
  ret.min = 2500000;
  ret.step = 1;
  return ret;
}

lms_range_t LMS7_Device::GetTxRateRange(size_t chan) const
{
  lms_range_t ret; 
  ret.max = 30000000;
  ret.min = 2500000;
  ret.step = 1;
  return ret;
}

std::vector<std::string> LMS7_Device::GetPathNames(bool dir_tx, size_t chan) const
{  
    if (dir_tx)
    {
       return {"NONE", "LNA_H", "LNA_L", "LNA_W"}; 
    }
    else
    {
       return {"NONE", "TX_PATH1", "TX_PATH2"}; 
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
        if ((Modify_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF),path==2,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF),path==1,true)!=0))
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
        if (Get_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF),true))
            return 2;
        else if (Get_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF),true))
            return 1;
        else
            return 0;  
    }
    
    return Get_SPI_Reg_bits(LMS7param(SEL_PATH_RFE),true);    
}

lms_range_t LMS7_Device::GetRxPathBand(size_t path, size_t chan) const
{
  lms_range_t ret; 
  switch (path)
  {
      case 1:
            ret.max = 3800000000;
            ret.min = 2000000000;
            ret.step = 1; 
            break;
      case 3:
            ret.max = 3800000000;
            ret.min = 1000000000;
            ret.step = 1; 
            break;
      case 2:
            ret.max = 1000000000;
            ret.min = 100000000;
            ret.step = 1; 
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
  switch (path)
  {
      case 1:
            ret.max = 3800000000;
            ret.min = 1500000000;
            ret.step = 1; 
            break;
      case 2:
            ret.max = 1500000000;
            ret.min = 100000000;
            ret.step = 1; 
            break;
      default: 
            ret.max = 0;
            ret.min = 0;
            ret.step = 0; 
            break;
  }

  return ret; 
}

int LMS7_Device::SetBandwidth(bool tx, size_t chan, float_type bandwidth)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),chan+1,true)!=0)
        return -1;    
    if (tx)
    {
        tx_channels[chan].bandwidth= bandwidth;
    }
    else
    {
        rx_channels[chan].bandwidth = bandwidth;
        if (TuneRxFilter(RX_TIA,bandwidth/1e6)!=0)
            return -1;
    }
    return 0;
}

float_type LMS7_Device::GetBandwidth(bool tx,size_t chan)
{
    if (tx)
        return tx_channels[chan].bandwidth;
    else
        return rx_channels[chan].bandwidth;

}

lms_range_t LMS7_Device::GetBandwidthRange(bool tx) const
{
    lms_range_t ret; 
    ret.max = 320000000;
    ret.min = 1000000;
    ret.step = 1;
    return ret;  
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
      ret.max = 160000000; 
    }
    else
    {
      ret.max = GetRate(false,chan)/1.03; 
    }
    ret.min = 1000000;
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


    L = div > 8 ? 8 : div;
    div -= 1;
    
    if (filt==LMS_GFIR3)
    {
       if (L*15 < count)
       {
           lime::ReportError(ERANGE, "Too many filter coefficients for current oversampling settings");         
           ret = -1;;
           L = 1+(count-1)/15;
           div = L-1;
       }
    }
    else
    {
       if (L*5 < count)
       {
           lime::ReportError(ERANGE, "Too many filter coefficients for current oversampling settings");
           ret = -1;
           L = 1+(count-1)/5;
           div = L-1;
       }
    }
    
    float_type max=0;
    for (int i=0; i< filt==LMS_GFIR3 ? 120 : 40; i++)
        if (fabs(coef[i])>max)
            max=fabs(coef[i]);
    
    if (max < 1.0)
        max = 1.0;
    
    int sample = 0;
    for(int i=0; i< filt==LMS_GFIR3 ? 15 : 5; i++)
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
 
    L-=1; 

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
       for (int i = 0; i < filt==LMS_GFIR3 ? 120 : 40 ; i++)
       {
           coef[i] = coef16[i];
           coef[i] /= (1<<15);
       }   
   }
   return filt==LMS_GFIR3 ? 120 : 40; 
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
    if (Modify_SPI_Reg_bits(LMS7param(MAC),chan+1,true)!=0)
        return -1;
    
    if (dir_tx)
    {
        int g = (63*gain+0.49);
        if (Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB),g,true)!=0)
            return -1;
    }
    else
    {
        const int w_lna = 2;
        const int w_tia = 5;
        const int w_pga = 1;
        const int w_total = 14*w_lna + 2*w_tia + 31*w_pga;
        int target = gain*w_total+0.49;
        unsigned lna = 0, tia = 0, pga = 11;
        int sum = w_lna*lna+w_tia*tia+w_pga*pga;
        
        while (target > sum)
        {
            if (tia < 2)
            {
                tia++;
                sum += w_tia;
            }
            else if (lna < 14)
            {
                lna++;
                sum += w_lna;
            }
            else
            {
                pga++;
                sum += w_pga;
            }
        }

        if (target < sum)
        {
            pga--;
            sum -= w_pga;
        }
             
        if ((Modify_SPI_Reg_bits(LMS7param(G_LNA_RFE),lna+1,true)!=0)
          ||(Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE),tia+1,true)!=0)
          ||(Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB),pga,true)!=0))
            return -1;
    }
    return 0;
}

double LMS7_Device::GetNormalizedGain(bool dir_tx, size_t chan)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),chan+1,true)!=0)
        return -1;
    if (dir_tx)
    {
        double ret = Get_SPI_Reg_bits(LMS7param(CG_IAMP_TBB),true);
        ret /= 63.0;
        return ret;
    }
    else
    {
        const int w_lna = 2;
        const int w_tia = 5;
        const int w_pga = 1;
        const int w_total = 14*w_lna + 2*w_tia + 31*w_pga;         
        int lna = Get_SPI_Reg_bits(LMS7param(G_LNA_RFE),true)-1;
        int tia = Get_SPI_Reg_bits(LMS7param(G_TIA_RFE),true)-1;
        int pga = Get_SPI_Reg_bits(LMS7param(G_PGA_RBB),true);
        double ret = lna*w_lna+tia*w_tia+pga*w_pga;
        ret /= w_total;
        return ret;
    }
    return 0;
}

int LMS7_Device::SetTestSignal(bool dir_tx, size_t chan,lms_testsig_t sig, int16_t dc_i, int16_t dc_q)
{     
    if (Modify_SPI_Reg_bits(LMS7param(MAC),chan+1,true)!=0)
        return -1;
    
    if (dir_tx==false)
    {      
        if ((Modify_SPI_Reg_bits(LMS7param(INSEL_RXTSP),sig!=LMS_TESTSIG_NONE,true)!=0)
        ||(Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP),sig,true)!=0)
        ||(Modify_SPI_Reg_bits(LMS7param(TSGMODE_RXTSP),sig==LMS_TESTSIG_DC,true)!=0))
            return -1;
    }
    else
    {
        if ((Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP),sig!=LMS_TESTSIG_NONE,true)!=0)
        ||(Modify_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP),sig,true)!=0)
        ||(Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP),sig==LMS_TESTSIG_DC,true)!=0))
            return -1;
    }
    
   if (sig==LMS_TESTSIG_DC)
        return LoadDC_REG_IQ(dir_tx,dc_i,dc_q);
    
    return 0;  
}

int LMS7_Device::GetTestSignal(bool dir_tx, size_t chan)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),chan+1,true)!=0)
        return -1;
    
    if (dir_tx==false)
    {   
        if (Get_SPI_Reg_bits(LMS7param(INSEL_TXTSP),true)==0)
        {
            return LMS_TESTSIG_NONE;
        }
        else if (Get_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP),true)!=0)
        {
            return LMS_TESTSIG_DC;
        }  
        else return Get_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP),true);
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
        else return Get_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP),true);
    }
    return 0;  
}


int LMS7_Device::SetNCOFreq(bool tx, size_t ch, const float_type *freq, float_type pho)
{ 
    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -1;
    
    float_type rf_rate;
    GetRate(tx,ch,&rf_rate);
    rf_rate /=2;
    for (int i = 0; i < 16; i++)
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
        || (Modify_SPI_Reg_bits(LMS7param(CMIX_SC_TXTSP),0,true)!=0))
            return -1;
        if (Modify_SPI_Reg_bits(LMS7param(MODE_TX),0,true)!=0)
            return -1;
        tx_channels[ch].nco_pho = pho;
    }
    else
    {
        if ((Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP),0,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(SEL_RX),0,true)!=0)
        || (Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP),0,true)!=0))
            return -1;
        if (Modify_SPI_Reg_bits(LMS7param(MODE_RX),0,true)!=0)
            return -1;
        rx_channels[ch].nco_pho = pho;
    }
    return SetNCOPhaseOffsetForMode0(tx,pho);
}

int LMS7_Device::SetNCO(bool tx,size_t ch,size_t ind,bool down)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -1;
    if (ind == -1)
    {
        if (tx)
        {
            if (Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP),1,true)!=0)
                return -1;
        }
        else
        {
            if (Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP),1,true)!=0)
                return -1;
        }
    }
    else
    {
        if (tx)
        {
            if ((Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP),0,true)!=0)
            || (Modify_SPI_Reg_bits(LMS7param(SEL_TX),ind,true)!=0)
            || (Modify_SPI_Reg_bits(LMS7param(CMIX_SC_TXTSP),down,true)!=0))
                return -1;
        }
        else
        {
            if ((Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP),0,true)!=0)
            || (Modify_SPI_Reg_bits(LMS7param(SEL_RX),ind,true)!=0)
            || (Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP),down,true)!=0))
                return -1;
        }
    }
    return 0;
}


int LMS7_Device::GetNCOFreq(bool tx, size_t ch, float_type *freq,float_type *pho)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -1;
    for (int i = 0; i < 16; i++)
    {
        freq[i] = GetNCOFrequency(tx,i,true);
        uint16_t neg = tx ? Get_SPI_Reg_bits(LMS7param(CMIX_SC_TXTSP),true) : Get_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP),true);
        freq[i] = neg ? -freq[i] : freq[i];
    }
    *pho = tx ? tx_channels[ch].nco_pho : rx_channels[ch].nco_pho;
    return 0;
}

int LMS7_Device::SetNCOPhase(bool tx, size_t ch, const float_type *phase, float_type fcw)
{ 
    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -1;

    for (int i = 0; i < 16; i++)
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
    for (int i = 0; i < 16; i++)
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
    rx_channels[0].half_duplex = false;
    rx_channels[1].half_duplex = false;
    if (SetFrequencySX(false,f_Hz)!=0)
        return -1;
    if (f_Hz < GetRxPathBand(LMS_PATH_LOW,chan).max)
    {
        SetPath(false,0,LMS_PATH_LOW);
        SetPath(false,1,LMS_PATH_LOW);
    }
    else 
    {
        SetPath(false,0,LMS_PATH_WIDE);
        SetPath(false,1,LMS_PATH_WIDE);
    }
    return 0;  
}

int LMS7_Device::SetTxFrequency(size_t chan, double f_Hz)
{    
    rx_channels[0].half_duplex = false;
    rx_channels[1].half_duplex = false;
    if (SetFrequencySX(true,f_Hz)!=0)
        return -1;
    if (f_Hz < GetTxPathBand(LMS_PATH_LOW,chan).max)
    {
        SetPath(true,0,LMS_PATH_LOW);
        SetPath(true,1,LMS_PATH_LOW);
    }
    else 
    {
        SetPath(true,0,LMS_PATH_HIGH);
        SetPath(true,1,LMS_PATH_HIGH);
    }
    return 0;  
}


lms_range_t LMS7_Device::GetFrequencyRange(bool tx) const
{
  lms_range_t ret; 
  ret.max = 3800000000;
  ret.min = 100000000;
  ret.step = 1;
  return ret; 
}

int LMS7_Device::Init()
{ 
   if ((ResetChip()!=0)
     ||(Modify_SPI_Reg_bits(LMS7param(MAC),2,true)!=0)
     ||(Modify_SPI_Reg_bits(0x208, 8, 0, 0x1F3, true)!=0)  //TXTSP bypasses
     ||(Modify_SPI_Reg_bits(0x40C, 7, 0, 0xFB, true)!=0)   //RXTSP bypasses
     ||(Modify_SPI_Reg_bits(LMS7param(ICT_LNA_RFE),0x1F,true)!=0)
     ||(Modify_SPI_Reg_bits(LMS7param(ICT_LODC_RFE),0x1F,true)!=0)  
     ||(Modify_SPI_Reg_bits(LMS7param(MAC),1,true)!=0)    
     ||(Modify_SPI_Reg_bits(0x208, 8, 0, 0x1F3, true)!=0)
     ||(Modify_SPI_Reg_bits(0x40C, 7, 0, 0xFB, true)!=0)
     ||(Modify_SPI_Reg_bits(LMS7param(ICT_LNA_RFE),0x1F,true)!=0)
     ||(Modify_SPI_Reg_bits(LMS7param(ICT_LODC_RFE),0x1F,true)!=0)
     ||(Modify_SPI_Reg_bits(LMS7param(LML1_MODE),0,true)!=0)
     ||(Modify_SPI_Reg_bits(LMS7param(LML2_MODE),0,true)!=0)
     ||(Modify_SPI_Reg_bits(LMS7param(DIQ1_DS),1,true)!=0)    
     ||(Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2),0,true)!=0)
     ||(Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2),0,true)!=0)) 
        return -1;
   DownloadAll();
   return ConfigureSamplePositions();
}


int LMS7_Device::EnableTX(size_t ch, bool enable)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -1;
    
    this->EnableChannel(true,enable);
    
    tx_channels[ch].enabled = enable;
    if (forced_mode == MODE_AUTO)
    {
        if (tx_channels[1].enabled || rx_channels[1].enabled)
            channelsCount = 2;
        else
            channelsCount = 1;
    }
    
    return 0;
}

int LMS7_Device::EnableRX(const size_t ch, const bool enable)
{
    if (Modify_SPI_Reg_bits(LMS7param(MAC),ch+1,true)!=0)
        return -1;
   
          
    this->EnableChannel(false,enable);
    
    rx_channels[ch].enabled = enable;
    if (forced_mode == MODE_AUTO)
    {
        if (tx_channels[1].enabled || rx_channels[1].enabled)
            channelsCount = 2;
        else
            channelsCount = 1;
    }
    
    return 0;
}

int LMS7_Device::SetStreamingMode(uint32_t flags)
{
    if ((flags & LMS_STREAM_MD_MIMO)==LMS_STREAM_MD_MIMO)
    {
        forced_mode = MODE_MIMO;
        channelsCount = 2;
    }
    else if ((flags & LMS_STREAM_MD_SISO)==LMS_STREAM_MD_SISO)
    {
        forced_mode = MODE_SISO;
        channelsCount = 1;
    }
    else
    {
        forced_mode = MODE_AUTO;
    }
    
    if ((flags & LMS_STREAM_FMT_F32)==LMS_STREAM_FMT_F32)
    {
        sample_fmt = FMT_FLOAT;
    }
    else sample_fmt = FMT_INT16;
    
    return 0;
}

int LMS7_Device::ConfigureRxStream(size_t numBuffers, size_t bufSize, size_t fifo)
{
    StartRx(bufSize/sizeof(lime::PacketLTE),numBuffers);
    return 0;
}

int LMS7_Device::ConfigureTxStream(size_t numBuffers, size_t bufSize, size_t fifo)
{
    StartTx(bufSize/sizeof(lime::PacketLTE),numBuffers);
    return 0;
}


int LMS7_Device::RecvStream(void **data,size_t numSamples, lms_stream_metadata *meta, unsigned timeout_ms)
{
    const long bufferSize = rx_packetsToBatch*SAMPLES12_PACKET; 
    static int16_t buffer[MAX_PACKETS_BATCH*SAMPLES12_PACKET*2];
    static uint64_t ts = 0;
    static uint64_t rx_meta = 0;
    static int index = bufferSize;
    static bool started = false;
    int ret;
    
    rx_lock.lock();
    
    if (meta->start_of_burst)
    {
        index = bufferSize;
        int size = rx_packetsToBatch*sizeof(lime::PacketLTE);
        if (started == false)
        {
            started = true;
            for (int i = 0; i<rx_buffersCount; ++i)
                rx_handles[i] = controlPort->BeginDataReading(&rx_buffers[i*size], size);
        }
    }
    
    if (sample_fmt == FMT_FLOAT)
        ret = RecvStreamFloat((float**)data, buffer, numSamples, &ts, index, &rx_meta, timeout_ms);
    else
        ret = RecvStreamInt16((int16_t**)data, buffer, numSamples, &ts, index, &rx_meta, timeout_ms);
                
    if (meta->end_of_burst)
    {
        index = bufferSize;  
        controlPort->AbortReading();
        int size = rx_packetsToBatch*sizeof(lime::PacketLTE);
        for (int j = 0; j<rx_buffersCount; j++)
        {
            long bytesToRead = size;
            controlPort->WaitForReading(rx_handles[j], 30);
            controlPort->FinishDataReading(&rx_buffers[j*size], bytesToRead, rx_handles[j]); 
        }
        started = false;
    }
    
   meta->timestamp = ts+index/channelsCount-numSamples;
   rx_lock.unlock();
   return ret;
}

int LMS7_Device::RecvStreamFloat(float **data, int16_t* buffer, size_t numSamples, uint64_t* ts, int &index, uint64_t* rx_meta, unsigned timeout_ms)
{
    const long bufferSize = rx_packetsToBatch*SAMPLES12_PACKET; 
    
    for (int i = 0; i < numSamples ; i++)
    {
        if (index == bufferSize)
        {
            index = 0;
            if ((_read(buffer,ts,rx_meta,timeout_ms))!=bufferSize)
            {
                return i;   
            }           
        }
        
        for (int ch = 0;ch<channelsCount;ch++)
        {
            data[ch][2*i] = (float)buffer[2*index]/2047.0;
            data[ch][2*i+1] = (float)buffer[2*index+1]/2047.0;
            index++;
        }
    }
    return numSamples;
}


int LMS7_Device::RecvStreamInt16(int16_t **data, int16_t* buffer, size_t numSamples, uint64_t* ts, int &index, uint64_t* rx_meta, unsigned timeout_ms)
{
    const long bufferSize = rx_packetsToBatch*SAMPLES12_PACKET; 
    
    for (int i = 0; i < numSamples ; i++)
    {
        if (index == bufferSize)
        {
            index = 0;
            if ((_read(buffer,ts,rx_meta,timeout_ms))!=bufferSize)
            {
                return i;   
            }           
        }
        
        for (int ch = 0;ch<channelsCount;ch++)
        {
            data[ch][2*i] = buffer[2*index];
            data[ch][2*i+1] = buffer[2*index+1];
            index++;
        }
    }
    return numSamples;
}

int LMS7_Device::ProgramFPGA(const char* data, size_t len, lms_target_t mode)
{
    if (mode > LMS_TARGET_BOOT)
    {
        lime::ReportError(ENOTSUP, "Unsupported target storage type");
        return -1;
    }
    //device FPGA(2)
    //mode to RAM(0), to FLASH (1)
    return GetConnection()->ProgramWrite(data,len,mode,2,nullptr);
}

int LMS7_Device::ProgramFPGA(std::string fname, lms_target_t mode)
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
    int ret = ProgramFPGA(data,len,mode);
    delete [] data;
    return ret;
}

//TODO: fx3 needs restart
int LMS7_Device::ProgramFW(const char* data, size_t len, lms_target_t mode)
{
    if (mode != LMS_TARGET_FLASH)
    {
        lime::ReportError(ENOTSUP, "Unsupported target storage type");
        return -1;
    }
    //device fx3(1)
    //mode program_flash(2))
    return GetConnection()->ProgramWrite(data,len,2,1,nullptr);
}

int LMS7_Device::ProgramFW(std::string fname, lms_target_t mode)
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
    int ret = ProgramFW(data,len,mode);
    delete [] data;
    return ret;
}

int LMS7_Device::ProgramMCU(const char* data, size_t len, lms_target_t target)
{
    lime::MCU_BD mcu;
    lime::MCU_BD::MEMORY_MODE mode;
    uint8_t bin[8192];
    
    if ((data == nullptr)||(target==LMS_TARGET_BOOT)) //boot from FLAH
    {
        mode = lime::MCU_BD::SRAM_FROM_EEPROM;
    }
    else
    { 
        memcpy(bin,data,len>sizeof(bin) ? sizeof(bin) : len);
    }
    
    if (target == LMS_TARGET_RAM)
    {
        mode = lime::MCU_BD::SRAM;
    }
    else if (target == LMS_TARGET_FLASH)
    {
        mode = lime::MCU_BD::EEPROM_AND_SRAM;
    }
    else
    {
        lime::ReportError(ENOTSUP, "Unsupported target storage type");
        return -1;
    }
    mcu.Initialize(GetConnection());
    mcu.Program_MCU(bin,mode);
    return 0;
}

int LMS7_Device::DACWrite(uint16_t val)
{
    uint8_t id=0;
    double dval= val; 
    return controlPort->CustomParameterWrite(&id,&dval,1,NULL);
}

int LMS7_Device::DACRead()
{
    uint8_t id=0;
    double dval=0; 
    int ret = controlPort->CustomParameterRead(&id,&dval,1,NULL);
    return ret >=0 ? dval : -1;
}



void SendCopyRaw(const void* src, int16_t* dst)
{
    *((uint32_t*)dst) = *((uint32_t*)src);
}

int LMS7_Device::SendStream(const void **data,size_t numSamples, lms_stream_metadata *meta, unsigned timeout_ms)
{
    const long bufferSize = rx_packetsToBatch*SAMPLES12_PACKET; 
    static int16_t buffer[MAX_PACKETS_BATCH*SAMPLES12_PACKET*2];
    uint64_t tx_meta = 0;
    static int index = 0;
    
    if (meta->start_of_burst)
        index = 0;
    
    if(meta->has_timestamp == false)
        tx_meta |= (1 << 4); //ignore timestamp
    
    tx_lock.lock();
    int ret;
    
    if (sample_fmt == FMT_FLOAT)
        ret = SendStreamFloat((const float**)data, buffer, numSamples, meta->timestamp, index, tx_meta, timeout_ms);
    else
        ret = SendStreamInt16((const int16_t**)data, buffer, numSamples, meta->timestamp, index, tx_meta, timeout_ms);
    
    if (meta->end_of_burst)
    {
        for (int i = index; i <bufferSize;i++)
        {
            buffer[2*i] = 0;
            buffer[2*i+1] = 0;     
        }    
       uint64_t ts = meta->timestamp + numSamples - index/channelsCount;
       _write(buffer,ts,tx_meta,timeout_ms);
       index = 0; 
    }
    
   tx_lock.unlock();
   return ret;
}


int LMS7_Device::SendStreamInt16(const int16_t **data, int16_t* buffer, size_t numSamples, uint64_t ts, int &index, uint64_t tx_meta, unsigned timeout_ms)
{
    const long bufferSize = rx_packetsToBatch*SAMPLES12_PACKET; 

    for (int i = 0; i < numSamples ; i++)
    {
       for (int ch = 0;ch<channelsCount;ch++)
        {
           buffer[2*index] = data[ch][2*i];
           buffer[2*index+1] = data[ch][2*i+1];
           index++;
        }
        
        if (index == bufferSize)
        {
             ts = ts + i - bufferSize/channelsCount;
             index = 0;
             if (_write(buffer,ts,tx_meta,timeout_ms)!=bufferSize)
             {
                return i;
             }
        }
    }  
   return numSamples;
}

int LMS7_Device::SendStreamFloat(const float **data, int16_t* buffer, size_t numSamples, uint64_t ts, int &index, uint64_t tx_meta, unsigned timeout_ms)
{
    const long bufferSize = rx_packetsToBatch*SAMPLES12_PACKET; 

    for (int i = 0; i < numSamples ; i++)
    {
       for (int ch = 0;ch<channelsCount;ch++)
        {
           buffer[2*index] = data[ch][2*i]*2047.0;
           buffer[2*index+1] = data[ch][2*i+1]*2047.0;
           index++;
        }
        
        if (index == bufferSize)
        {
             ts = ts + i - bufferSize/channelsCount;
             index = 0;
             if (_write(buffer,ts,tx_meta,timeout_ms)!=bufferSize)
             {
                return i;
             }
        }
    }  
   return numSamples;
}


int LMS7_Device::_read(int16_t *data, uint64_t *timestamp, uint64_t *metadata, unsigned timeout)
{
    if (rx_running == false)
        return 0;
    
    long bufferSize = rx_packetsToBatch*sizeof(lime::PacketLTE);
    const int buffersCountMask = rx_buffersCount - 1;
    static int bi = 0;
    int n = 0;

    if (controlPort->WaitForReading(rx_handles[bi], timeout) == false)
    {
     
        return -1;
    }

    long bytesReceived = controlPort->FinishDataReading(&rx_buffers[bi*bufferSize], bufferSize, rx_handles[bi]);
    if (bytesReceived > 0)
    {    
	lime::PacketLTE* pkt= (lime::PacketLTE*)&rx_buffers[bi*bufferSize];
        
        *timestamp = pkt->counter; 
        *metadata = *((uint64_t*)(&pkt->reserved));
        for (int p = 0; p<rx_packetsToBatch;p++)
        {     
            for (int i = 0; i<sizeof(pkt[p].data);i+=3)
            {
                //I sample
                int16_t sample;
                sample = (pkt[p].data[i +1] & 0x0F) << 8;
                sample |= (pkt[p].data[i] & 0xFF);
                sample = sample << 4;
                sample = sample >> 4;
                data[n++] = sample;

                //Q sample
                sample = pkt[p].data[i + 2] << 4;
                sample |= (pkt[p].data[i + 1] >> 4) & 0x0F;
                sample = sample << 4;
                sample = sample >> 4;
                data[n++] = sample;
            }
        }
        
    }
    else return -1;

    // Re-submit this request to keep the queue full
    memset(&rx_buffers[bi*bufferSize], 0, bufferSize);
    rx_handles[bi] = controlPort->BeginDataReading(&rx_buffers[bi*bufferSize], bufferSize);
    bi = (bi + 1) & buffersCountMask;
    return n/2;    
}

void LMS7_Device::ResetUSBFIFO()
{
	lime::LMS64CProtocol* port = dynamic_cast<lime::LMS64CProtocol *>(controlPort);
// TODO : USB FIFO reset command for IConnection
    if (port == nullptr) return;
	lime::LMS64CProtocol::GenericPacket ctrPkt;
    ctrPkt.cmd = lime::CMD_USB_FIFO_RST;
    ctrPkt.outBuffer.push_back(0x01);
    port->TransferPacket(ctrPkt);
    ctrPkt.outBuffer[0] = 0x00;
    port->TransferPacket(ctrPkt);
}

int LMS7_Device::Start()
{
    static bool started = false;
    if (started)
        return 1;
    
    uint32_t dataRd = 0;
    // Stop Tx Rx if they were active
    controlPort->ReadRegister(0x000A,dataRd);
    controlPort->WriteRegister(0x000A, dataRd & ~0x3);
    //reset hardware timestamp to 0
    controlPort->ReadRegister(0x0009,dataRd);
    controlPort->WriteRegister(0x0009, dataRd & ~0x3);
    controlPort->WriteRegister(0x0009, dataRd | 0x3);
    controlPort->WriteRegister(0x0009, dataRd & ~0x3);


    //enable MIMO mode, 12 bit compressed values
    if ((channelsCount == 2 && forced_mode == MODE_AUTO) || (forced_mode == MODE_MIMO))
    {
        controlPort->WriteRegister(0x0008, (1<<8)|(0x0002));
        controlPort->WriteRegister(0x0007, 0x0003);
    }
    else
    {
        controlPort->WriteRegister(0x0008, (1<<8)|(0x0002));
        controlPort->WriteRegister(0x0007, 0x0001);
    }

    //USB FIFO reset
    ResetUSBFIFO();

    //switch on Rx
    controlPort->ReadRegister(0x000A,dataRd);
    controlPort->WriteRegister(0x000A, dataRd & ~0x3); 
    controlPort->WriteRegister(0x000A, dataRd | 0x1);   
}

int LMS7_Device::Stop()
{
    uint32_t dataRd = 0;
    controlPort->ReadRegister(0x000A,dataRd);
    controlPort->WriteRegister(0x000A, dataRd & ~0x3);
	return 0;
}


int LMS7_Device::StartRx(int packetsToBatch, int numTransfers)
{
    rx_lock.lock();
    rx_packetsToBatch = packetsToBatch > MAX_PACKETS_BATCH ? MAX_PACKETS_BATCH : packetsToBatch;
    rx_packetsToBatch = rx_packetsToBatch == 0 ? 4 : rx_packetsToBatch;
    rx_buffersCount = 1; // must be power of 2
    while (rx_buffersCount < numTransfers)
        rx_buffersCount <<=1;
    int bufferSize = packetsToBatch*sizeof(lime::PacketLTE);
    
    if (rx_handles != nullptr)
        delete [] rx_handles;
    rx_handles = new int[numTransfers];
    memset(rx_handles, 0, numTransfers*sizeof(int));
    
    if (rx_buffers != nullptr)
        delete [] rx_buffers;
    rx_buffers = new char[numTransfers*bufferSize];
    memset(rx_buffers,0,numTransfers*bufferSize);
    
    rx_running = true;
    if (tx_running == false)
        Start();
    rx_lock.unlock();
	return 0;
}

int LMS7_Device::StartTx(int packetsToBatch, int numTransfers)
{
    tx_lock.lock();
    tx_packetsToBatch = packetsToBatch > MAX_PACKETS_BATCH ? MAX_PACKETS_BATCH : packetsToBatch;
    tx_packetsToBatch = tx_packetsToBatch == 0 ? 4 : tx_packetsToBatch;
    tx_buffersCount = 1; // must be power of 2
    while (tx_buffersCount < numTransfers)
        tx_buffersCount <<=1;
    
    int bufferSize = packetsToBatch*sizeof(lime::PacketLTE);
    
    if (tx_handles != nullptr)
        delete [] tx_handles;
    tx_handles = new int[numTransfers];
    memset(tx_handles, 0, numTransfers*sizeof(int));
    
    if (tx_buffers != nullptr)
        delete [] tx_buffers;
    tx_buffers = new char[numTransfers*bufferSize];
    memset(tx_buffers,0,numTransfers*bufferSize);
    
    if (tx_bufferUsed != nullptr)
        delete [] tx_bufferUsed;
    tx_bufferUsed = new bool[numTransfers];
    memset(tx_bufferUsed,0,numTransfers);
    tx_running = true;
    if (rx_running == false)
        Start();
    tx_lock.unlock();
	return 0;
}

int LMS7_Device::StopRx()
{
    rx_lock.lock();
    int bufferSize = rx_packetsToBatch*sizeof(lime::PacketLTE);
    rx_running = false;
    controlPort->AbortReading();
    
    if (rx_handles != nullptr)
    for (int j = 0; j<rx_buffersCount; j++)
    {
        long bytesToRead = bufferSize;
        controlPort->WaitForReading(rx_handles[j], 30);
        controlPort->FinishDataReading(&rx_buffers[j*bufferSize], bytesToRead, rx_handles[j]); 
    }
    
    if (rx_handles != nullptr)
    {
        delete [] rx_handles;
        rx_handles = nullptr;
    }
    
    if (rx_buffers != nullptr)
    {
        delete [] rx_buffers;
        rx_buffers = nullptr;
    }
        
    if (tx_running == false)
        Stop();  
    rx_lock.unlock();
	return 0;
}

int LMS7_Device::StopTx()
{
    tx_lock.lock();
    int bufferSize = tx_packetsToBatch*sizeof(lime::PacketLTE);
    tx_running = false;
    // Wait for all the queued requests to be cancelled
    controlPort->AbortSending();
    
    if (tx_bufferUsed != nullptr)
        for (int j = 0; j<tx_buffersCount; j++)
        {
            long bytesToSend = bufferSize;
            if (tx_bufferUsed[j])
            {
                controlPort->WaitForSending(tx_handles[j], 30);
                controlPort->FinishDataSending(&tx_buffers[j*bufferSize], bytesToSend, tx_handles[j]);
            }
        }
    
    if (tx_handles != nullptr)
    {
        delete [] tx_handles;
        tx_handles = nullptr;
    }
    
    if (tx_buffers != nullptr)
    {
        delete [] tx_buffers;
        tx_buffers = nullptr;
    }
    
    if (tx_bufferUsed != nullptr)
    {
        delete [] tx_bufferUsed; 
        tx_bufferUsed = nullptr;
    }
        
    if (rx_running == false)
        Stop(); 
    tx_lock.unlock();
	return 0;
}


int LMS7_Device::_write(int16_t *data, uint64_t timestamp, uint64_t meta, unsigned timeout)
{
    if (tx_running == false)
        return 0;

    long bufferSize = tx_packetsToBatch*sizeof(lime::PacketLTE);
    const int buffersCountMask = tx_buffersCount - 1;
    static int bi = 0; //buffer index
    
    if (tx_bufferUsed[bi])
    {
        if (controlPort->WaitForSending(tx_handles[bi], timeout) == false)
        {
            return -1;
        }
        
        int bytesSent;
        if ((bytesSent = controlPort->FinishDataSending(&tx_buffers[bi*bufferSize], bufferSize, tx_handles[bi]))<bufferSize)
            return bytesSent;

        tx_bufferUsed[bi] = false;
    }
    
    int n = 0;
    for (int p = 0; p < tx_packetsToBatch;p++)
    {
        uint32_t statusFlags = 0;

        lime::PacketLTE* pkt = (lime::PacketLTE*)&tx_buffers[bi*bufferSize];
        pkt[p].counter = timestamp;
        
        pkt[p].reserved[0] = (meta & 0xFF);
       
        for (int i = 0; i<sizeof(pkt[p].data);i+=3)
        {
                //I sample
                pkt[p].data[i] = data[n] & 0xFF;
                pkt[p].data[i + 1] = (data[n] >> 8) & 0x0F;
                n++;
                //Q sample
                pkt[p].data[i + 1] |= (data[n] << 4) & 0xF0;
                pkt[p].data[i + 2] = (data[n] >> 4) & 0xFF;
                n++;
        }
        timestamp += SAMPLES12_PACKET/channelsCount;
    }

    tx_handles[bi] = controlPort->BeginDataSending(&tx_buffers[bi*bufferSize], bufferSize);
    tx_bufferUsed[bi] = true;

    bi = (bi + 1) & buffersCountMask;   
    return tx_packetsToBatch*SAMPLES12_PACKET;
}








