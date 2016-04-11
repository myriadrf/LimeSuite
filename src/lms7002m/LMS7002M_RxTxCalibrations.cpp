#include "LMS7002M.h"
#include "ErrorReporting.h"
#include <assert.h>
#include "MCU_BD.h"
#include "IConnection.h"
#include "mcu_programs.h"

//#define RSSI_FROM_MCU
#define LMS_VERBOSE_OUTPUT

///define for parameter enumeration if prefix might be needed
#define LMS7param(id) id

using namespace lime;

const float_type CGEN_FREQ_CALIBRATIONS = 368.64;
float_type calibUserBwDivider = 5;
const static uint16_t MCU_PARAMETER_ADDRESS = 0x002D; //register used to pass parameter values to MCU
#define MCU_ID_DC_IQ_CALIBRATIONS 0x01
#define MCU_ID_DC_IQ_CALIBRATIONS_TDD 0x02
#define MCU_FUNCTION_CALIBRATE_TX 1
#define MCU_FUNCTION_CALIBRATE_RX 2
#define MCU_FUNCTION_READ_RSSI 3

const int16_t firCoefs[] =
{
	-15,
	-14,
	-11,
	-6,
	0,
	7,
	15,
	23,
	30,
	35,
	37,
	34,
	26,
	11,
	-8,
	-32,
	-57,
	-81,
	-99,
	-109,
	-106,
	-89,
	-57,
	-11,
	47,
	110,
	172,
	224,
	259,
	267,
	244,
	187,
	95,
	-26,
	-167,
	-315,
	-452,
	-562,
	-624,
	-623,
	-543,
	-378,
	-125,
	211,
	617,
	1075,
	1558,
	2038,
	2484,
	2869,
	3163,
	3349,
	3411,
	3349,
	3163,
	2869,
	2484,
	2038,
	1558,
	1075,
	617,
	211,
	-125,
	-378,
	-543,
	-623,
	-624,
	-562,
	-452,
	-315,
	-167,
	-26,
	95,
	187,
	244,
	267,
	259,
	224,
	172,
	110,
	47,
	-11,
	-57,
	-89,
	-106,
	-109,
	-99,
	-81,
	-57,
	-32,
	-8,
	11,
	26,
	34,
	37,
	35,
	30,
	23,
	15,
	7,
	0,
	-6,
	-11,
	-14,
	-15
};

const uint16_t backupAddrs[] = {
0x0020, 0x0081, 0x0082, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088,
0x0089, 0x008A, 0x008B, 0x008C, 0x0100, 0x0101, 0x0102, 0x0103,
0x0104, 0x0105, 0x0106, 0x0107, 0x0108, 0x0109, 0x010A, 0x010C,
0x010D, 0x010E, 0x010F, 0x0110, 0x0111, 0x0112, 0x0113, 0x0114,
0x0115, 0x0116, 0x0117, 0x0118, 0x0119, 0x011A, 0x0200, 0x0201,
0x0202, 0x0203, 0x0204, 0x0205, 0x0206, 0x0207, 0x0208, 0x0209,
0x020A, 0x020B, 0x020C, 0x0242, 0x0243, 0x0400, 0x0401, 0x0402,
0x0403, 0x0404, 0x0405, 0x0406, 0x0407, 0x0408, 0x0409, 0x040A,
0x040B, 0x040C, 0x040D, 0x0442, 0x0443
};
uint16_t backupRegs[sizeof(backupAddrs) / 2];
const uint16_t backupSXAddr[] = { 0x011C, 0x011D, 0x011E, 0x011F, 0x01200, 0x0121, 0x0122, 0x0123, 0x0124 };
uint16_t backupRegsSXR[sizeof(backupSXAddr) / 2];
uint16_t backupRegsSXT[sizeof(backupSXAddr) / 2];
int16_t rxGFIR3_backup[sizeof(firCoefs)];
uint16_t backup0x010D;
uint16_t backup0x0100;

inline uint16_t pow2(const uint8_t power)
{
    assert(power >= 0 && power < 16);
    return 1 << power;
}

bool sign(const int number)
{
	return number < 0;
}

/** @brief Parameters setup instructions for Tx calibration
    @return 0-success, other-failure
*/
int LMS7002M::CalibrateTxSetup(float_type bandwidth_MHz)
{
	//Stage 2
	Channel ch = this->GetActiveChannel();
	uint8_t sel_band1_trf = (uint8_t)Get_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF));
	uint8_t sel_band2_trf = (uint8_t)Get_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF));

	//rfe
	//reset RFE to defaults
	SetDefaults(RFE);
	if (sel_band1_trf == 1)
		Modify_SPI_Reg_bits(LMS7param(SEL_PATH_RFE), 3); //SEL_PATH_RFE 3
	else if (sel_band2_trf == 1)
		Modify_SPI_Reg_bits(LMS7param(SEL_PATH_RFE), 2);
	else
		return ReportError("CalibrateTxSetup() - TRF band not selected");

	Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), 7);
	Modify_SPI_Reg_bits(0x010C, 4, 3, 0); //PD_MXLOBUF_RFE 0, PD_QGEN_RFE 0
	Modify_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE), 4);
	Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), 50);
	Modify_SPI_Reg_bits(LMS7param(ICT_LODC_RFE), 31); //ICT_LODC_RFE 31
	if (sel_band1_trf)
	{
		Modify_SPI_Reg_bits(LMS7param(PD_RLOOPB_1_RFE), 0);
		Modify_SPI_Reg_bits(LMS7param(PD_RLOOPB_2_RFE), 1);
		Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_LB1_RFE), 0);
		Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_LB2_RFE), 1);
	}
	else
	{
		Modify_SPI_Reg_bits(LMS7param(PD_RLOOPB_1_RFE), 1);
		Modify_SPI_Reg_bits(LMS7param(PD_RLOOPB_2_RFE), 0);
		Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_LB1_RFE), 1);
		Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_LB2_RFE), 0);
	}
	Modify_SPI_Reg_bits(LMS7param(EN_DCOFF_RXFE_RFE), 1);

	//RBB
	//reset RBB to defaults
	SetDefaults(RBB);
	Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), 1);
	Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), 12);
	Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB), 2);

	//TRF
	Modify_SPI_Reg_bits(LMS7param(L_LOOPB_TXPAD_TRF), 0); //L_LOOPB_TXPAD_TRF 0
	Modify_SPI_Reg_bits(LMS7param(EN_LOOPB_TXPAD_TRF), 1); //EN_LOOPB_TXPAD_TRF 1

	//AFE
	Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0); //PD_RX_AFE2 0

    //BIAS
    uint16_t backup = Get_SPI_Reg_bits(LMS7param(RP_CALIB_BIAS));
    SetDefaults(BIAS);
    Modify_SPI_Reg_bits(LMS7param(RP_CALIB_BIAS), backup); //RP_CALIB_BIAS

    //XBUF
    Modify_SPI_Reg_bits(0x0085, 2, 0, 1); //PD_XBUF_RX 0, PD_XBUF_TX 0, EN_G_XBUF 1

    //CGEN
    //reset CGEN to defaults
    SetDefaults(CGEN);
    //power up VCO
    Modify_SPI_Reg_bits(LMS7param(PD_VCO_CGEN), 0);

    int status = SetFrequencyCGEN(CGEN_FREQ_CALIBRATIONS);
    if (status != 0) return status;
    status = TuneVCO(VCO_CGEN);
    if (status != 0) return status;

    //SXR
    this->SetActiveChannel(ChA);
    SetDefaults(SX);
    Modify_SPI_Reg_bits(LMS7param(PD_VCO), 0);
    {
        float_type SXTfreqMHz = GetFrequencySX_MHz(Tx);
        float_type SXRfreqMHz = SXTfreqMHz - bandwidth_MHz / calibUserBwDivider - 1;
        status = SetFrequencySX(Rx, SXRfreqMHz);
        if (status != 0) return status;
        status = TuneVCO(VCO_SXR);
        if (status != 0) return status;
    }

    //SXT
    this->SetActiveChannel(ChB);
    Modify_SPI_Reg_bits(PD_LOCH_T2RBUF, 1);
    this->SetActiveChannel(ch);

    //TXTSP
    SetDefaults(TxTSP);
    SetDefaults(TxNCO);
    Modify_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), 1);
	Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), 1);
    Modify_SPI_Reg_bits(0x0208, 6, 4, 0x7); //GFIR3_BYP 1, GFIR2_BYP 1, GFIR1_BYP 1
	Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_TXTSP), 0);
    LoadDC_REG_IQ(Tx, (int16_t)0x7FFF, (int16_t)0x8000);

    SetNCOFrequency(Tx, 0, bandwidth_MHz / calibUserBwDivider);

    //RXTSP
    SetDefaults(RxTSP);
    SetDefaults(RxNCO);
    Modify_SPI_Reg_bits(LMS7param(AGC_MODE_RXTSP), 1);
	Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);
	Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), 1);
	Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), 4); //Decimation HBD ratio
	Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(AGC_AVG_RXTSP), 0x1);
	Modify_SPI_Reg_bits(LMS7param(GFIR3_L_RXTSP), 7);
	Modify_SPI_Reg_bits(LMS7param(GFIR3_N_RXTSP), 31);

    SetGFIRCoefficients(Rx, 2, firCoefs, sizeof(firCoefs) / sizeof(int16_t));

	Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP), 0);

	if (ch == ChB)
	{
		this->SetActiveChannel(ChA);
		Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0);
		Modify_SPI_Reg_bits(LMS7param(EN_NEXTRX_RFE), 1); // EN_NEXTTX_RFE 1
		Modify_SPI_Reg_bits(LMS7param(EN_NEXTTX_TRF), 1); //EN_NEXTTX_TRF 1
		this->SetActiveChannel(ch);
	}

    return 0;
}

/** @brief Flips the CAPTURE bit and returns digital RSSI value
*/
uint32_t LMS7002M::GetRSSI()
{
#ifndef RSSI_FROM_MCU

    //bulk write to trigger the CAPTURE register off and on
    uint16_t data[2];
    uint16_t addrsWr[] = {0x0400, 0x0400};
    int reg0x0400 = SPI_read(0x0400, false);
    data[0] = reg0x0400 & ~(1 << 15);
    data[1] = reg0x0400 | (1 << 15);
    SPI_write_batch(addrsWr, data, 2);

    //bulk read to get both high and low parts of the RSSI
    uint16_t addrsRd[] = {0x040F, 0x040E};
    SPI_read_batch(addrsRd, data, 2);
    return (data[0] << 2) | data[1];
#else
    mcuControl->CallMCU(MCU_FUNCTION_READ_RSSI);
    int status = mcuControl->WaitForMCU(500);
    if (status == 0)
    {
        printf("MCU working too long\n");
    }

    mcuControl->DebugModeSet_MCU(1, 0);
    //read result value from MCU RAM
    unsigned char tempc1, tempc2, tempc3 = 0x00;

    uint8_t ramData[3];
    for (int i = 0; i < 3; ++i)
    {
        const uint8_t asm_read_iram = 0x78;
        const uint8_t rssi_iram_addr = 0xF9;
        int retval = mcuControl->Three_byte_command(asm_read_iram, ((unsigned char)(rssi_iram_addr + i)), 0x00, &tempc1, &tempc2, &tempc3);
        if (retval == 0)
            ramData[i] = tempc3;
        else
        {
            return ~0;
        }
    }
    uint32_t rssiAvg = 0;
    rssiAvg |= (ramData[0] & 0xFF) << 16;
    rssiAvg |= (ramData[1] & 0xFF) << 8;
    rssiAvg |= (ramData[2] & 0xFF);
    mcuControl->DebugModeExit_MCU(1, 0);
    return rssiAvg;
#endif
}

/** @brief Calibrates Transmitter. DC correction, IQ gains, IQ phase correction
    @return 0-success, other-failure
*/
int LMS7002M::CalibrateTx(float_type bandwidth_MHz)
{
    Channel ch = this->GetActiveChannel();
    uint8_t sel_band1_trf = (uint8_t)Get_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF));
    uint8_t sel_band2_trf = (uint8_t)Get_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF));

    uint32_t boardId = controlPort->GetDeviceInfo().boardSerialNumber;
    double txFreq = GetFrequencySX_MHz(Tx);
    uint8_t channel = ch==1 ? 0 : 1;
    bool foundInCache = false;
    int band = sel_band1_trf ? 0 : 1;

    uint16_t gainAddr;
    uint16_t gcorri;
    uint16_t gcorrq;
    uint16_t dccorri;
    uint16_t dccorrq;
    int16_t phaseOffset;
    int16_t gain = 1983;
    const uint16_t gainMSB = 10;
    const uint16_t gainLSB = 0;

    if(useCache)
    {
        int dcI, dcQ, gainI, gainQ, phOffset;

        foundInCache = (valueCache.GetDC_IQ(boardId, txFreq*1e6, channel, true, band, &dcI, &dcQ, &gainI, &gainQ, &phOffset) == 0);
        if(foundInCache)
        {
            printf("Tx calibration: using cached values\n");
            dccorri = dcI;
            dccorrq = dcQ;
            gcorri = gainI;
            gcorrq = gainQ;
            phaseOffset = phOffset;
            Modify_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP), dcI);
            Modify_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP), dcQ);
            Modify_SPI_Reg_bits(LMS7param(GCORRI_TXTSP), gainI);
            Modify_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP), gainQ);
            Modify_SPI_Reg_bits(LMS7param(IQCORR_TXTSP), phaseOffset);
            Modify_SPI_Reg_bits(LMS7param(DC_BYP_TXTSP), 0); //DC_BYP
            Modify_SPI_Reg_bits(0x0208, 1, 0, 0); //GC_BYP PH_BYP
            return 0;
        }
    }

    LMS7002M_SelfCalState state(this);

    uint8_t mcuID = mcuControl->ReadMCUProgramID();
    if (mcuID != MCU_ID_DC_IQ_CALIBRATIONS)
    {
        int st = mcuControl->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, MCU_BD::SRAM);
        if (st != 0) return st;
    }

    int status;
    Log("Tx calibration started", LOG_INFO);
    BackupAllRegisters();
    Log("Setup stage", LOG_INFO);
    status = CalibrateTxSetup(bandwidth_MHz);
    if (status != 0)
        goto TxCalibrationEnd; //go to ending stage to restore registers
    if (mCalibrationByMCU)
    {
    //set bandwidth for MCU to read from register, value is integer stored in MHz
    SPI_write(MCU_PARAMETER_ADDRESS, (uint16_t)bandwidth_MHz);
    mcuControl->CallMCU(MCU_FUNCTION_CALIBRATE_TX);
    auto statusMcu = mcuControl->WaitForMCU(30000);
    if (statusMcu == 0)
    {
        printf("MCU working too long %i\n", statusMcu);
    }
    }
    else
    {
    CheckSaturationTxRx(bandwidth_MHz);

    Modify_SPI_Reg_bits(EN_G_TRF, 0);

    CalibrateRxDC_RSSI();
    CalibrateTxDC_RSSI(bandwidth_MHz);

    //TXIQ
    Modify_SPI_Reg_bits(EN_G_TRF, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_TXTSP, 0);

    SetNCOFrequency(LMS7002M::Rx, 0, 0.9);

    //coarse gain
    {
        uint32_t rssiIgain;
        uint32_t rssiQgain;
        Modify_SPI_Reg_bits(GCORRI_TXTSP, 2047 - 64);
        Modify_SPI_Reg_bits(GCORRQ_TXTSP, 2047);
        rssiIgain = GetRSSI();
        Modify_SPI_Reg_bits(GCORRI_TXTSP, 2047);
        Modify_SPI_Reg_bits(GCORRQ_TXTSP, 2047 - 64);
        rssiQgain = GetRSSI();

        Modify_SPI_Reg_bits(GCORRI_TXTSP, 2047);
        Modify_SPI_Reg_bits(GCORRQ_TXTSP, 2047);

        if (rssiIgain < rssiQgain)
            gainAddr = GCORRI_TXTSP.address;
        else
            gainAddr = GCORRQ_TXTSP.address;
    }
    CoarseSearch(gainAddr, gainMSB, gainLSB, gain, 7);
#ifdef LMS_VERBOSE_OUTPUT
    printf("Coarse search Tx GAIN_%s: %i\n", gainAddr == GCORRI_TXTSP.address ? "I" : "Q", gain);
#endif

    //coarse phase offset
    {
        uint32_t rssiUp;
        uint32_t rssiDown;
        Modify_SPI_Reg_bits(IQCORR_TXTSP, 15);
        rssiUp = GetRSSI();
        Modify_SPI_Reg_bits(IQCORR_TXTSP, -15);
        rssiDown = GetRSSI();
        if (rssiUp > rssiDown)
            phaseOffset = -64;
        else if (rssiUp < rssiDown)
            phaseOffset = 192;
        else
            phaseOffset = 64;
    }
    Modify_SPI_Reg_bits(IQCORR_TXTSP, phaseOffset);
    CoarseSearch(IQCORR_TXTSP.address, IQCORR_TXTSP.msb, IQCORR_TXTSP.lsb, phaseOffset, 7);
#ifdef LMS_VERBOSE_OUTPUT
    printf("Coarse search Tx IQCORR: %i\n", phaseOffset);
#endif

    CoarseSearch(gainAddr, gainMSB, gainLSB, gain, 4);
#ifdef LMS_VERBOSE_OUTPUT
    printf("Coarse search Tx GAIN_%s: %i\n", gainAddr == GCORRI_TXTSP.address ? "I" : "Q", gain);
#endif

#ifdef LMS_VERBOSE_OUTPUT
    printf("Fine search Tx GAIN_%s/IQCORR...\n", gainAddr == GCORRI_TXTSP.address ? "I" : "Q");
#endif
    FineSearch(gainAddr, gainMSB, gainLSB, gain, IQCORR_TXTSP.address, IQCORR_TXTSP.msb, IQCORR_TXTSP.lsb, phaseOffset, 7);
#ifdef LMS_VERBOSE_OUTPUT
    printf("Fine search Tx GAIN_%s: %i, IQCORR: %i\n", gainAddr == GCORRI_TXTSP.address ? "I" : "Q", gain, phaseOffset);
#endif
    Modify_SPI_Reg_bits(gainAddr, gainMSB, gainLSB, gain);
    Modify_SPI_Reg_bits(IQCORR_TXTSP.address, IQCORR_TXTSP.msb, IQCORR_TXTSP.lsb, phaseOffset);
    }
	dccorri = Get_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP));
    dccorrq = Get_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP));
    gcorri = Get_SPI_Reg_bits(LMS7param(GCORRI_TXTSP));
    gcorrq = Get_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP));
    phaseOffset = Get_SPI_Reg_bits(LMS7param(IQCORR_TXTSP));

TxCalibrationEnd:
    Log("Restoring registers state", LOG_INFO);
    this->SetActiveChannel(ch);
    RestoreAllRegisters();
    if (status != 0)
    {
        Log("Tx calibration failed", LOG_WARNING);
        return status;
    }

    if(useCache)
        valueCache.InsertDC_IQ(boardId, txFreq*1e6, channel, true, band, dccorri, dccorrq, gcorri, gcorrq, phaseOffset);


    this->SetActiveChannel(ch);
    Modify_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP), dccorri);
    Modify_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP), dccorrq);
    Modify_SPI_Reg_bits(LMS7param(GCORRI_TXTSP), gcorri);
    Modify_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP), gcorrq);
    Modify_SPI_Reg_bits(LMS7param(IQCORR_TXTSP), phaseOffset);

    Modify_SPI_Reg_bits(LMS7param(DC_BYP_TXTSP), 0); //DC_BYP
    Modify_SPI_Reg_bits(0x0208, 1, 0, 0); //GC_BYP PH_BYP
    Log("Tx calibration finished", LOG_INFO);
    return 0;
}

/** @brief Performs Rx DC offsets calibration
*/
void LMS7002M::CalibrateRxDC_RSSI()
{
    int16_t offsetI = 32;
    int16_t offsetQ = 32;
    Modify_SPI_Reg_bits(DC_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(CAPSEL, 0);
    SetRxDCOFF(offsetI, offsetQ);
    //find I
    CoarseSearch(DCOFFI_RFE.address, DCOFFI_RFE.msb, DCOFFI_RFE.lsb, offsetI, 6);

    //find Q
    CoarseSearch(DCOFFQ_RFE.address, DCOFFQ_RFE.msb, DCOFFQ_RFE.lsb, offsetQ, 6);

    CoarseSearch(DCOFFI_RFE.address, DCOFFI_RFE.msb, DCOFFI_RFE.lsb, offsetI, 3);
    CoarseSearch(DCOFFQ_RFE.address, DCOFFQ_RFE.msb, DCOFFQ_RFE.lsb, offsetQ, 3);
#ifdef LMS_VERBOSE_OUTPUT
    printf("Fine search Rx DCOFFI/DCOFFQ\n");
#endif
	FineSearch(DCOFFI_RFE.address, DCOFFI_RFE.msb, DCOFFI_RFE.lsb, offsetI, DCOFFQ_RFE.address, DCOFFQ_RFE.msb, DCOFFQ_RFE.lsb, offsetQ, 5);
#ifdef LMS_VERBOSE_OUTPUT
    printf("Fine search Rx DCOFFI: %i, DCOFFQ: %i\n", offsetI, offsetQ);
#endif
	SetRxDCOFF(offsetI, offsetQ);
    Modify_SPI_Reg_bits(DC_BYP_RXTSP, 0); // DC_BYP 0
}

/** @brief Tries to detect and fix gains if Rx is saturated
    @return 0-success, other-failure
*/
int LMS7002M::FixRXSaturation()
{
    uint8_t g_rxloopb = 0;
    Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), g_rxloopb);
    Modify_SPI_Reg_bits(LMS7param(L_LOOPB_TXPAD_TRF), 3);

    int8_t lLoopb = 3;
    const uint32_t rssi_saturation_level = 0xD000;
    while (g_rxloopb < 15)
    {
        g_rxloopb += 1;
        Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), g_rxloopb);
        Modify_SPI_Reg_bits(LMS7param(L_LOOPB_TXPAD_TRF), 3);
        if (GetRSSI() < rssi_saturation_level)
        {
            for (lLoopb = 3; lLoopb >= 0; --lLoopb)
            {
                Modify_SPI_Reg_bits(LMS7param(L_LOOPB_TXPAD_TRF), lLoopb);
                if (GetRSSI() > rssi_saturation_level)
                {
                    ++lLoopb;
                    Modify_SPI_Reg_bits(LMS7param(L_LOOPB_TXPAD_TRF), lLoopb);
                    goto finished;
                }
            }
        }
        else
        {
            g_rxloopb -= 1;
            Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), g_rxloopb);
            break;
        }
    }
finished:
    if (GetRSSI() < rssi_saturation_level) return 0;
    return ReportError("FixRXSaturation() - failed");
}

/** @brief Parameters setup instructions for Rx calibration
    @param bandwidth_MHz filter bandwidth in MHz
    @return 0-success, other-failure
*/
int LMS7002M::CalibrateRxSetup(const float_type bandwidth_MHz, const bool TDD)
{
    Channel ch = this->GetActiveChannel();

    //rfe
    Modify_SPI_Reg_bits(LMS7param(EN_DCOFF_RXFE_RFE), 1);
    Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), 3);
    Modify_SPI_Reg_bits(0x010C, 4, 3, 0); //PD_MXLOBUF_RFE 0, PD_QGEN_RFE 0
    Modify_SPI_Reg_bits(0x010C, 1, 1, 0); //PD_TIA 0
    Modify_SPI_Reg_bits(0x0110, 4, 0, 31); //ICT_LO_RFE 31

    //RBB
    Modify_SPI_Reg_bits(0x0115, 15, 14, 0); //Loopback switches disable
    Modify_SPI_Reg_bits(0x0119, 15, 15, 0); //OSW_PGA 0

    //TRF
    //reset TRF to defaults
    SetDefaults(TRF);
    Modify_SPI_Reg_bits(L_LOOPB_TXPAD_TRF, 0);
    Modify_SPI_Reg_bits(EN_LOOPB_TXPAD_TRF, 1);
    Modify_SPI_Reg_bits(EN_G_TRF, 0);

    {
        uint8_t selPath = Get_SPI_Reg_bits(SEL_PATH_RFE);
        if (selPath == 2)
        {
            Modify_SPI_Reg_bits(SEL_BAND2_TRF, 1);
            Modify_SPI_Reg_bits(SEL_BAND1_TRF, 0);
        }
        else if (selPath == 3)
        {
            Modify_SPI_Reg_bits(SEL_BAND2_TRF, 0);
            Modify_SPI_Reg_bits(SEL_BAND1_TRF, 1);
        }
        else
            return ReportError("CalibrateRxSetup() - SEL_PATH_RFE must be LNAL or LNAW"); //todo restore settings
    }

    //TBB
    //reset TBB to defaults
    SetDefaults(TBB);
    Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), 2);
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_FRP_TBB), 1); //ICT_IAMP_FRP_TBB 1
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_GG_FRP_TBB), 6); //ICT_IAMP_GG_FRP_TBB 6

    //AFE
    Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0); //PD_RX_AFE2

    //BIAS
    {
        uint16_t rp_calib_bias = Get_SPI_Reg_bits(0x0084, 10, 6);
        SetDefaults(BIAS);
        Modify_SPI_Reg_bits(0x0084, 10, 6, rp_calib_bias); //RP_CALIB_BIAS
    }

    //XBUF
    Modify_SPI_Reg_bits(0x0085, 2, 0, 1); //PD_XBUF_RX 0, PD_XBUF_TX 0, EN_G_XBUF 1

    //CGEN
    //reset CGEN to defaults
    SetDefaults(CGEN);
    //power up VCO
    Modify_SPI_Reg_bits(0x0086, 2, 2, 0);

    int status = SetFrequencyCGEN(CGEN_FREQ_CALIBRATIONS);
    if (status != 0) return status;

    if (TDD == false) //in TDD do nothing
    {
        //SXR
        this->SetActiveChannel(ChA);
        float_type SXRfreqMHz = GetFrequencySX_MHz(Rx);

        //SXT
        this->SetActiveChannel(ChB);
        SetDefaults(SX);
        Modify_SPI_Reg_bits(LMS7param(PD_VCO), 0);
        status = SetFrequencySX(Tx, SXRfreqMHz + bandwidth_MHz / calibUserBwDivider);
        if (status != 0) return status;
        this->SetActiveChannel(ch);
    }

    //TXTSP
    SetDefaults(TxTSP);
    SetDefaults(TxNCO);
    Modify_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), 1);
	Modify_SPI_Reg_bits(CMIX_BYP_TXTSP, 1);
    Modify_SPI_Reg_bits(TSGMODE_TXTSP, 0x1); //TSGMODE 1
	Modify_SPI_Reg_bits(INSEL_TXTSP, 1);
    Modify_SPI_Reg_bits(0x0208, 6, 4, 0x7); //GFIR3_BYP 1, GFIR2_BYP 1, GFIR1_BYP 1
	Modify_SPI_Reg_bits(CMIX_GAIN_TXTSP, 0);
    LoadDC_REG_IQ(Tx, (int16_t)0x7FFF, (int16_t)0x8000);
    SetNCOFrequency(Tx, 0, 0);

    //RXTSP
    SetDefaults(RxTSP);
    SetDefaults(RxNCO);
    Modify_SPI_Reg_bits(LMS7param(AGC_MODE_RXTSP), 1); //AGC_MODE 1
    Modify_SPI_Reg_bits(0x040C, 7, 7, 0x1); //CMIX_BYP 1
    Modify_SPI_Reg_bits(0x040C, 4, 4, 1); //
    Modify_SPI_Reg_bits(0x040C, 3, 3, 1); //

    Modify_SPI_Reg_bits(LMS7param(CAPSEL), 0);
    Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), 4);
    Modify_SPI_Reg_bits(LMS7param(AGC_AVG_RXTSP), 1); //agc_avg iq corr
    Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_RXTSP), 0);
    Modify_SPI_Reg_bits(LMS7param(GFIR3_L_RXTSP), 7);
    Modify_SPI_Reg_bits(LMS7param(GFIR3_N_RXTSP), 31);

    SetGFIRCoefficients(Rx, 2, firCoefs, sizeof(firCoefs) / sizeof(int16_t));

    //modifications when calibrating channel B
    if (ch == ChB)
    {
		this->SetActiveChannel(ChA);
        Modify_SPI_Reg_bits(LMS7param(EN_NEXTRX_RFE), 1);
        Modify_SPI_Reg_bits(EN_NEXTTX_TRF, 1);
        Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0);
		this->SetActiveChannel(ch);
    }
    return 0;
}

/** @brief Calibrates Receiver. DC offset, IQ gains, IQ phase correction
    @return 0-success, other-failure
*/
int LMS7002M::CalibrateRx(float_type bandwidth_MHz, const bool TDD)
{
    Channel ch = this->GetActiveChannel();
    uint32_t boardId = controlPort->GetDeviceInfo().boardSerialNumber;
    uint8_t channel = ch == 1 ? 0 : 1;
    uint8_t sel_path_rfe = (uint8_t)Get_SPI_Reg_bits(LMS7param(SEL_PATH_RFE));
    int lna = sel_path_rfe;

    int status;
	int16_t iqcorr_rx = 0;
    int16_t dcoffi;
	int16_t dcoffq;
    int16_t gain;
    uint16_t gainAddr = 0;
    const uint8_t gainMSB = 10;
    const uint8_t gainLSB = 0;
    uint16_t mingcorri;
    uint16_t mingcorrq;
    int16_t phaseOffset;

    double rxFreq = GetFrequencySX_MHz(Rx);
    bool foundInCache = false;
    if(useCache)
    {
        int dcI, dcQ, gainI, gainQ, phOffset;
        foundInCache = (valueCache.GetDC_IQ(boardId, rxFreq*1e6, channel, false, lna, &dcI, &dcQ, &gainI, &gainQ, &phOffset) == 0);
        dcoffi = dcI;
        dcoffq = dcQ;
        mingcorri = gainI;
        mingcorrq = gainQ;
        phaseOffset = phOffset;
        if(foundInCache)
        {
            printf("Rx calibration: using cached values\n");
            SetRxDCOFF(dcoffi, dcoffq);
            Modify_SPI_Reg_bits(LMS7param(EN_DCOFF_RXFE_RFE), 1);
            Modify_SPI_Reg_bits(LMS7param(GCORRI_RXTSP), gainI);
            Modify_SPI_Reg_bits(LMS7param(GCORRQ_RXTSP), gainQ);
            Modify_SPI_Reg_bits(LMS7param(IQCORR_RXTSP), phaseOffset);
            Modify_SPI_Reg_bits(0x040C, 2, 0, 0); //DC_BYP 0, GC_BYP 0, PH_BYP 0
            Modify_SPI_Reg_bits(0x0110, 4, 0, 31); //ICT_LO_RFE 31
            return 0;
        }
    }
    LMS7002M_SelfCalState state(this);

    uint8_t mcuID = mcuControl->ReadMCUProgramID();
    if (TDD)
    {
        if (mcuID != MCU_ID_DC_IQ_CALIBRATIONS_TDD)
        {
            int st = mcuControl->Program_MCU(mcu_program_lms7_dc_iq_calibration_RxTDD_bin, MCU_BD::SRAM);
            if (st != 0) return st;
        }
    }
    else
    {
        if (mcuID != MCU_ID_DC_IQ_CALIBRATIONS)
        {
            int st = mcuControl->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, MCU_BD::SRAM);
            if (st != 0) return st;
        }
    }

	Log("Rx calibration started", LOG_INFO);
	Log("Saving registers state", LOG_INFO);
    BackupAllRegisters();
    if (sel_path_rfe == 1 || sel_path_rfe == 0)
        return ReportError("CalibrateRx() - SEL_PATH_RFE must be LNAH or LNAW");

	Log("Setup stage", LOG_INFO);
    status = CalibrateRxSetup(bandwidth_MHz, TDD);
	if (status != 0)
		goto RxCalibrationEndStage;

    if (mCalibrationByMCU)
    {
    //set bandwidth for MCU to read from register, value is integer stored in MHz
    SPI_write(MCU_PARAMETER_ADDRESS, (uint16_t)bandwidth_MHz);
    mcuControl->CallMCU(MCU_FUNCTION_CALIBRATE_RX);
    auto statusMcu = mcuControl->WaitForMCU(30000);
    if (statusMcu == 0)
    {
        printf("MCU working too long %i\n", statusMcu);
    }
    }
    else
    {
    Log("Rx DC calibration", LOG_INFO);

    CalibrateRxDC_RSSI();
    Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_RXTSP), 1);

    // RXIQ calibration
    Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), 1);

    if (sel_path_rfe == 2)
    {
        Modify_SPI_Reg_bits(LMS7param(PD_RLOOPB_2_RFE), 0);
        Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_LB2_RFE), 0);
    }
    if (sel_path_rfe == 3)
    {
        Modify_SPI_Reg_bits(LMS7param(PD_RLOOPB_1_RFE), 0);
        Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_LB1_RFE), 0);
    }
    Modify_SPI_Reg_bits(DC_BYP_RXTSP, 0); //DC_BYP 0

    if (TDD == true)
    {
        //SXR
        this->SetActiveChannel(ChA);
        SetDefaults(SX);
        const float SXTfreq_MHz = GetFrequencySX_MHz(Tx);
        int status = SetFrequencySX(Rx, SXTfreq_MHz);

        //SXT
        this->SetActiveChannel(ChB);
        Modify_SPI_Reg_bits(PD_LOCH_T2RBUF, 1);

        status = SetFrequencySX(Tx, SXTfreq_MHz + bandwidth_MHz / calibUserBwDivider);
        this->SetActiveChannel(ch);
    }
    CheckSaturation();

    Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
    {
        const float_type RxFreq = GetFrequencySX_MHz(LMS7002M::Rx);
        const float_type TxFreq = GetFrequencySX_MHz(LMS7002M::Tx);
        SetNCOFrequency(LMS7002M::Rx, 0, TxFreq - RxFreq + 0.1);
    }

    Modify_SPI_Reg_bits(IQCORR_RXTSP, 0);
    Modify_SPI_Reg_bits(GCORRI_RXTSP, 2047);
    Modify_SPI_Reg_bits(GCORRQ_RXTSP, 2047);

    //coarse gain
    {
        Modify_SPI_Reg_bits(GCORRI_RXTSP, 2047 - 64);
        Modify_SPI_Reg_bits(GCORRQ_RXTSP, 2047);
        uint32_t rssiIgain = GetRSSI();
        Modify_SPI_Reg_bits(GCORRI_RXTSP, 2047);
        Modify_SPI_Reg_bits(GCORRQ_RXTSP, 2047 - 64);
        uint32_t rssiQgain = GetRSSI();

        Modify_SPI_Reg_bits(GCORRI_RXTSP, 2047);
        Modify_SPI_Reg_bits(GCORRQ_RXTSP, 2047);

        gain = 1983;
        if (rssiIgain < rssiQgain)
        {
            gainAddr = 0x0402;
            Modify_SPI_Reg_bits(GCORRI_RXTSP, gain);
        }
        else
        {
            gainAddr = 0x0401;
            Modify_SPI_Reg_bits(GCORRQ_RXTSP, gain);
        }
    }

    CoarseSearch(gainAddr, gainMSB, gainLSB, gain, 7);
#ifdef LMS_VERBOSE_OUTPUT
    printf("Coarse search Rx GAIN_%s: %i\n", gainAddr == GCORRI_RXTSP.address ? "I" : "Q", gain);
#endif

    //find phase offset
    {
        uint32_t rssiUp;
        uint32_t rssiDown;
        Modify_SPI_Reg_bits(IQCORR_RXTSP, 15);
        rssiUp = GetRSSI();
        Modify_SPI_Reg_bits(IQCORR_RXTSP, -15);
        rssiDown = GetRSSI();

        if (rssiUp > rssiDown)
            phaseOffset = -64;
        else if (rssiUp < rssiDown)
            phaseOffset = 192;
        else
            phaseOffset = 64;
        Modify_SPI_Reg_bits(IQCORR_RXTSP, phaseOffset);
    }
    CoarseSearch(IQCORR_RXTSP.address, IQCORR_RXTSP.msb, IQCORR_RXTSP.lsb, phaseOffset, 7);
#ifdef LMS_VERBOSE_OUTPUT
    printf("Coarse search Rx IQCORR: %i\n", phaseOffset);
#endif

    CoarseSearch(gainAddr, gainMSB, gainLSB, gain, 4);
#ifdef LMS_VERBOSE_OUTPUT
    printf("Coarse search Rx GAIN_%s: %i\n", gainAddr == GCORRI_RXTSP.address ? "I" : "Q", gain);
#endif

    CoarseSearch(IQCORR_RXTSP.address, IQCORR_RXTSP.msb, IQCORR_RXTSP.lsb, phaseOffset, 4);
#ifdef LMS_VERBOSE_OUTPUT
    printf("Coarse search Rx IQCORR: %i\n", phaseOffset);
#endif

#ifdef LMS_VERBOSE_OUTPUT
    printf("Fine search Rx GAIN_%s/IQCORR\n", gainAddr == GCORRI_RXTSP.address ? "I" : "Q");
#endif
    FineSearch(gainAddr, gainMSB, gainLSB, gain, IQCORR_RXTSP.address, IQCORR_RXTSP.msb, IQCORR_RXTSP.lsb, phaseOffset, 7);
    Modify_SPI_Reg_bits(gainAddr, gainMSB, gainLSB, gain);
    Modify_SPI_Reg_bits(IQCORR_RXTSP.address, IQCORR_RXTSP.msb, IQCORR_RXTSP.lsb, phaseOffset);
#ifdef LMS_VERBOSE_OUTPUT
    printf("Fine search Rx GAIN_%s: %i, IQCORR: %i\n", gainAddr == GCORRI_RXTSP.address ? "I" : "Q", gain, phaseOffset);
#endif

    }
	mingcorri = Get_SPI_Reg_bits(GCORRI_RXTSP);
	mingcorrq = Get_SPI_Reg_bits(GCORRQ_RXTSP);
	dcoffi = Get_SPI_Reg_bits(DCOFFI_RFE);
	dcoffq = Get_SPI_Reg_bits(DCOFFQ_RFE);
    phaseOffset = Get_SPI_Reg_bits(IQCORR_RXTSP);

RxCalibrationEndStage:
    Log("Restoring registers state", LOG_INFO);
    RestoreAllRegisters();
    if (status != 0)
    {
        Log("Rx calibration failed", LOG_WARNING);
        return status;
    }

    if(useCache)
        valueCache.InsertDC_IQ(boardId, rxFreq*1e6, channel, false, lna, dcoffi, dcoffq, mingcorri, mingcorrq, phaseOffset);


    this->SetActiveChannel(ch);
    SetRxDCOFF((int8_t)dcoffi, (int8_t)dcoffq);
    Modify_SPI_Reg_bits(LMS7param(EN_DCOFF_RXFE_RFE), 1);
    Modify_SPI_Reg_bits(LMS7param(GCORRI_RXTSP), mingcorri);
    Modify_SPI_Reg_bits(LMS7param(GCORRQ_RXTSP), mingcorrq);
    Modify_SPI_Reg_bits(LMS7param(IQCORR_RXTSP), phaseOffset);
    Modify_SPI_Reg_bits(0x040C, 2, 0, 0); //DC_BYP 0, GC_BYP 0, PH_BYP 0
    Modify_SPI_Reg_bits(0x0110, 4, 0, 31); //ICT_LO_RFE 31
    Log("Rx calibration finished", LOG_INFO);
    return 0;
}

/** @brief Stores chip current registers state into memory for later restoration
*/
void LMS7002M::BackupAllRegisters()
{
    Channel ch = this->GetActiveChannel();
    SPI_read_batch(backupAddrs, backupRegs, sizeof(backupAddrs) / sizeof(uint16_t));
    this->SetActiveChannel(ChA); // channel A
    SPI_read_batch(backupSXAddr, backupRegsSXR, sizeof(backupRegsSXR) / sizeof(uint16_t));
    //backup GFIR3 coefficients
    GetGFIRCoefficients(LMS7002M::Rx, 2, rxGFIR3_backup, 105);
    //EN_NEXTRX_RFE could be modified in channel A
    backup0x010D = SPI_read(0x010D);
    //EN_NEXTTX_TRF could be modified in channel A
    backup0x0100 = SPI_read(0x0100);
    this->SetActiveChannel(ChB); // channel B
    SPI_read_batch(backupSXAddr, backupRegsSXT, sizeof(backupRegsSXR) / sizeof(uint16_t));
    this->SetActiveChannel(ch);
}

/** @brief Sets chip registers to state that was stored in memory using BackupAllRegisters()
*/
void LMS7002M::RestoreAllRegisters()
{
    Channel ch = this->GetActiveChannel();
    SPI_write_batch(backupAddrs, backupRegs, sizeof(backupAddrs) / sizeof(uint16_t));
    //restore GFIR3
    SetGFIRCoefficients(LMS7002M::Rx, 2, rxGFIR3_backup, 105);
    this->SetActiveChannel(ChA); // channel A
    SPI_write(0x010D, backup0x010D); //restore EN_NEXTRX_RFE
    SPI_write(0x0100, backup0x0100); //restore EN_NEXTTX_TRF
    SPI_write_batch(backupSXAddr, backupRegsSXR, sizeof(backupRegsSXR) / sizeof(uint16_t));
    this->SetActiveChannel(ChB); // channel B
    SPI_write_batch(backupSXAddr, backupRegsSXT, sizeof(backupRegsSXR) / sizeof(uint16_t));
    this->SetActiveChannel(ch);
    //reset Tx logic registers, fixes interpolator
    uint16_t x0020val = SPI_read(0x0020);
    SPI_write(0x0020, x0020val & ~0xA000);
    SPI_write(0x0020, x0020val);
}

int LMS7002M::CheckSaturation()
{
	Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 0);
	Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
	const float_type RxFreq = GetFrequencySX_MHz(LMS7002M::Rx);
	const float_type TxFreq = GetFrequencySX_MHz(LMS7002M::Tx);
	SetNCOFrequency(LMS7002M::Rx, 0, TxFreq - RxFreq - 0.1);

	uint32_t rssi = GetRSSI();

    int g_rxloopb_rfe = Get_SPI_Reg_bits(G_RXLOOPB_RFE);
    while (rssi < 0x0B000 && g_rxloopb_rfe  < 15)
    {
        rssi = GetRSSI();
        if (rssi < 0x0B000)
            g_rxloopb_rfe += 2;
        if (rssi > 0x0B000)
            break;
        Modify_SPI_Reg_bits(G_RXLOOPB_RFE, g_rxloopb_rfe);
    }

	int cg_iamp = Get_SPI_Reg_bits(CG_IAMP_TBB);
	while (rssi < 0x01000 && cg_iamp < 63-6)
	{
		rssi = GetRSSI();
		if (rssi < 0x01000)
			cg_iamp += 4;
		if (rssi > 0x01000)
			break;
		Modify_SPI_Reg_bits(CG_IAMP_TBB, cg_iamp);
	}

	while (rssi < 0x0B000 && cg_iamp < 63-2)
	{
		rssi = GetRSSI();
		if (rssi < 0x0B000)
			cg_iamp += 2;
		Modify_SPI_Reg_bits(CG_IAMP_TBB, cg_iamp);
	}
	return 0;
}

int32_t bin2complementry(const int32_t value, const uint8_t bitCount, bool signedInt)
{
    uint32_t negativemask = 0;
    for (int i = 31; i > bitCount; --i)
    {
        negativemask |= (0x1 << i);
    }
    if (value & (0x1 << bitCount) && signedInt)
    {
        return value | negativemask;
    }
    return value;
}

static uint16_t toDCOffset(int16_t offset)
{
    uint16_t valToSend = 0;
    if (offset < 0)
        valToSend |= 0x40;
    valToSend |= labs(offset);
    return valToSend;
}

void LMS7002M::CoarseSearch(const uint16_t addr, const uint8_t msb, const uint8_t lsb, int16_t &value, const uint8_t maxIterations)
{
    const uint16_t DCOFFaddr = 0x010E;
	uint8_t rssi_counter = 0;
	uint32_t rssiUp;
	uint32_t rssiDown;
    int16_t upval;
    int16_t downval;
    upval = value;
	for (rssi_counter = 0; rssi_counter < maxIterations-1; ++rssi_counter)
	{
		rssiUp = GetRSSI();
        value -= pow2(maxIterations - rssi_counter);
        Modify_SPI_Reg_bits(addr, msb, lsb, addr != DCOFFaddr ? value : toDCOffset(value));
        downval = value;
		rssiDown = GetRSSI();

		if (rssiUp >= rssiDown)
			value += pow2(maxIterations - 2 - rssi_counter);
		else
			value = value + pow2(maxIterations - rssi_counter) + pow2(maxIterations - 1 - rssi_counter) - pow2(maxIterations-2 - rssi_counter);
        Modify_SPI_Reg_bits(addr, msb, lsb, addr != DCOFFaddr ? value : toDCOffset(value));
        upval = value;
	}
	value -= pow2(maxIterations - rssi_counter);
	rssiUp = GetRSSI();
    if (addr != DCOFFaddr)
	    Modify_SPI_Reg_bits(addr, msb, lsb, value - pow2(maxIterations - rssi_counter));
    else
        Modify_SPI_Reg_bits(addr, msb, lsb, toDCOffset(value - pow2(maxIterations - rssi_counter)));
	rssiDown = GetRSSI();
	if (rssiUp < rssiDown)
		value += 1;

    Modify_SPI_Reg_bits(addr, msb, lsb, addr != DCOFFaddr ? value : toDCOffset(value));
	rssiDown = GetRSSI();

	if (rssiUp < rssiDown)
	{
		value += 1;
        Modify_SPI_Reg_bits(addr, msb, lsb, addr != DCOFFaddr ? value : toDCOffset(value));
	}
}

int LMS7002M::CheckSaturationTxRx(const float_type bandwidth_MHz)
{
	Modify_SPI_Reg_bits(LMS7param(DC_BYP_RXTSP), 0);
	Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 0);
	SetNCOFrequency(LMS7002M::Rx, 0, 0.9 + (bandwidth_MHz / calibUserBwDivider) * 2);

	uint32_t rssi = GetRSSI();
	int g_pga = Get_SPI_Reg_bits(G_PGA_RBB);
	int g_rxlooop = Get_SPI_Reg_bits(G_RXLOOPB_RFE);
	while (rssi < 0x0B000 && g_rxlooop < 15)
	{
		rssi = GetRSSI();
		if (rssi < 0x0B000)
		{
			g_rxlooop += 1;
			Modify_SPI_Reg_bits(G_RXLOOPB_RFE, g_rxlooop);
		}
		else
			break;
	}
	rssi = GetRSSI();
	while (g_pga < 18 && g_rxlooop == 15 && rssi < 0x0B000)
	{
		g_pga += 1;
		Modify_SPI_Reg_bits(G_PGA_RBB, g_pga);
        rssi = GetRSSI();
	}
	Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 1);
	Modify_SPI_Reg_bits(DC_BYP_RXTSP, 1);
	return 0;
}

void LMS7002M::CalibrateTxDC_RSSI(const float_type bandwidth)
{
	Modify_SPI_Reg_bits(EN_G_TRF, 1);

	Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);

	SetNCOFrequency(LMS7002M::Rx, 0, 0.9 + (bandwidth / calibUserBwDivider));

	int16_t corrI = 64;
	int16_t corrQ = 64;
	Modify_SPI_Reg_bits(DCCORRI_TXTSP, 64);
	Modify_SPI_Reg_bits(DCCORRQ_TXTSP, 0);


	CoarseSearch(DCCORRI_TXTSP.address, DCCORRI_TXTSP.msb, DCCORRI_TXTSP.lsb, corrI, 7);
	Modify_SPI_Reg_bits(DCCORRI_TXTSP, corrI);

	Modify_SPI_Reg_bits(DCCORRQ_TXTSP, 64);

	CoarseSearch(DCCORRQ_TXTSP.address, DCCORRQ_TXTSP.msb, DCCORRQ_TXTSP.lsb, corrQ, 7);
	Modify_SPI_Reg_bits(DCCORRQ_TXTSP, corrQ);


	CoarseSearch(DCCORRI_TXTSP.address, DCCORRI_TXTSP.msb, DCCORRI_TXTSP.lsb, corrI, 4);
	Modify_SPI_Reg_bits(DCCORRI_TXTSP, corrI);

	CoarseSearch(DCCORRQ_TXTSP.address, DCCORRQ_TXTSP.msb, DCCORRQ_TXTSP.lsb, corrQ, 4);
	Modify_SPI_Reg_bits(DCCORRQ_TXTSP, corrQ);


#ifdef LMS_VERBOSE_OUTPUT
    printf("Fine search Tx DCCORRI/DCCORRQ\n");
#endif
    FineSearch(DCCORRI_TXTSP.address, DCCORRI_TXTSP.msb, DCCORRI_TXTSP.lsb, corrI, DCCORRQ_TXTSP.address, DCCORRQ_TXTSP.msb, DCCORRQ_TXTSP.lsb, corrQ, 7);
#ifdef LMS_VERBOSE_OUTPUT
    printf("Fine search Tx DCCORRI: %i, DCCORRQ: %i\n", corrI, corrQ);
#endif
	Modify_SPI_Reg_bits(DCCORRI_TXTSP, corrI);
	Modify_SPI_Reg_bits(DCCORRQ_TXTSP, corrQ);
}

void LMS7002M::FineSearch(const uint16_t addrI, const uint8_t msbI, const uint8_t lsbI, int16_t &valueI, const uint16_t addrQ, const uint8_t msbQ, const uint8_t lsbQ, int16_t &valueQ, const uint8_t fieldSize)
{
	const uint16_t DCOFFaddr = 0x010E;
	uint32_t **rssiField = new uint32_t*[fieldSize];
	for (int i = 0; i < fieldSize; ++i)
	{
		rssiField[i] = new uint32_t[fieldSize];
		for (int q = 0; q < fieldSize; ++q)
			rssiField[i][q] = ~0;
	}

	uint32_t minRSSI = ~0;
	int16_t minI = 0;
	int16_t minQ = 0;

	for (int i = 0; i < fieldSize; ++i)
	{
		for (int q = 0; q < fieldSize; ++q)
		{
			int16_t ival = valueI + (i - fieldSize / 2);
			int16_t qval = valueQ + (q - fieldSize / 2);
			Modify_SPI_Reg_bits(addrI, msbI, lsbI, addrI != DCOFFaddr ? ival : toDCOffset(ival), true);
			Modify_SPI_Reg_bits(addrQ, msbQ, lsbQ, addrQ != DCOFFaddr ? qval : toDCOffset(qval), true);
			rssiField[i][q] = GetRSSI();
			if (rssiField[i][q] < minRSSI)
			{
				minI = ival;
				minQ = qval;
				minRSSI = rssiField[i][q];
			}
		}
	}

#ifdef LMS_VERBOSE_OUTPUT
    printf("      |");
    for (int i = 0; i < fieldSize; ++i)
        printf("%6i|", valueQ - fieldSize / 2 + i);
    printf("\n");
    for (int i = 0; i < fieldSize + 1; ++i)
        printf("------+");
    printf("\n");
    for (int i = 0; i < fieldSize; ++i)
    {
        printf("%5i |", valueI + (i - fieldSize / 2));
        for (int q = 0; q < fieldSize; ++q)
            printf("%6i|", rssiField[i][q]);
        printf("\n");
    }
#endif

	valueI = minI;
	valueQ = minQ;
    for (int i = 0; i < fieldSize; ++i)
        delete rssiField[i];
    delete rssiField;
}

/** @brief Loads given DC_REG values into registers
    @param tx TxTSP or RxTSP selection
    @param I DC_REG I value
    @param Q DC_REG Q value
*/
int LMS7002M::LoadDC_REG_IQ(bool tx, int16_t I, int16_t Q)
{
    if(tx)
    {
        Modify_SPI_Reg_bits(LMS7param(DC_REG_TXTSP), I);
        Modify_SPI_Reg_bits(LMS7param(TSGDCLDI_TXTSP), 0);
        Modify_SPI_Reg_bits(LMS7param(TSGDCLDI_TXTSP), 1);
        Modify_SPI_Reg_bits(LMS7param(TSGDCLDI_TXTSP), 0);
        Modify_SPI_Reg_bits(LMS7param(DC_REG_TXTSP), Q);
        Modify_SPI_Reg_bits(LMS7param(TSGDCLDQ_TXTSP), 0);
        Modify_SPI_Reg_bits(LMS7param(TSGDCLDQ_TXTSP), 1);
        Modify_SPI_Reg_bits(LMS7param(TSGDCLDQ_TXTSP), 0);
    }
    else
    {
        Modify_SPI_Reg_bits(LMS7param(DC_REG_RXTSP), I);
        Modify_SPI_Reg_bits(LMS7param(TSGDCLDI_RXTSP), 0);
        Modify_SPI_Reg_bits(LMS7param(TSGDCLDI_RXTSP), 1);
        Modify_SPI_Reg_bits(LMS7param(TSGDCLDI_RXTSP), 0);
        Modify_SPI_Reg_bits(LMS7param(DC_REG_TXTSP), Q);
        Modify_SPI_Reg_bits(LMS7param(TSGDCLDQ_RXTSP), 0);
        Modify_SPI_Reg_bits(LMS7param(TSGDCLDQ_RXTSP), 1);
        Modify_SPI_Reg_bits(LMS7param(TSGDCLDQ_RXTSP), 0);
    }
    return 0;
}

int LMS7002M::StoreDigitalCorrections(const bool isTx)
{
    const int idx = this->GetActiveChannelIndex();
    const uint32_t boardId = controlPort->GetDeviceInfo().boardSerialNumber;
    const double freq = this->GetFrequencySX_MHz(isTx)*1e6;
    int band = 0; //TODO
    int dccorri, dccorrq, gcorri, gcorrq, phaseOffset;

    if (isTx)
    {
        dccorri = int8_t(Get_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP))); //signed 8-bit
        dccorrq = int8_t(Get_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP))); //signed 8-bit
        gcorri = int16_t(Get_SPI_Reg_bits(LMS7param(GCORRI_TXTSP))); //unsigned 11-bit
        gcorrq = int16_t(Get_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP))); //unsigned 11-bit
        phaseOffset = int16_t(Get_SPI_Reg_bits(LMS7param(IQCORR_TXTSP)) << 4) >> 4; //sign extend 12-bit
    }
    else
    {
        dccorri = 0;
        dccorrq = 0;
        gcorri = int16_t(Get_SPI_Reg_bits(LMS7param(GCORRI_RXTSP)) << 4) >> 4;
        gcorrq = int16_t(Get_SPI_Reg_bits(LMS7param(GCORRQ_RXTSP)) << 4) >> 4;
        phaseOffset = int16_t(Get_SPI_Reg_bits(LMS7param(IQCORR_RXTSP)) << 4) >> 4;
    }

    return valueCache.InsertDC_IQ(boardId, freq, idx, isTx, band, dccorri, dccorrq, gcorri, gcorrq, phaseOffset);
}

int LMS7002M::ApplyDigitalCorrections(const bool isTx)
{
    const int idx = this->GetActiveChannelIndex();
    const uint32_t boardId = controlPort->GetDeviceInfo().boardSerialNumber;
    const double freq = this->GetFrequencySX_MHz(isTx)*1e6;
    int band = 0; //TODO

    int dccorri, dccorrq, gcorri, gcorrq, phaseOffset;
    int rc = valueCache.GetDC_IQ_Interp(boardId, freq, idx, isTx, band, &dccorri, &dccorrq, &gcorri, &gcorrq, &phaseOffset);
    if (rc != 0) return rc;

    if (isTx)
    {
        Modify_SPI_Reg_bits(DCCORRI_TXTSP, dccorri);
        Modify_SPI_Reg_bits(DCCORRQ_TXTSP, dccorrq);
        Modify_SPI_Reg_bits(GCORRI_TXTSP, gcorri);
        Modify_SPI_Reg_bits(GCORRQ_TXTSP, gcorrq);
        Modify_SPI_Reg_bits(IQCORR_TXTSP, phaseOffset);

        Modify_SPI_Reg_bits(DC_BYP_TXTSP, 0);
        Modify_SPI_Reg_bits(PH_BYP_TXTSP, 0);
        Modify_SPI_Reg_bits(GC_BYP_TXTSP, 0);
    }
    else
    {
        Modify_SPI_Reg_bits(GCORRI_RXTSP, gcorri);
        Modify_SPI_Reg_bits(GCORRQ_RXTSP, gcorrq);
        Modify_SPI_Reg_bits(IQCORR_RXTSP, phaseOffset);

        Modify_SPI_Reg_bits(PH_BYP_RXTSP, 0);
        Modify_SPI_Reg_bits(GC_BYP_RXTSP, 0);
    }
    return 0;
}
