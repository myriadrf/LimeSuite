/**
@file	LMS7002M_filtersCalibration.cpp
@author Lime Microsystems (www.limemicro.com)
@brief	Implementation of LMS7002M transceiver filters calibration algorithms
*/

#include "LMS7002M.h"
#include "IConnection.h"
#include "Logger.h"
#include "CalibrationCache.h"
#include "LMS7002M_RegistersMap.h"
#include <cmath>
#include <iostream>
#include <assert.h>
#include "MCU_BD.h"
#include "mcu_programs.h"
#include "Logger.h"

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
        SPI_write_batch(restoreAddrs.data(), restoreData.data(), restoreData.size(), false);
    }

    //cleanup
    delete backup;
    backup = nullptr;
    this->SetActiveChannel(chBck);
}

int LMS7002M::TuneRxFilter(float_type rx_lpf_freq_RF)
{
    int status;
    if(RxLPF_RF_LimitLow > rx_lpf_freq_RF || rx_lpf_freq_RF > RxLPF_RF_LimitHigh)
        return ReportError(ERANGE, "RxLPF frequency out of range, available range from %g to %g MHz", RxLPF_RF_LimitLow/1e6, RxLPF_RF_LimitHigh/1e6);

    if(mcuControl->ReadMCUProgramID() != MCU_ID_CALIBRATIONS_SINGLE_IMAGE)
    {
        if((status = mcuControl->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, IConnection::MCU_PROG_MODE::SRAM)))
            return ReportError(status, "Tune Rx Filter: failed to program MCU");
    }

    //set reference clock parameter inside MCU
    long refClk = GetReferenceClk_SX(false);
    mcuControl->SetParameter(MCU_BD::MCU_REF_CLK, refClk);
    lime::debug("MCU Ref. clock: %g MHz", refClk / 1e6);
    //set bandwidth for MCU to read from register, value is integer stored in MHz
    mcuControl->SetParameter(MCU_BD::MCU_BW, rx_lpf_freq_RF);
    mcuControl->RunProcedure(5);

    status = mcuControl->WaitForMCU(1000);
    if(status != MCU_BD::MCU_NO_ERROR)
    {
        lime::error("MCU error code(%i): %s", status, MCU_BD::MCUStatusMessage(status));
        return -1;
    }
    //sync registers to cache
    std::vector<uint16_t> regsToSync = {0x0112, 0x0117, 0x011A, 0x0116, 0x0118, 0x0114, 0x0019, 0x0115};
    for(const auto addr : regsToSync)
        this->SPI_read(addr, true);

    return status;
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

    if (!controlPort){
        lime::error("No device connected");
        return -1;
    }

    if(mcuControl->ReadMCUProgramID() != MCU_ID_CALIBRATIONS_SINGLE_IMAGE)
    {
        if((status = mcuControl->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, IConnection::MCU_PROG_MODE::SRAM)))
            return ReportError(status, "Tune Tx Filter: failed to program MCU");
    }

    //set reference clock parameter inside MCU
    long refClk = GetReferenceClk_SX(false);
    mcuControl->SetParameter(MCU_BD::MCU_REF_CLK, refClk);
    lime::debug("MCU Ref. clock: %g MHz", refClk / 1e6);
    //set bandwidth for MCU to read from register, value is integer stored in MHz
    mcuControl->SetParameter(MCU_BD::MCU_BW, tx_lpf_freq_RF);
    mcuControl->RunProcedure(6);

    status = mcuControl->WaitForMCU(1000);
    if(status != MCU_BD::MCU_NO_ERROR)
    {
        lime::error("MCU error code(%i): %s", status, MCU_BD::MCUStatusMessage(status));
        return -1;
    }
    //sync registers to cache
    std::vector<uint16_t> regsToSync = {0x0105, 0x0106, 0x0109, 0x010A, 0x010B};
    for(const auto addr : regsToSync)
        this->SPI_read(addr, true);

    if(tx_lpf_IF <= TxLPF_RF_LimitLowMid/2)
        Log(LOG_INFO, "Filter calibrated. Filter order-4th, filter bandwidth set to %g MHz."
            "Real pole 1st order filter set to 2.5 MHz. Preemphasis filter not active", tx_lpf_IF/1e6 * 2);
    else
        Log(LOG_INFO, "Filter calibrated. Filter order-2nd, set to %g MHz", tx_lpf_IF/1e6 * 2);
    return 0;
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
