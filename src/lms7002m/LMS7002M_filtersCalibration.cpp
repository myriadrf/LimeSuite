/**
@file	LMS7002M_filtersCalibration.cpp
@author Lime Microsystems (www.limemicro.com)
@brief	Implementation of LMS7002M transceiver filters calibration algorithms
*/

#include "LMS7002M.h"
#include "IConnection.h"
#include "ErrorReporting.h"
#include "CalibrationCache.h"
#include "LMS7002M_RegistersMap.h"
#include <cmath>
#include <iostream>
#include <assert.h>
#include "MCU_BD.h"
#include "mcu_programs.h"
static const uint16_t MCU_PARAMETER_ADDRESS = 0x002D; //register used to pass parameter values to MCU

#ifdef _MSC_VER
#include <ciso646>
#endif
using namespace lime;

//rx lpf range limits
static const float_type RxLPF_RF_LimitLow = 1.4e6;
static const float_type RxLPF_RF_LimitHigh = 130e6;

//tx lpf range limits
const float_type TxLPF_RF_LimitLow = 5e6;
const float_type TxLPF_RF_LimitLowMid = 40e6;
const float_type TxLPF_RF_LimitMidHigh = 50e6;
const float_type TxLPF_RF_LimitHigh = 130e6;

///define for parameter enumeration if prefix might be needed

static const int E_DECREASE_R = 0x0080;
static const int E_INCREASE_R = 0x0081;

inline uint16_t pow2(const uint8_t power)
{
    assert(power >= 0 && power < 16);
    return 1 << power;
}

static int clamp(int value, int minBound, int maxBound)
{
    if(value < minBound)
        return minBound;
    if(value > maxBound)
        return maxBound;
    return value;
}

LMS7002M_RegistersMap *LMS7002M::BackupRegisterMap(void)
{
    //BackupAllRegisters(); return NULL;
    auto backup = new LMS7002M_RegistersMap();
    Channel chBck = this->GetActiveChannel();
    this->SetActiveChannel(ChA);
    *backup = *mRegistersMap;
    this->SetActiveChannel(chBck);
    return backup;
}

void LMS7002M::RestoreRegisterMap(LMS7002M_RegistersMap *backup)
{
    //RestoreAllRegisters(); return;
    Channel chBck = this->GetActiveChannel();

    for (int ch = 0; ch < 2; ch++)
    {
        //determine addresses that have been changed
        //and restore backup to the main register map
        std::vector<uint16_t> restoreAddrs, restoreData;
        for (const uint16_t addr : mRegistersMap->GetUsedAddresses(ch))
        {
            uint16_t original = backup->GetValue(ch, addr);
            uint16_t current = mRegistersMap->GetValue(ch, addr);
            mRegistersMap->SetValue(ch, addr, original);

            if (ch == 1 and addr < 0x0100) continue;
            if (original == current) continue;
            restoreAddrs.push_back(addr);
            restoreData.push_back(original);
        }

        //bulk write the original register values from backup
        this->SetActiveChannel((ch==0)?ChA:ChB);
        SPI_write_batch(restoreAddrs.data(), restoreData.data(), restoreData.size());
    }

    //cleanup
    delete backup;
    backup = nullptr;
    this->SetActiveChannel(chBck);
}

uint32_t LMS7002M::GetAvgRSSI(const int avgCount)
{
    float_type sum = 0;
    for(int i=0; i<avgCount; ++i)
        sum += GetRSSI();
    return sum/avgCount;
}

int LMS7002M::TuneRxFilter(float_type rx_lpf_freq_RF)
{
    int status;
    if(RxLPF_RF_LimitLow > rx_lpf_freq_RF || rx_lpf_freq_RF > RxLPF_RF_LimitHigh)
        return ReportError(ERANGE, "RxLPF frequency out of range, available range from %g to %g MHz", RxLPF_RF_LimitLow/1e6, RxLPF_RF_LimitHigh/1e6);

    if(mCalibrationByMCU)
    {
        uint8_t mcuID = mcuControl->ReadMCUProgramID();
        if(mcuID != MCU_ID_CALIBRATIONS_SINGLE_IMAGE)
        {
            status = mcuControl->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, IConnection::MCU_PROG_MODE::SRAM);
            if(status != 0)
                return status;
        }

        //set reference clock parameter inside MCU
        long refClk = GetReferenceClk_SX(false);
        mcuControl->SetParameter(MCU_BD::MCU_REF_CLK, refClk);
        printf("MCU Ref. clock: %g MHz\n", refClk / 1e6);
        //set bandwidth for MCU to read from register, value is integer stored in MHz
        mcuControl->SetParameter(MCU_BD::MCU_BW, rx_lpf_freq_RF);
        mcuControl->RunProcedure(5);
        status = mcuControl->WaitForMCU(1000);
        if(status != 0)
        {
            printf("MCU working too long %i\n", status);
        }
        //sync registers to cache
        for (int a = 0x010c; a <= 0x0114; a++) this->SPI_read(a, true);
        for (int a = 0x0115; a <= 0x011b; a++) this->SPI_read(a, true);
        //logic reset
        Modify_SPI_Reg_bits(LMS7param(LRST_TX_A), 0);
        Modify_SPI_Reg_bits(LMS7param(LRST_TX_B), 0);
        Modify_SPI_Reg_bits(LMS7param(LRST_TX_A), 1);
        Modify_SPI_Reg_bits(LMS7param(LRST_TX_B), 1);

        return status;
    }

    //calculate intermediate frequency
    const float rx_lpf_IF = rx_lpf_freq_RF/2;
    auto registersBackup = BackupRegisterMap();
    int g_tia_rfe = Get_SPI_Reg_bits(LMS7param(G_TIA_RFE));

    status = TuneRxFilterSetup(rx_lpf_IF);
    if(status != 0)
    {
        RestoreRegisterMap(registersBackup);
        return status;
    }

    int g_rxloopb_rfe = Get_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE));
    uint32_t rssi = GetRSSI();
    while(rssi < 0x2700 && g_rxloopb_rfe < 14)
    {
        g_rxloopb_rfe += 2;
        Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), g_rxloopb_rfe);
        rssi = GetRSSI();
    }
    int cg_iamp_tbb = Get_SPI_Reg_bits(LMS7param(CG_IAMP_TBB));
    while(rssi < 0x2700 && cg_iamp_tbb < 30)
    {
        cg_iamp_tbb += 2;
        Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), cg_iamp_tbb);
        rssi = GetRSSI();
    }

    const int rssiAvgCount = 5;
    uint32_t rssi_value_dc = GetAvgRSSI(rssiAvgCount);
    const uint32_t rssi_3dB = rssi_value_dc * 0.7071 * pow(10, (-0.0018 * rx_lpf_IF/1e6)/20);

    if(rx_lpf_IF <= 54e6)
    {
        status = SetFrequencySX(LMS7002M::Rx, 539.9e6-rx_lpf_IF*1.3);
        if(status != 0)
        {
            RestoreRegisterMap(registersBackup);
            return status;
        }
        status = SetNCOFrequency(LMS7002M::Rx, 0, rx_lpf_IF*1.3);
        if(status != 0)
        {
            RestoreRegisterMap(registersBackup);
            return status;
        }

        if(rx_lpf_IF < 18e6)
        {
            //LPFL START
            status = RxFilterSearch(LMS7param(C_CTL_LPFL_RBB), rssi_3dB, rssiAvgCount, 2048);
            if(status == E_DECREASE_R)
            {
                int r_ctl_lpf = Get_SPI_Reg_bits(LMS7param(R_CTL_LPF_RBB));
                while(r_ctl_lpf > 1)
                {
                    r_ctl_lpf /= 2;
                    Modify_SPI_Reg_bits(LMS7param(R_CTL_LPF_RBB), r_ctl_lpf);
                    status = RxFilterSearch(LMS7param(C_CTL_LPFL_RBB), rssi_3dB, rssiAvgCount, 2048);
                }
            }
            if(status == E_INCREASE_R)
            {
                int r_ctl_lpf = Get_SPI_Reg_bits(LMS7param(R_CTL_LPF_RBB));
                while(r_ctl_lpf < 31)
                {
                    r_ctl_lpf += 4;
                    if(r_ctl_lpf > 31)
                        break;
                    Modify_SPI_Reg_bits(LMS7param(R_CTL_LPF_RBB), r_ctl_lpf);
                    status = RxFilterSearch(LMS7param(C_CTL_LPFL_RBB), rssi_3dB, rssiAvgCount, 2048);
                }
            }
            else if(status != 0)
                return status;
            //LPFL END
        }
        if(rx_lpf_IF >= 18e6)
        {
            //LPFH START
            status = RxFilterSearch(LMS7param(C_CTL_LPFH_RBB), rssi_3dB, rssiAvgCount, 256);
            if(status == E_DECREASE_R)
            {
                int r_ctl_lpf = Get_SPI_Reg_bits(LMS7param(R_CTL_LPF_RBB));
                while(r_ctl_lpf > 0)
                {
                    r_ctl_lpf -= 1;
                    Modify_SPI_Reg_bits(LMS7param(R_CTL_LPF_RBB), r_ctl_lpf);
                    rssi = GetRSSI();
                    if(rssi < rssi_3dB)
                    {
                        status = 0;
                        break;
                    }

                }
            }
            if(status == E_INCREASE_R)
            {
                int r_ctl_lpf = Get_SPI_Reg_bits(LMS7param(R_CTL_LPF_RBB));
                while(r_ctl_lpf < 31)
                {
                    r_ctl_lpf += 1;
                    if(r_ctl_lpf > 31)
                        break;
                    Modify_SPI_Reg_bits(LMS7param(R_CTL_LPF_RBB), r_ctl_lpf);
                    if(rssi > rssi_3dB)
                    {
                        status = 0;
                        break;
                    }
                }
            }
            else if(status != 0)
                return status;
            //LPFH END
        }
        status = SetFrequencySX(LMS7002M::Rx, 539.9e6-rx_lpf_IF);
        if(status != 0)
        {
            RestoreRegisterMap(registersBackup);
            return status;
        }
        status = SetNCOFrequency(LMS7002M::Rx, 0, rx_lpf_IF);
        if(status != 0)
        {
            RestoreRegisterMap(registersBackup);
            return status;
        }
        Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), g_tia_rfe);
        int cfb_tia_rfe;
        if(g_tia_rfe == 3 || g_tia_rfe == 2)
            cfb_tia_rfe = int( 1680e6 / (rx_lpf_IF * 0.72) - 10);
        else if(g_tia_rfe == 1)
            cfb_tia_rfe = int( 5400e6 / (rx_lpf_IF * 0.72) - 15);
        else
        {
            RestoreRegisterMap(registersBackup);
            return ReportError(EINVAL, "g_tia_rfe not allowed value");
        }
        cfb_tia_rfe = clamp(cfb_tia_rfe, 0, 4095);
        Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), cfb_tia_rfe);

        int ccomp_tia_rfe;
        if(g_tia_rfe == 3 || g_tia_rfe == 2)
            ccomp_tia_rfe = cfb_tia_rfe / 100;
        else if(g_tia_rfe == 1)
            ccomp_tia_rfe = cfb_tia_rfe / 100 + 1;
        else
        {
            RestoreRegisterMap(registersBackup);
            return ReportError(EINVAL, "g_tia_rfe not allowed value");
        }
        ccomp_tia_rfe = clamp(ccomp_tia_rfe, 0, 15);
        Modify_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE), ccomp_tia_rfe);

        int rcomp_tia_rfe = 15 - cfb_tia_rfe/100;
        rcomp_tia_rfe = clamp(rcomp_tia_rfe, 0, 15);
        Modify_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE), rcomp_tia_rfe);

        //START TIA
        status = RxFilterSearch(LMS7param(CFB_TIA_RFE), rssi_3dB, rssiAvgCount, 4096);
        if(status != 0)
        {
            RestoreRegisterMap(registersBackup);
            return status;
        }
        //END TIA
    }
    if(rx_lpf_IF > 54e6)
    {
        status = SetFrequencySX(LMS7002M::Rx, 539.9e6 - rx_lpf_IF);
        if(status != 0)
        {
            RestoreRegisterMap(registersBackup);
            return status;
        }
        status = SetNCOFrequency(LMS7002M::Rx, 0, rx_lpf_IF);
        if(status != 0)
        {
            RestoreRegisterMap(registersBackup);
            return status;
        }
        //START TIA
        status = RxFilterSearch(LMS7param(CFB_TIA_RFE), rssi_3dB, rssiAvgCount, 4096);
        if(status != 0)
        {
            RestoreRegisterMap(registersBackup);
            return status;
        }
        //END TIA
    }

    //Restore settings
    int cfb_tia_rfe = Get_SPI_Reg_bits(LMS7param(CFB_TIA_RFE));
    int ccomp_tia_rfe = Get_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE));
    int rcomp_tia_rfe = Get_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE));
    int rcc_ctl_lpfl_rbb = Get_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB));
    int c_ctl_lpfl_rbb = Get_SPI_Reg_bits(LMS7param(C_CTL_LPFL_RBB));
    int c_ctl_pga_rbb = Get_SPI_Reg_bits(LMS7param(C_CTL_PGA_RBB));
    int rcc_ctl_pga_rbb = Get_SPI_Reg_bits(LMS7param(RCC_CTL_PGA_RBB));
    int rcc_ctl_lpfh_rbb = Get_SPI_Reg_bits(LMS7param(RCC_CTL_LPFH_RBB));
    int c_ctl_lpfh_rbb = Get_SPI_Reg_bits(LMS7param(C_CTL_LPFH_RBB));
    int pd_lpfl_rbb = Get_SPI_Reg_bits(LMS7param(PD_LPFL_RBB));
    int pd_lpfh_rbb = Get_SPI_Reg_bits(LMS7param(PD_LPFH_RBB));
    int input_ctl_pga_rbb = Get_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB));

    RestoreRegisterMap(registersBackup);
    Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), cfb_tia_rfe);
    Modify_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE), ccomp_tia_rfe);
    Modify_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE), rcomp_tia_rfe);
    Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB), rcc_ctl_lpfl_rbb);
    Modify_SPI_Reg_bits(LMS7param(C_CTL_LPFL_RBB), c_ctl_lpfl_rbb);
    Modify_SPI_Reg_bits(LMS7param(C_CTL_PGA_RBB), c_ctl_pga_rbb);
    Modify_SPI_Reg_bits(LMS7param(RCC_CTL_PGA_RBB), rcc_ctl_pga_rbb);
    Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFH_RBB), rcc_ctl_lpfh_rbb);
    Modify_SPI_Reg_bits(LMS7param(C_CTL_LPFH_RBB), c_ctl_lpfh_rbb);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), pd_lpfl_rbb);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB), pd_lpfh_rbb);
    Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB), input_ctl_pga_rbb);
    Modify_SPI_Reg_bits(LMS7param(ICT_LPF_IN_RBB), 12);
    Modify_SPI_Reg_bits(LMS7param(ICT_LPF_OUT_RBB), 12);
    Modify_SPI_Reg_bits(LMS7param(ICT_PGA_OUT_RBB), 20);
    Modify_SPI_Reg_bits(LMS7param(ICT_PGA_IN_RBB), 20);
    Modify_SPI_Reg_bits(LMS7param(R_CTL_LPF_RBB), 16);
    Modify_SPI_Reg_bits(LMS7param(RFB_TIA_RFE), 16);
    return 0;
}

int LMS7002M::RxFilterSearch(const LMS7Parameter &param, const uint32_t rssi_3dB, uint8_t rssiAvgCnt, const int stepLimit)
{
    int value = Get_SPI_Reg_bits(param);
    const int maxValue = pow2(param.msb-param.lsb+1)-1;
    int stepIncrease = 0;
    int stepSize = 1;
    uint32_t rssi = GetAvgRSSI(rssiAvgCnt);
    if(rssi == rssi_3dB)
        return 0;
    if(rssi < rssi_3dB)
        while(rssi < rssi_3dB)
        {
            stepSize = pow2(stepIncrease);
            value -= stepSize;
            if(value < 0)
                value = 0;
            Modify_SPI_Reg_bits(param, value);
            rssi = GetAvgRSSI(rssiAvgCnt);
            stepIncrease += 1;
            if(stepSize == stepLimit)
                return ReportError(E_INCREASE_R, "%s step size out of range", param.name);
        }
    else if(rssi > rssi_3dB)
        while(rssi > rssi_3dB)
        {
            stepSize = pow2(stepIncrease);
            value += stepSize;
            if(value > maxValue)
                value = maxValue;
            Modify_SPI_Reg_bits(param, value);
            rssi = GetAvgRSSI(rssiAvgCnt);
            stepIncrease += 1;
            if(stepSize == stepLimit)
                return ReportError(E_DECREASE_R, "%s step size out of range", param.name);
        }

    if(stepSize == 1)
        return 0;
    while(stepSize != 1)
    {
        rssi = GetAvgRSSI(rssiAvgCnt);
        stepSize /= 2;
        if(rssi >= rssi_3dB)
            value += stepSize;
        else
            value -= stepSize;
        Modify_SPI_Reg_bits(param, value);
    }
    return 0;
}

int LMS7002M::TxFilterSearch(const LMS7Parameter &param, const uint32_t rssi_3dB, uint8_t rssiAvgCnt, const int stepLimit)
{
    int value = Get_SPI_Reg_bits(param);
    const int maxValue = pow2(param.msb-param.lsb+1)-1;
    int stepIncrease = 0;
    int stepSize = 1;
    uint32_t rssi = GetAvgRSSI(rssiAvgCnt);
    if(rssi > rssi_3dB)
        while(rssi > rssi_3dB)
        {
            stepSize = pow2(stepIncrease);
            value -= stepSize;
            if(value < 0)
                value = 0;
            Modify_SPI_Reg_bits(param, value);
            rssi = GetAvgRSSI(rssiAvgCnt);
            stepIncrease += 1;
            if(stepSize == stepLimit)
                return ReportError(ERANGE, "%s step size out of range", param.name);
        }
    else if(rssi < rssi_3dB)
        while(rssi < rssi_3dB)
        {
            stepSize = pow2(stepIncrease);
            value += stepSize;
            if(value > maxValue)
                value = maxValue;
            Modify_SPI_Reg_bits(param, value);
            rssi = GetAvgRSSI(rssiAvgCnt);
            stepIncrease += 1;
            if(stepSize == stepLimit)
                return ReportError(ERANGE, "%s step size out of range", param.name);
        }

    if(stepSize == 1)
        return 0;
    while(stepSize != 1)
    {
        rssi = GetAvgRSSI(rssiAvgCnt);
        stepSize /= 2;
        if(rssi <= rssi_3dB)
            value += stepSize;
        else
            value -= stepSize;
        Modify_SPI_Reg_bits(param, value);
    }
    return 0;
}

int LMS7002M::TxFilterSearch_S5(const LMS7Parameter &param, const uint32_t rssi_3dB, uint8_t rssiAvgCnt, const int stepLimit)
{
    int value = Get_SPI_Reg_bits(param);
    const int maxValue = pow2(param.msb-param.lsb+1)-1;
    int stepIncrease = 0;
    int stepSize = 1;
    uint32_t rssi = GetAvgRSSI(rssiAvgCnt);
    if(rssi > rssi_3dB)
        while(rssi > rssi_3dB)
        {
            stepSize = pow2(stepIncrease);
            value -= stepSize;
            if(value < 0)
                value = 0;
            Modify_SPI_Reg_bits(param, value);
            rssi = GetAvgRSSI(rssiAvgCnt);
            stepIncrease += 1;
            if(stepSize == stepLimit)
                return ReportError(ERANGE, "%s step size out of range", param.name);
        }
    else if(rssi < rssi_3dB)
        while(rssi < rssi_3dB)
        {
            stepSize = pow2(stepIncrease);
            value += stepSize;
            if(value > maxValue)
                value = maxValue;
            Modify_SPI_Reg_bits(param, value);
            rssi = GetAvgRSSI(rssiAvgCnt);
            stepIncrease += 1;
            if(stepSize == stepLimit)
                return ReportError(ERANGE, "%s step size out of range", param.name);
        }

    if(stepSize == 1)
        return 0;
    while(stepSize != 1)
    {
        rssi = GetAvgRSSI(rssiAvgCnt);
        stepSize /= 2;
        if(rssi <= rssi_3dB)
            value += stepSize;
        else
            value -= stepSize;
        Modify_SPI_Reg_bits(param, value);
    }
    return 0;
}

int LMS7002M::TuneRxFilterSetup(const float_type rx_lpf_IF)
{
    if(RxLPF_RF_LimitLow/2 > rx_lpf_IF || rx_lpf_IF > RxLPF_RF_LimitHigh/2)
        return ReportError(ERANGE, "RxLPF frequency out of range, available range from 0.7 to 65 MHz");
    int status;
    int ch = Get_SPI_Reg_bits(LMS7param(MAC));
    int g_tia_rfe = Get_SPI_Reg_bits(LMS7param(G_TIA_RFE));
    int g_pga_rbb = Get_SPI_Reg_bits(LMS7param(G_PGA_RBB));
    //RFE
    SetDefaults(RFE);
    Modify_SPI_Reg_bits(LMS7param(SEL_PATH_RFE), 2);
    Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), 1);
    Modify_SPI_Reg_bits(LMS7param(PD_RLOOPB_2_RFE), 0);
    Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_LB2_RFE), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_MXLOBUF_RFE), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_QGEN_RFE), 0);
    Modify_SPI_Reg_bits(LMS7param(RFB_TIA_RFE), 16);
    Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE), g_tia_rfe);

    if(rx_lpf_IF <= 54e6)
    {
        Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), 1);
        Modify_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE), 0);
        Modify_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE), 15);
    }
    else
    {
        int cfb_tia_rfe;
        int ccomp_tia_rfe;
        int rcomp_tia_rfe;
        if(g_tia_rfe == 3 || g_tia_rfe == 2)
        {
            cfb_tia_rfe = int( 1680e6/rx_lpf_IF - 10);
            ccomp_tia_rfe = cfb_tia_rfe/100;
        }
        else if(g_tia_rfe == 1)
        {
            cfb_tia_rfe = int( 5400e6/rx_lpf_IF - 15);
            ccomp_tia_rfe = cfb_tia_rfe/100 + 1;
        }
        else
            return ReportError(EINVAL ,"Calibration setup: G_TIA_RFE value not allowed");
        cfb_tia_rfe = clamp(cfb_tia_rfe, 0, 4095);
        ccomp_tia_rfe = clamp(ccomp_tia_rfe, 0, 15);
        rcomp_tia_rfe = 15-cfb_tia_rfe/100;
        rcomp_tia_rfe = clamp(rcomp_tia_rfe, 0, 15);
        Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), cfb_tia_rfe);
        Modify_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE), ccomp_tia_rfe);
        Modify_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE), rcomp_tia_rfe);
    }

    //RBB
    SetDefaults(RBB);
    Modify_SPI_Reg_bits(LMS7param(ICT_PGA_OUT_RBB), 20);
    Modify_SPI_Reg_bits(LMS7param(ICT_PGA_IN_RBB), 20);
    if(g_pga_rbb == 31)
        Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), 22);
    else
        Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), g_pga_rbb);

    int c_ctl_pga_rbb;
    if(g_pga_rbb < 8)
        c_ctl_pga_rbb = 3;
    else if(g_pga_rbb < 13)
        c_ctl_pga_rbb = 2;
    else if(g_pga_rbb < 21)
        c_ctl_pga_rbb = 1;
    else
        c_ctl_pga_rbb = 0;
    Modify_SPI_Reg_bits(LMS7param(C_CTL_PGA_RBB), c_ctl_pga_rbb);

    int rcc_ctl_pga_rbb = (430 * pow(0.65, g_pga_rbb/10) - 110.35)/20.45 + 16;
    rcc_ctl_pga_rbb = clamp(rcc_ctl_pga_rbb, 0, 31);
    Modify_SPI_Reg_bits(LMS7param(RCC_CTL_PGA_RBB), rcc_ctl_pga_rbb);

    if(rx_lpf_IF < 18e6)
    {

        Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), 0);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB), 1);
        Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB), 0);
        int c_ctl_lpfl_rbb = int(2160e6/(rx_lpf_IF*1.3) - 103);
        c_ctl_lpfl_rbb = clamp(c_ctl_lpfl_rbb, 0, 2047);
        Modify_SPI_Reg_bits(LMS7param(C_CTL_LPFL_RBB), c_ctl_lpfl_rbb);
        int rcc_ctl_lpfl_rbb;
        if(rx_lpf_IF*1.3 < 1.4e6)
            rcc_ctl_lpfl_rbb = 0;
        else if(rx_lpf_IF*1.3 < 3e6)
            rcc_ctl_lpfl_rbb = 1;
        else if(rx_lpf_IF*1.3 < 5e6)
            rcc_ctl_lpfl_rbb = 2;
        else if(rx_lpf_IF*1.3 < 10e6)
            rcc_ctl_lpfl_rbb = 3;
        else if(rx_lpf_IF*1.3 < 15e6)
            rcc_ctl_lpfl_rbb = 4;
        else
            rcc_ctl_lpfl_rbb = 5;
        Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB), rcc_ctl_lpfl_rbb);
    }
    else if(rx_lpf_IF <= 54e6)
    {
        Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), 1);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB), 0);
        Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB), 1);
        int c_ctl_lpfh_rbb = int( 6000e6/(rx_lpf_IF*1.3) - 50);
        c_ctl_lpfh_rbb = clamp(c_ctl_lpfh_rbb, 0, 255);
        Modify_SPI_Reg_bits(LMS7param(C_CTL_LPFH_RBB), c_ctl_lpfh_rbb);
        int rcc_ctl_lpfh_rbb = int(rx_lpf_IF*1.3/10) - 3;
        rcc_ctl_lpfh_rbb = clamp(rcc_ctl_lpfh_rbb, 0, 8);
        Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFH_RBB), rcc_ctl_lpfh_rbb);
    }
    else // rx_lpf_IF > 54e6
    {
        Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), 1);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB), 1);
        Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB), 2);
    }

    //TRF
    SetDefaults(TRF);
    Modify_SPI_Reg_bits(LMS7param(L_LOOPB_TXPAD_TRF), 0);
    Modify_SPI_Reg_bits(LMS7param(EN_LOOPB_TXPAD_TRF), 1);
    Modify_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF), 0);
    Modify_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF), 1);

    //TBB
    SetDefaults(TBB);
    Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_FRP_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_GG_FRP_TBB), 6);

    //AFE
    SetDefaults(AFE);
    if(ch == 2)
        Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);

    //BIAS
    int rp_calib_bias = Get_SPI_Reg_bits(LMS7param(RP_CALIB_BIAS));
    SetDefaults(BIAS);
    Modify_SPI_Reg_bits(LMS7param(RP_CALIB_BIAS), rp_calib_bias);

    //LDO
    //do nothing
    //XBUF
    Modify_SPI_Reg_bits(LMS7param(PD_XBUF_RX), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_XBUF_TX), 0);
    Modify_SPI_Reg_bits(LMS7param(EN_G_XBUF), 1);

    //CGEN
    SetDefaults(CGEN);
    int cgenMultiplier = rx_lpf_IF*20 / 46.08e6 + 0.5;
    if(cgenMultiplier > 9 && cgenMultiplier < 12)
        cgenMultiplier = 12;
    cgenMultiplier = clamp(cgenMultiplier, 2, 13);

    status = SetFrequencyCGEN(46.08e6 * cgenMultiplier + 10e6);
    if(status != 0)
        return status;

    //SXR
    Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    SetDefaults(SX);
    status = SetFrequencySX(LMS7002M::Rx, 539.9e6);
    if(status != 0)
        return status;

    //SXT
    Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    SetDefaults(SX);
    status = SetFrequencySX(LMS7002M::Tx, 550e6);
    if(status != 0)
        return status;

    Modify_SPI_Reg_bits(LMS7param(MAC), ch);
    //LimeLight & PAD
    //do nothing

    //TxTSP
    SetDefaults(TxTSP);
    SetDefaults(TxNCO);
    Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(CMIX_SC_TXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_TXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_TXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_TXTSP), 1);
    LoadDC_REG_IQ(LMS7002M::Tx, 0x7fff, 0x8000);
    SetNCOFrequency(LMS7002M::Tx, 0, 10e6);

    //RxTSP
    SetDefaults(RxTSP);
    SetDefaults(RxNCO);
    Modify_SPI_Reg_bits(LMS7param(AGC_MODE_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 0);
    Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(AGC_AVG_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), 4);
    Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_RXTSP), 0);
    if (Get_SPI_Reg_bits(LMS7_MASK, true) != 0)
        Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP), 1);
    SetNCOFrequency(LMS7002M::Rx, 0, 0);

    if(ch == 2)
    {
        Modify_SPI_Reg_bits(LMS7param(MAC), 1);
        Modify_SPI_Reg_bits(LMS7param(EN_NEXTRX_RFE), 1);
        Modify_SPI_Reg_bits(LMS7param(EN_NEXTTX_TRF), 1);
        Modify_SPI_Reg_bits(LMS7param(MAC), ch);
    }

    return 0;
}

int LMS7002M::TuneTxFilterSetup(const float_type tx_lpf_IF)
{
    int status;
    int ch = Get_SPI_Reg_bits(LMS7param(MAC));

    if(Get_SPI_Reg_bits(LMS7param(MASK), true) == 0)
        return ReportError(ENOTSUP, "Chip mask revision 0 not supported, contact support");

    //RFE
    Modify_SPI_Reg_bits(LMS7param(EN_G_RFE), 0);
    Modify_SPI_Reg_bits(0x010D, 4, 1, 0xF);

    //RBB
    SetDefaults(RBB);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), 1);
    Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB), 3);
    Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), 12);
    Modify_SPI_Reg_bits(LMS7param(RCC_CTL_PGA_RBB), 23);

    //TRF
    Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), 0);

    //TBB
    SetDefaults(TBB);
    Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(LOOPB_TBB), 3);

    if(tx_lpf_IF <= TxLPF_RF_LimitLowMid/2)
    {
        Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB), 1);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB), 0);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB), 0);
        Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), 16);
        Modify_SPI_Reg_bits(LMS7param(R5_LPF_BYP_TBB), 1);

        const float_type freq = (16.0/20.0)*tx_lpf_IF/1e6;
        const float_type p1= 1.29858903647958e-16;
        const float_type p2= -0.000110746929967704;
        const float_type p3= 0.00277593485991029;
        const float_type p4= 21.0384293169607;
        const float_type p5= -48.4092606238297;
        int rcal_lpflad_tbb = pow(freq, 4)*p1 + pow(freq, 3)*p2 + pow(freq,2)*p3 + freq*p4 + p5;
        rcal_lpflad_tbb = clamp(rcal_lpflad_tbb, 0, 255);
        Modify_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB), rcal_lpflad_tbb);
    }
    else
    {
        Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB), 0);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB), 1);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB), 1);
        Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), 16);

        const float_type freq = tx_lpf_IF/1e6;
        const float_type p1= 1.10383261611112e-06;
        const float_type p2= -0.000210800032517545;
        const float_type p3= 0.0190494874803309;
        const float_type p4= 1.43317445923528;
        const float_type p5= -47.6950779298333;
        int rcal_lpfh_tbb = pow(freq, 4)*p1 + pow(freq, 3)*p2 + pow(freq,2)*p3 + freq*p4 + p5;
        rcal_lpfh_tbb = clamp(rcal_lpfh_tbb, 0, 255);
        Modify_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB), rcal_lpfh_tbb);
    }

    //AFE
    const int isel_dac_afe = Get_SPI_Reg_bits(LMS7param(ISEL_DAC_AFE));
    SetDefaults(AFE);
    Modify_SPI_Reg_bits(LMS7param(ISEL_DAC_AFE), isel_dac_afe);
    if(ch == 2)
    {
        Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);
        Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0);
    }

    //BIAS
    const int rp_calib_bias = Get_SPI_Reg_bits(LMS7param(RP_CALIB_BIAS));
    SetDefaults(BIAS);
    Modify_SPI_Reg_bits(LMS7param(RP_CALIB_BIAS), rp_calib_bias);

    //LDO
    //do nothing

    //XBUF
    Modify_SPI_Reg_bits(LMS7param(PD_XBUF_RX), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_XBUF_TX), 0);
    Modify_SPI_Reg_bits(LMS7param(EN_G_XBUF), 1);

    //CGEN
    SetDefaults(CGEN);
    int cgenMultiplier = tx_lpf_IF*20/46.08e6 + 0.5;
    cgenMultiplier = clamp(cgenMultiplier, 2, 13);

    status = SetFrequencyCGEN(46.08e6 * cgenMultiplier + 10e6);
    if(status != 0)
        return status;

    //SXR
    Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    Modify_SPI_Reg_bits(LMS7param(PD_VCO), 1);

    //SXT
    Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    Modify_SPI_Reg_bits(LMS7param(PD_VCO), 1);

    Modify_SPI_Reg_bits(LMS7param(MAC), ch);

    //TxTSP
    SetDefaults(TxTSP);
    SetDefaults(TxNCO);
    Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), 1);
    LoadDC_REG_IQ(LMS7002M::Tx, (int16_t)0x7FFF, (int16_t)0x8000);
    const float_type txNCO_freqs[] = {1e6, tx_lpf_IF};
    for(int i=0; i<2; ++i)
        SetNCOFrequency(LMS7002M::Tx, i, txNCO_freqs[i]);

    //RxTSP
    SetDefaults(RxTSP);
    SetDefaults(RxNCO);
    Modify_SPI_Reg_bits(LMS7param(AGC_MODE_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(AGC_AVG_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), 4);
    if (Get_SPI_Reg_bits(LMS7_MASK, true) != 0)
        Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP), 1);
    const float_type rxNCO_freqs[] = {0.9e6, tx_lpf_IF-0.1e6};
    for(int i=0; i<2; ++i)
        SetNCOFrequency(LMS7002M::Rx, i, rxNCO_freqs[i]);
    return 0;
}

int LMS7002M::TuneTxFilter(const float_type tx_lpf_freq_RF)
{
    int status;

    if(tx_lpf_freq_RF < TxLPF_RF_LimitLow || tx_lpf_freq_RF > TxLPF_RF_LimitHigh)
        return ReportError(ERANGE, "Tx lpf(%g MHz) out of range %g-%g MHz and %g-%g MHz", tx_lpf_freq_RF/1e6,
                        TxLPF_RF_LimitLow/1e6, TxLPF_RF_LimitLowMid/1e6,
                        TxLPF_RF_LimitMidHigh/1e6, TxLPF_RF_LimitHigh/1e6);
    //calculate intermediate frequency
    float_type tx_lpf_IF = tx_lpf_freq_RF/2;
    if(tx_lpf_freq_RF > TxLPF_RF_LimitLowMid && tx_lpf_freq_RF < TxLPF_RF_LimitMidHigh)
    {
        Log(LOG_WARNING, "Tx lpf(%g MHz) out of range %g-%g MHz and %g-%g MHz. Setting to %g MHz", tx_lpf_freq_RF/1e6,
                        TxLPF_RF_LimitLow/1e6, TxLPF_RF_LimitLowMid/1e6,
                        TxLPF_RF_LimitMidHigh/1e6, TxLPF_RF_LimitHigh/1e6,
                        TxLPF_RF_LimitMidHigh/1e6);
        tx_lpf_IF = TxLPF_RF_LimitMidHigh/2;
    }

    if(mCalibrationByMCU)
    {
        uint8_t mcuID = mcuControl->ReadMCUProgramID();
        if(mcuID != MCU_ID_CALIBRATIONS_SINGLE_IMAGE)
        {
            status = mcuControl->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, IConnection::MCU_PROG_MODE::SRAM);
            if(status != 0)
                return status;
        }

        //set reference clock parameter inside MCU
        long refClk = GetReferenceClk_SX(false);
        mcuControl->SetParameter(MCU_BD::MCU_REF_CLK, refClk);
        printf("MCU Ref. clock: %g MHz\n", refClk / 1e6);
        //set bandwidth for MCU to read from register, value is integer stored in MHz
        mcuControl->SetParameter(MCU_BD::MCU_BW, tx_lpf_freq_RF);
        mcuControl->RunProcedure(6);
        status = mcuControl->WaitForMCU(1000);
        if(status != 0)
        {
            printf("MCU working too long %i\n", status);
        }
        //sync registers to cache
        for (int a = 0x0105; a <= 0x010b; a++) this->SPI_read(a, true);
        return status;
    }

    auto registersBackup = BackupRegisterMap();
    status = TuneTxFilterSetup(tx_lpf_IF);
    if(status != 0)
    {
        RestoreRegisterMap(registersBackup);
        return status;
    }

    Modify_SPI_Reg_bits(LMS7param(SEL_RX), 0);
    Modify_SPI_Reg_bits(LMS7param(SEL_TX), 0);
    uint32_t rssi = GetRSSI();
    int cg_iamp_tbb = Get_SPI_Reg_bits(LMS7param(CG_IAMP_TBB));
    while(rssi < 0x2700 && cg_iamp_tbb < 43)
    {
        cg_iamp_tbb += 1;
        Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), cg_iamp_tbb);
        rssi = GetRSSI();
    }

    const int rssiAvgCount = 5;
    if(tx_lpf_IF <= TxLPF_RF_LimitLowMid/2)
    {
        bool targetLevelNotReached = false;
        int iterationsLeft = 5;
        do
        {
            Modify_SPI_Reg_bits(LMS7param(SEL_TX), 0);
            Modify_SPI_Reg_bits(LMS7param(SEL_RX), 0);
            uint32_t rssi_dc_lad = GetAvgRSSI(rssiAvgCount);
            uint32_t rssi_3dB_lad = rssi_dc_lad * 0.7071;
            Modify_SPI_Reg_bits(LMS7param(SEL_TX), 1);
            Modify_SPI_Reg_bits(LMS7param(SEL_RX), 1);
            rssi = GetAvgRSSI(rssiAvgCount);

            int ccal_lpflad_tbb = Get_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB));
            if(rssi < rssi_3dB_lad)
            {
                while(rssi < rssi_3dB_lad && ccal_lpflad_tbb > 0)
                {
                    ccal_lpflad_tbb -= 1;
                    Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ccal_lpflad_tbb);
                    rssi = GetAvgRSSI(rssiAvgCount);
                }
                if(rssi < rssi_3dB_lad && ccal_lpflad_tbb == 0)
                {
                    targetLevelNotReached = true;
                    Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), 16);
                    int R = Get_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB))+25;
                    R = clamp(R, 0, 255);
                    Modify_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB), R);
                }
                else
                    targetLevelNotReached = false;
            }
            else if(rssi > rssi_3dB_lad)
            {
                while(rssi > rssi_3dB_lad && ccal_lpflad_tbb < 31)
                {
                    ccal_lpflad_tbb += 1;
                    Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ccal_lpflad_tbb);
                    rssi = GetAvgRSSI(rssiAvgCount);
                }
                if(rssi > rssi_3dB_lad && ccal_lpflad_tbb == 31)
                {
                    targetLevelNotReached = true;
                    Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), 16);
                    int R = Get_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB))-10;
                    R = clamp(R, 0, 255);
                    Modify_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB), R);
                }
                else
                    targetLevelNotReached = false;
                ccal_lpflad_tbb = clamp(++ccal_lpflad_tbb, 0, 31);
                Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ccal_lpflad_tbb);
            }
            --iterationsLeft;
            int R = Get_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB));
            if (R==0 || R==255)
                break;
        } while(targetLevelNotReached && iterationsLeft>=0);
    }
    else // LPFH
    {
        Modify_SPI_Reg_bits(LMS7param(C_CTL_PGA_RBB), 2);

        bool targetLevelNotReached = false;
        int iterationsLeft = 5;
        do
        {
            Modify_SPI_Reg_bits(LMS7param(SEL_TX), 0);
            Modify_SPI_Reg_bits(LMS7param(SEL_RX), 0);
            uint32_t rssi_dc_h = GetAvgRSSI(rssiAvgCount);
            uint32_t rssi_3dB_h = rssi_dc_h * 0.7071;
            Modify_SPI_Reg_bits(LMS7param(SEL_TX), 1);
            Modify_SPI_Reg_bits(LMS7param(SEL_RX), 1);
            rssi = GetAvgRSSI(rssiAvgCount);

            int ccal_lpflad_tbb = Get_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB));
            if(rssi < rssi_3dB_h)
            {
                while(rssi < rssi_3dB_h && ccal_lpflad_tbb > 0)
                {
                    ccal_lpflad_tbb -= 1;
                    Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ccal_lpflad_tbb);
                    rssi = GetAvgRSSI(rssiAvgCount);
                }
                if(rssi < rssi_3dB_h && ccal_lpflad_tbb == 0)
                {
                    targetLevelNotReached = true;
                    Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), 16);
                    int R = Get_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB))+25;
                    R = clamp(R, 0, 255);
                    Modify_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB), R);
                }
                else
                    targetLevelNotReached = false;
            }
            else if(rssi > rssi_3dB_h)
            {
                while(rssi > rssi_3dB_h && ccal_lpflad_tbb < 31)
                {
                    ccal_lpflad_tbb += 1;
                    Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ccal_lpflad_tbb);
                    rssi = GetAvgRSSI(rssiAvgCount);
                }
                if(rssi > rssi_3dB_h && ccal_lpflad_tbb == 31)
                {
                    targetLevelNotReached = true;
                    Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), 16);
                    int R = Get_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB))-10;
                    R = clamp(R, 0, 255);
                    Modify_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB), R);
                }
                else
                    targetLevelNotReached = false;
                ccal_lpflad_tbb = clamp(++ccal_lpflad_tbb, 0, 31);
                Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ccal_lpflad_tbb);
            }
            --iterationsLeft;
            int R = Get_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB));
            if (R==0 || R==255)
                break;
        }
        while(targetLevelNotReached && iterationsLeft>=0);
    }

    //ENDING
    int rcal_lpflad_tbb = Get_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB));
    int ccal_lpflad_tbb = Get_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB));
    int rcal_lpfh_tbb = Get_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB));
    RestoreRegisterMap(registersBackup);
    SPI_write(0x0106, 0x318C);
    SPI_write(0x0107, 0x318C);

    if(tx_lpf_IF <= TxLPF_RF_LimitLowMid/2)
    {
        Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB), 1);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB), 0);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB), 0);
        Modify_SPI_Reg_bits(LMS7param(R5_LPF_BYP_TBB), 1);
        Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ccal_lpflad_tbb);
        Modify_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB), rcal_lpflad_tbb);
        Log(LOG_INFO, "Filter calibrated. Filter order-4th, filter bandwidth set to %g MHz."
            "Real pole 1st order filter set to 2.5 MHz. Preemphasis filter not active", tx_lpf_IF/1e6 * 2);
        return 0;
    }
    else
    {
        Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB), 0);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB), 1);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB), 1);
        Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ccal_lpflad_tbb);
        Modify_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB), rcal_lpfh_tbb);
        Log(LOG_INFO, "Filter calibrated. Filter order-2nd, set to %g MHz", tx_lpf_IF/1e6 * 2);
        return 0;
    }
}

//arbitrary IDs for filter table lookup
static int RFE_TIA_FILTNUM = 100;
static int RBB_LPFL_FILTNUM = 200;
static int RBB_LPFH_FILTNUM = 300;
static int TBB_LPFLAD_FILTNUM = 400;
static int TBB_LPFS5_FILTNUM = 500;
static int TBB_LPFH_FILTNUM = 600;

int LMS7002M::TuneTxFilterWithCaching(const float_type bandwidth)
{
    if (!useCache)
        return TuneTxFilter(bandwidth);
    
    int ret = 0;
    bool found = true;
    const int idx = this->GetActiveChannelIndex();
    const uint32_t boardId = controlPort->GetDeviceInfo().boardSerialNumber;

    //read filter cache
    int rcal_lpflad_tbb(0), ccal_lpflad_tbb(0);
    int rcal_lpfs5_tbb(0);
    int rcal_lpfh_tbb(0);
    const bool lpfladFound = (mValueCache->GetFilter_RC(boardId, bandwidth, idx, Tx, TBB_LPFLAD_FILTNUM, &rcal_lpflad_tbb, &ccal_lpflad_tbb) == 0);
    const bool lpfs5Found = (mValueCache->GetFilter_RC(boardId, bandwidth, idx, Tx, TBB_LPFS5_FILTNUM, &rcal_lpfs5_tbb, &ccal_lpflad_tbb) == 0);
    const bool lpfhFound = (mValueCache->GetFilter_RC(boardId, bandwidth, idx, Tx, TBB_LPFH_FILTNUM, &rcal_lpfh_tbb, &ccal_lpflad_tbb) == 0);

    //apply the calibration
    SPI_write(0x0106, 0x318C);
    SPI_write(0x0107, 0x318C);
    if (bandwidth <= TxLPF_RF_LimitLowMid)
    {
        Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB), 1);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB), 0);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB), 0);
        Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ccal_lpflad_tbb);
        Modify_SPI_Reg_bits(LMS7param(RCAL_LPFS5_TBB), rcal_lpfs5_tbb);
        Modify_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB), rcal_lpflad_tbb);
        if (not lpfladFound) found = false;
        if (not lpfs5Found) found = false;
    }
    else
    {
        Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB), 0);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB), 1);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB), 1);
        Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ccal_lpflad_tbb);
        Modify_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB), rcal_lpfh_tbb);
        if (not lpfhFound) found = false;
    }

    if (found)
    {
        //all results found in cache, done applying!
        Log(LOG_INFO, "Tx Filter calibrated from cache");
        return ret;
    }

    //perform calibration and store results
    ret = this->TuneTxFilter(bandwidth);
    if (ret != 0) return ret;

    if (Get_SPI_Reg_bits(LMS7param(PD_LPFH_TBB)) == 0) ret = mValueCache->InsertFilter_RC(
        boardId, bandwidth, idx, Tx, TBB_LPFH_FILTNUM,
        Get_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB)), Get_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB)));

    if (Get_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB)) == 0) ret = mValueCache->InsertFilter_RC(
        boardId, bandwidth, idx, Tx, TBB_LPFLAD_FILTNUM,
        Get_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB)), Get_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB)));

    if (Get_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB)) == 0) ret = mValueCache->InsertFilter_RC(

        boardId, bandwidth, idx, Tx, TBB_LPFS5_FILTNUM,
        Get_SPI_Reg_bits(LMS7param(RCAL_LPFS5_TBB)), Get_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB)));

    return ret;
}

int LMS7002M::TuneRxFilterWithCaching(const float_type bandwidth)
{
    if(!useCache)
        return TuneRxFilter(bandwidth);

    int ret = 0;
    bool found = true;
    const int idx = this->GetActiveChannelIndex();
    const uint32_t boardId = controlPort->GetDeviceInfo().boardSerialNumber;

    //read filter cache
    int rcomp_tia_rfe(0), ccomp_tia_rfe(0), cfb_tia_rfe(0);
    int rcc_ctl_lpfl_rbb(0), c_ctl_lpfl_rbb(0);
    int rcc_ctl_lpfh_rbb(0), c_ctl_lpfh_rbb(0);
    const bool tiaFound = (mValueCache->GetFilter_RC(boardId, bandwidth, idx, Rx, RFE_TIA_FILTNUM, &rcomp_tia_rfe, &ccomp_tia_rfe, &cfb_tia_rfe) == 0);
    const bool lphlFound = (mValueCache->GetFilter_RC(boardId, bandwidth, idx, Rx, RBB_LPFL_FILTNUM, &rcc_ctl_lpfl_rbb, &c_ctl_lpfl_rbb) == 0);
    const bool lphhFound = (mValueCache->GetFilter_RC(boardId, bandwidth, idx, Rx, RBB_LPFH_FILTNUM, &rcc_ctl_lpfh_rbb, &c_ctl_lpfh_rbb) == 0);

    //apply the calibration
    if (not tiaFound) found = false;

    Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), cfb_tia_rfe);
    Modify_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE), ccomp_tia_rfe);
    Modify_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE), rcomp_tia_rfe);
    Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB), rcc_ctl_lpfl_rbb);
    Modify_SPI_Reg_bits(LMS7param(C_CTL_LPFL_RBB), c_ctl_lpfl_rbb);
    Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFH_RBB), rcc_ctl_lpfh_rbb);
    Modify_SPI_Reg_bits(LMS7param(C_CTL_LPFH_RBB), c_ctl_lpfh_rbb);
    if (bandwidth < 36e6)
    {
        Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), 0);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB), 1);
        Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB), 0);
        if (not lphlFound) found = false;
    }
    else if (bandwidth <= 108e6)
    {
        Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), 1);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB), 0);
        Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB), 1);
        if (not lphhFound) found = false;
    }
    else // bandwidth > 108e6
    {
        Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), 1);
        Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB), 1);
        Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB), 2);
    }
    Modify_SPI_Reg_bits(LMS7param(ICT_LPF_IN_RBB), 12);
    Modify_SPI_Reg_bits(LMS7param(ICT_LPF_OUT_RBB), 12);
    Modify_SPI_Reg_bits(LMS7param(ICT_PGA_OUT_RBB), 20);
    Modify_SPI_Reg_bits(LMS7param(ICT_PGA_IN_RBB), 20);
    Modify_SPI_Reg_bits(LMS7param(R_CTL_LPF_RBB), 16);
    Modify_SPI_Reg_bits(LMS7param(RFB_TIA_RFE), 16);

    if (found)
    {
        //all results found in cache, done applying!
        Log(LOG_INFO, "Rx Filter calibrated from cache");
        return ret;
    }

    //perform calibration and store results
    ret = this->TuneRxFilter(bandwidth);
    if (ret != 0) return ret;

    ret = mValueCache->InsertFilter_RC(boardId, bandwidth, idx, Rx, RFE_TIA_FILTNUM,
        Get_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE)), Get_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE)), Get_SPI_Reg_bits(LMS7param(CFB_TIA_RFE)));

    switch(Get_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB)))
    {
    case 0: ret = mValueCache->InsertFilter_RC(boardId, bandwidth, idx, Rx, RBB_LPFL_FILTNUM,
        Get_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB)), Get_SPI_Reg_bits(LMS7param(C_CTL_LPFL_RBB))); break;
    case 1: ret = mValueCache->InsertFilter_RC(boardId, bandwidth, idx, Rx, RBB_LPFH_FILTNUM,
        Get_SPI_Reg_bits(LMS7param(RCC_CTL_LPFH_RBB)), Get_SPI_Reg_bits(LMS7param(C_CTL_LPFH_RBB))); break;
    default: break;
    }

    return ret;
}
