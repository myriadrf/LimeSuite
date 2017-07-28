/*
 * File:   lms7_device.cpp
 * Author: ignas
 *
 * Created on March 9, 2016, 12:54 PM
 */

#include "lms7_device.h"
#include "qLimeSDR.h"
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
#include "ConnectionRegistry.h"
#include "ADF4002.h"
#include "mcu_programs.h"

const double LMS7_Device::LMS_CGEN_MAX = 640e6;

LMS7_Device* LMS7_Device::CreateDevice(lime::IConnection* conn, LMS7_Device *obj)
{
    LMS7_Device* device;
    if (conn != nullptr)
    {
        auto info = conn->GetDeviceInfo();
        if (info.deviceName == "LimeSDR-QPCIe")
            device = new LMS7_qLimeSDR(obj);
        else
            device = new LMS7_Device(obj);
        device->_Initialize(conn);
    }
    else
    {
        device = new LMS7_Device(obj);
        device->_Initialize(nullptr);
    }
    if (obj != nullptr)
        delete obj;
    return device;
}

LMS7_Device::LMS7_Device(LMS7_Device *obj) : connection(nullptr), lms_chip_id(0)
{
    if (obj != nullptr)
    {
        this->lms_list = obj->lms_list;
        obj->lms_list.clear();
        this->rx_channels = obj->rx_channels;
        this->tx_channels = obj->tx_channels;
        this->connection = obj->connection;
        obj->connection = nullptr;
    }
}

LMS7_Device::~LMS7_Device()
{
    for (unsigned i = 0; i < this->lms_list.size();i++)
        delete this->lms_list[i];

    if (this->connection != nullptr)
	lime::ConnectionRegistry::freeConnection(this->connection);
}

void LMS7_Device::_Initialize(lime::IConnection* conn)
{
    this->tx_channels.resize(this->GetNumChannels());
    this->rx_channels.resize(this->GetNumChannels());

    while (this->lms_list.size() > this->GetNumChannels()/2)
    {
        delete this->lms_list.back();
        this->lms_list.pop_back();
    }

    while (this->lms_list.size() < this->GetNumChannels()/2)
    {
        this->lms_list.push_back(new lime::LMS7002M());
    }

    for (unsigned i = 0; i < this->lms_list.size(); i++)
        this->lms_list[i]->EnableValuesCache(false);
    this->SetConnection(conn);
}

int LMS7_Device::SetConnection(lime::IConnection* conn)
{
    if (this->connection != nullptr)
	lime::ConnectionRegistry::freeConnection(this->connection);

    for (unsigned i = 0; i < this->lms_list.size(); i++)
        this->lms_list[i]->SetConnection(conn, i);
    this->connection = conn;
    return 0;
}

lime::IConnection* LMS7_Device::GetConnection(unsigned chan)
{
    return this->connection;
}

int LMS7_Device::ConfigureRXLPF(bool enabled,int ch,float_type bandwidth)
{
    lime::LMS7002M* lms = lms_list[ch/2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC),(ch%2)+1,true)!=0)
        return -1;

    if (enabled)
    {
        if (lms->Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB),0,true)!=0)
            return -1;
        lms->Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB),0,true);
        lms->Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB),0,true);

        if (bandwidth > 0)
        {
            if (lms->SPI_read(0x2F, true)== 0x3840)
                lms->EnableCalibrationByMCU(false);
            else
                lms->EnableCalibrationByMCU(true);
            if (lms->TuneRxFilter(bandwidth)!=0)
                return -1;
        }
    }
    else
    {
        if (lms->Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB),1,true)!=0)
            return -1;
        lms->Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB),1,true);
        lms->Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB),2,true);
    }

    return 0;
}

int LMS7_Device::ConfigureGFIR(bool enabled,bool tx, double bandwidth, size_t ch)
{
    double w,w2;
    int L;
    int div = 1;

    bandwidth /= 1e6;
    lime::LMS7002M* lms = lms_list[ch / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC),(ch%2)+1,true)!=0)
        return -1;

    if (tx)
    {
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_TXTSP),enabled==false,true);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_TXTSP),enabled==false,true);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_TXTSP),enabled==false,true);

    }
    else
    {
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), enabled == false, true);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), enabled == false, true);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), enabled == false, true);
        if (lms->Get_SPI_Reg_bits(LMS7_MASK, true) != 0)
        {
            if (ch%2)
                lms->Modify_SPI_Reg_bits(LMS7param(CDSN_RXBLML), enabled == false, true);
            else
                lms->Modify_SPI_Reg_bits(LMS7param(CDSN_RXALML), enabled == false, true);
        }
    }

    if (bandwidth < 0)
        return 0;

    if (enabled)
    {

        double interface_MHz;
        int ratio;

        if (tx)
        {
            ratio = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
            interface_MHz = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx) / 1e6;
        }
        else
        {
            ratio = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
            interface_MHz = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx) / 1e6;
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
        if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_L_TXTSP), L, true) != 0)
           return -1;
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_N_TXTSP), div, true);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_L_TXTSP), L, true);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_N_TXTSP), div, true);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_L_TXTSP), L, true);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_N_TXTSP), div, true);

    }
    else
    {
        if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_L_RXTSP), L, true) != 0)
          return -1;
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_N_RXTSP), div, true);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_L_RXTSP), L, true);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_N_RXTSP), div, true);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_L_RXTSP), L, true);
        lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_N_RXTSP), div, true);

    }

    if ((lms->SetGFIRCoefficients(tx, 0, gfir2, 40) != 0)
        || (lms->SetGFIRCoefficients(tx, 1, gfir2, 40) != 0)
        || (lms->SetGFIRCoefficients(tx, 2, gfir1, 120) != 0))
        return -1;

  return 0;
}

int LMS7_Device::ConfigureTXLPF(bool enabled,int ch,double bandwidth)
{
    lime::LMS7002M* lms = lms_list[ch / 2];
    if ((ch&1) == 1)
    {
        if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2, true) != 0)
            return -1;
    }
    else
    {
        if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1, true) != 0)
           return -1;
    }
    if (enabled)
    {
        if (lms->Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB), 0, true) != 0)
            return -1;
        lms->Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB), 0, true);
        lms->Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB), 0, true);

            if (bandwidth > 0)
            {
                if (lms->SPI_read(0x2F, true)== 0x3840)
                    lms->EnableCalibrationByMCU(false);
                else
                    lms->EnableCalibrationByMCU(true);
                if (lms->TuneTxFilter(bandwidth) != 0)
                    return -1;
            }
    }
    else
    {
        lms->Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB), 1, true);
        lms->Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB), 1, true);
        return lms->Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB), 1, true);
    }
    return 0;
}

size_t LMS7_Device::GetNumChannels(const bool tx) const
{
    return 2;
}

int LMS7_Device::SetRate(double f_Hz, int oversample)
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

    if (oversample == 0)
        oversample =  LMS_CGEN_MAX/(8*f_Hz);

   if (oversample > 1)
   {
       for (decim = 0; decim < 4; decim++)
       {
            if ( (1<<decim) >= (oversample+1)/2)
                break;
       }
   }
   else decim = 7;

   int ratio = oversample <= 2 ? 2 : (2<<decim);
   float_type cgen = f_Hz*4*ratio;
   if (cgen > LMS_CGEN_MAX)
   {
       lime::ReportError(ERANGE, "Cannot set desired sample rate. CGEN clock out of range");
       return -1;
   }
   for (unsigned i = 0; i < lms_list.size(); i++)
   {
        lime::LMS7002M* lms = lms_list[i];
       if ((lms->SetFrequencyCGEN(cgen) != 0)
           || (lms->Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0) != 0)
           || (lms->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2) != 0)
           || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2, true) != 0)
           || (lms->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), decim) != 0)
           || (lms->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), decim) != 0)
           || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1, true) != 0)
           || (lms->SetInterfaceFrequency(lms->GetFrequencyCGEN(), decim, decim) != 0))
           return -1;

        float_type fpgaTxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx);
        float_type fpgaRxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx);
        if (decim != 7)
        {
            fpgaTxPLL /= pow(2.0, decim);
            fpgaRxPLL /= pow(2.0, decim);
        }
        if (this->connection->UpdateExternalDataRate(i, fpgaTxPLL / 2, fpgaRxPLL / 2) != 0)
           return -1;
   }

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

int LMS7_Device::SetRate(bool tx, double f_Hz, unsigned oversample)
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

   lime::LMS7002M* lms = lms_list[0];

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

    if (oversample == 0)
        oversample = tx ? LMS_CGEN_MAX/(2*f_Hz) : LMS_CGEN_MAX/(8*f_Hz);

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
    else tmp = 7;

    int ratio = oversample <= 2 ? 2 : (2<<tmp);

    if (tx)
    {
        interpolation = tmp;
        decimation = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
        rx_clock = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx);
        tx_clock = f_Hz*ratio;
    }
    else
    {
        decimation = tmp;
        interpolation = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
        tx_clock = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx);
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

    for (unsigned i = 0; i < lms_list.size(); i++)
      {
        lms = lms_list[i];
        if ((lms->SetFrequencyCGEN(cgen, retain_nco) != 0)
	    || (lms->Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), clk_mux) != 0)
	    || (lms->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), clk_div) != 0)
	    || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2, true) != 0)
	    || (lms->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), decimation) != 0)
	    || (lms->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), interpolation) != 0)
	    || (lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1, true) != 0)
	    || (lms->SetInterfaceFrequency(lms->GetFrequencyCGEN(), interpolation, decimation) != 0))
	  return -1;

	float_type fpgaTxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx);
	float_type fpgaRxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx);
	if (interpolation != 7) {
	  fpgaTxPLL /= pow(2.0, interpolation);
	}
	if (decimation != 7) {
	  fpgaRxPLL /= pow(2.0, decimation);
	}
        if (this->connection->UpdateExternalDataRate(i, fpgaTxPLL / 2, fpgaRxPLL / 2) != 0)
	  return -1;
      }

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

int LMS7_Device::SetRate(unsigned ch, double rxRate, double txRate, unsigned oversample)
{
    //TODO: low importance : implement this and expose via LimeSuite.h
    return lime::ReportError(ERANGE, "Not implemented");;
}

double LMS7_Device::GetRate(bool tx, unsigned chan, double *rf_rate_Hz)
{

    double interface_Hz;
    int ratio;
    lime::LMS7002M* lms = lms_list[chan / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan%2) + 1, true) != 0)
        return -1;

    if (tx)
    {
        ratio = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP),true);
        interface_Hz = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx);
    }
    else
    {
        ratio = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP),true);
        interface_Hz = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx);
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
  ret.max = 80e6;
  ret.min = 100e3;
  ret.step = 1;
  return ret;
}

lms_range_t LMS7_Device::GetTxRateRange(size_t chan) const
{
  lms_range_t ret;
  ret.max = 80e6;
  ret.min = 100e3;
  ret.step = 1;
  return ret;
}

std::vector<std::string> LMS7_Device::GetPathNames(bool dir_tx, size_t chan) const
{
    if (dir_tx)
        return {"NONE", "TX_PATH1", "TX_PATH2"};
    else
        return {"NONE", "LNA_H", "LNA_L", "LNA_W"};
}

int LMS7_Device::SetPath(bool tx, size_t chan, size_t path)
{
    lime::LMS7002M* lms = lms_list[chan / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan%2) + 1, true) != 0)
        return -1;
    if (tx==false)
    {
        if ((lms->Modify_SPI_Reg_bits(LMS7param(SEL_PATH_RFE),path,true)!=0)
        || (lms->Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_L_RFE), path != 2, true) != 0)
        || (lms->Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_W_RFE), path != 3, true) != 0))
            return -1;
    }
    else
    {
        if ((lms->Modify_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF), path == LMS_PATH_TX1, true) != 0)
        || (lms->Modify_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF), path == LMS_PATH_TX2, true) != 0))
            return -1;
    }
    return 0;
}

size_t LMS7_Device::GetPath(bool tx, size_t chan)
{
    lime::LMS7002M* lms = lms_list[chan / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan%2) + 1, true) != 0)
        return -1;
    if (tx)
    {
        if (lms->Get_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF), true))
            return LMS_PATH_TX2;
        else if (lms->Get_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF), true))
            return LMS_PATH_TX1;
        else
            return 0;
    }

    return lms->Get_SPI_Reg_bits(LMS7param(SEL_PATH_RFE), true);
}

lms_range_t LMS7_Device::GetRxPathBand(size_t path, size_t chan) const
{
  lms_range_t ret;
  ret.step = 1;
  if (this->connection->GetDeviceInfo().deviceName == lime::GetDeviceName(lime::LMS_DEV_ULIMESDR))
  {
      if (path == LMS_PATH_LNAL)
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
      case LMS_PATH_LNAH:
            ret.max = 3800000000;
            ret.min = 2000000000;
            break;
      case LMS_PATH_LNAW:
            ret.max = 2600000000;
            ret.min = 700000000;
            break;
      case LMS_PATH_LNAL:
            ret.max = 1000000000;
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
  if (this->connection->GetDeviceInfo().deviceName == lime::GetDeviceName(lime::LMS_DEV_ULIMESDR))
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
            ret.min = 2000000000;
            break;
      case LMS_PATH_TX1:
            ret.max = 2000000000;
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
    return ConfigureGFIR(en,tx,bandwidth,chan);
}

float_type LMS7_Device::GetLPFBW(bool tx,size_t chan, bool filt)
{
    if (filt)
    {
        if (tx)
            return tx_channels[chan].lpf_bw;
        else
            return rx_channels[chan].lpf_bw;
    }
    return -1;
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
    lime::LMS7002M* lms = lms_list[chan / 2];
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

    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan%2) + 1, true) != 0)
        return -1;

    double interface_Hz;
    int ratio;

    if (tx)
    {
        ratio = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
        interface_Hz = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx);
    }
    else
    {
        ratio = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
        interface_Hz = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx);
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
    for (unsigned i=0; i< count; i++)
        if (fabs(coef[i])>max)
            max=fabs(coef[i]);

    if (max < 1.0)
        max = 1.0f;
    else if (max > 10.0)
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
          if ((lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_L_TXTSP), L, true) != 0)
              || (lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_N_TXTSP), div, true) != 0))
              return -1;
      }
      else if (filt ==LMS_GFIR2)
      {
          if ((lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_L_TXTSP), L, true) != 0)
              || (lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_N_TXTSP), div, true) != 0))
              return -1;
      }
      else if (filt ==LMS_GFIR3)
      {
          if ((lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_L_TXTSP), L, true) != 0)
              || (lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_N_TXTSP), div, true) != 0))
              return -1;
      }
    }
    else
    {
      if (filt ==LMS_GFIR1)
      {
          if ((lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_L_RXTSP), L, true) != 0)
              || (lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_N_RXTSP), div, true) != 0))
            return -1;
      }
      else if (filt ==LMS_GFIR2)
      {
          if ((lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_L_RXTSP), L, true) != 0)
              || (lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_N_RXTSP), div, true) != 0))
            return -1;
      }
      else if (filt ==LMS_GFIR3)
      {
          if ((lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_L_RXTSP), L, true) != 0)
              || (lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_N_RXTSP), div, true) != 0))
            return -1;
      }
    }

    if (lms->SetGFIRCoefficients(tx, filt, gfir, filt == LMS_GFIR3 ? 120 : 40) != 0)
       return -1;
   return ret;
}

int LMS7_Device::GetGFIRCoef(bool tx, size_t chan, lms_gfir_t filt, float_type* coef)
{
    lime::LMS7002M* lms = lms_list[chan / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan%2) + 1, true) != 0)
       return -1;
   int16_t coef16[120];

   if (lms->GetGFIRCoefficients(tx, filt, coef16, filt == LMS_GFIR3 ? 120 : 40) != 0)
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
    lime::LMS7002M* lms = lms_list[chan / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan%2) + 1, true) != 0)
        return -1;
    if (tx)
    {
        if (filt ==LMS_GFIR1)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_TXTSP), enabled == false, true) != 0)
                return -1;
        }
        else if (filt == LMS_GFIR2)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_TXTSP), enabled == false, true) != 0)
                return -1;
        }
        else if (filt == LMS_GFIR3)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_TXTSP), enabled == false, true) != 0)
                return -1;
        }
    }
    else
    {
        if (filt ==LMS_GFIR1)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), enabled == false, true) != 0)
                return -1;
        }
        else if (filt == LMS_GFIR2)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), enabled == false, true) != 0)
                return -1;
        }
        else if (filt == LMS_GFIR3)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), enabled == false, true) != 0)
                return -1;
        }
    }

   return 0;
}

int LMS7_Device::SetNormalizedGain(bool dir_tx, size_t chan,double gain)
{
    const int gain_total = 27 + 12 + 31; //LNA + TIA + PGA
    if (dir_tx)
        return SetGain(dir_tx, chan, maxTxGain*gain+0.49);
    else
        return SetGain(dir_tx, chan, gain*gain_total+0.49);
}

int LMS7_Device::SetGain(bool dir_tx, size_t chan, unsigned gain)
{
    lime::LMS7002M* lms = lms_list[chan / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan%2) + 1, true) != 0)
        return -1;

    if (dir_tx) //TX valid gain range 0-52
    {
        if (lms->CalibrateTxGain(0,nullptr)!=0) //find optimal BB gain
            return -1;
        //From datasheet: TXPAD: 0<=Loss<=10 – Pout=Pout_max-Loss; 11<=Loss<31 – Pout=Pout_max-10-2*(Loss-10)
        //therefore MAX TDPAD gain: 52 = 10+21*2
        if (gain > 52) //increase BB gain over optimal, can cause saturation in amp
        {
            int gbb = lms->Get_SPI_Reg_bits(LMS7param(CG_IAMP_TBB),true);
            double dgain = gain-52;
            gain = (double)gbb*pow(10.0,dgain/20.0)+0.5;
            lms->Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB),gain > 63 ? 63 : gain, true);
            gain = 0;
        }
        else if (gain >= 42)
            gain = 52-gain;
        else
            gain = 31-gain/2;
        if (lms->Modify_SPI_Reg_bits(LMS7param(LOSS_LIN_TXPAD_TRF),gain,true)!=0)
            return -1;
        if (lms->Modify_SPI_Reg_bits(LMS7param(LOSS_MAIN_TXPAD_TRF),gain,true)!=0)
            return -1;
    }
    else //Rx valid gain range 0-70
    {
        if (gain > 70) //do not exceed gain table index
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

        int rcc_ctl_pga_rbb = (430*(pow(0.65,((double)pga/10)))-110.35)/20.4516+16; //from datasheet

        if ((lms->Modify_SPI_Reg_bits(LMS7param(G_LNA_RFE),lna+1,true)!=0)
          ||(lms->Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE),tia+1,true)!=0)
          ||(lms->Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB),pga,true)!=0)
          ||(lms->Modify_SPI_Reg_bits(LMS7param(RCC_CTL_PGA_RBB),rcc_ctl_pga_rbb,true)!=0))
            return -1;
    }
    return 0;
}

double LMS7_Device::GetNormalizedGain(bool dir_tx, size_t chan)
{
    const double gain_total = 27 + 12 + 31; //LNA + TIA + PGA
    double ret = GetGain(dir_tx, chan);
    if (dir_tx)
        return ret > maxTxGain ? 1.0 : ret / maxTxGain;
    else
        return ret / gain_total;
}

int LMS7_Device::GetGain(bool dir_tx, size_t chan)
{
    lime::LMS7002M* lms = lms_list[chan / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan%2) + 1, true) != 0)
        return -1;
    if (dir_tx)
    {
        int gain = lms->Get_SPI_Reg_bits(LMS7param(LOSS_MAIN_TXPAD_TRF),true);
        if (gain <= 10)
            gain = 52-gain;
        else
            gain = 62-gain*2;
        int bak = lms->Get_SPI_Reg_bits(LMS7param(CG_IAMP_TBB),true); //backup
        if (lms->CalibrateTxGain(0,nullptr)!=0)
            return -1;
        int opt = lms->Get_SPI_Reg_bits(LMS7param(CG_IAMP_TBB),true);
        gain += 20.0*log10((double)bak / (double) opt)+0.5;

        lms->Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB),bak, true); //restore

        return gain;
    }
    else
    {
        const int max_gain_lna = 27;
        const int max_gain_tia = 12;

        int pga = lms->Get_SPI_Reg_bits(LMS7param(G_PGA_RBB),true);
        int ret = pga;

        int tia = lms->Get_SPI_Reg_bits(LMS7param(G_TIA_RFE),true);
        if (tia == 3)
            ret +=max_gain_tia;
        else if (tia == 2)
            ret += max_gain_tia-3;

        int lna = lms->Get_SPI_Reg_bits(LMS7param(G_LNA_RFE),true);

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
    lime::LMS7002M* lms = lms_list[chan / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan%2) + 1, true) != 0)
        return -1;

    if (dir_tx == false)
    {
        if (lms->Modify_SPI_Reg_bits(LMS7param(INSEL_RXTSP), sig != LMS_TESTSIG_NONE, true) != 0)
            return -1;

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV8F)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1, true);
        else if (sig == LMS_TESTSIG_NCODIV4 || sig == LMS_TESTSIG_NCODIV4F)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 2, true);

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV4)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 0, true);
        else if (sig == LMS_TESTSIG_NCODIV8F || sig == LMS_TESTSIG_NCODIV4F)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 1, true);

        return lms->Modify_SPI_Reg_bits(LMS7param(TSGMODE_RXTSP), sig == LMS_TESTSIG_DC, true);
        ;
    }
    else
    {
        if (lms->Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), sig != LMS_TESTSIG_NONE, true) != 0)
            return -1;

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV8F)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), 1, true);
        else if (sig == LMS_TESTSIG_NCODIV4 || sig == LMS_TESTSIG_NCODIV4F)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), 2, true);

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV4)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_TXTSP), 0, true);
        else if (sig == LMS_TESTSIG_NCODIV8F || sig == LMS_TESTSIG_NCODIV4F)
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_TXTSP), 1, true);

        return lms->Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), sig == LMS_TESTSIG_DC, true);
        ;
    }

    if (sig == LMS_TESTSIG_DC)
        return lms->LoadDC_REG_IQ(dir_tx, dc_i, dc_q);

    return 0;
}

int LMS7_Device::GetTestSignal(bool dir_tx, size_t chan)
{
    lime::LMS7002M* lms = lms_list[chan / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan%2) + 1, true) != 0)
        return -1;

    if (dir_tx)
    {
        if (lms->Get_SPI_Reg_bits(LMS7param(INSEL_TXTSP), true) == 0)
        {
            return LMS_TESTSIG_NONE;
        }
        else if (lms->Get_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), true) != 0)
        {
            return LMS_TESTSIG_DC;
        }
        else return lms->Get_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), true) + 2 * lms->Get_SPI_Reg_bits(LMS7param(TSGFC_TXTSP), true);
    }
    else
    {
        if (lms->Get_SPI_Reg_bits(LMS7param(INSEL_RXTSP), true) == 0)
        {
            return LMS_TESTSIG_NONE;
        }
        else if (lms->Get_SPI_Reg_bits(LMS7param(TSGMODE_RXTSP), true) != 0)
        {
            return LMS_TESTSIG_DC;
        }
        else return lms->Get_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), true) + 2 * lms->Get_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), true);
    }
    return -1;
}

int LMS7_Device::SetNCOFreq(bool tx, size_t ch, const float_type *freq, float_type pho)
{
    lime::LMS7002M* lms = lms_list[ch / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (ch%2) + 1, true) != 0)
        return -1;

    float_type rf_rate;
    GetRate(tx,ch,&rf_rate);
    rf_rate /=2;
    if (freq != nullptr)
    {
        for (size_t i = 0; i < LMS_NCO_VAL_COUNT; i++)
        {
            if (freq[i] < 0 || freq[i] > rf_rate)
            {
                lime::ReportError(ERANGE, "NCO frequency is negative or outside of RF bandwidth range");
                return -1;
            }
        if (lms->SetNCOFrequency(tx, i, freq[i]) != 0)
                return -1;
        }
        if (tx)
        {
            if ((lms->Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP), 0, true) != 0)
                || (lms->Modify_SPI_Reg_bits(LMS7param(SEL_TX), 0, true) != 0))
                    return -1;
            if (lms->Modify_SPI_Reg_bits(LMS7param(MODE_TX), 0, true) != 0)
                    return -1;
        }
        else
        {
            if ((lms->Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 0, true) != 0)
                || (lms->Modify_SPI_Reg_bits(LMS7param(SEL_RX), 0, true) != 0))
                    return -1;
            if (lms->Modify_SPI_Reg_bits(LMS7param(MODE_RX), 0, true) != 0)
                    return -1;
        }
    }
    return lms->SetNCOPhaseOffsetForMode0(tx, pho);
}

int LMS7_Device::SetNCO(bool tx,size_t ch,int ind,bool down)
{
    lime::LMS7002M* lms = lms_list[ch / 2];
    if ((!tx) && (lms->Get_SPI_Reg_bits(LMS7_MASK, true) != 0))
        down = !down;
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC),(ch%2)+1,true)!=0)
        return -1;
    if (ind < 0)
    {
        if (tx)
        {
            if ((lms->Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP),1,true)!=0)
            || (lms->Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_TXTSP), 0, true)!=0))
                return -1;
        }
        else
        {
            if ((lms->Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP),1,true)!=0)
            || (lms->Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_RXTSP), 0, true)!=0))
                return -1;
        }
    }
    else if (ind < LMS_NCO_VAL_COUNT)
    {
        if (tx)
        {
            if ((lms->Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP),0,true)!=0)
            || (lms->Modify_SPI_Reg_bits(LMS7param(SEL_TX),ind,true)!=0)
            || (lms->Modify_SPI_Reg_bits(LMS7param(CMIX_SC_TXTSP),down,true)!=0)
            || (lms->Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_TXTSP), 1, true)!=0))
                return -1;
        }
        else
        {
            if ((lms->Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP),0,true)!=0)
            || (lms->Modify_SPI_Reg_bits(LMS7param(SEL_RX),ind,true)!=0)
            || (lms->Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP),down,true)!=0)
            || (lms->Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_RXTSP), 1, true)!=0))
                return -1;
        }
    }
    else
        return lime::ReportError("Invalid NCO index value");
    return 0;
}

int LMS7_Device::GetNCOFreq(bool tx, size_t ch, float_type *freq,float_type *pho)
{
    lime::LMS7002M* lms = lms_list[ch / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (ch%2) + 1, true) != 0)
        return -1;
    if (freq != nullptr)
    {
        for (size_t i = 0; i < LMS_NCO_VAL_COUNT; i++)
        {
            freq[i] = lms->GetNCOFrequency(tx,i,true);
        }
    }

    if (pho != nullptr)
    {
        uint16_t value = lms->SPI_read(tx ? 0x0241 : 0x0441,true);
        *pho = 360.0 * value / 65536.0;
    }

    return 0;
}

int LMS7_Device::SetNCOPhase(bool tx, size_t ch, const float_type *phase, float_type fcw)
{
    lime::LMS7002M* lms = lms_list[ch / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (ch%2) + 1, true) != 0)
        return -1;

    if (phase != nullptr)
    {
        for (size_t i = 0; i < LMS_NCO_VAL_COUNT; i++)
        {
            if (lms->SetNCOPhaseOffset(tx,i,phase[i])!=0)
                return -1;
        }

        if (tx)
        {
            if ((lms->Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP), 0, true) != 0)
                || (lms->Modify_SPI_Reg_bits(LMS7param(SEL_TX), 0, true) != 0)
                || (lms->Modify_SPI_Reg_bits(LMS7param(CMIX_SC_TXTSP), 0, true) != 0))
                return -1;
            if (lms->Modify_SPI_Reg_bits(LMS7param(MODE_TX), 1, true) != 0)
                return -1;
        }
        else
        {
            if ((lms->Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 0, true) != 0)
                || (lms->Modify_SPI_Reg_bits(LMS7param(SEL_RX), 0, true) != 0)
                || (lms->Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP), 0, true) != 0))
                return -1;
            if (lms->Modify_SPI_Reg_bits(LMS7param(MODE_RX), 1, true) != 0)
                return -1;
        }
    }

    return lms->SetNCOFrequency(tx,0,fcw);
}


int LMS7_Device::GetNCOPhase(bool tx, size_t ch, float_type *phase,float_type *fcw)
{
    lime::LMS7002M* lms = lms_list[ch / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (ch%2) + 1, true) != 0)
        return -1;
    if (phase != nullptr)
    {
        for (size_t i = 0; i < LMS_NCO_VAL_COUNT; i++)
        {
            phase[i] = lms->GetNCOPhaseOffset_Deg(tx,i);
        }
    }
    if (fcw != nullptr)
        *fcw = lms->GetNCOFrequency(tx,0,true);
    return 0;
}

int LMS7_Device::GetNCO(bool tx, size_t ch)
{
    lime::LMS7002M* lms = lms_list[ch / 2];
    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (ch%2) + 1, true) != 0)
        return -2;
    if (tx)
    {
        if (lms->Get_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP), true) != 0)
        {
            lime::ReportError(ENODEV, "NCO is disabled");
            return -1;
        }
        return lms->Get_SPI_Reg_bits(LMS7param(SEL_TX), true);
    }

    if (lms->Get_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), true) != 0)
    {
        lime::ReportError(ENODEV, "NCO is disabled");
        return -1;
    }
    return lms->Get_SPI_Reg_bits(LMS7param(SEL_RX), true);
}

int LMS7_Device::Calibrate(bool dir_tx, size_t chan, double bw, unsigned flags)
{
    lime::LMS7002M* lms = lms_list[chan / 2];
    if (chan >= this->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }
    lms->EnableCalibrationByMCU((flags&1) == 0);
    lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan%2) + 1, true);
    if (dir_tx)
        return lms->CalibrateTx(bw, false);
    else
        return lms->CalibrateRx(bw, false);
}

int LMS7_Device::SetRxFrequency(size_t chan, double f_Hz)
{
    lime::LMS7002M* lms = lms_list[chan / 2];
    if (f_Hz < 30e6)
    {
        if (lms->SetFrequencySX(false, 30e6) != 0)
            return -1;
        rx_channels[chan].cF_offset_nco = 30e6-f_Hz;
        if (SetRate(false,GetRate(true,chan),2)!=0)
            return -1;
    }
    else
    {
        if (rx_channels[chan].cF_offset_nco != 0)
            SetNCO(false,chan,-1,true);
        rx_channels[chan].cF_offset_nco = 0;
        if (lms->SetFrequencySX(false, f_Hz) != 0)
            return -1;
    }
    return 0;
}

float_type LMS7_Device::GetTRXFrequency(bool tx, size_t chan)
{
   lime::LMS7002M* lms = lms_list[chan / 2];
   double offset = tx ? tx_channels[chan].cF_offset_nco : rx_channels[chan].cF_offset_nco;
   return lms->GetFrequencySX(tx) - offset;
}

int LMS7_Device::SetTxFrequency(size_t chan, double f_Hz)
{
    lime::LMS7002M* lms = lms_list[chan / 2];
    if (f_Hz < 30e6)
    {
        if (lms->SetFrequencySX(true, 30e6) != 0)
            return -1;
        tx_channels[chan].cF_offset_nco = 30e6-f_Hz;
        if (SetRate(true,GetRate(true,chan),2)!=0)
            return -1;
    }
    else
    {
        if (tx_channels[chan].cF_offset_nco != 0)
            SetNCO(true,chan,-1,false);
        tx_channels[chan].cF_offset_nco = 0;
        if (lms->SetFrequencySX(true, f_Hz) != 0)
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
    struct regVal
    {
        uint16_t adr;
        uint16_t val;
     };

    const std::vector<regVal> initVals = {
        {0x0022, 0x07FF}, {0x0023, 0x5550}, {0x002B, 0x0038}, {0x002C, 0x0000},
        {0x002D, 0x0641}, {0x0086, 0x4101}, {0x0087, 0x5555}, {0x0088, 0x0525},
        {0x0089, 0x1078}, {0x008B, 0x1F8C}, {0x008C, 0x267B}, {0x00A6, 0x000F},
        {0x00A9, 0x8000}, {0x00AC, 0x2000}, {0x0108, 0x3026}, {0x010C, 0x8865},
        {0x010E, 0x0000}, {0x0110, 0x0BFF}, {0x0113, 0x03C2}, {0x011C, 0xA941},
        {0x011D, 0x0000}, {0x011E, 0x0984}, {0x0120, 0xB9FF}, {0x0121, 0x3650},
        {0x0122, 0x033F}, {0x0123, 0x267B}, {0x0200, 0x00E1}, {0x0208, 0x0170},
        {0x020B, 0x4000}, {0x020C, 0x8000}, {0x0400, 0x8081}, {0x040B, 0x1020},
        {0x040C, 0x00F8}
    };

    for (unsigned i = 0; i < lms_list.size(); i++)
    {
        lime::LMS7002M* lms = lms_list[i];
        if (lms->ResetChip() != 0)
            return -1;

        lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
        for (auto i : initVals)
            lms->SPI_write(i.adr, i.val);

        lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
        for (auto i : initVals)
            if (i.adr >= 0x100)
                lms->SPI_write(i.adr, i.val);
        lms->EnableChannel(false, false);
        lms->EnableChannel(true, false);

        lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);

        if (lms->UploadAll()!=0)
            return -1;
    }
    return 0;
}

int LMS7_Device::Reset()
{
    for (unsigned i = 0; i < lms_list.size(); i++)
    {
        lime::LMS7002M* lms = lms_list[i];
        if (lms->ResetChip() != 0)
            return -1;
        lms->DownloadAll();
    }
    return LMS_SUCCESS;
}

int LMS7_Device::EnableChannel(bool dir_tx, size_t chan, bool enabled)
{
    lime::LMS7002M* lms = lms_list[chan / 2];
    if (chan >= this->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    if (lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan%2) + 1, true) != 0)
        return -1;

    lms->EnableChannel(dir_tx, enabled);

    return LMS_SUCCESS;
}

int LMS7_Device::Program(const char* data, size_t len, lms_prog_trg_t target, lms_prog_md_t mode, lime::IConnection::ProgrammingCallback callback)
{
    switch (target)
    {
        case LMS_PROG_TRG_FX3:
            if (mode == LMS_PROG_MD_FLASH)
                return this->connection->ProgramWrite(data, len, 2, 1, callback);
            else if (mode == LMS_PROG_MD_RAM)
                return this->connection->ProgramWrite(data, len, 1, 1, callback);
            else
                return this->connection->ProgramWrite(nullptr, 0, 0, 1, callback);

        case LMS_PROG_TRG_FPGA:
            return this->connection->ProgramWrite(data, len, mode, 2, callback);

        case LMS_PROG_TRG_MCU:
        {
            lime::MCU_BD *mcu = lms_list.at(lms_chip_id)->GetMCUControls();
            lime::IConnection::MCU_PROG_MODE prog_mode;
            uint8_t bin[lime::MCU_BD::cMaxFWSize];

            if(data != nullptr)
                memcpy(bin,data,len>sizeof(bin) ? sizeof(bin) : len);

            if (mode == LMS_PROG_MD_RAM)
                prog_mode = lime::IConnection::MCU_PROG_MODE::SRAM;
            else if (mode == LMS_PROG_MD_FLASH)
                prog_mode = lime::IConnection::MCU_PROG_MODE::EEPROM_AND_SRAM;
            else
            {
                mcu->Reset_MCU();
                return 0;
            }

            mcu->callback = callback;
            mcu->Program_MCU(bin,prog_mode);
            mcu->callback = nullptr;
            return 0;
        }

        case LMS_PROG_TRG_HPM7:
            return this->connection->ProgramWrite(data, len, mode, 0, callback);

        default:
            lime::ReportError(ENOTSUP, "Unsupported programming target");
            return -1;
    }
}

int LMS7_Device::ProgramUpdate(const bool download,lime::IConnection::ProgrammingCallback callback)
{
    return this->connection->ProgramUpdate(download,callback);
}

int LMS7_Device::DACWrite(uint16_t val)
{
    uint8_t id=0;
    double dval= val;
    return this->connection->CustomParameterWrite(&id, &dval, 1, NULL);
}

int LMS7_Device::DACRead()
{
    uint8_t id=0;
    double dval=0;
    int ret = this->connection->CustomParameterRead(&id, &dval, 1, NULL);
    return ret >=0 ? dval : -1;
}

int LMS7_Device::GetClockFreq(size_t clk_id, float_type *freq)
{
    switch (clk_id)
    {
    case LMS_CLOCK_REF:
        *freq = lms_list.at(lms_chip_id)->GetReferenceClk_SX(lime::LMS7002M::Rx);
        return 0;
    case LMS_CLOCK_SXR:
        *freq = lms_list.at(lms_chip_id)->GetFrequencySX(false);
        return 0;
    case LMS_CLOCK_SXT:
        *freq = lms_list.at(lms_chip_id)->GetFrequencySX(true);
        return 0;
    case LMS_CLOCK_CGEN:
        *freq = lms_list.at(lms_chip_id)->GetFrequencyCGEN();
        return 0;
    case LMS_CLOCK_RXTSP:
        *freq = lms_list.at(lms_chip_id)->GetReferenceClk_TSP(false);
        return 0;
    case LMS_CLOCK_TXTSP:
        *freq = lms_list.at(lms_chip_id)->GetReferenceClk_TSP(true);
        return 0;
    case LMS_CLOCK_EXTREF:
        lime::ReportError(ENOTSUP, "Reading external reference clock is not supported");
        return -1;
    default:
        lime::ReportError(EINVAL, "Invalid clock ID.");
        return -1;
    }
}

int LMS7_Device::SetClockFreq(size_t clk_id, float_type freq)
{
    lime::LMS7002M* lms = lms_list[lms_chip_id];
    switch (clk_id)
    {
    case LMS_CLOCK_REF:
        if (freq <= 0)
        {
            lime::ReportError(EINVAL, "Invalid frequency value.");
            return -1;
        }
        lms->SetReferenceClk_SX(lime::LMS7002M::Tx, freq);
        lms->SetReferenceClk_SX(lime::LMS7002M::Rx, freq);
        return 0;
    case LMS_CLOCK_SXR:
        if (freq <= 0)
            return lms->TuneVCO(lime::LMS7002M::VCO_SXR);
        return lms->SetFrequencySX(false, freq);
    case LMS_CLOCK_SXT:
        if (freq <= 0)
            return lms->TuneVCO(lime::LMS7002M::VCO_SXT);
        return lms->SetFrequencySX(true, freq);
    case LMS_CLOCK_CGEN:
    {
        int ret =0;
        lms->Modify_SPI_Reg_bits(LMS7param(MAC),1,true);
        if (freq <= 0)
        {
            ret = lms->TuneVCO(lime::LMS7002M::VCO_CGEN);
        }
        else
        {
            ret = lms->SetInterfaceFrequency(freq, lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP)), lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP)));
        }
        if (ret != 0)
            return -1;
        auto conn = lms->GetConnection();
        if (conn == nullptr)
        {
            lime::ReportError(EINVAL, "Device not connected");
            return -1;
        }
        int interp = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
        int decim = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
        float_type fpgaTxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx);
        if (interp != 7)
            fpgaTxPLL /= pow(2.0, interp);
        float_type fpgaRxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx);
        if (decim != 7)
            fpgaRxPLL /= pow(2.0, decim);
        return conn->UpdateExternalDataRate(lms_chip_id, fpgaTxPLL / 2, fpgaRxPLL / 2);
    }
    case LMS_CLOCK_RXTSP:
        lime::ReportError(ENOTSUP, "Setting TSP clocks is not supported.");
        return -1;
    case LMS_CLOCK_TXTSP:
        lime::ReportError(ENOTSUP, "Setting TSP clocks is not supported.");
        return -1;
    case LMS_CLOCK_EXTREF:
        {
            if (freq <= 0)
            {
                lime::ReportError(EINVAL, "Invalid frequency value.");
                return -1;
            }

            lime::ADF4002 module;
            module.SetDefaults();
            double fvco = lms_list.at(lms_chip_id)->GetReferenceClk_SX(lime::LMS7002M::Rx);
            int dummy;
            module.SetFrefFvco(freq/1e6, fvco/1e6, dummy, dummy);
            unsigned char data[12];
            module.GetConfig(data);

            std::vector<uint32_t> dataWr;
            for(int i=0; i<12; i+=3)
                dataWr.push_back((uint32_t)data[i] << 16 | (uint32_t)data[i+1] << 8 | data[i+2]);
            return connection->TransactSPI(connection->GetDeviceInfo().addrADF4002, dataWr.data(), nullptr, 4);
        }
    default:
        lime::ReportError(EINVAL, "Invalid clock ID.");
        return -1;
    }
}

lms_dev_info_t* LMS7_Device::GetInfo()
{
    memset(&devInfo,0,sizeof(lms_dev_info_t));
    auto info = this->connection->GetDeviceInfo();
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

int LMS7_Device::Synchronize(bool toChip)
{
    for (unsigned i = 0; i < lms_list.size(); i++)
    {
        lime::LMS7002M* lms = lms_list[i];
        int ret=0;
        if (toChip)
        {
            if (lms->UploadAll()==0)
            {
                lms->Modify_SPI_Reg_bits(LMS7param(MAC),1,true);
                int interp = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
                int decim = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
                float_type fpgaTxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx);
                if (interp != 7)
                    fpgaTxPLL /= pow(2.0, interp);
                float_type fpgaRxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx);
                if (decim != 7)
                    fpgaRxPLL /= pow(2.0, decim);
                lms->SetInterfaceFrequency(lms->GetFrequencyCGEN(), interp, decim);
                ret = connection->UpdateExternalDataRate(i,fpgaTxPLL/2,fpgaRxPLL/2);
            }
        }
        else
            ret = lms->DownloadAll();
        if (ret != 0)
            return ret;
    }
    return 0;
}

int LMS7_Device::SetLogCallback(void(*func)(const char* cstr, const unsigned int type))
{
    for (unsigned i = 0; i < lms_list.size(); i++)
        lms_list[i]->SetLogCallback(func);
    return 0;
}

int LMS7_Device::EnableCalibCache(bool enable)
{
    for (unsigned i = 0; i < lms_list.size(); i++)
        lms_list[i]->EnableValuesCache(enable);
    return 0;
}

int LMS7_Device::GetChipTemperature(size_t ind, float_type *temp)
{
    *temp = lms_list[this->lms_chip_id]->GetTemperature();
    return 0;
}

int LMS7_Device::LoadConfig(const char *filename)
{
    lime::LMS7002M* lms = lms_list[lms_chip_id];
    if (lms->LoadConfig(filename)==0)
    {
        lms->Modify_SPI_Reg_bits(LMS7param(MAC),1,true);
        int interp = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
        int decim = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
        float_type fpgaTxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Tx);
        if (interp != 7)
            fpgaTxPLL /= pow(2.0, interp);
        float_type fpgaRxPLL = lms->GetReferenceClk_TSP(lime::LMS7002M::Rx);
        if (decim != 7)
            fpgaRxPLL /= pow(2.0, decim);
        lms->SetInterfaceFrequency(lms->GetFrequencyCGEN(), interp, decim);
        return connection->UpdateExternalDataRate(lms_chip_id,fpgaTxPLL/2,fpgaRxPLL/2);
    }
    return -1;
}

int LMS7_Device::SaveConfig(const char *filename)
{
    return lms_list[this->lms_chip_id]->SaveConfig(filename);
}

int LMS7_Device::ReadLMSReg(uint16_t address, uint16_t *val)
{
    int status;
    *val = lms_list.at(lms_chip_id)->SPI_read(address & 0xFFFF, true, &status);
    return status;
}

int LMS7_Device::WriteLMSReg(uint16_t address, uint16_t val)
{
    return lms_list.at(lms_chip_id)->SPI_write(address & 0xFFFF, val);
}

int LMS7_Device::ReadParam(struct LMS7Parameter param, uint16_t *val, bool forceReadFromChip)
{
    //registers containing read only registers, which values can change
    const uint16_t readOnlyRegs[] = { 0, 1, 2, 3, 4, 5, 6, 0x002F, 0x008C, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x0123, 0x0209, 0x020A, 0x020B, 0x040E, 0x040F, 0x05C3, 0x05C4, 0x05C5, 0x05C6, 0x05C7, 0x05C8, 0x05C9, 0x05CA};
    for (unsigned i = 0; i < sizeof(readOnlyRegs) / sizeof(uint16_t); ++i)
    {
        if (param.address == readOnlyRegs[i])
        {
            forceReadFromChip = true;
            break;
        }
    }
    *val = lms_list.at(lms_chip_id)->Get_SPI_Reg_bits(param, forceReadFromChip);
    return LMS_SUCCESS;
}

int LMS7_Device::WriteParam(struct LMS7Parameter param, uint16_t val)
{
    return lms_list.at(lms_chip_id)->Modify_SPI_Reg_bits(param, val);
}

int LMS7_Device::SetActiveChip(unsigned ind)
{
    if (ind >= this->GetNumChannels() / 2)
    {
        lime::ReportError("Invalid chip ID");
        return -1;
    }
    this->lms_chip_id = ind;
    return 0;
}

lime::LMS7002M* LMS7_Device::GetLMS(int index)
{
    return lms_list.at( index < 0 ? lms_chip_id : index);
}

int LMS7_Device::UploadWFM(const void **samples, uint8_t chCount, int sample_count, lime::StreamConfig::StreamDataFormat fmt)
{
    return connection->UploadWFM(samples, chCount%2 ? 1 : 2, sample_count, fmt, (chCount-1)/2);
}

int LMS7_Device::MCU_AGCStart(uint8_t rssiMin, uint8_t pgaCeil)
{
    lime::MCU_BD *mcu = lms_list.at(lms_chip_id)->GetMCUControls();
    lms_list.at(lms_chip_id)->Modify_SPI_Reg_bits(0x0006, 0, 0, 0);

    uint8_t mcuID = mcu->ReadMCUProgramID();
    printf("Current MCU firmware: %i, expected %i \n", mcuID, MCU_ID_AGC_IMAGE);
    if(mcuID != MCU_ID_AGC_IMAGE)
    {
        printf("Uploading MCU AGC firmware\n");
        int status = mcu->Program_MCU(mcu_program_lms7_agc_bin, lime::IConnection::MCU_PROG_MODE::SRAM);
        printf("Done\n");
        if(status != 0)
            return status;
    }

    lms_list.at(lms_chip_id)->Modify_SPI_Reg_bits(0x002D, 15, 0, pgaCeil << 8 | rssiMin);
    mcu->RunProcedure(254);
    return 0;
}

int LMS7_Device::MCU_AGCStop()
{
    lime::MCU_BD *mcu = lms_list.at(lms_chip_id)->GetMCUControls();
    mcu->RunProcedure(0);
    lms_list.at(lms_chip_id)->Modify_SPI_Reg_bits(0x0006, 0, 0, 0);
    return 0;
}
