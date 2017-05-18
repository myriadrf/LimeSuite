#include "lms7002m_controls.h"
#include "lms7002m_calibrations.h"
#include "lms7002m_filters.h"
#include "LMS7002M_parameters_compact.h"
#include "spi.h"
#include <math.h>

#ifdef __cplusplus
    #include <vector>
    #include "ErrorReporting.h"
    #include <math.h>
    using namespace lime;
#endif // __cplusplus

#define E_DECREASE_R 0x0080
#define E_INCREASE_R 0x0081

//rx lpf range limits
static ROM const float_type RxLPF_RF_LimitLow = 1.4e6;
static ROM const float_type RxLPF_RF_LimitHigh = 130e6;

//tx lpf range limits
static ROM const float_type TxLPF_RF_LimitLow = 5e6;
static ROM const float_type TxLPF_RF_LimitLowMid = 40e6;
static ROM const float_type TxLPF_RF_LimitMidHigh = 50e6;
static ROM const float_type TxLPF_RF_LimitHigh = 130e6;

static int16_t clamp(int16_t value, int16_t minBound, int16_t maxBound)
{
    if(value < minBound)
        return minBound;
    if(value > maxBound)
        return maxBound;
    return value;
}

static uint8_t RxFilterSearch(const uint16_t addr, const uint8_t msblsb, const uint16_t rssi_3dB, const uint16_t stepLimit)
{
    const bool doDecrement = GetRSSI() < rssi_3dB;
    int16_t value = Get_SPI_Reg_bits(addr, msblsb);
    const uint16_t maxValue = pow2((msblsb>>4)-(msblsb&0xF)+1)-1;
    uint16_t stepSize = 1;
    while(1)
    {
        stepSize <<= 1;
        if(doDecrement)
            value -= stepSize;
        else
            value += stepSize;
        value = clamp(value, 0, maxValue);
        Modify_SPI_Reg_bits(addr, msblsb, value);
        if(doDecrement != (GetRSSI() < rssi_3dB))
            break;
        if(stepSize >= stepLimit)
            return doDecrement ? E_INCREASE_R : E_DECREASE_R;
    }
    while(stepSize > 1)
    {
        stepSize /= 2;
        if(GetRSSI() >= rssi_3dB)
            value += stepSize;
        else
            value -= stepSize;
        Modify_SPI_Reg_bits(addr, msblsb, value);
    }
    return 0;
}

uint8_t TuneRxFilterSetup(const float_type rx_lpf_IF)
{
    uint8_t status;
    const uint16_t ch = SPI_read(0x0020);
    uint8_t g_tia_rfe = Get_SPI_Reg_bits(G_TIA_RFE);
    uint8_t g_pga_rbb = Get_SPI_Reg_bits(G_PGA_RBB);

	if(RxLPF_RF_LimitLow/2 > rx_lpf_IF || rx_lpf_IF > RxLPF_RF_LimitHigh/2)
        return -1;//ReportError(ERANGE, "RxLPF frequency out of range, available range from 0.7 to 65 MHz");
#define BATCH_RX_SETUP 1
#if BATCH_RX_SETUP
    {
        ROM const uint16_t RxFilterSetupAddr[] = {0x0085,0x010D,0x0113,0x0114, 0x0084};
        ROM const uint16_t RxFilterSetupData[] = {0x0001,0x0100,0x0004,0x0010, 0x0400};
        ROM const uint16_t RxFilterSetupMask[] = {0x0007,0x0188,0x003C,0x001F, 0xF83F};
        uint8_t i;
        for(i=sizeof(RxFilterSetupAddr)/sizeof(uint16_t); i; --i)
            SPI_write(RxFilterSetupAddr[i-1], ( SPI_read(RxFilterSetupAddr[i-1]) & ~RxFilterSetupMask[i-1] ) | RxFilterSetupData[i-1]);
    }
    {
        ROM const uint16_t RxFilterSetupAddrWrOnly[] = {0x0082,0x0086,0x0087,0x0088,0x0089,0x008A,0x008B,0x008C,0x0100,0x0101,0x0102,0x0103,0x0104,0x0105,0x0106,0x0107,0x0108,0x0109,0x010A,0x010C,0x0115,0x0116,0x0117,0x0118,0x0119,0x011A,0x0200,0x0201,0x0202,0x0203,0x0204,0x0205,0x0206,0x0207,0x0208,0x0209,0x0240,0x0400,0x0401,0x0402,0x0403,0x0404,0x0405,0x0406,0x0407,0x0408,0x0409,0x040A,0x040C,0x0440};
        ROM const uint16_t RxFilterSetupDataWrOnly[] = {0x8003,0x4901,0x0400,0x0780,0x0020,0x0514,0x2100,0x067B,0x3409,0x6001,0x3180,0x0612,0x0088,0x0007,0x318C,0x318C,0x0426,0x61C1,0x104C,0x88C5,0x0009,0x8180,0x280C,0x018C,0x528B,0x2E02,0x008D,0x07FF,0x07FF,0x0000,0x0000,0x0000,0x0000,0x0000,0x2070,0x0000,0x0020,0x0081,0x07FF,0x07FF,0x4000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1001,0x2038,0x0020};
        uint16_t i;
        for(i=sizeof(RxFilterSetupAddrWrOnly)/sizeof(uint16_t); i; --i)
            SPI_write(RxFilterSetupAddrWrOnly[i-1], RxFilterSetupDataWrOnly[i-1]);
    }
#else
    BeginBatch("RxFilterSetup");

    SetDefaults(SECTION_RFE);
    Modify_SPI_Reg_bits(SEL_PATH_RFE, 2);
    Modify_SPI_Reg_bits(G_RXLOOPB_RFE, 1);
    Modify_SPI_Reg_bits(PD_RLOOPB_2_RFE, 0);
    Modify_SPI_Reg_bits(EN_INSHSW_LB2_RFE, 0);
    Modify_SPI_Reg_bits(PD_MXLOBUF_RFE, 0);
    Modify_SPI_Reg_bits(PD_QGEN_RFE, 0);
    Modify_SPI_Reg_bits(RFB_TIA_RFE, 16);

    //RBB
    SetDefaults(SECTION_RBB);
    Modify_SPI_Reg_bits(ICT_PGA_OUT_RBB, 20);
    Modify_SPI_Reg_bits(ICT_PGA_IN_RBB, 20);

    //TRF
    SetDefaults(SECTION_TRF);
    Modify_SPI_Reg_bits(L_LOOPB_TXPAD_TRF, 0);
    Modify_SPI_Reg_bits(EN_LOOPB_TXPAD_TRF, 1);
    Modify_SPI_Reg_bits(SEL_BAND1_TRF, 0);
    Modify_SPI_Reg_bits(SEL_BAND2_TRF, 1);

    //TBB
    SetDefaults(SECTION_TBB);
    Modify_SPI_Reg_bits(CG_IAMP_TBB, 1);
    Modify_SPI_Reg_bits(ICT_IAMP_FRP_TBB, 1);
    Modify_SPI_Reg_bits(ICT_IAMP_GG_FRP_TBB, 6);

    //AFE
    SetDefaults(SECTION_AFE);
    Modify_SPI_Reg_bits(PD_RX_AFE2, 0);

    //LDO
    //do nothing
    //XBUF
    Modify_SPI_Reg_bits(PD_XBUF_RX, 0);
    Modify_SPI_Reg_bits(PD_XBUF_TX, 0);
    Modify_SPI_Reg_bits(EN_G_XBUF, 1);

    //TxTSP
    SetDefaults(SECTION_TxTSP);
    SetDefaults(SECTION_TxNCO);
    Modify_SPI_Reg_bits(TSGMODE_TXTSP, 1);
    Modify_SPI_Reg_bits(INSEL_TXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_SC_TXTSP, 1);
    Modify_SPI_Reg_bits(GFIR3_BYP_TXTSP, 1);
    Modify_SPI_Reg_bits(GFIR2_BYP_TXTSP, 1);
    Modify_SPI_Reg_bits(GFIR1_BYP_TXTSP, 1);

    //RxTSP
    SetDefaults(SECTION_RxTSP);
    SetDefaults(SECTION_RxNCO);
    Modify_SPI_Reg_bits(AGC_MODE_RXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
    Modify_SPI_Reg_bits(GFIR3_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(GFIR2_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(GFIR1_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(AGC_AVG_RXTSP, 1);
    Modify_SPI_Reg_bits(HBD_OVR_RXTSP, 4);
    Modify_SPI_Reg_bits(CMIX_GAIN_RXTSP, 0);
    Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 1);

    //CGEN
    SetDefaults(SECTION_CGEN);
    EndBatch();

    //BIAS
/*
    {
        const uint8_t rp_calib_bias = Get_SPI_Reg_bits(RP_CALIB_BIAS);
        SetDefaults(SECTION_BIAS);
        Modify_SPI_Reg_bits(RP_CALIB_BIAS, rp_calib_bias);
    }
*/
#endif
    Modify_SPI_Reg_bits(G_TIA_RFE, g_tia_rfe);
    if(g_pga_rbb == 31) {
        Modify_SPI_Reg_bits(G_PGA_RBB, 22);
    }
    else {
        Modify_SPI_Reg_bits(G_PGA_RBB, g_pga_rbb);
    }

    {
        uint8_t cgenMultiplier = rx_lpf_IF*20 / 46.08e6 + 0.5;
        cgenMultiplier = clamp(cgenMultiplier, 2, 13);
        status = SetFrequencyCGEN(46.08e6 * cgenMultiplier + 10e6);
        if(status != 0)
            return status;
    }

    //SXR
    Modify_SPI_Reg_bits(MAC, 1);
    SetDefaultsSX();
    status = SetFrequencySX(LMS7002M_Rx, 539.9e6);
    if(status != 0)
        return status;

    //SXT
    Modify_SPI_Reg_bits(MAC, 2);
    SetDefaultsSX();
    status = SetFrequencySX(LMS7002M_Tx, 550e6);
    if(status != 0)
        return status;
    SPI_write(0x0020, ch);
    //LimeLight & PAD
    //do nothing

    LoadDC_REG_TX_IQ();
    SetNCOFrequency(LMS7002M_Tx, 10e6, 0); //0
    SetNCOFrequency(LMS7002M_Rx, 0, 0); //0


    if(rx_lpf_IF <= 54e6)
    {
        Modify_SPI_Reg_bits(CFB_TIA_RFE, 1);
        Modify_SPI_Reg_bits(CCOMP_TIA_RFE, 0);
        Modify_SPI_Reg_bits(RCOMP_TIA_RFE, 15);
    }
    else
    {
        int16_t cfb_tia_rfe;
        int8_t ccomp_tia_rfe;
        if(g_tia_rfe == 3 || g_tia_rfe == 2)
        {
            cfb_tia_rfe = (int16_t)( 1680e6/rx_lpf_IF - 10);
            ccomp_tia_rfe = cfb_tia_rfe/100;
        }
        else if(g_tia_rfe == 1)
        {
            cfb_tia_rfe = (int16_t)( 5400e6/rx_lpf_IF - 15);
            ccomp_tia_rfe = cfb_tia_rfe/100 + 1;
        }
        else
            return 0xAA;//ReportError(EINVAL ,"Calibration setup: G_TIA_RFE value not allowed");
        Modify_SPI_Reg_bits(CFB_TIA_RFE,   clamp(cfb_tia_rfe, 0, 4095));
        Modify_SPI_Reg_bits(CCOMP_TIA_RFE, clamp(ccomp_tia_rfe, 0, 15));
        Modify_SPI_Reg_bits(RCOMP_TIA_RFE, clamp(15-cfb_tia_rfe/100, 0, 15));
    }

    {
        uint8_t c_ctl_pga_rbb = 0;
        if(g_pga_rbb < 21)
            c_ctl_pga_rbb = 1;
        if(g_pga_rbb < 13)
            c_ctl_pga_rbb = 2;
        if(g_pga_rbb < 8)
            c_ctl_pga_rbb = 3;
        Modify_SPI_Reg_bits(C_CTL_PGA_RBB, c_ctl_pga_rbb);
    }
    {
        int8_t rcc_ctl_pga_rbb = (430 * pow(0.65, g_pga_rbb/10) - 110.35)/20.45 + 16;
        Modify_SPI_Reg_bits(RCC_CTL_PGA_RBB, clamp(rcc_ctl_pga_rbb, 0, 31));
    }
    if(rx_lpf_IF < 18e6)
    {
        Modify_SPI_Reg_bits(PD_LPFL_RBB, 0);
        Modify_SPI_Reg_bits(PD_LPFH_RBB, 1);
        Modify_SPI_Reg_bits(INPUT_CTL_PGA_RBB, 0);
        {
            int16_t c_ctl_lpfl_rbb = (int16_t)(2160e6/(rx_lpf_IF*1.3) - 103);
            Modify_SPI_Reg_bits(C_CTL_LPFL_RBB, clamp(c_ctl_lpfl_rbb, 0, 2047));
        }

        {
            uint8_t rcc_ctl_lpfl_rbb = 5;
            if(rx_lpf_IF*1.3 < 15e6)
                rcc_ctl_lpfl_rbb = 4;
            if(rx_lpf_IF*1.3 < 10e6)
                rcc_ctl_lpfl_rbb = 3;
            if(rx_lpf_IF*1.3 < 5e6)
                rcc_ctl_lpfl_rbb = 2;
            if(rx_lpf_IF*1.3 < 3e6)
                rcc_ctl_lpfl_rbb = 1;
            if(rx_lpf_IF*1.3 < 1.4e6)
                rcc_ctl_lpfl_rbb = 0;
            Modify_SPI_Reg_bits(RCC_CTL_LPFL_RBB, rcc_ctl_lpfl_rbb);
        }
    }
    else if(rx_lpf_IF <= 54e6)
    {
        Modify_SPI_Reg_bits(PD_LPFL_RBB, 1);
        Modify_SPI_Reg_bits(PD_LPFH_RBB, 0);
        Modify_SPI_Reg_bits(INPUT_CTL_PGA_RBB, 1);
        {
            int16_t c_ctl_lpfh_rbb = (int)( 6000e6/(rx_lpf_IF*1.3) - 50);
            Modify_SPI_Reg_bits(C_CTL_LPFH_RBB, clamp(c_ctl_lpfh_rbb, 0, 255));
        }
        {
            int8_t rcc_ctl_lpfh_rbb = (int)(rx_lpf_IF*1.3/10) - 3;
            Modify_SPI_Reg_bits(RCC_CTL_LPFH_RBB, clamp(rcc_ctl_lpfh_rbb, 0, 8));
        }
    }
    else // rx_lpf_IF > 54e6
    {
        Modify_SPI_Reg_bits(PD_LPFL_RBB, 1);
        Modify_SPI_Reg_bits(PD_LPFH_RBB, 1);
        Modify_SPI_Reg_bits(INPUT_CTL_PGA_RBB, 2);
    }

    if( (ch&0x3) == 2)
    {
        Modify_SPI_Reg_bits(PD_TX_AFE2, 0);
        Modify_SPI_Reg_bits(MAC, 1);
        Modify_SPI_Reg_bits(EN_NEXTRX_RFE, 1);
        Modify_SPI_Reg_bits(EN_NEXTTX_TRF, 1);
        Modify_SPI_Reg_bits(MAC, ch);
    }
    return 0;
}

uint8_t TuneRxFilter(const float_type rx_lpf_freq_RF)
{
	uint16_t rssi_3dB ;
    uint8_t status = 0;
    //calculate intermediate frequency
    const float_type rx_lpf_IF = rx_lpf_freq_RF/2;
    SaveChipState();

    status = TuneRxFilterSetup(rx_lpf_IF);
    if(status != 0)
        return status;

    {
        uint8_t g_rxloopb_rfe = Get_SPI_Reg_bits(G_RXLOOPB_RFE);
        while(GetRSSI() < 0x2700 && g_rxloopb_rfe < 14)
        {
            g_rxloopb_rfe += 2;
            Modify_SPI_Reg_bits(G_RXLOOPB_RFE, g_rxloopb_rfe);
        }
    }
    {
        uint8_t cg_iamp_tbb = Get_SPI_Reg_bits(CG_IAMP_TBB);
        while(GetRSSI() < 0x2700 && cg_iamp_tbb < 30)
        {
            cg_iamp_tbb += 2;
            Modify_SPI_Reg_bits(CG_IAMP_TBB, cg_iamp_tbb);
        }
    }

    rssi_3dB = GetRSSI() * 0.7071 * pow(10, (-0.0018 * rx_lpf_IF/1e6)/20);

    if(rx_lpf_IF <= 54e6)
    {
        status = SetFrequencySX(LMS7002M_Rx, 539.9e6-rx_lpf_IF*1.3);
        if(status != 0)
            return status;
        SetNCOFrequency(LMS7002M_Rx, rx_lpf_IF*1.3, 0); //0

        if(rx_lpf_IF < 18e6)
        {
            //LPFL START
            status = RxFilterSearch(C_CTL_LPFL_RBB, rssi_3dB, 2048);
            if(status == E_DECREASE_R)
            {
                uint8_t r_ctl_lpf = Get_SPI_Reg_bits(R_CTL_LPF_RBB);
                while(r_ctl_lpf > 1)
                {
                    r_ctl_lpf /= 2;
                    Modify_SPI_Reg_bits(R_CTL_LPF_RBB, r_ctl_lpf);
                    status = RxFilterSearch(C_CTL_LPFL_RBB, rssi_3dB, 2048);
                }
            }
            if(status == E_INCREASE_R)
            {
                uint8_t r_ctl_lpf = Get_SPI_Reg_bits(R_CTL_LPF_RBB);
                while(r_ctl_lpf < 31)
                {
                    r_ctl_lpf += 4;
                    if(r_ctl_lpf > 31)
                        break;
                    Modify_SPI_Reg_bits(R_CTL_LPF_RBB, r_ctl_lpf);
                    status = RxFilterSearch(C_CTL_LPFL_RBB, rssi_3dB, 2048);
                }
            }
            else if(status != 0)
                return status;
            //LPFL END
        }
        else
        {
            //LPFH START
            status = RxFilterSearch(C_CTL_LPFH_RBB, rssi_3dB, 256);
            if(status == E_DECREASE_R)
            {
                uint8_t r_ctl_lpf = Get_SPI_Reg_bits(R_CTL_LPF_RBB);
                while(r_ctl_lpf > 0)
                {
                    r_ctl_lpf -= 1;
                    Modify_SPI_Reg_bits(R_CTL_LPF_RBB, r_ctl_lpf);
                    if(GetRSSI() < rssi_3dB)
                    {
                        status = 0;
                        break;
                    }

                }
            }
            if(status == E_INCREASE_R)
            {
                uint8_t r_ctl_lpf = Get_SPI_Reg_bits(R_CTL_LPF_RBB);
                while(r_ctl_lpf < 31)
                {
                    r_ctl_lpf += 1;
                    if(r_ctl_lpf > 31)
                        break;
                    Modify_SPI_Reg_bits(R_CTL_LPF_RBB, r_ctl_lpf);
                    if(GetRSSI() > rssi_3dB)
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
        status = SetFrequencySX(LMS7002M_Rx, 539.9e6-rx_lpf_IF);
        if(status != 0)
            return status;
        SetNCOFrequency(LMS7002M_Rx, rx_lpf_IF, 0); //0

        {
            uint16_t cfb_tia_rfe;
            uint8_t g_tia_rfe = Get_SPI_Reg_bits(G_TIA_RFE);
            if(g_tia_rfe == 3 || g_tia_rfe == 2)
                cfb_tia_rfe = (int)( 1680e6 / (rx_lpf_IF * 0.72) - 10);
            else if(g_tia_rfe == 1)
                cfb_tia_rfe = (int)( 5400e6 / (rx_lpf_IF * 0.72) - 15);
            else
                return -3;//ReportError(EINVAL, "g_tia_rfe not allowed value");
            Modify_SPI_Reg_bits(CFB_TIA_RFE, clamp(cfb_tia_rfe, 0, 4095));

			{
                uint8_t ccomp_tia_rfe;
                if(g_tia_rfe > 1)
                    ccomp_tia_rfe = cfb_tia_rfe / 100;
                else if(g_tia_rfe == 1)
                    ccomp_tia_rfe = cfb_tia_rfe / 100 + 1;
                else
                    return -4;//ReportError(EINVAL, "g_tia_rfe not allowed value");

                Modify_SPI_Reg_bits(CCOMP_TIA_RFE, clamp(ccomp_tia_rfe, 0, 15));
			}
			Modify_SPI_Reg_bits(RCOMP_TIA_RFE, clamp(15 - cfb_tia_rfe/100, 0, 15));
        }
    }
    else//if(rx_lpf_IF > 54e6)
    {
        status = SetFrequencySX(LMS7002M_Rx, 539.9e6 - rx_lpf_IF);
        if(status != 0)
            return status;
        SetNCOFrequency(LMS7002M_Rx, rx_lpf_IF, 0); //0
    }
    //START TIA
    status = RxFilterSearch(CFB_TIA_RFE, rssi_3dB, 4096);
    if(status != 0)
        return status;
    //END TIA

    {
    //Restore settings
    uint16_t cfb_tia_rfe = Get_SPI_Reg_bits(CFB_TIA_RFE);
    uint8_t ccomp_tia_rfe = Get_SPI_Reg_bits(CCOMP_TIA_RFE);
    uint8_t rcomp_tia_rfe = Get_SPI_Reg_bits(RCOMP_TIA_RFE);
    uint16_t rcc_ctl_lpfl_rbb = Get_SPI_Reg_bits(RCC_CTL_LPFL_RBB);
    uint16_t c_ctl_lpfl_rbb = Get_SPI_Reg_bits(C_CTL_LPFL_RBB);
    uint8_t c_ctl_pga_rbb = Get_SPI_Reg_bits(C_CTL_PGA_RBB);
    uint8_t rcc_ctl_pga_rbb = Get_SPI_Reg_bits(RCC_CTL_PGA_RBB);
    uint8_t rcc_ctl_lpfh_rbb = Get_SPI_Reg_bits(RCC_CTL_LPFH_RBB);
    uint8_t c_ctl_lpfh_rbb = Get_SPI_Reg_bits(C_CTL_LPFH_RBB);
    uint8_t pd_lpfl_rbb = Get_SPI_Reg_bits(PD_LPFL_RBB);
    uint8_t pd_lpfh_rbb = Get_SPI_Reg_bits(PD_LPFH_RBB);
    uint8_t input_ctl_pga_rbb = Get_SPI_Reg_bits(INPUT_CTL_PGA_RBB);

    RestoreChipState();
    Modify_SPI_Reg_bits(CFB_TIA_RFE, cfb_tia_rfe);
    Modify_SPI_Reg_bits(CCOMP_TIA_RFE, ccomp_tia_rfe);
    Modify_SPI_Reg_bits(RCOMP_TIA_RFE, rcomp_tia_rfe);
    Modify_SPI_Reg_bits(RCC_CTL_LPFL_RBB, rcc_ctl_lpfl_rbb);
    Modify_SPI_Reg_bits(C_CTL_LPFL_RBB, c_ctl_lpfl_rbb);
    Modify_SPI_Reg_bits(C_CTL_PGA_RBB, c_ctl_pga_rbb);
    Modify_SPI_Reg_bits(RCC_CTL_PGA_RBB, rcc_ctl_pga_rbb);
    Modify_SPI_Reg_bits(RCC_CTL_LPFH_RBB, rcc_ctl_lpfh_rbb);
    Modify_SPI_Reg_bits(C_CTL_LPFH_RBB, c_ctl_lpfh_rbb);
    Modify_SPI_Reg_bits(PD_LPFL_RBB, pd_lpfl_rbb);
    Modify_SPI_Reg_bits(PD_LPFH_RBB, pd_lpfh_rbb);
    Modify_SPI_Reg_bits(INPUT_CTL_PGA_RBB, input_ctl_pga_rbb);
    Modify_SPI_Reg_bits(ICT_LPF_IN_RBB, 12);
    Modify_SPI_Reg_bits(ICT_LPF_OUT_RBB, 12);
    Modify_SPI_Reg_bits(ICT_PGA_OUT_RBB, 20);
    Modify_SPI_Reg_bits(ICT_PGA_IN_RBB, 20);
    Modify_SPI_Reg_bits(R_CTL_LPF_RBB, 16);
    Modify_SPI_Reg_bits(RFB_TIA_RFE, 16);
    }
    return 0;
}

uint8_t TuneTxFilterSetup(const float_type tx_lpf_IF)
{
    uint8_t status;
    const uint16_t ch = Get_SPI_Reg_bits(MAC);

#define BATCH_TX_SETUP 1
#if BATCH_TX_SETUP
    {
        ROM const uint16_t TxFilterSetupAddr[] = {0x0082,0x0085,0x0100,0x010C,0x010D, 0x0084};
        ROM const uint16_t TxFilterSetupData[] = {0x8001,0x0001,0x0000,0x0000,0x001E, 0x0400};
        ROM const uint16_t TxFilterSetupMask[] = {0x1FFF,0x0007,0x0001,0x0001,0x001E, 0xF83F};
        uint8_t i;
        for(i=sizeof(TxFilterSetupAddr)/sizeof(uint16_t); i; --i)
            SPI_write(TxFilterSetupAddr[i-1], ( SPI_read(TxFilterSetupAddr[i-1]) & ~TxFilterSetupMask[i-1] ) | TxFilterSetupData[i-1]);
    }
    {
        ROM const uint16_t TxFilterSetupAddrWrOnly[] = {0x0086,0x0087,0x0088,0x0089,0x008A,0x008B,0x008C,0x0105,0x0106,0x0107,0x0108,0x0109,0x010A,0x0115,0x0116,0x0117,0x0118,0x0119,0x011A,0x0200,0x0201,0x0202,0x0203,0x0204,0x0205,0x0206,0x0207,0x0208,0x0240,0x0241,0x0400,0x0401,0x0402,0x0403,0x0404,0x0405,0x0406,0x0407,0x0408,0x0409,0x040A,0x040C,0x0440,0x0441};
        ROM const uint16_t TxFilterSetupDataWrOnly[] = {0x4901,0x0400,0x0780,0x0020,0x0514,0x2100,0x067B,0x3007,0x318C,0x318C,0x058C,0x61C1,0x104C,0x000D,0x8180,0x280C,0x618C,0x528C,0x2E02,0x008D,0x07FF,0x07FF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0070,0x0020,0x0000,0x0081,0x07FF,0x07FF,0x4000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1001,0x2038,0x0020,0x0000};
        uint8_t i;
        for(i=sizeof(TxFilterSetupAddrWrOnly)/sizeof(uint16_t); i; --i)
            SPI_write(TxFilterSetupAddrWrOnly[i-1], TxFilterSetupDataWrOnly[i-1]);
    }
#else
    BeginBatch("TxFilterSetup");
    //RFE
    Modify_SPI_Reg_bits(EN_G_RFE, 0);
    Modify_SPI_Reg_bits(0x010D, 4 << 4 | 1, 0xF);

    //RBB
    SetDefaults(SECTION_RBB);
    Modify_SPI_Reg_bits(PD_LPFL_RBB, 1);
    Modify_SPI_Reg_bits(INPUT_CTL_PGA_RBB, 3);
    Modify_SPI_Reg_bits(G_PGA_RBB, 12);
    Modify_SPI_Reg_bits(RCC_CTL_PGA_RBB, 23);

    //TRF
    Modify_SPI_Reg_bits(EN_G_TRF, 0);

    //TBB
    SetDefaults(SECTION_TBB);
    Modify_SPI_Reg_bits(CG_IAMP_TBB, 1);
    Modify_SPI_Reg_bits(LOOPB_TBB, 3);

    //AFE
    //if(ch == 2)
    {
        Modify_SPI_Reg_bits(PD_RX_AFE2, 0);
        Modify_SPI_Reg_bits(PD_TX_AFE2, 0);
    {
    const uint8_t isel_dac_afe = Get_SPI_Reg_bits(ISEL_DAC_AFE);
    SetDefaults(SECTION_AFE);
    Modify_SPI_Reg_bits(ISEL_DAC_AFE, isel_dac_afe);
    }

    //XBUF
    Modify_SPI_Reg_bits(PD_XBUF_RX, 0);
    Modify_SPI_Reg_bits(PD_XBUF_TX, 0);
    Modify_SPI_Reg_bits(EN_G_XBUF, 1);

    SetDefaults(SECTION_CGEN);

    //TxTSP
    SetDefaults(SECTION_TxTSP);
    SetDefaults(SECTION_TxNCO);
    Modify_SPI_Reg_bits(TSGMODE_TXTSP, 1);
    Modify_SPI_Reg_bits(INSEL_TXTSP, 1);

    //RXTSP
    SetDefaults(SECTION_RxTSP);
    SetDefaults(SECTION_RxNCO);
    Modify_SPI_Reg_bits(AGC_MODE_RXTSP, 1);
    Modify_SPI_Reg_bits(AGC_AVG_RXTSP, 1);
    Modify_SPI_Reg_bits(HBD_OVR_RXTSP, 4);
    Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 1);

    EndBatch();
#endif // BATCH_TX_SETUP
    //BIAS
    /*{
    const uint8_t rp_calib_bias = Get_SPI_Reg_bits(RP_CALIB_BIAS);
    //SetDefaults(SECTION_BIAS);
    Modify_SPI_Reg_bits(RP_CALIB_BIAS, rp_calib_bias);
    }*/

    if(tx_lpf_IF <= TxLPF_RF_LimitLowMid/2)
    {
        int16_t rcal_lpflad_tbb;
        const float_type freq = (16.0/20.0)*tx_lpf_IF/1e6;
        Modify_SPI_Reg_bits(PD_LPFH_TBB, 1);
        Modify_SPI_Reg_bits(PD_LPFLAD_TBB, 0);
        Modify_SPI_Reg_bits(PD_LPFS5_TBB, 0);
        //Modify_SPI_Reg_bits(CCAL_LPFLAD_TBB, 16);
        Modify_SPI_Reg_bits(R5_LPF_BYP_TBB, 1);

        rcal_lpflad_tbb =
              pow(freq, 4)*1.29858903647958e-16
            + pow(freq, 3)*(-0.000110746929967704)
            + pow(freq, 2)*0.00277593485991029
            + freq*21.0384293169607
            + (-48.4092606238297);
        Modify_SPI_Reg_bits(RCAL_LPFLAD_TBB, clamp(rcal_lpflad_tbb, 0, 255));
    }
    else
    {
        int16_t rcal_lpfh_tbb;
        const float_type freq = tx_lpf_IF/1e6;
        Modify_SPI_Reg_bits(PD_LPFH_TBB, 0);
        Modify_SPI_Reg_bits(PD_LPFLAD_TBB, 1);
        Modify_SPI_Reg_bits(PD_LPFS5_TBB, 1);
        //Modify_SPI_Reg_bits(CCAL_LPFLAD_TBB, 16);

        rcal_lpfh_tbb = pow(freq, 4)*1.10383261611112e-06
            + pow(freq, 3)*(-0.000210800032517545)
            + pow(freq,2)*0.0190494874803309
            + freq*1.43317445923528
            + (-47.6950779298333);
        Modify_SPI_Reg_bits(RCAL_LPFH_TBB, clamp(rcal_lpfh_tbb, 0, 255));
    }

    //CGEN
    {
        uint8_t cgenMultiplier = tx_lpf_IF*20/46.08e6 + 0.5;
        cgenMultiplier = clamp(cgenMultiplier, 2, 13);

        status = SetFrequencyCGEN(46.08e6 * cgenMultiplier + 10e6);
    }
    if(status != 0)
        return status;

    //SXR
    Modify_SPI_Reg_bits(MAC, 1);
    Modify_SPI_Reg_bits(PD_VCO, 1);

    //SXT
    Modify_SPI_Reg_bits(MAC, 2);
    Modify_SPI_Reg_bits(PD_VCO, 1);

    Modify_SPI_Reg_bits(MAC, ch);

    //TXTSP
    LoadDC_REG_TX_IQ();
    SetNCOFrequency(LMS7002M_Tx, 1e6, 0);
    SetNCOFrequency(LMS7002M_Tx, tx_lpf_IF, 1);

    //RxTSP
    SetNCOFrequency(LMS7002M_Rx, 0.9e6, 0);
    SetNCOFrequency(LMS7002M_Rx, tx_lpf_IF-0.1e6, 1);

    return 0;
}

uint8_t TuneTxFilter(const float_type tx_lpf_freq_RF)
{
    uint16_t rssi;
    float_type tx_lpf_IF;
    int status;
    uint16_t ch = SPI_read(0x0020);

    if(tx_lpf_freq_RF < TxLPF_RF_LimitLow || tx_lpf_freq_RF > TxLPF_RF_LimitHigh)
        return 0x22;
    //calculate intermediate frequency
    tx_lpf_IF = tx_lpf_freq_RF/2;
    if(tx_lpf_freq_RF > TxLPF_RF_LimitLowMid && tx_lpf_freq_RF < TxLPF_RF_LimitMidHigh)
    {
        /*printf("Tx lpf(%g MHz) out of range %g-%g MHz and %g-%g MHz. Setting to %g MHz", tx_lpf_freq_RF/1e6,
                        TxLPF_RF_LimitLow/1e6, TxLPF_RF_LimitLowMid/1e6,
                        TxLPF_RF_LimitMidHigh/1e6, TxLPF_RF_LimitHigh/1e6,
                        TxLPF_RF_LimitMidHigh/1e6);*/
        tx_lpf_IF = TxLPF_RF_LimitMidHigh/2;
    }
    SaveChipState();
    status = TuneTxFilterSetup(tx_lpf_IF);
    if(status != 0)
        return status;

    Modify_SPI_Reg_bits(SEL_RX, 0);
    Modify_SPI_Reg_bits(SEL_TX, 0);
    rssi = GetRSSI();
    {
        uint8_t cg_iamp_tbb = Get_SPI_Reg_bits(CG_IAMP_TBB);
        while(rssi < 0x2700 && cg_iamp_tbb < 43)
        {
            ++cg_iamp_tbb;
            Modify_SPI_Reg_bits(CG_IAMP_TBB, cg_iamp_tbb);
            rssi = GetRSSI();
        }
    }

    if(tx_lpf_IF <= TxLPF_RF_LimitLowMid/2)
    {
        bool targetLevelNotReached = false;
        int8_t iterationsLeft = 5;
        do
        {
            uint16_t rssi_dc_lad;
            uint16_t rssi_3dB_lad;
            int16_t ccal_lpflad_tbb;

            Modify_SPI_Reg_bits(SEL_TX, 0);
            Modify_SPI_Reg_bits(SEL_RX, 0);
            rssi_dc_lad = GetRSSI();
            rssi_3dB_lad = rssi_dc_lad * 0.7071;
            Modify_SPI_Reg_bits(SEL_TX, 1);
            Modify_SPI_Reg_bits(SEL_RX, 1);
            rssi = GetRSSI();

            ccal_lpflad_tbb = Get_SPI_Reg_bits(CCAL_LPFLAD_TBB);
            if(rssi < rssi_3dB_lad)
            {
                while(rssi < rssi_3dB_lad && ccal_lpflad_tbb > 0)
                {
                    ccal_lpflad_tbb -= 1;
                    Modify_SPI_Reg_bits(CCAL_LPFLAD_TBB, ccal_lpflad_tbb);
                    rssi = GetRSSI();
                }
                if(rssi < rssi_3dB_lad && ccal_lpflad_tbb == 0)
                {
                    uint16_t R;
                    targetLevelNotReached = true;
                    Modify_SPI_Reg_bits(CCAL_LPFLAD_TBB, 16);
                    R = clamp(Get_SPI_Reg_bits(RCAL_LPFLAD_TBB)+25, 0, 255);
                    Modify_SPI_Reg_bits(RCAL_LPFLAD_TBB, R);
                }
                else
                    targetLevelNotReached = false;
            }
            else if(rssi > rssi_3dB_lad)
            {
                while(rssi > rssi_3dB_lad && ccal_lpflad_tbb < 31)
                {
                    ccal_lpflad_tbb += 1;
                    Modify_SPI_Reg_bits(CCAL_LPFLAD_TBB, ccal_lpflad_tbb);
                    rssi = GetRSSI();
                }
                if(rssi > rssi_3dB_lad && ccal_lpflad_tbb == 31)
                {
                    int16_t R;
                    targetLevelNotReached = true;
                    Modify_SPI_Reg_bits(CCAL_LPFLAD_TBB, 16);
                    R = clamp((int16_t)Get_SPI_Reg_bits(RCAL_LPFLAD_TBB)-10, 0, 255);
                    Modify_SPI_Reg_bits(RCAL_LPFLAD_TBB, R);
                }
                else
                    targetLevelNotReached = false;
                ccal_lpflad_tbb = clamp(++ccal_lpflad_tbb, 0, 31);
                Modify_SPI_Reg_bits(CCAL_LPFLAD_TBB, ccal_lpflad_tbb);
            }
            --iterationsLeft;
            {
                const uint8_t R = Get_SPI_Reg_bits(RCAL_LPFLAD_TBB);
                if (R==0 || R==255)
                    break;
            }
        } while(targetLevelNotReached && iterationsLeft>=0);
    }

    else // LPFH
    {
        bool targetLevelNotReached = false;
        int8_t iterationsLeft = 5;
        Modify_SPI_Reg_bits(C_CTL_PGA_RBB, 2);

        do
        {
            uint16_t rssi_dc_h;
            uint16_t rssi_3dB_h;
            int16_t ccal_lpflad_tbb;
            Modify_SPI_Reg_bits(SEL_TX, 0);
            Modify_SPI_Reg_bits(SEL_RX, 0);
            rssi_dc_h = GetRSSI();
            rssi_3dB_h = rssi_dc_h * 0.7071;
            Modify_SPI_Reg_bits(SEL_TX, 1);
            Modify_SPI_Reg_bits(SEL_RX, 1);
            rssi = GetRSSI();

            ccal_lpflad_tbb = Get_SPI_Reg_bits(CCAL_LPFLAD_TBB);
            if(rssi < rssi_3dB_h)
            {
                while(rssi < rssi_3dB_h && ccal_lpflad_tbb > 0)
                {
                    ccal_lpflad_tbb -= 1;
                    Modify_SPI_Reg_bits(CCAL_LPFLAD_TBB, ccal_lpflad_tbb);
                    rssi = GetRSSI();
                }
                if(rssi < rssi_3dB_h && ccal_lpflad_tbb == 0)
                {
                    uint8_t R;
                    targetLevelNotReached = true;
                    Modify_SPI_Reg_bits(CCAL_LPFLAD_TBB, 16);
                    R = clamp(Get_SPI_Reg_bits(RCAL_LPFH_TBB)+25, 0, 255);
                    Modify_SPI_Reg_bits(RCAL_LPFH_TBB, R);
                }
                else
                    targetLevelNotReached = false;
            }
            else if(rssi > rssi_3dB_h)
            {
                while(rssi > rssi_3dB_h && ccal_lpflad_tbb < 31)
                {
                    ccal_lpflad_tbb += 1;
                    Modify_SPI_Reg_bits(CCAL_LPFLAD_TBB, ccal_lpflad_tbb);
                    rssi = GetRSSI();
                }
                if(rssi > rssi_3dB_h && ccal_lpflad_tbb == 31)
                {
                    uint8_t R;
                    targetLevelNotReached = true;
                    Modify_SPI_Reg_bits(CCAL_LPFLAD_TBB, 16);
                    //R = (int16_t)Get_SPI_Reg_bits(RCAL_LPFH_TBB)-10;
                    R = clamp((int16_t)Get_SPI_Reg_bits(RCAL_LPFH_TBB)-10, 0, 255);
                    Modify_SPI_Reg_bits(RCAL_LPFH_TBB, R);
                }
                else
                    targetLevelNotReached = false;
                ccal_lpflad_tbb = clamp(++ccal_lpflad_tbb, 0, 31);
                Modify_SPI_Reg_bits(CCAL_LPFLAD_TBB, ccal_lpflad_tbb);
            }
            --iterationsLeft;
            {
            const uint8_t R = Get_SPI_Reg_bits(RCAL_LPFH_TBB);
            if (R==0 || R==255)
                break;
            }
        }
        while(targetLevelNotReached && iterationsLeft>=0);
    }
    {
        uint8_t rcal_lpflad_tbb = Get_SPI_Reg_bits(RCAL_LPFLAD_TBB);
        uint16_t ccal_lpflad_tbb = Get_SPI_Reg_bits(CCAL_LPFLAD_TBB);
        uint16_t rcal_lpfh_tbb = Get_SPI_Reg_bits(RCAL_LPFH_TBB);
        RestoreChipState();
        SPI_write(0x0020, ch);
        SPI_write(0x0106, 0x318C);
        SPI_write(0x0107, 0x318C);
        Modify_SPI_Reg_bits(CCAL_LPFLAD_TBB, ccal_lpflad_tbb);
        if(tx_lpf_IF <= TxLPF_RF_LimitLowMid/2)
        {
            Modify_SPI_Reg_bits(PD_LPFH_TBB, 1);
            Modify_SPI_Reg_bits(PD_LPFLAD_TBB, 0);
            Modify_SPI_Reg_bits(PD_LPFS5_TBB, 0);
            Modify_SPI_Reg_bits(R5_LPF_BYP_TBB, 1);
            Modify_SPI_Reg_bits(RCAL_LPFLAD_TBB, rcal_lpflad_tbb);
        }
        else
        {
            Modify_SPI_Reg_bits(PD_LPFH_TBB, 0);
            Modify_SPI_Reg_bits(PD_LPFLAD_TBB, 1);
            Modify_SPI_Reg_bits(PD_LPFS5_TBB, 1);
            Modify_SPI_Reg_bits(RCAL_LPFH_TBB, rcal_lpfh_tbb);
        }
    }

    return 0;
}
