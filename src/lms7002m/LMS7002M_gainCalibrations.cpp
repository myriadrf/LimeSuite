#include "LMS7002M.h"
#include "Logger.h"
using namespace lime;

int LMS7002M::CalibrateTxGainSetup()
{
    int status;
    int ch = Get_SPI_Reg_bits(LMS7param(MAC));

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
    Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(LOOPB_TBB), 3);

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
    status = SetFrequencyCGEN(61.44e6);
    if(status != 0)
        return status;

    //SXR
    Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    Modify_SPI_Reg_bits(LMS7param(PD_VCO), 1);

    Modify_SPI_Reg_bits(LMS7param(MAC), ch);

    //TxTSP
    const int isinc = Get_SPI_Reg_bits(LMS7param(ISINC_BYP_TXTSP));
    const int txcmixGainLSB = Get_SPI_Reg_bits(LMS7param(CMIX_GAIN_TXTSP));
    const int txcmixGainMSB = Get_SPI_Reg_bits(LMS7param(CMIX_GAIN_TXTSP_R3));
    SetDefaults(TxTSP);
    SetDefaults(TxNCO);
    Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_TXTSP), txcmixGainLSB);
    Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_TXTSP_R3), txcmixGainMSB);
    Modify_SPI_Reg_bits(LMS7param(ISINC_BYP_TXTSP), isinc);
    Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), 1);
    int16_t tsgValue = 0x7FFF;
    if(txcmixGainMSB == 0 && txcmixGainLSB == 1)
        tsgValue = 0x3FFF;
    else if(txcmixGainMSB == 1 && txcmixGainLSB == 0)
        tsgValue = 0x5A85;
    else
        tsgValue = 0x7FFF;
    LoadDC_REG_IQ(LMS7002M::Tx, tsgValue , tsgValue);
    SetNCOFrequency(LMS7002M::Tx, 0, 0.5e6);

    //RxTSP
    SetDefaults(RxTSP);
    SetDefaults(RxNCO);
    Modify_SPI_Reg_bits(LMS7param(AGC_MODE_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(AGC_AVG_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);
    return 0;
}

int LMS7002M::CalibrateTxGain(float maxGainOffset_dBFS, float *actualGain_dBFS)
{
    if (!controlPort){
        lime::error("No device connected");
        return -1;
    }
    int status;
    int cg_iamp;
    auto registersBackup = BackupRegisterMap();
    status = CalibrateTxGainSetup();
    if(status == 0)
    {
        uint32_t rssi = GetRSSI();
        cg_iamp = Get_SPI_Reg_bits(LMS7param(CG_IAMP_TBB));
        while(rssi < 0x7FFF && cg_iamp <= 63)
        {
            ++cg_iamp;
            if(cg_iamp > 63)
                break;
            Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), cg_iamp);
            rssi = GetRSSI();
        }
    }
    RestoreRegisterMap(registersBackup);
    if (status == 0)
        Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), cg_iamp-1);
    //logic reset
    Modify_SPI_Reg_bits(LMS7param(LRST_TX_A), 0);
    Modify_SPI_Reg_bits(LMS7param(LRST_TX_B), 0);
    Modify_SPI_Reg_bits(LMS7param(LRST_TX_A), 1);
    Modify_SPI_Reg_bits(LMS7param(LRST_TX_B), 1);

    return status;
}
