/**
@file	LMS7002M_filtersCalibration.cpp
@author Lime Microsystems (www.limemicro.com)
@brief	Implementation of LMS7002M transceiver filters calibration algorithms
*/

#include "LMS7002M.h"
#include <cmath>

///define for parameter enumeration if prefix might be needed
#define LMS7param(id) id

const float_type LMS7002M::gLadder_lower_limit = 2;
const float_type LMS7002M::gLadder_higher_limit = 16;
const float_type LMS7002M::gRealpole_lower_limit = 0.8;
const float_type LMS7002M::gRealpole_higher_limit = 3.2;
const float_type LMS7002M::gHighband_lower_limit = 28;
const float_type LMS7002M::gHighband_higher_limit = 70;

const float_type LMS7002M::gRxTIA_higher_limit = 60;
const float_type LMS7002M::gRxTIA_lower_limit_g1 = 1.5;
const float_type LMS7002M::gRxTIA_lower_limit_g23 = 0.5;
const float_type LMS7002M::gRxLPF_low_lower_limit = 1;
const float_type LMS7002M::gRxLPF_low_higher_limit = 20;
const float_type LMS7002M::gRxLPF_high_lower_limit = 20;
const float_type LMS7002M::gRxLPF_high_higher_limit = 70;

liblms7_status LMS7002M::TuneTxFilterSetup(LMS7002M::TxFilter type, float_type cutoff_MHz)
{
    Modify_SPI_Reg_bits(LMS7param(EN_G_RFE), 0);
    Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), 0);

    //RBB
    SetDefaults(RBB);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), 1);
    Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB), 3);
    Modify_SPI_Reg_bits(LMS7param(ICT_PGA_OUT_RBB), 20);
    Modify_SPI_Reg_bits(LMS7param(ICT_PGA_IN_RBB), 20);
    Modify_SPI_Reg_bits(LMS7param(C_CTL_PGA_RBB), 3);

    //TBB
    SetDefaults(TBB);
    Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_FRP_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_GG_FRP_TBB), 6);

    //AFE
    uint8_t isel_dac_afe = (uint8_t)Get_SPI_Reg_bits(0x0082, 15, 13);
    SetDefaults(AFE);
    if (Get_SPI_Reg_bits(LMS7param(MAC)) == 2)
    {
        Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0); //PD_RX_AFE2 0
        Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0); //PD_RX_AFE2 0
    }
    Modify_SPI_Reg_bits(0x0082, 15, 13, isel_dac_afe);

    //BIAS
    uint8_t rpcalib_bias = (uint8_t)Get_SPI_Reg_bits(LMS7param(RP_CALIB_BIAS));
    SetDefaults(BIAS);
    Modify_SPI_Reg_bits(LMS7param(RP_CALIB_BIAS), rpcalib_bias);

    //XBUF
    Modify_SPI_Reg_bits(LMS7param(PD_XBUF_RX), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_XBUF_TX), 0);
    Modify_SPI_Reg_bits(LMS7param(EN_G_XBUF), 1);

    //CGEN
    SetDefaults(CGEN);

    //txtsp
    SetDefaults(TxTSP);
    Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), 1);
    Modify_SPI_Reg_bits(0x0208, 6, 4, 7);
    LoadDC_REG_IQ(Tx, (int16_t)0x7FFF, (int16_t)0x8000);
    float_type txNCOfreq = 0.05;
    SetNCOFrequency(Tx, 0, txNCOfreq);

    //rxtsp
    SetDefaults(RxTSP);    
    SetNCOFrequency(Rx, 0, txNCOfreq - 1);

    Modify_SPI_Reg_bits(LMS7param(AGC_MODE_RXTSP), 1);
    Modify_SPI_Reg_bits(0x040C, 6, 3, 0x07);

    Modify_SPI_Reg_bits(LMS7param(AGC_AVG_RXTSP), 7);
    Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_RXTSP), 1);

    return LIBLMS7_SUCCESS;
}


liblms7_status LMS7002M::TuneTxFilter(LMS7002M::TxFilter type, float_type cutoff_MHz)
{   
    liblms7_status status;
    float_type lowLimit = 0;
    float_type highLimit = 1000;
    uint32_t rssi = 0;
    int8_t dir;
    uint8_t ccal_lpflad_tbb;
    uint32_t rssi_value_100k;
    int16_t rcal;

    float_type ncoFreq = 0.05;
    float_type cgenFreq;
    uint8_t loopb_tbb;
    uint8_t cg_iamp_tbb = 1;
    uint8_t bypladder_tbb;
    uint8_t pd_lpfh_tbb;
    uint8_t pd_lpflad_tbb;
    uint8_t pd_lpfs5;
    uint8_t en_g_tbb = 1;
    uint8_t pd_iamp_tbb = 0;
    uint8_t tstin_tbb = 0;

    BackupAllRegisters();    
    //float_type userCLKGENfreq = GetFrequencyCGEN_MHz();

    status = TuneTxFilterSetup(type, cutoff_MHz);
    if (status != LIBLMS7_SUCCESS)
        goto TxFilterTuneEnd;    
    cgenFreq = cutoff_MHz * 20;
    if (cgenFreq < 60)
        cgenFreq = 60;
    if (cgenFreq > 640)
        cgenFreq = 640;
    if (type == TX_LADDER)
    {
        loopb_tbb = 2;
        bypladder_tbb = 0;
        pd_lpfh_tbb = 1;
        pd_lpflad_tbb = 0;
        pd_lpfs5 = 1;
        lowLimit = gLadder_lower_limit;
        highLimit = gLadder_higher_limit;
    }
    else if (type == TX_REALPOLE)
    {
        loopb_tbb = 3;
        bypladder_tbb = 1;
        pd_lpfh_tbb = 1;
        pd_lpflad_tbb = 1;
        pd_lpfs5 = 0;
        lowLimit = gRealpole_lower_limit;
        highLimit = gRealpole_higher_limit;
    }
    else if (type == TX_HIGHBAND)
    {
        loopb_tbb = 3;
        bypladder_tbb = 0;
        pd_lpfh_tbb = 0;
        pd_lpflad_tbb = 1;
        pd_lpfs5 = 1;
        lowLimit = gHighband_lower_limit;
        highLimit = gHighband_higher_limit;
    }
    if (cutoff_MHz == cgenFreq / 16)
        cgenFreq -= 10;

    if (cutoff_MHz < lowLimit || cutoff_MHz > highLimit)
    {
        status = LIBLMS7_FREQUENCY_OUT_OF_RANGE;
        goto TxFilterTuneEnd;
    }

    status = SetFrequencyCGEN(cgenFreq);
    if (status != LIBLMS7_SUCCESS)
        goto TxFilterTuneEnd;

    Modify_SPI_Reg_bits(LMS7param(LOOPB_TBB), loopb_tbb);
    Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), cg_iamp_tbb);
    Modify_SPI_Reg_bits(LMS7param(BYPLADDER_TBB), bypladder_tbb);
    Modify_SPI_Reg_bits(LMS7param(TSTIN_TBB), tstin_tbb);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB), pd_lpfh_tbb);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFIAMP_TBB), pd_iamp_tbb);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB), pd_lpflad_tbb);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB), pd_lpfs5);
    Modify_SPI_Reg_bits(LMS7param(EN_G_TBB), en_g_tbb);

    //B
    //LADDER coefficients
    float_type p1, p2, p3, p4, p5;
    switch (type)
    {
    case TX_LADDER:
        p1 = 1.29858903647958E-16;
        p2 = -0.000110746929967704;
        p3 = 0.00277593485991029;
        p4 = 21.0384293169607;
        p5 = -48.4092606238297;
        break;
    case TX_REALPOLE:
        p1 = 1.93821841029921E-15;
        p2 = -0.0429694461214244;
        p3 = 0.253501254059498;
        p4 = 88.9545445989649;
        p5 = -48.0847491316861;
        break;
    case TX_HIGHBAND:
        p1 = 1.10383E-06;
        p2 = -0.0002108;
        p3 = 0.019049487;
        p4 = 1.433174459;
        p5 = -47.69507793;
        break;
    }
    rcal = (int16_t)(pow(cutoff_MHz, 4)*p1 + pow(cutoff_MHz, 3)*p2 + pow(cutoff_MHz, 2)*p3 + cutoff_MHz * p4 + p5);
    if (rcal < 0)
        rcal = 0;
    if (rcal > 255)
        rcal = 255;

    if (type == TX_REALPOLE)
        Modify_SPI_Reg_bits(LMS7param(RCAL_LPFS5_TBB), rcal);
    else if (type == TX_LADDER)
        Modify_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB), rcal);
    else if (type == TX_HIGHBAND)
        Modify_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB), rcal);

    FilterTuning_AdjustGains();

    rssi_value_100k = (uint32_t)( GetRSSI()*0.707 );

    SetNCOFrequency(Tx, 0, cutoff_MHz);
    SetNCOFrequency(Rx, 0, cutoff_MHz - 1);

    for (ccal_lpflad_tbb = 31; ccal_lpflad_tbb > 0; --ccal_lpflad_tbb)
    {
        Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ccal_lpflad_tbb);
        rssi = GetRSSI();
        if (rssi > rssi_value_100k && ccal_lpflad_tbb == 31)
            break; //skip this search, continue to advanced search
        if (rssi > rssi_value_100k)
        {
            status = LIBLMS7_SUCCESS;
            goto TxFilterTuneEnd; //found correct value
        }
    }


    //advanced search for c and r values
    status = LIBLMS7_FAILURE;
    dir = ccal_lpflad_tbb == 31 ? -1 : 1;
    while (rcal > 0 && rcal < 255)
    {
        rcal += 5 * dir;
        if (rcal < 0 && rcal > 255)
            break;
        if (type == TX_REALPOLE)
            Modify_SPI_Reg_bits(LMS7param(RCAL_LPFS5_TBB), rcal);
        else if (type == TX_LADDER)
            Modify_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB), rcal);
        else if (type == TX_HIGHBAND)
            Modify_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB), rcal);
        SetNCOFrequency(Tx, 0, ncoFreq);
        SetNCOFrequency(Rx, 0, ncoFreq - 1);
        Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), 16);
        rssi_value_100k = GetRSSI();
        SetNCOFrequency(Tx, 0, cutoff_MHz);
        SetNCOFrequency(Rx, 0, cutoff_MHz - 1);
        for (ccal_lpflad_tbb = 31; ccal_lpflad_tbb > 0; --ccal_lpflad_tbb)
        {
            Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ccal_lpflad_tbb);
            rssi = GetRSSI();
            if (rssi > rssi_value_100k*0.707 && ccal_lpflad_tbb == 31)
                break; //skip c search, need to change r value
            if (rssi > rssi_value_100k*0.707)
            {
                status = LIBLMS7_SUCCESS;
                goto TxFilterTuneEnd;
            }
        }
    }

    //end
TxFilterTuneEnd:
    RestoreAllRegisters();
    if (status != LIBLMS7_SUCCESS)
        return status;    
    Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ccal_lpflad_tbb);
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_FRP_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_GG_FRP_TBB), 6);

    if (type == TX_REALPOLE)
        Modify_SPI_Reg_bits(LMS7param(RCAL_LPFS5_TBB), rcal);
    else if (type == TX_LADDER)
        Modify_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB), rcal);
    else if (type == TX_HIGHBAND)
    {
        Modify_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB), rcal);
        Modify_SPI_Reg_bits(0x0105, 4, 0, 0x7); //set powerdowns
    }
    return LIBLMS7_SUCCESS;
}

void LMS7002M::FilterTuning_AdjustGains()
{
    uint8_t cg_iamp_tbb;
    uint32_t rssi = 0;
    const uint32_t rssi_required = 0x8400;
    uint8_t g_pga_rbb = (uint8_t)Get_SPI_Reg_bits(LMS7param(G_PGA_RBB));

    while (g_pga_rbb < 31)
    {
        cg_iamp_tbb = 0;
        while (cg_iamp_tbb < 63 && rssi < rssi_required)
        {
            Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), cg_iamp_tbb);
            rssi = GetRSSI();
            if (rssi > rssi_required)
                return;
            ++cg_iamp_tbb;
        }
        g_pga_rbb += 6;
        if (g_pga_rbb > 31)
            g_pga_rbb = 31;
        Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), g_pga_rbb);
    }
}

liblms7_status LMS7002M::TuneTxFilterLowBandChain(float_type bandwidth, float_type realpole_MHz)
{
    uint32_t rssi;
    uint32_t rssi_value_10k;
    bool prevRSSIbigger;
    int16_t rcal;
    float_type p1,p2,p3,p4,p5;
    float_type ncoFreq = 0.05;
    float_type cgenFreq;
    BackupAllRegisters();

    liblms7_status status = TuneTxFilter(TX_LADDER, bandwidth);
    uint8_t ladder_c_value = (uint8_t)Get_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB));
    uint8_t ladder_r_value = (uint8_t)Get_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB));
    status = TuneTxFilterSetup(TX_LADDER, bandwidth);

    if (bandwidth < gLadder_lower_limit || bandwidth > gLadder_higher_limit)
    {
        status = LIBLMS7_FREQUENCY_OUT_OF_RANGE;
        goto TxFilterLowBandChainEnd;
    }

    //realpole calibration
    //float userCLKGENfreq = GetFrequencyCGEN();

    cgenFreq = realpole_MHz * 20;
    if (cgenFreq < 60)
        cgenFreq = 60;
    if (cgenFreq > 640)
        cgenFreq = 640;

    if (realpole_MHz < gRealpole_lower_limit || realpole_MHz > gRealpole_higher_limit)
    {
        status = LIBLMS7_FREQUENCY_OUT_OF_RANGE;
        goto TxFilterLowBandChainEnd;
    }

    status = SetFrequencyCGEN(cgenFreq);
    if (status != LIBLMS7_SUCCESS)
        goto TxFilterLowBandChainEnd;

    Modify_SPI_Reg_bits(LMS7param(LOOPB_TBB), 3);
    Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(BYPLADDER_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(TSTIN_TBB), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFH_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFIAMP_TBB), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFLAD_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFS5_TBB), 0);
    Modify_SPI_Reg_bits(LMS7param(EN_G_TBB), 1);

    //B
    p1 = 1.93821841029921E-15;
    p2 = -0.0429694461214244;
    p3 = 0.253501254059498;
    p4 = 88.9545445989649;
    p5 = -48.0847491316861;
    rcal = (int16_t)(pow(realpole_MHz, 4)*p1 + pow(realpole_MHz, 3)*p2 + pow(realpole_MHz, 2)*p3 + realpole_MHz * p4 + p5);
    if (rcal < 0)
        rcal = 0;
    if (rcal > 255)
        rcal = 255;
    Modify_SPI_Reg_bits(LMS7param(RCAL_LPFS5_TBB), rcal);

    FilterTuning_AdjustGains();

    rssi_value_10k = GetRSSI();
    SetNCOFrequency(Tx, 0, realpole_MHz);
    SetNCOFrequency(Rx, 0, realpole_MHz - 1);

    prevRSSIbigger = GetRSSI() > rssi_value_10k*0.707;
    status = LIBLMS7_FAILURE; //assuming r value is not found
    while (rcal >= 0 && rcal < 256)
    {
        Modify_SPI_Reg_bits(LMS7param(RCAL_LPFS5_TBB), rcal);
        SetNCOFrequency(Tx, 0, ncoFreq);
        SetNCOFrequency(Rx, 0, ncoFreq - 1);
        rssi_value_10k = (uint32_t)( GetRSSI()*0.707 );

        SetNCOFrequency(Tx, 0, realpole_MHz);
        SetNCOFrequency(Rx, 0, realpole_MHz - 1);

        rssi = GetRSSI();
        if (rssi > rssi_value_10k)
            --rcal;
        else
        {
            if (prevRSSIbigger)
            {
                --rcal;
                status = LIBLMS7_SUCCESS;
                goto TxFilterLowBandChainEnd;
            }
            ++rcal;
        }
        prevRSSIbigger = rssi > rssi_value_10k;
    }

    //end
TxFilterLowBandChainEnd:
    RestoreAllRegisters();    
    if (status != LIBLMS7_SUCCESS)
        return status;

    Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ladder_c_value);
    Modify_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB), ladder_r_value);
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_FRP_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_GG_FRP_TBB), 6);
    Modify_SPI_Reg_bits(LMS7param(RCAL_LPFS5_TBB), rcal);
    Modify_SPI_Reg_bits(0x0105, 4, 0, 0x11); //set powerdowns

    return LIBLMS7_SUCCESS;
}

liblms7_status LMS7002M::TuneRxFilter(RxFilter filter, float_type bandwidth_MHz)
{
    liblms7_status status;
    uint16_t cfb_tia_rfe;
    uint16_t c_ctl_lpfl_rbb;
    uint8_t ccomp_tia_rfe;
    uint8_t rcomp_tia_rfe;
    uint8_t c_ctl_lpfh_rbb;
    uint8_t ict_pga_out;
    uint8_t ict_pga_in;
    uint8_t r_ctl_lpf_rbb;
    uint8_t c_ctl_pga_rbb;
    uint8_t rcc_ctl_lpfl_rbb;
    uint8_t rcc_ctl_lpfh_rbb;
    float_type lowerLimit;
    float_type higherLimit;
    if (filter == RX_TIA)
    {
        lowerLimit = Get_SPI_Reg_bits(LMS7param(G_TIA_RFE)) == 1 ? gRxTIA_lower_limit_g1 : gRxTIA_lower_limit_g23;
        higherLimit = gRxTIA_higher_limit;
    }
    else if (filter == RX_LPF_LOWBAND)
    {
        lowerLimit = gRxLPF_low_lower_limit;
        higherLimit = gRxLPF_low_higher_limit;
    }
    else if (filter == RX_LPF_HIGHBAND)
    {
        lowerLimit = gRxLPF_high_lower_limit;
        higherLimit = gRxLPF_high_higher_limit;
    }
    if (bandwidth_MHz < lowerLimit || bandwidth_MHz > higherLimit)
        return LIBLMS7_FREQUENCY_OUT_OF_RANGE;

    BackupAllRegisters();

    status = TuneRxFilterSetup(filter, bandwidth_MHz);
    if (status != LIBLMS7_SUCCESS)
        goto RxFilterTuneEnd;

    if (filter == RX_TIA)
        status = RFE_TIA_Calibration(bandwidth_MHz);
    else if (filter == RX_LPF_LOWBAND)
        status = RxLPFLow_Calibration(bandwidth_MHz);
    else if (filter == RX_LPF_HIGHBAND)
        status = RxLPFHigh_Calibration(bandwidth_MHz);

    cfb_tia_rfe = Get_SPI_Reg_bits(LMS7param(CFB_TIA_RFE));
    c_ctl_lpfl_rbb = Get_SPI_Reg_bits(LMS7param(C_CTL_LPFL_RBB));
    ccomp_tia_rfe = (int8_t)Get_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE));
    rcomp_tia_rfe = (int8_t)Get_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE));
    c_ctl_lpfh_rbb = (int8_t)Get_SPI_Reg_bits(LMS7param(C_CTL_LPFH_RBB));
    ict_pga_out = (int8_t)Get_SPI_Reg_bits(LMS7param(ICT_PGA_OUT_RBB));
    ict_pga_in = (int8_t)Get_SPI_Reg_bits(LMS7param(ICT_PGA_IN_RBB));
    r_ctl_lpf_rbb = (int8_t)Get_SPI_Reg_bits(LMS7param(R_CTL_LPF_RBB));
    c_ctl_pga_rbb = (int8_t)Get_SPI_Reg_bits(LMS7param(C_CTL_PGA_RBB));
    rcc_ctl_lpfl_rbb = (int8_t)Get_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB));
    rcc_ctl_lpfh_rbb = (int8_t)Get_SPI_Reg_bits(LMS7param(RCC_CTL_LPFH_RBB));

RxFilterTuneEnd:
    RestoreAllRegisters();
    if (status != LIBLMS7_SUCCESS)
        return status;

    if (filter == RX_TIA)
    {
        Modify_SPI_Reg_bits(LMS7param(ICT_TIAMAIN_RFE), 2);
        Modify_SPI_Reg_bits(LMS7param(ICT_TIAOUT_RFE), 2);
        Modify_SPI_Reg_bits(LMS7param(RFB_TIA_RFE), 16);
        Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), cfb_tia_rfe);
        Modify_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE), ccomp_tia_rfe);
        Modify_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE), rcomp_tia_rfe);
        Modify_SPI_Reg_bits(0x010c, 1, 0, 0x1);
    }
    else if (filter == RX_LPF_LOWBAND)
    {
        Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB), rcc_ctl_lpfl_rbb);
        Modify_SPI_Reg_bits(LMS7param(C_CTL_LPFL_RBB), c_ctl_lpfl_rbb);
        Modify_SPI_Reg_bits(LMS7param(ICT_PGA_OUT_RBB), ict_pga_out);
        Modify_SPI_Reg_bits(LMS7param(ICT_PGA_IN_RBB), ict_pga_in);
        Modify_SPI_Reg_bits(LMS7param(R_CTL_LPF_RBB), r_ctl_lpf_rbb);
        Modify_SPI_Reg_bits(LMS7param(C_CTL_PGA_RBB), c_ctl_pga_rbb);
        Modify_SPI_Reg_bits(0x0115, 3, 0, 0x9);
        Modify_SPI_Reg_bits(0x0118, 15, 13, 0x1);
    }
    else if (filter == RX_LPF_HIGHBAND)
    {
        Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFH_RBB), rcc_ctl_lpfh_rbb);
        Modify_SPI_Reg_bits(LMS7param(C_CTL_LPFH_RBB), c_ctl_lpfh_rbb);
        Modify_SPI_Reg_bits(LMS7param(ICT_PGA_OUT_RBB), ict_pga_out);
        Modify_SPI_Reg_bits(LMS7param(ICT_PGA_IN_RBB), ict_pga_in);
        Modify_SPI_Reg_bits(LMS7param(R_CTL_LPF_RBB), r_ctl_lpf_rbb);
        Modify_SPI_Reg_bits(LMS7param(C_CTL_PGA_RBB), c_ctl_pga_rbb);
        Modify_SPI_Reg_bits(0x0115, 3, 0, 0x5);
        Modify_SPI_Reg_bits(0x0118, 15, 13, 0x0);
    }
    return LIBLMS7_SUCCESS;
}

liblms7_status LMS7002M::TuneRxFilterSetup(RxFilter type, float_type cutoff_MHz)
{
    liblms7_status status;
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));

    //RFE
    uint8_t g_tia_rfe = (uint8_t)Get_SPI_Reg_bits(LMS7param(G_TIA_RFE));
    SetDefaults(RFE);
    Modify_SPI_Reg_bits(LMS7param(SEL_PATH_RFE), 2);

    if (ch == 2)
        Modify_SPI_Reg_bits(LMS7param(EN_NEXTRX_RFE), 1);
    else
        Modify_SPI_Reg_bits(LMS7param(EN_NEXTRX_RFE), 0);

    Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), 8);
    Modify_SPI_Reg_bits(LMS7param(PD_RLOOPB_2_RFE), 0);
    Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_LB2_RFE), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_MXLOBUF_RFE), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_QGEN_RFE), 0);
    Modify_SPI_Reg_bits(LMS7param(ICT_TIAMAIN_RFE), 2);
    Modify_SPI_Reg_bits(LMS7param(ICT_TIAOUT_RFE), 2);
    Modify_SPI_Reg_bits(LMS7param(RFB_TIA_RFE), 16);
    Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE), g_tia_rfe);

    //RBB
    SetDefaults(RBB);
    Modify_SPI_Reg_bits(LMS7param(ICT_PGA_OUT_RBB), 20);
    Modify_SPI_Reg_bits(LMS7param(ICT_PGA_IN_RBB), 20);
    Modify_SPI_Reg_bits(LMS7param(C_CTL_PGA_RBB), 3);

    //TRF
    SetDefaults(TRF);
    Modify_SPI_Reg_bits(LMS7param(L_LOOPB_TXPAD_TRF), 0);
    Modify_SPI_Reg_bits(LMS7param(EN_LOOPB_TXPAD_TRF), 1);
    if (ch == 2)
        Modify_SPI_Reg_bits(LMS7param(EN_NEXTTX_TRF), 1);
    else
        Modify_SPI_Reg_bits(LMS7param(EN_NEXTTX_TRF), 0);
    Modify_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF), 0);
    Modify_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF), 1);

    //TBB
    SetDefaults(TBB);
    Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_FRP_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_GG_FRP_TBB), 6);

    //AFE
    SetDefaults(AFE);
    if (ch == 2)
    {
        Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0);
        Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);
    }
    //BIAS
    uint8_t rp_calib_bias = (uint8_t)Get_SPI_Reg_bits(LMS7param(RP_CALIB_BIAS));
    SetDefaults(BIAS);
    Modify_SPI_Reg_bits(LMS7param(RP_CALIB_BIAS), rp_calib_bias);

    //XBUF
    Modify_SPI_Reg_bits(LMS7param(PD_XBUF_RX), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_XBUF_TX), 0);
    Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), 1);

    //CLKGEN
    SetDefaults(CGEN);

    //SXR
    Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    SetDefaults(SX);
    status = SetFrequencySX(Rx, 499.95, mRefClkSXR_MHz);
    if (status != LIBLMS7_SUCCESS)
        return status;
    Modify_SPI_Reg_bits(LMS7param(PD_VCO), 0);

    //SXT
    Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    SetDefaults(SX);
    status = SetFrequencySX(Tx, 500, mRefClkSXT_MHz);
    if (status != LIBLMS7_SUCCESS)
        return status;
    Modify_SPI_Reg_bits(LMS7param(PD_VCO), 0);

    Modify_SPI_Reg_bits(LMS7param(MAC), ch);
    //TxTSP
    SetDefaults(TxTSP);
    Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), 1);
    Modify_SPI_Reg_bits(0x0208, 8, 8, 1);
    Modify_SPI_Reg_bits(0x0208, 6, 4, 0x7);
    LoadDC_REG_IQ(Tx, (int16_t)0x7FFF, (int16_t)0x8000);
    SetNCOFrequency(Tx, 0, 0);

    //RxTSP
    SetDefaults(RxTSP);
    Modify_SPI_Reg_bits(LMS7param(AGC_MODE_RXTSP), 1);
    Modify_SPI_Reg_bits(0x040C, 5, 3, 0x7);
    Modify_SPI_Reg_bits(LMS7param(AGC_AVG_RXTSP), 7);
    Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_RXTSP), 1);

    float_type sxtfreq = GetFrequencySX_MHz(Tx, mRefClkSXT_MHz);
    float_type sxrfreq = GetFrequencySX_MHz(Rx, mRefClkSXR_MHz);
    SetNCOFrequency(Rx, 0, sxtfreq - sxrfreq - 1);
    return LIBLMS7_SUCCESS;
}

liblms7_status LMS7002M::RFE_TIA_Calibration(float_type TIA_freq_MHz)
{
    liblms7_status status;
    bool prevRSSIbigger;
    uint8_t ccomp_tia_rfe_value;
    int16_t rcomp_tia_rfe;
    float_type cgenFreq = TIA_freq_MHz * 20;
    uint32_t rssi;
    uint32_t rssi_value_50k;
    //RFE
    uint8_t g_tia_rfe = (uint8_t)Get_SPI_Reg_bits(LMS7param(G_TIA_RFE));
    int16_t cfb_tia_rfe_value;
    if (g_tia_rfe == 1)
        cfb_tia_rfe_value = (uint16_t)(5400 / TIA_freq_MHz - 15);
    else if (g_tia_rfe > 1)
        cfb_tia_rfe_value = (uint16_t)(1680 / TIA_freq_MHz - 10);
    else
        return LIBLMS7_FAILURE;
    Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), cfb_tia_rfe_value);

    if (g_tia_rfe == 1)
        ccomp_tia_rfe_value = (uint8_t)(cfb_tia_rfe_value / 100 + 1);
    else if (g_tia_rfe > 1)
        ccomp_tia_rfe_value = (uint8_t)(cfb_tia_rfe_value / 100);
    else
        return LIBLMS7_FAILURE;
    if (ccomp_tia_rfe_value > 15)
        ccomp_tia_rfe_value = 15;

    Modify_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE), ccomp_tia_rfe_value);

    rcomp_tia_rfe = (int16_t)(15 - cfb_tia_rfe_value * 2 / 100);
    if (rcomp_tia_rfe < 0)
        rcomp_tia_rfe = 0;
    Modify_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE), rcomp_tia_rfe);

    //RBB
    Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB), 2);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), 1);

    //CLKGEN
    if (cgenFreq < 60)
        cgenFreq = 60;
    if (cgenFreq > 640)
        cgenFreq = 640;

    if (cgenFreq / 16 == TIA_freq_MHz)
        status = SetFrequencyCGEN(cgenFreq - 10);
    else
        status = SetFrequencyCGEN(cgenFreq);
    if (status != LIBLMS7_SUCCESS)
        return status;

    FilterTuning_AdjustGains();

    rssi_value_50k = (uint32_t)( GetRSSI() * 0.707 );
    status = SetFrequencySX(Rx, GetFrequencySX_MHz(Tx, mRefClkSXT_MHz) - TIA_freq_MHz, mRefClkSXR_MHz);
    if (status != LIBLMS7_SUCCESS)
        return status;
    SetNCOFrequency(Rx, 0, GetFrequencySX_MHz(Tx, mRefClkSXT_MHz) - GetFrequencySX_MHz(Rx, mRefClkSXR_MHz) - 1);

    prevRSSIbigger = GetRSSI() > rssi_value_50k;
    while (cfb_tia_rfe_value >= 0 && cfb_tia_rfe_value < 4096)
    {
        Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), cfb_tia_rfe_value);
        rssi = GetRSSI();
        if (rssi > rssi_value_50k)
            ++cfb_tia_rfe_value;
        else
        {
            --cfb_tia_rfe_value;
            if (prevRSSIbigger)
                return LIBLMS7_SUCCESS; //found correct value
        }
        prevRSSIbigger = rssi > rssi_value_50k;
    }
    return LIBLMS7_FAILURE;
}

liblms7_status LMS7002M::RxLPFLow_Calibration(float_type RxLPFL_freq_MHz)
{
    liblms7_status status;
    uint32_t rssi;
    uint32_t rssi_value_50k;
    int32_t c_ctl_lpfl_rbb;
    bool prevRSSIbigger;
    float_type cgenFreq_MHz = RxLPFL_freq_MHz * 20;
    //RFE
    Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), 15);
    Modify_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE), 1);
    Modify_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE), 15);
    Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE), 1);

    //RBB
    c_ctl_lpfl_rbb = (int32_t)(2160 / RxLPFL_freq_MHz - 103);
    if (c_ctl_lpfl_rbb < 0)
        c_ctl_lpfl_rbb = 0;
    if (c_ctl_lpfl_rbb > 2047)
        c_ctl_lpfl_rbb = 2047;
    Modify_SPI_Reg_bits(LMS7param(C_CTL_LPFL_RBB), c_ctl_lpfl_rbb);

    if (RxLPFL_freq_MHz >= 15)
        Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB), 5);
    else if (RxLPFL_freq_MHz >= 10)
        Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB), 4);
    else if (RxLPFL_freq_MHz >= 5)
        Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB), 3);
    else if (RxLPFL_freq_MHz >= 3)
        Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB), 2);
    else if (RxLPFL_freq_MHz >= 1.4)
        Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB), 1);
    else
        Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB), 0);

    //CLKGEN
    if (cgenFreq_MHz < 60)
        cgenFreq_MHz = 60;
    if (cgenFreq_MHz > 640)
        cgenFreq_MHz = 640;
    if (cgenFreq_MHz / 16 == RxLPFL_freq_MHz)
        status = SetFrequencyCGEN(cgenFreq_MHz - 10);
    else
        status = SetFrequencyCGEN(cgenFreq_MHz);
    if (status != LIBLMS7_SUCCESS)
        return status;

    FilterTuning_AdjustGains();

    rssi_value_50k = (uint32_t)( GetRSSI() * 0.707 );
    status = SetFrequencySX(Rx, GetFrequencySX_MHz(Tx, mRefClkSXT_MHz) - RxLPFL_freq_MHz, mRefClkSXR_MHz);
    if (status != LIBLMS7_SUCCESS)
        return status;
    SetNCOFrequency(Rx, 0, GetFrequencySX_MHz(Tx, mRefClkSXT_MHz) - GetFrequencySX_MHz(Rx, mRefClkSXR_MHz) - 1);

    prevRSSIbigger = GetRSSI() > rssi_value_50k;
    while (c_ctl_lpfl_rbb >= 0 && c_ctl_lpfl_rbb < 2048)
    {
        Modify_SPI_Reg_bits(LMS7param(C_CTL_LPFL_RBB), c_ctl_lpfl_rbb);
        rssi = GetRSSI();
        if (rssi > rssi_value_50k)
            ++c_ctl_lpfl_rbb;
        else
        {
            --c_ctl_lpfl_rbb;
            if (prevRSSIbigger)
                return LIBLMS7_SUCCESS; //found correct value
        }
        prevRSSIbigger = rssi > rssi_value_50k;
    }
    return LIBLMS7_FAILURE;
}

liblms7_status LMS7002M::RxLPFHigh_Calibration(float_type RxLPFH_freq_MHz)
{
    liblms7_status status;
    int16_t c_ctl_lpfh_rbb;
    int16_t rcc_ctl_lpfh_rbb;
    float_type cgenFreq = RxLPFH_freq_MHz * 20;
    uint32_t rssi;
    uint32_t rssi_value_50k;
    bool prevRSSIbigger;
    //RFE
    Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), 15);
    Modify_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE), 1);
    Modify_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE), 15);
    Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE), 1);

    //RBB
    c_ctl_lpfh_rbb = (int16_t)(6000 / RxLPFH_freq_MHz - 50);
    if (c_ctl_lpfh_rbb < 0)
        c_ctl_lpfh_rbb = 0;
    if (c_ctl_lpfh_rbb > 255)
        c_ctl_lpfh_rbb = 255;
    Modify_SPI_Reg_bits(LMS7param(C_CTL_LPFH_RBB), c_ctl_lpfh_rbb);

    rcc_ctl_lpfh_rbb = (int16_t)(RxLPFH_freq_MHz / 10 - 3);
    if (rcc_ctl_lpfh_rbb < 0)
        rcc_ctl_lpfh_rbb = 0;
    Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFH_RBB), rcc_ctl_lpfh_rbb);

    Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB), 1);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), 1);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB), 0);

    //CLKGEN
    if (cgenFreq < 60)
        cgenFreq = 60;
    if (cgenFreq > 640)
        cgenFreq = 640;
    if (cgenFreq / 16 == RxLPFH_freq_MHz)
        status = SetFrequencyCGEN(cgenFreq - 10);
    else
        status = SetFrequencyCGEN(cgenFreq);
    if (status != LIBLMS7_SUCCESS)
        return status;

    FilterTuning_AdjustGains();

    rssi_value_50k = (uint32_t)( GetRSSI() * 0.707);
    status = SetFrequencySX(Rx, GetFrequencySX_MHz(Tx, mRefClkSXT_MHz) - RxLPFH_freq_MHz, mRefClkSXR_MHz);
    if (status != LIBLMS7_SUCCESS)
        return status;
    SetNCOFrequency(Rx, 0, GetFrequencySX_MHz(Tx, mRefClkSXT_MHz) - GetFrequencySX_MHz(Rx, mRefClkSXR_MHz) - 1);

    prevRSSIbigger = GetRSSI() > rssi_value_50k;
    while (c_ctl_lpfh_rbb >= 0 && c_ctl_lpfh_rbb < 256)
    {
        Modify_SPI_Reg_bits(LMS7param(C_CTL_LPFH_RBB), c_ctl_lpfh_rbb);
        rssi = GetRSSI();
        if (rssi > rssi_value_50k)
            ++c_ctl_lpfh_rbb;
        else
        {
            --c_ctl_lpfh_rbb;
            if (prevRSSIbigger)
                return LIBLMS7_SUCCESS; //found correct value
        }
        prevRSSIbigger = rssi > rssi_value_50k;
    }
    return LIBLMS7_FAILURE;
}
