#include "LMS7002M.h"
#include "CalibrationCache.h"
#include "ErrorReporting.h"
#include <assert.h>
#include "MCU_BD.h"
#include "IConnection.h"
#include "mcu_programs.h"
#include "LMS64CProtocol.h"
#include <vector>
#include <ciso646>
#include <vector>
#include <stdio.h>
#include <cmath>
#include <chrono>
#include <fstream>
#include "dataTypes.h"
#include <thread>
#define LMS_VERBOSE_OUTPUT

#include "LMS7002M_RegistersMap.h"

#ifdef LMS_VERBOSE_OUTPUT
static const bool verboseEnabled = true;
#else
static const bool verboseEnabled = false;
#endif

using namespace std;

namespace lime{

enum SearchTarget
    {
        NONE,
        RX_DC_I,
        RX_DC_Q,
        TX_DC_I,
        TX_DC_Q,
        RX_IQ_GAIN,
        RX_IQ_PHASE,
        TX_IQ_GAIN,
        TX_IQ_PHASE
    };

struct BinSearchParam
{
    LMS7Parameter param;
    int16_t result;
    int16_t minValue;
    int16_t maxValue;
    SearchTarget target;
};

struct GridSearchParam
{
    BinSearchParam a;
    BinSearchParam b;
    uint32_t signalLevel;
};

}

static const char cSquaresLine[] =
    "############################################################\n";
static const char cDashLine[] =
    "------------------------------------------------------------\n";

#define verbose_printf(...) \
    do { if (verboseEnabled) {\
        fprintf(stderr, __VA_ARGS__);\
    }} while (0)

///define for parameter enumeration if prefix might be needed
#define LMS7param(id) LMS7_ ## id

using namespace lime;

static const float_type calibUserBwDivider = 5;
static const uint16_t MCU_PARAMETER_ADDRESS = 0x002D; //register used to pass parameter values to MCU

//#define ENABLE_CALIBRATION_USING_FFT

#include "goertzel.h"

int fftBin = 0; //which bin to use when calibrating using FFT
#ifdef ENABLE_CALIBRATION_USING_FFT
    static size_t streamId; //stream for calibration samples

#include "windowFunction.h"
    bool useFFT = true; //digital RSSI or FFT from GetRSSI()
    const int gFFTSize = 4096;
    int srcBin = 569; //recalculated to be at 100 kHz bin;

//  #define DRAW_GNU_PLOTS
    #ifdef DRAW_GNU_PLOTS
        float gRSSI_TXDC_measurements[256][256];
        float gRSSI_RXDC_measurements[128][128];
        std::string calibrationStart = "";
        #include <gnuPlotPipe.h>
        GNUPlotPipe spectrumPlot;
        GNUPlotPipe searchPlot;
        GNUPlotPipe saturationPlot;
    #endif
    #include "kiss_fft.h"
    #include "FPGA_common.h"
    #include <thread>
    #include <chrono>
    bool useGoertzel = false;
    extern void CalcGoertzelI(int x[][2], int64_t real[], int64_t imag[], int Sp);
    extern void CalcGoertzelF(int x[][2], float real[], float imag[], int Sp);

static void SelectFFTBin(IConnection* port, uint16_t binIndex)
{
    fftBin = binIndex;
    SelectGoertzelBin(port, binIndex, gFFTSize/2);
}

std::vector<float> windowF;
float amplitudeCorr = 1;

std::vector<float> windowG;
float amplitudeCorrG = 1;
#else
bool useFFT = false; //digital RSSI or FFT from GetRSSI()
bool useGoertzel = false;
#endif // ENABLE_CALIBRATION_USING_FFT

static const float_type targetOffsetNCO = 0.1e6; //NCO offset during calibration
static float_type offsetNCO = targetOffsetNCO; //gets changed when using FFT
const static float_type calibrationSXOffset_Hz = 4e6;

namespace lime{

}

static const int16_t firCoefs[] =
{
    8,
    4,
    0,
    -6,
    -11,
    -16,
    -20,
    -22,
    -22,
    -20,
    -14,
    -5,
    6,
    20,
    34,
    46,
    56,
    61,
    58,
    48,
    29,
    3,
    -29,
    -63,
    -96,
    -123,
    -140,
    -142,
    -128,
    -94,
    -44,
    20,
    93,
    167,
    232,
    280,
    302,
    291,
    244,
    159,
    41,
    -102,
    -258,
    -409,
    -539,
    -628,
    -658,
    -614,
    -486,
    -269,
    34,
    413,
    852,
    1328,
    1814,
    2280,
    2697,
    3038,
    3277,
    3401,
    3401,
    3277,
    3038,
    2697,
    2280,
    1814,
    1328,
    852,
    413,
    34,
    -269,
    -486,
    -614,
    -658,
    -628,
    -539,
    -409,
    -258,
    -102,
    41,
    159,
    244,
    291,
    302,
    280,
    232,
    167,
    93,
    20,
    -44,
    -94,
    -128,
    -142,
    -140,
    -123,
    -96,
    -63,
    -29,
    3,
    29,
    48,
    58,
    61,
    56,
    46,
    34,
    20,
    6,
    -5,
    -14,
    -20,
    -22,
    -22,
    -20,
    -16,
    -11,
    -6,
    0,
    4,
    8
};

const uint16_t backupAddrs[] = {
0x0020, 0x0082, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088,
0x0089, 0x008A, 0x008B, 0x008C, 0x0100, 0x0101, 0x0102, 0x0103,
0x0104, 0x0105, 0x0106, 0x0107, 0x0108, 0x0109, 0x010A, 0x010C,
0x010D, 0x010E, 0x010F, 0x0110, 0x0111, 0x0112, 0x0113, 0x0114,
0x0115, 0x0116, 0x0117, 0x0118, 0x0119, 0x011A, 0x0200, 0x0201,
0x0202, 0x0203, 0x0204, 0x0205, 0x0206, 0x0207, 0x0208, 0x0240,
0x0241, 0x0242, 0x0243, 0x0244, 0x0245, 0x0246, 0x0247, 0x0248,
0x0249, 0x024A, 0x024B, 0x024C, 0x024D, 0x024E, 0x024F, 0x0250,
0x0251, 0x0252, 0x0253, 0x0254, 0x0255, 0x0256, 0x0257, 0x0258,
0x0259, 0x025A, 0x025B, 0x025C, 0x025D, 0x025E, 0x025F, 0x0260,
0x0261, 0x0400, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406,
0x0407, 0x0408, 0x0409, 0x040A, 0x040C, 0x040D, 0x0440, 0x0441,
0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449,
0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F, 0x0450, 0x0451,
0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457, 0x0458, 0x0459,
0x045A, 0x045B, 0x045C, 0x045D, 0x045E, 0x045F, 0x0460, 0x0461
};
uint16_t backupRegs[sizeof(backupAddrs) / sizeof(int16_t)];
const uint16_t backupSXAddr[] = { 0x011C, 0x011D, 0x011E, 0x011F, 0x0120, 0x0121, 0x0122, 0x0123, 0x0124 };
uint16_t backupRegsSXR[sizeof(backupSXAddr) / sizeof(int16_t)];
uint16_t backupRegsSXT[sizeof(backupSXAddr) / sizeof(int16_t)];
int16_t rxGFIR3_backup[sizeof(firCoefs) / sizeof(int16_t)];
uint16_t backup0x010D;
uint16_t backup0x0100;

int SetExtLoopback(IConnection* port, uint8_t ch, bool enable)
{
    //enable external loopback switches
    const uint16_t LoopbackCtrAddr = 0x0017;
    uint16_t value = 0;
    const uint16_t mask = 0x7;
    const uint8_t shiftCount = (ch==2 ? 4 : 0);
    int status;
    status = port->ReadRegister(LoopbackCtrAddr, value);
    if(status != 0)
        return ReportError(-1, "Failed to read Ext. Loopback controls");
    value &= ~(mask << shiftCount);
    value |= enable << shiftCount;   //EN_Loopback
    value |= enable << (shiftCount+1); //EN_Attenuator
    value |= !enable << (shiftCount+2); //EN_Shunt
    status = port->WriteRegister(LoopbackCtrAddr, value);
    if(status != 0)
        return ReportError(-1, "Failed to write Ext. Loopback controls");
    return status;
}

inline static uint16_t pow2(const uint8_t power)
{
    assert(power >= 0 && power < 16);
    return 1 << power;
}

///convert FFT dBFS to relative unsigned integer RSSI
uint32_t dBFS_2_RSSI(const float_type dbFS)
{
    uint32_t rssi = pow(2.0, 18)-1+dbFS*1000;
    return rssi;
}

///convert RSSI(not LMS7002 RSSI!) to dBFS
float RSSI_2_dBFS(uint32_t rssi)
{
    float dbFS = (pow(2.0, 18)-1-rssi)/1000.0;
    return dbFS*(-1);
}

/** @brief Parameters setup instructions for Tx calibration
    @return 0-success, other-failure
*/
int LMS7002M::CalibrateTxSetup(float_type bandwidth_Hz, const bool useExtLoopback)
{
#ifdef DRAW_GNU_PLOTS
    char timestamp[64];
    std::time_t now_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *parts = std::localtime(&now_c);
    sprintf(timestamp, "%02i-%02i_%02i:%02i:%02i", 1+parts->tm_mon, parts->tm_mday, parts->tm_hour, parts->tm_min, parts->tm_sec);
    calibrationStart = timestamp;
#endif // DRAW_GNU_PLOTS

#ifndef ENABLE_CALIBRATION_USING_FFT
    if(useExtLoopback)
        return ReportError(EPERM, "External loopback calibration requires FFT module");
#endif // ENABLE_CALIBRATION_USING_FFT
    //Stage 2
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));
    uint8_t sel_band1_trf = (uint8_t)Get_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF));
    uint8_t sel_band2_trf = (uint8_t)Get_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF));

    //rfe
    //reset RFE to defaults
    SetDefaults(RFE);
    //Modify_SPI_Reg_bits(LMS7param(ICT_LOOPB_RFE), 31);
    if(useExtLoopback)
    {
        int LNAselection = 1; //LNAH
        Modify_SPI_Reg_bits(LMS7param(SEL_PATH_RFE), LNAselection);
        Modify_SPI_Reg_bits(LMS7param(G_LNA_RFE), 1);
        Modify_SPI_Reg_bits(0x010C, 4, 3, 0); //PD_MXLOBUF_RFE 0, PD_QGEN_RFE 0
        Modify_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE), 4);
        Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), 50);
        Modify_SPI_Reg_bits(LMS7param(ICT_LODC_RFE), 31);
        if(LNAselection == 2)
        {
            Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_L_RFE), 0);
            Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_W_RFE), 1);
        }
        else if(LNAselection == 3)
        {
            Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_L_RFE), 1);
            Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_W_RFE), 0);
        }
        else
        {
            Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_L_RFE), 1);
            Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_W_RFE), 1);
        }
        Modify_SPI_Reg_bits(LMS7param(PD_LNA_RFE), 0);
        Modify_SPI_Reg_bits(LMS7param(EN_DCOFF_RXFE_RFE), 1);
        Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE), 1);
    }
    else
    {
        if(sel_band1_trf == 1)
            Modify_SPI_Reg_bits(LMS7param(SEL_PATH_RFE), 1); //use LNAH
        else if(sel_band2_trf == 1)
            Modify_SPI_Reg_bits(LMS7param(SEL_PATH_RFE), 2);
        else
            return ReportError(EINVAL, "Tx Calibration: band not selected");

        Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), 7);
        Modify_SPI_Reg_bits(0x010C, 4, 3, 0); //PD_MXLOBUF_RFE 0, PD_QGEN_RFE 0
        Modify_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE), 4);
        Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), 50);
        if(sel_band1_trf)
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
    }

    //RBB
    //reset RBB to defaults
    SetDefaults(RBB);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), 1);
    Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), 0);
    Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB), 1);
    Modify_SPI_Reg_bits(LMS7param(ICT_PGA_OUT_RBB), 12);
    Modify_SPI_Reg_bits(LMS7param(ICT_PGA_IN_RBB), 12);

    //TRF
    if(useExtLoopback)
    {
        Modify_SPI_Reg_bits(LMS7param(EN_LOOPB_TXPAD_TRF), 0);
        Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), 1);
    }
    else
    {
        Modify_SPI_Reg_bits(LMS7param(L_LOOPB_TXPAD_TRF), 0);
        Modify_SPI_Reg_bits(LMS7param(EN_LOOPB_TXPAD_TRF), 1);
    }

    //AFE
    Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE1), 0);
    if(ch == 2)
        Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);

    //BIAS
    uint16_t backup = Get_SPI_Reg_bits(LMS7param(RP_CALIB_BIAS));
    SetDefaults(BIAS);
    Modify_SPI_Reg_bits(LMS7param(RP_CALIB_BIAS), backup);

    //XBUF
    Modify_SPI_Reg_bits(0x0085, 2, 0, 1); //PD_XBUF_RX 0, PD_XBUF_TX 0, EN_G_XBUF 1

    //CGEN
    const float_type cgenFreq = GetFrequencyCGEN();
    int cgenMultiplier = int((cgenFreq / 46.08e6) + 0.5);
    if(cgenMultiplier < 2)
        cgenMultiplier = 2;
    if(cgenMultiplier > 9 && cgenMultiplier < 12)
        cgenMultiplier = 12;
    if(cgenMultiplier > 13)
        cgenMultiplier = 13;
    //power up VCO
    //CGEN VCO is powered up in SetFrequencyCGEN/Tune
    int status = SetFrequencyCGEN(46.08e6 * cgenMultiplier);
    if(status != 0)
        return status;

    //SXR
    Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    SetDefaults(SX);
    //SX VCO is powered up in SetFrequencySX/Tune
    Modify_SPI_Reg_bits(LMS7param(ICT_VCO), 200);
    {
        float_type SXTfreq = GetFrequencySX(LMS7002M::Tx);
        float_type SXRfreq = SXTfreq - bandwidth_Hz / calibUserBwDivider - calibrationSXOffset_Hz;
        status = SetFrequencySX(LMS7002M::Rx, SXRfreq);
        if(status != 0)
            return status;
    }

    //SXT
    Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    Modify_SPI_Reg_bits(LMS7param(PD_LOCH_T2RBUF), 1);
    Modify_SPI_Reg_bits(LMS7param(MAC), ch);

    //TXTSP
    Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), 1);
    if(useExtLoopback)
        Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP), 1);
    else
        Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP), 0);
    //Modify_SPI_Reg_bits(LMS7param(DC_BYP_TXTSP), 0);
    Modify_SPI_Reg_bits(LMS7param(GC_BYP_TXTSP), 0);
    Modify_SPI_Reg_bits(LMS7param(PH_BYP_TXTSP), 0);
    Modify_SPI_Reg_bits(LMS7param(GCORRI_TXTSP), 2047);
    Modify_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP), 2047);
    Modify_SPI_Reg_bits(LMS7param(CMIX_SC_TXTSP), 0);
    LoadDC_REG_IQ(Tx, (int16_t)0x7FFF, (int16_t)0x8000);
    SetNCOFrequency(Tx, 0, bandwidth_Hz / calibUserBwDivider);

    //RXTSP
    SetDefaults(RxTSP);
    SetDefaults(RxNCO);
    Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), 4); //Decimation HBD ratio
    if (Get_SPI_Reg_bits(LMS7_MASK, true) != 0)
        Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP), 1);

    //CDS
    Modify_SPI_Reg_bits(LMS7param(CDS_TXATSP), 3);
    Modify_SPI_Reg_bits(LMS7param(CDS_TXBTSP), 3);

    if(useExtLoopback)
    {
        Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), 1);
        Modify_SPI_Reg_bits(LMS7param(AGC_BYP_RXTSP), 1);
        Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), 700); //to filter aliasing
        Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), 0);
        Modify_SPI_Reg_bits(LMS7param(CGSIN_LNA_RFE), 0);
    }
    else
    {
        uint8_t gfir3n = 4 * cgenMultiplier;
        if(Get_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN)) == 1)
            gfir3n /= pow2(Get_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN)));
        gfir3n = pow2(int(log2(gfir3n)));
        Modify_SPI_Reg_bits(LMS7param(GFIR3_N_RXTSP), gfir3n - 1);
        Modify_SPI_Reg_bits(LMS7param(AGC_MODE_RXTSP), 1);
        Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);
        Modify_SPI_Reg_bits(LMS7param(AGC_AVG_RXTSP), 0x1);
        Modify_SPI_Reg_bits(LMS7param(GFIR3_L_RXTSP), 7);
        SetGFIRCoefficients(Rx, 2, firCoefs, sizeof(firCoefs) / sizeof(int16_t));
#ifdef ENABLE_CALIBRATION_USING_FFT
        if(useFFT) //fft does not need GFIR
            Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), 1);
#endif
    }
    if(ch == 2)
    {
        Modify_SPI_Reg_bits(LMS7param(MAC), 1);
        Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0);
        Modify_SPI_Reg_bits(LMS7param(EN_NEXTRX_RFE), 1);
        Modify_SPI_Reg_bits(LMS7param(EN_NEXTTX_TRF), 1);
        Modify_SPI_Reg_bits(LMS7param(MAC), ch);
    }

#ifdef ENABLE_CALIBRATION_USING_FFT
    if(useExtLoopback || useFFT)
    {
        //limelight
        Modify_SPI_Reg_bits(LMS7param(LML1_FIDM), 0);
        Modify_SPI_Reg_bits(LMS7param(LML2_FIDM), 0);
        Modify_SPI_Reg_bits(LMS7param(LML1_MODE), 0);
        Modify_SPI_Reg_bits(LMS7param(LML2_MODE), 0);
        if(ch == 1)
        {
            Modify_SPI_Reg_bits(LMS7param(LML2_S0S), 1); //pos0, AQ
            Modify_SPI_Reg_bits(LMS7param(LML2_S1S), 0); //pos1, AI
        }
        else if(ch == 2)
        {
            Modify_SPI_Reg_bits(LMS7param(LML2_S0S), 3); //pos0, BQ
            Modify_SPI_Reg_bits(LMS7param(LML2_S1S), 2); //pos1, BI
        }
        //need to update interface frequency for samples acquisition
        //if decimation/interpolation is 0(2^1) or 7(bypass), interface clocks should not be divided
        int decimation = Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
        float interfaceRx_Hz = GetReferenceClk_TSP(LMS7002M::Rx);
        //need to adjust decimation to fit into USB speed
        float rateLimit_Bps;
        DeviceInfo info = dataPort->GetDeviceInfo();
        if(info.deviceName == GetDeviceName(LMS_DEV_STREAM))
            rateLimit_Bps = 110e6;
        else if(info.deviceName == GetDeviceName(LMS_DEV_LIMESDR))
            rateLimit_Bps = 300e6;
        else if(info.deviceName == GetDeviceName(LMS_DEV_ULIMESDR))
            rateLimit_Bps = 100e6;
        else
            rateLimit_Bps = 100e6;
#ifndef N_DEBUG
        rateLimit_Bps = 50e6; //debug mode is slower to receive data
#endif

        decimation = ceil(log2((interfaceRx_Hz*3/2)/rateLimit_Bps));
        if(decimation < 0)
            decimation = 0;
        if(decimation > 4)
            decimation = 4;
        if (decimation != 7)
            interfaceRx_Hz /= pow(2.0, decimation);

        int interpolation = Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
        float interfaceTx_Hz = GetReferenceClk_TSP(LMS7002M::Tx);
        if (interpolation != 7)
            interfaceTx_Hz /= pow(2.0, interpolation);
        SetInterfaceFrequency(GetFrequencyCGEN(), interpolation, decimation);
        dataPort->UpdateExternalDataRate(0, interfaceTx_Hz/2, interfaceRx_Hz/2);

        GenerateWindowCoefficients(3, gFFTSize, windowF, amplitudeCorr);
        GenerateWindowCoefficients(3, gFFTSize/2, windowG, amplitudeCorrG);

        StreamConfig config;
        config.channelID = ch==1 ? 0 : 1;
        config.format = StreamConfig::STREAM_12_BIT_IN_16;
        config.linkFormat = StreamConfig::STREAM_12_BIT_COMPRESSED;
        config.isTx = false;
        config.bufferLength = 2*gFFTSize;
        config.performanceLatency = 1.0;

        dataPort->SetupStream(streamId, config);
        dataPort->ControlStream(streamId, true);
    }
#endif
    return 0;
}

/** @brief Flips the CAPTURE bit and returns digital RSSI value

    If calibration using FFT is enabled, GetRSSI() can return value calculated
    from FFT result at bin selected by fftBIN
*/
int avgCount = 1;
uint32_t LMS7002M::GetRSSI(RSSI_measurements *measurements)
{
#ifdef ENABLE_CALIBRATION_USING_FFT
    if(useFFT)
    {

    bool calcFFT = true;
    bool calcGeortzelFloat = false;
    bool calcGeortzelFPGA = false;

    int fftSize = gFFTSize;
    //goertzel
    // number of spectral points, i.e. number of bins all the interval is divided to
    const int SP  = fftSize/2;

    IConnection* port = dataPort;
    kiss_fft_cfg m_fftCalcPlan = kiss_fft_alloc(fftSize, 0, 0, 0);
    kiss_fft_cpx* m_fftCalcIn = nullptr;
    kiss_fft_cpx* m_fftCalcOut = nullptr;

    if(calcFFT)
    {
        m_fftCalcIn = new kiss_fft_cpx[fftSize];
        m_fftCalcOut = new kiss_fft_cpx[fftSize];
    }

    auto xi = new int[SP][2];    // Raw data (integer numbers)
    // Calculated Goertzel bins, integer numbers
    auto *realf = new float[SP];
    auto *imagf = new float[SP];

    //number of packets to skip from the buffer start
    complex16_t *buffer = nullptr;
    if(calcFFT || calcGeortzelFloat)
        buffer = new complex16_t[fftSize];

    std::vector<float> fftBins_dbFS;
    if(calcFFT)
        fftBins_dbFS.resize(fftSize, 0);
    std::vector<float> goertzBins_dbFSF;
    if(calcGeortzelFloat)
        goertzBins_dbFSF.resize(SP, 0);
    std::vector<float> goertzBins_dbFSI;
    if(calcGeortzelFPGA)
        goertzBins_dbFSI.resize(SP, 0);

    for(int a=0; a<avgCount; ++a)
    {
        if(calcGeortzelFPGA)
        {
            int64_t real_hw = 0;
            int64_t imag_hw = 0;
            int status = CalculateGoertzelBin(port, &real_hw, &imag_hw);
            if(status != 0)
                printf("Failed to get Geortzel from FPGA\n");
            goertzBins_dbFSI[fftBin] += (real_hw/float(SP) * real_hw/float(SP) + imag_hw/float(SP) * imag_hw/float(SP));
        }

        if(not calcFFT && not calcGeortzelFloat)
            continue;

        StreamMetadata meta;
        port->ControlStream(streamId, true); //clears FIFO to get newest samples
        int samplesReceived = port->ReadStream(streamId, buffer, fftSize, 200, meta);
        if(samplesReceived < fftSize)
            printf("Samples not received for FFT, %i/%i\n", samplesReceived, fftSize);

        for(int i=0; i<samplesReceived; ++i)
        {
            m_fftCalcIn[i].r = buffer[i].i;
            if(i < SP && calcGeortzelFloat)
                xi[i][0] = buffer[i].i;

            //Q sample
            m_fftCalcIn[i].i = buffer[i].q;
            if (i < SP && calcGeortzelFloat)
                xi[i][1] = buffer[i].q;
        }
        if(calcFFT)
        {
            for(int i=0; i<fftSize; ++i)
            {
                m_fftCalcIn[i].i *= windowF[i] * amplitudeCorr;
                m_fftCalcIn[i].r *= windowF[i] * amplitudeCorr;
            }
            kiss_fft(m_fftCalcPlan, m_fftCalcIn, m_fftCalcOut);
            for (int i = 0; i < fftSize; ++i)
            {
                m_fftCalcOut[i].r /= fftSize;
                m_fftCalcOut[i].i /= fftSize;
                fftBins_dbFS[i] += sqrt(m_fftCalcOut[i].r * m_fftCalcOut[i].r + m_fftCalcOut[i].i * m_fftCalcOut[i].i);
            }
        }
        if(calcGeortzelFloat)
        {
            for(int i=0; i<SP; ++i)
            {
                xi[i][0] *= windowG[i]*amplitudeCorrG;
                xi[i][1] *= windowG[i]*amplitudeCorrG;
            }
            CalcGoertzelF(xi, realf, imagf, SP);
            for (int i=0; i<SP; ++i)
            {
                realf[i] /= float(SP);
                imagf[i] /= float(SP);
                float magnitude = (realf[i] * realf[i] + imagf[i] * imagf[i]);
                goertzBins_dbFSF[i] += magnitude;
            }
        }
    }

    if(calcFFT)
        for (int i = 0; i < fftSize; ++i)
        {
            fftBins_dbFS[i] /= avgCount;
            fftBins_dbFS[i] = (fftBins_dbFS[i] != 0) ? (20 * log10((fftBins_dbFS[i])) - 69.2369) : -150;
        }
    if(calcGeortzelFloat)
        for (int i = 0; i < SP; ++i)
        {
            goertzBins_dbFSF[i] /= avgCount;
            goertzBins_dbFSF[i] = (goertzBins_dbFSF[i] != 0) ? (20 * log10(sqrt(goertzBins_dbFSF[i])) - 69.2369) : -150;
        }
    if(calcGeortzelFPGA)
        for (int i = 0; i < SP; ++i)
        {
            goertzBins_dbFSI[i] /= avgCount;
            goertzBins_dbFSI[i] = (goertzBins_dbFSI[i] != 0) ? (20 * log10(sqrt(goertzBins_dbFSI[i])) - 69.2369) : -150;
        }

    if(measurements)
    {
        measurements->clear();
        measurements->amplitudeFFT.push_back(fftBins_dbFS[fftBin]);
        measurements->amplitudeGeortzelF.push_back(goertzBins_dbFSF[fftBin]);
        measurements->amplitudeGeortzelFPGA.push_back(goertzBins_dbFSI[fftBin]);
    }
#ifdef DRAW_GNU_PLOTS
    const int binsToShow = srcBin*12;
    spectrumPlot.write("set yrange [-155:0]\n");
    spectrumPlot.writef("set xrange [%i:%i]\n", -srcBin/2, binsToShow);
    spectrumPlot.write("plot\
'-' u 1:2 with points ps 4 pt 23 notitle,\
'-' u 1:2:2 with labels offset 4,0.5 notitle");
    if(calcFFT)
        spectrumPlot.write(", '-' u 1:2 with lines title 'FFT'");
    if(calcGeortzelFloat)
        spectrumPlot.write(", '-' u 1:2 with lines title 'Goertz Float'");
    if(calcGeortzelFPGA)
        spectrumPlot.write(", '-' u 1:2 with lines title 'Goertz FPGA' lt rgb 'green'");
    spectrumPlot.write("\n");
    spectrumPlot.writef("%i, %f\ne\n", fftBin, (useGoertzel ? goertzBins_dbFSI[fftBin] : fftBins_dbFS[fftBin])); //marker
    spectrumPlot.writef("%i, %f\ne\n", fftBin, (useGoertzel ? goertzBins_dbFSI[fftBin] : fftBins_dbFS[fftBin])); //value label
    if(calcFFT)
    {
        for (int i = 0; i<binsToShow; ++i)
            spectrumPlot.writef("%i %f\n", i, fftBins_dbFS[i]);
        spectrumPlot.write("e\n");
    }
    if(calcGeortzelFloat)
    {
        for (int i = 0; i<binsToShow; ++i)
            spectrumPlot.writef("%i %f\n", i, goertzBins_dbFSF[i]);
        spectrumPlot.write("e\n");
    }
    if(calcGeortzelFPGA)
    {
        for (int i = 0; i<binsToShow; ++i)
            spectrumPlot.writef("%i %f\n", i, goertzBins_dbFSI[i]);
        spectrumPlot.write("e\n");
    }
    spectrumPlot.flush();
#endif
    if(calcFFT || calcGeortzelFloat)
        delete[] buffer;
    if(calcFFT)
    {
        kiss_fft_free(m_fftCalcPlan);
        delete[] m_fftCalcOut;
        delete[] m_fftCalcIn;
    }
    delete[] xi;
    delete[] realf;
    delete[] imagf;
    return dBFS_2_RSSI(useGoertzel ? goertzBins_dbFSI[fftBin] : fftBins_dbFS[fftBin]);
    }
#endif
    //delay to make sure RSSI gets enough samples to refresh before reading it
    this_thread::sleep_for(chrono::microseconds(50));
    Modify_SPI_Reg_bits(LMS7param(CAPTURE), 0);
    Modify_SPI_Reg_bits(LMS7param(CAPTURE), 1);
    uint32_t rssi = (Get_SPI_Reg_bits(0x040F, 15, 0, true) << 2) | Get_SPI_Reg_bits(0x040E, 1, 0, true);
    return rssi;
}

/** @brief Calibrates Transmitter. DC correction, IQ gains, IQ phase correction
@return 0-success, other-failure
*/
int LMS7002M::CalibrateTx(float_type bandwidth_Hz, bool useExtLoopback)
{
    if(controlPort == nullptr)
        return ReportError(EINVAL, "Device not connected");
#ifdef __cplusplus
    auto beginTime = std::chrono::high_resolution_clock::now();
#endif
#ifndef ENABLE_CALIBRATION_USING_FFT
    if(useExtLoopback)
        return ReportError(EPERM, "External loopback calibration requires ENABLE_CALIBRATION_USING_FFT");
#endif // ENABLE_CALIBRATION_USING_FFT
    int status;
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));
    uint8_t sel_band1_trf = (uint8_t)Get_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF));

    //caching variables
    DeviceInfo info = controlPort->GetDeviceInfo();
    uint32_t boardId = info.boardSerialNumber;
    double txFreq = GetFrequencySX(LMS7002M::Tx);
    uint8_t channel = ch == 1 ? 0 : 1;
    int band = sel_band1_trf ? 0 : 1;
    if(useCache)
    {
        int dcI, dcQ, gainI, gainQ, phOffset;
        bool foundInCache = (mValueCache->GetDC_IQ(boardId, txFreq, channel, true, band, &dcI, &dcQ, &gainI, &gainQ, &phOffset) == 0);
        if(foundInCache)
        {
            Modify_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP), dcI);
            Modify_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP), dcQ);
            Modify_SPI_Reg_bits(LMS7param(GCORRI_TXTSP), gainI);
            Modify_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP), gainQ);
            Modify_SPI_Reg_bits(LMS7param(IQCORR_TXTSP), phOffset);
            Modify_SPI_Reg_bits(LMS7param(DC_BYP_TXTSP), 0); //DC_BYP
            Modify_SPI_Reg_bits(0x0208, 1, 0, 0); //GC_BYP PH_BYP
            const int8_t dcIsigned = dcI;
            const int8_t dcQsigned = dcQ;
            int16_t phaseSigned = phOffset << 4;
            phaseSigned >>= 4;
            verbose_printf(cSquaresLine);
            verbose_printf("Tx calibration values found in cache:\n");
            verbose_printf("   | DC  | GAIN | PHASE\n");
            verbose_printf("---+-----+------+------\n");
            verbose_printf("I: | %3i | %4i | %i\n", dcIsigned, gainI, phaseSigned);
            verbose_printf("Q: | %3i | %4i |\n", dcQsigned, gainQ);
            verbose_printf(cSquaresLine);
            return 0;
        }
    }

    uint16_t gcorri, gcorrq;
    int16_t dccorri, dccorrq, phaseOffset;

    bool useOnBoardLoopback = (info.deviceName == GetDeviceName(LMS_DEV_LIMESDR) && std::stoi(info.hardwareVersion) >= 3);
    const char* methodName;
    if(useExtLoopback)
    {
        useFFT = true;
        methodName = useGoertzel ? "Geortzel" : "FFT";
    }
    if(mCalibrationByMCU)
    {
        useExtLoopback = useFFT = false;
        methodName = "RSSI";
    }
    verbose_printf(cSquaresLine);
    verbose_printf("Tx calibration using %s %s %s loopback\n",
                    methodName,
                    useExtLoopback ? "EXTERNAL" : "INTERNAL",
                    useOnBoardLoopback ? "ON BOARD" : (useExtLoopback ? "CABLE" : "CHIP"));
    verbose_printf("Tx ch.%s @ %4g MHz, BW: %g MHz, RF output: %s, Gain: %i\n",
                    ch == 0x1 ? "A" : "B",
                    txFreq/1e6,
                    bandwidth_Hz/1e6,
                    sel_band1_trf==1 ? "BAND1" : "BAND2",
                    Get_SPI_Reg_bits(LMS7param(CG_IAMP_TBB)));
    verbose_printf("Performed by: %s\n", mCalibrationByMCU ? "MCU" : "PC");
    verbose_printf(cDashLine);
    //LMS7002M_SelfCalState state(this);
    auto registersBackup = BackupRegisterMap();
    if(mCalibrationByMCU && not useExtLoopback)
    {
        uint8_t mcuID = mcuControl->ReadMCUProgramID();
        verbose_printf("Current MCU firmware: %i, %s\n", mcuID,
            mcuID == MCU_ID_CALIBRATIONS_SINGLE_IMAGE ? "DC/IQ calibration full":"unknown");
        if(mcuID != MCU_ID_CALIBRATIONS_SINGLE_IMAGE)
        {
            verbose_printf("Uploading DC/IQ calibration firmware\n");
            status = mcuControl->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, IConnection::MCU_PROG_MODE::SRAM);
            if(status != 0)
                return status;
        }

        //set reference clock parameter inside MCU
        long refClk = GetReferenceClk_SX(false);
        mcuControl->SetParameter(MCU_BD::MCU_REF_CLK, refClk);
        verbose_printf("MCU Ref. clock: %g MHz\n", refClk / 1e6);
        //set bandwidth for MCU to read from register, value is integer stored in MHz
        mcuControl->SetParameter(MCU_BD::MCU_BW, bandwidth_Hz);
        mcuControl->RunProcedure(MCU_FUNCTION_CALIBRATE_TX);
        status = mcuControl->WaitForMCU(1000);
        if(status != 0)
        {
            ReportError("MCU working too long %i", status);
        }
        //need to read back calibration results
        dccorri = Get_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP), true);
        dccorrq = Get_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP), true);
        gcorri = Get_SPI_Reg_bits(LMS7param(GCORRI_TXTSP), true);
        gcorrq = Get_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP), true);
        phaseOffset = Get_SPI_Reg_bits(LMS7param(IQCORR_TXTSP), true);

        if(useCache)
            mValueCache->InsertDC_IQ(boardId, txFreq, channel, true, band, dccorri, dccorrq, gcorri, gcorrq, phaseOffset);

        return status;
    }

    Log("Tx calibration started", LOG_INFO);
    if(useExtLoopback && useOnBoardLoopback)
    {
        status = SetExtLoopback(dataPort, ch, true);
        if(status != 0)
            return status;
    }

    Log("Setup stage", LOG_INFO);
    status = CalibrateTxSetup(bandwidth_Hz, useExtLoopback);
#ifdef __cplusplus
    verbose_printf("Setup duration: %li ms\n",
        std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::high_resolution_clock::now()-beginTime).count());
#endif
    if(status != 0)
        goto TxCalibrationEnd; //go to ending stage to restore registers
#ifdef ENABLE_CALIBRATION_USING_FFT
    {
        //calculate NCO offset, that the signal would be in FFT bin
        float_type binWidth = GetSampleRate(LMS7002M::Rx, ch==1 ? ChA:ChB)/gFFTSize;
        offsetNCO = int(targetOffsetNCO / binWidth+0.5)*binWidth+binWidth/2;
    }
    srcBin = gFFTSize*offsetNCO/GetSampleRate(LMS7002M::Rx, ch==1 ? ChA:ChB);
    SelectFFTBin(dataPort, srcBin);
#endif // ENABLE_CALIBRATION_USING_FFT
    status = CheckSaturationTxRx(bandwidth_Hz, useExtLoopback);
    if(status != 0)
        goto TxCalibrationEnd;

    if(useExtLoopback == false)
        CalibrateRxDCAuto();
    SetNCOFrequency(LMS7002M::Rx, 0, calibrationSXOffset_Hz - offsetNCO + (bandwidth_Hz / calibUserBwDivider));
    CalibrateTxDCAuto();
    //CalibrateTxDC(&dccorri, &dccorrq);
    //TXIQ
    SetNCOFrequency(LMS7002M::Rx, 0, calibrationSXOffset_Hz - offsetNCO);
    CalibrateIQImbalance(LMS7002M::Tx, &gcorri, &gcorrq, &phaseOffset);
TxCalibrationEnd:
#ifdef ENABLE_CALIBRATION_USING_FFT
    dataPort->ControlStream(streamId, false);
    dataPort->CloseStream(streamId);
#endif
    Log("Restoring registers state", LOG_INFO);
    RestoreRegisterMap(registersBackup);
    UploadAll();

    //RestoreAllRegisters();

    if(useExtLoopback && useOnBoardLoopback)
        SetExtLoopback(dataPort, ch, false);

    if(status != 0)
    {
        Log("Tx calibration failed", LOG_WARNING);
        return status;
    }

    if(useCache)
        mValueCache->InsertDC_IQ(boardId, txFreq, channel, true, band, dccorri, dccorrq, gcorri, gcorrq, phaseOffset);

    Modify_SPI_Reg_bits(LMS7param(MAC), ch);
    //Modify_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP), dccorri);
    //Modify_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP), dccorrq);
    Modify_SPI_Reg_bits(LMS7param(GCORRI_TXTSP), gcorri);
    Modify_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP), gcorrq);
    Modify_SPI_Reg_bits(LMS7param(IQCORR_TXTSP), phaseOffset);
    Modify_SPI_Reg_bits(LMS7param(DCMODE), 1);
    if(ch == 1)
        Modify_SPI_Reg_bits(LMS7param(PD_DCDAC_TXA), 0);
    else
        Modify_SPI_Reg_bits(LMS7param(PD_DCDAC_TXB), 0);
    Modify_SPI_Reg_bits(LMS7param(DC_BYP_TXTSP), 1);

    Modify_SPI_Reg_bits(0x0208, 1, 0, 0); //GC_BYP PH_BYP
    LoadDC_REG_IQ(Tx, (int16_t)0x7FFF, (int16_t)0x8000);
    Log("Tx calibration finished", LOG_INFO);
#ifdef LMS_VERBOSE_OUTPUT
    verbose_printf("#####Tx calibration RESULTS:###########################\n");
    verbose_printf("Method: %s %s %s loopback\n",
                    methodName,
                    useExtLoopback ? "EXTERNAL" : "INTERNAL",
                    useOnBoardLoopback ? "ON BOARD" : (useExtLoopback ? "CABLE" : "CHIP"));
    verbose_printf("Tx ch.%s @ %4g MHz, BW: %g MHz, RF output: %s, Gain: %i\n",
                    ch == Channel::ChA ? "A" : "B", txFreq/1e6,
                    bandwidth_Hz/1e6, band==0 ? "BAND1" : "BAND2",
                    Get_SPI_Reg_bits(LMS7param(CG_IAMP_TBB)));
    {
    int8_t dcIsigned = dccorri;
    int8_t dcQsigned = dccorrq;
    int16_t phaseSigned = phaseOffset << 4;
    phaseSigned >>= 4;
    verbose_printf("   | DC  | GAIN | PHASE\n");
    verbose_printf("---+-----+------+------\n");
    verbose_printf("I: | %3i | %4i | %i\n", dcIsigned, gcorri, phaseSigned);
    verbose_printf("Q: | %3i | %4i |\n", dcQsigned, gcorrq);
    }
#ifdef __cplusplus
    int32_t duration = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::high_resolution_clock::now()-beginTime).count();
    verbose_printf("Duration: %i ms\n", duration);
#endif
    verbose_printf(cSquaresLine);
#endif //LMS_VERBOSE_OUTPUT
    return 0;
}

/** @brief Performs Rx DC offsets calibration
*/
void LMS7002M::CalibrateRxDC()
{
#ifdef DRAW_GNU_PLOTS
    searchPlot.write("set title 'Rx DC search\n");
    spectrumPlot.write("set title 'Rx DC search\n");
#endif // DRAW_GNU_PLOTS
    BinSearchParam argsI;
    BinSearchParam argsQ;
#ifdef ENABLE_CALIBRATION_USING_FFT
    SelectFFTBin(dataPort, 0);
#endif
    Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), 0);
    Modify_SPI_Reg_bits(LMS7param(DC_BYP_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(CAPSEL), 0);
    SetRxDCOFF(32, 32);
    verbose_printf(cDashLine);
    verbose_printf("Calibrating Rx DC...\n");

    //find I
    argsI.param = LMS7param(DCOFFI_RFE);
    argsQ.param = LMS7param(DCOFFQ_RFE);
    argsI.maxValue = argsQ.maxValue = 63;
    argsI.minValue = argsQ.minValue = -63;

    BinarySearch(&argsI);
    BinarySearch(&argsQ);

    argsI.maxValue = argsI.result+4;
    argsI.minValue = argsI.result-4;
    argsQ.maxValue = argsQ.result+4;
    argsQ.minValue = argsQ.result-4;

    BinarySearch(&argsI);
    BinarySearch(&argsQ);

    //FineSearch(DCOFFI_RFE.address, DCOFFI_RFE.msb, DCOFFI_RFE.lsb, offsetI, DCOFFQ_RFE.address, DCOFFQ_RFE.msb, DCOFFQ_RFE.lsb, offsetQ, 5);
    Modify_SPI_Reg_bits(LMS7param(DC_BYP_RXTSP), 0); // DC_BYP 0

    verbose_printf("Rx DCOFFI: %i, DCOFFQ: %i\n", argsI.result, argsQ.result);
    Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), 1);
#ifdef ENABLE_CALIBRATION_USING_FFT
    SelectFFTBin(dataPort, srcBin); //fft bin 100 kHz
#endif
}

void WriteAnalogDC(LMS7002M* lmsControl, const LMS7Parameter& param, int value)
{
    uint16_t mask = param.address < 0x05C7 ? 0x03FF : 0x003F;
    int16_t regValue = 0;
    if(value < 0)
    {
        regValue |= (mask+1);
        regValue |= (abs(value+mask) & mask);
    }
    else
        regValue |= (abs(value+mask+1) & mask);
    lmsControl->SPI_write(param.address, regValue);
    lmsControl->SPI_write(param.address, regValue | 0x8000);
}

int16_t ReadAnalogDC(LMS7002M* lmsControl, const LMS7Parameter& param)
{
    uint16_t mask = param.address < 0x05C7 ? 0x03FF : 0x003F;

    lmsControl->SPI_write(param.address, 0);
    lmsControl->SPI_write(param.address, 0x4000);
    uint16_t value = lmsControl->SPI_read(param.address, true);
    lmsControl->SPI_write(param.address, value & ~0xC000);
    int16_t result = (value & mask);
    if(value& (mask+1))
        result *= -1;
    return result;
}

void LMS7002M::AdjustAutoDC(const uint16_t address, bool tx)
{
    const uint16_t mask = tx ? 0x03FF : 0x003F;
    SPI_write(address, 0);
    SPI_write(address, 0x4000);
    int16_t initVal = SPI_read(address, true);
    SPI_write(address, 0);
    if(initVal & (mask+1))
    {
        initVal &= mask;
        initVal *= -1;
    }
    else
        initVal &= mask;


    uint16_t rssi = GetRSSI();
    {
        int tempValue = initVal+1;
        int regValue = 0;
        if(tempValue < 0)
        {
            regValue |= (mask+1);
            regValue |= (abs(tempValue+mask) & mask);
        }
        else
            regValue |= (abs(tempValue+mask+1) & mask);
        SPI_write(address, regValue);
        SPI_write(address, regValue | 0x8000);
    }

    bool increment = GetRSSI() < rssi;
    uint16_t rssiNext = rssi;
    int8_t iterations = 8;
    while (iterations > 0)
    {
        SPI_write(address, 0);
        if(increment)
            ++initVal;
        else
            --initVal;
        {
            int regValue = 0;
            if(initVal < 0)
            {
                regValue |= (mask+1);
                regValue |= (abs(initVal+mask) & mask);
            }
            else
                regValue |= (abs(initVal+mask+1) & mask);
            SPI_write(address, regValue);
            SPI_write(address, regValue | 0x8000);
        }
        rssi = rssiNext;
        rssiNext = GetRSSI();
        if((rssiNext < rssi) != increment)
        {
            if(increment)
                --initVal;
            else
                ++initVal;
            int regValue = 0;
            if(initVal < 0)
            {
                regValue |= (mask+1);
                regValue |= (abs(initVal+mask) & mask);
            }
            else
                regValue |= (abs(initVal+mask+1) & mask);
            SPI_write(address, regValue);
            SPI_write(address, regValue | 0x8000);
            SPI_write(address, 0);

            printf("Found: %i\n", initVal);
            break;
        }
        --iterations;
    }
}

void LMS7002M::CalibrateRxDCAuto()
{
#ifdef DRAW_GNU_PLOTS
    searchPlot.write("set title 'Rx DC search\n");
    spectrumPlot.write("set title 'Rx DC search\n");
#endif // DRAW_GNU_PLOTS
#ifdef ENABLE_CALIBRATION_USING_FFT
    SelectFFTBin(dataPort, 0);
#endif
    Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), 0);
    Modify_SPI_Reg_bits(LMS7param(DC_BYP_RXTSP), 1);

    verbose_printf(cDashLine);
    verbose_printf("Calibrating Rx DC...\n");

    //auto calibration
    uint16_t statusMask;
    const uint8_t ch = Get_SPI_Reg_bits(LMS7param(MAC));
    uint16_t dcRegAddr = 0x5C7;
    Modify_SPI_Reg_bits(LMS7param(DCMODE), 1);
    if(ch == 1)
    {
        Modify_SPI_Reg_bits(LMS7param(PD_DCDAC_RXA), 0);
        Modify_SPI_Reg_bits(LMS7param(PD_DCCMP_RXA), 0);
        SPI_write(0x05C2, 0xFF30);
        statusMask = 0x0F00;
    }
    else
    {
        Modify_SPI_Reg_bits(LMS7param(PD_DCDAC_RXB), 0);
        Modify_SPI_Reg_bits(LMS7param(PD_DCCMP_RXB), 0);
        SPI_write(0x05C2, 0xFFC0);
        statusMask = 0xF000;
        dcRegAddr += 2;
    }

    bool  busy = SPI_read(0x05C1) & statusMask;
    while(busy)
    {
        busy = SPI_read(0x05C1) & statusMask;
    }

    //manual adjustments
    Modify_SPI_Reg_bits(LMS7param(GCORRQ_RXTSP), 0);
    AdjustAutoDC(dcRegAddr, false);
    Modify_SPI_Reg_bits(LMS7param(GCORRI_RXTSP), 0);
    Modify_SPI_Reg_bits(LMS7param(GCORRQ_RXTSP), 2047);
    AdjustAutoDC(dcRegAddr+1, false);
    Modify_SPI_Reg_bits(LMS7param(GCORRI_RXTSP), 2047);

    Modify_SPI_Reg_bits(LMS7param(DC_BYP_RXTSP), 0); // DC_BYP 0
    Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), 1);
#ifdef ENABLE_CALIBRATION_USING_FFT
    SelectFFTBin(dataPort, srcBin); //fft bin 100 kHz
#endif
}

void LMS7002M::TxDcBinarySearch(BinSearchParam* args)
{
    uint32_t rssiLeft = ~0;
    uint32_t rssiRight;
    int16_t left = args->minValue;
    int16_t right = args->maxValue;
    int16_t step;

    WriteAnalogDC(this, args->param, right);
    rssiRight = GetRSSI();

    while(right-left >= 1)
    {
        step = (right-left)/2;
        if(rssiLeft < rssiRight)
        {
            WriteAnalogDC(this, args->param, right);
            rssiRight = GetRSSI();
        }
        else
        {
            WriteAnalogDC(this, args->param, left);
            rssiLeft = GetRSSI();
        }
        if(step == 0)
            break;
        if(rssiLeft < rssiRight)
            right -= step;
        else
            left += step;
    }

    args->result = rssiLeft < rssiRight ? left : right;
    WriteAnalogDC(this, args->param, args->result);
}

void LMS7002M::CalibrateTxDCAuto()
{
    verbose_printf("Searching Tx DC...\n");
    Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), 1);
    Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP), 0);
    Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 0);

    Modify_SPI_Reg_bits(LMS7param(DC_BYP_TXTSP), 1);

    BinSearchParam iparams;
    BinSearchParam qparams;

    //auto calibration
    uint16_t statusMask;
    const uint8_t ch = Get_SPI_Reg_bits(LMS7param(MAC));
    uint16_t dcRegAddr = 0x5C3;
    Modify_SPI_Reg_bits(LMS7param(DCMODE), 1);
    Modify_SPI_Reg_bits(LMS7param(GCORRI_TXTSP), 0);
    Modify_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP), 0);
    if(ch == 1)
    {
        iparams.param = LMS7_DC_TXAI;
        qparams.param = LMS7_DC_TXAQ;
        Modify_SPI_Reg_bits(LMS7param(PD_DCDAC_TXA), 0);
        Modify_SPI_Reg_bits(LMS7param(PD_DCCMP_TXA), 0);
        SPI_write(0x05C2, 0x0F03);
        statusMask = 0x0F00;
    }
    else
    {
        iparams.param = LMS7_DC_TXBI;
        qparams.param = LMS7_DC_TXBQ;
        Modify_SPI_Reg_bits(LMS7param(PD_DCDAC_TXB), 0);
        Modify_SPI_Reg_bits(LMS7param(PD_DCCMP_TXB), 0);
        SPI_write(0x05C2, 0x0F0C);
        statusMask = 0xF000;
        dcRegAddr += 2;
    }

    bool  busy = SPI_read(0x05C1) & statusMask;
    while(busy)
    {
        busy = SPI_read(0x05C1) & statusMask;
    }

    int16_t ivalue = ReadAnalogDC(this, iparams.param);
    int16_t qvalue = ReadAnalogDC(this, qparams.param);
    int offset = 64;
    iparams.minValue = ivalue-offset;
    iparams.maxValue = ivalue+offset;
    qparams.minValue = qvalue-offset;
    qparams.maxValue = qvalue+offset;

    TxDcBinarySearch(&iparams);
    ivalue = iparams.result;
    TxDcBinarySearch(&qparams);
    qvalue = qparams.result;

    offset = 8;
    iparams.minValue = ivalue-offset;
    iparams.maxValue = ivalue+offset;
    qparams.minValue = qvalue-offset;
    qparams.maxValue = qvalue+offset;

    TxDcBinarySearch(&iparams);
    ivalue = iparams.result;
    TxDcBinarySearch(&qparams);
    qvalue = qparams.result;

    Modify_SPI_Reg_bits(LMS7param(GCORRI_TXTSP), 2047);
    Modify_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP), 2047);
    printf("I: %i, Q: %i\n", ivalue, qvalue);
    verbose_printf("Tx DC DONE\n");
}


/** @brief Parameters setup instructions for Rx calibration
@param bandwidth_Hz filter bandwidth in Hz
@return 0-success, other-failure
*/
int LMS7002M::CalibrateRxSetup(float_type bandwidth_Hz, const bool useExtLoopback)
{
#ifndef ENABLE_CALIBRATION_USING_FFT
    if(useExtLoopback)
        return ReportError(EPERM, "External loopback calibration requires FFT module");
#endif // ENABLE_CALIBRATION_USING_FFT
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));

    //rfe
    Modify_SPI_Reg_bits(LMS7param(EN_DCOFF_RXFE_RFE), 1);
    if(not useExtLoopback)
        Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), 3);
    Modify_SPI_Reg_bits(0x010C, 4, 3, 0); //PD_MXLOBUF_RFE 0, PD_QGEN_RFE 0
    Modify_SPI_Reg_bits(0x010C, 1, 1, 0); //PD_TIA 0

    //RBB
    Modify_SPI_Reg_bits(0x0115, 15, 14, 0); //Loopback switches disable
    Modify_SPI_Reg_bits(0x0119, 15, 15, 0); //OSW_PGA 0

    //TRF
    //reset TRF to defaults
    SetDefaults(TRF);
    if(not useExtLoopback)
    {
        Modify_SPI_Reg_bits(LMS7param(L_LOOPB_TXPAD_TRF), 0);
        Modify_SPI_Reg_bits(LMS7param(EN_LOOPB_TXPAD_TRF), 1);
    }
    else
        Modify_SPI_Reg_bits(LMS7param(LOSS_MAIN_TXPAD_TRF), 10);
    Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), 0);

    {
        uint8_t selPath;
        if(useExtLoopback) //use PA1
            selPath = 3;
        else
            selPath = Get_SPI_Reg_bits(LMS7param(SEL_PATH_RFE));

        if (selPath == 2)
        {
            Modify_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF), 1);
            Modify_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF), 0);
        }
        else if (selPath == 3)
        {
            if(controlPort->GetDeviceInfo().hardwareVersion == "3")
            {
                Modify_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF), 1);
                Modify_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF), 0);
            }
            else
            {
                Modify_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF), 0);
                Modify_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF), 1);
            }
        }
        else if(selPath == 1)
        {
            Modify_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF), 0);
            Modify_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF), 1);
        }
        else
            return ReportError(EINVAL, "CalibrateRxSetup() - Invalid SEL_PATH");
    }

    //TBB
    //reset TBB to defaults
    SetDefaults(TBB);
    Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_FRP_TBB), 1);
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_GG_FRP_TBB), 6);

    //AFE
    Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);

    //BIAS
    {
        uint16_t rp_calib_bias = Get_SPI_Reg_bits(0x0084, 10, 6);
        SetDefaults(BIAS);
        Modify_SPI_Reg_bits(0x0084, 10, 6, rp_calib_bias);
    }

    //XBUF
    Modify_SPI_Reg_bits(0x0085, 2, 0, 1); //PD_XBUF_RX 0, PD_XBUF_TX 0, EN_G_XBUF 1

    //CGEN
    //reset CGEN to defaults
    const float_type cgenFreq = GetFrequencyCGEN();
    int cgenMultiplier = int(cgenFreq / 46.08e6 + 0.5);
    if(cgenMultiplier < 2)
        cgenMultiplier = 2;
    if(cgenMultiplier > 9 && cgenMultiplier < 12)
        cgenMultiplier = 12;
    if(cgenMultiplier > 13)
        cgenMultiplier = 13;
    //power up VCO
    Modify_SPI_Reg_bits(0x0086, 2, 2, 0);

    int status = SetFrequencyCGEN(46.08e6 * cgenMultiplier);
    if(status != 0)
        return status;

    Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    bool isTDD = Get_SPI_Reg_bits(LMS7param(PD_LOCH_T2RBUF), true) == 0;
    if(isTDD)
    {
        //in TDD do nothing
        Modify_SPI_Reg_bits(LMS7param(MAC), 1);
        SetDefaults(SX);
        status = SetFrequencySX(LMS7002M::Rx, GetFrequencySX(LMS7002M::Tx) - bandwidth_Hz / calibUserBwDivider - 9e6);
        if(status != 0)
            return status;
        Modify_SPI_Reg_bits(LMS7param(PD_VCO), 0);
    }
    else
    {
        //SXR
        Modify_SPI_Reg_bits(LMS7param(MAC), 1);
        float_type SXRfreqHz = GetFrequencySX(LMS7002M::Rx);

        //SXT
        Modify_SPI_Reg_bits(LMS7param(MAC), 2);
        SetDefaults(SX);
        Modify_SPI_Reg_bits(LMS7param(PD_VCO), 0);

        status = SetFrequencySX(LMS7002M::Tx, SXRfreqHz + bandwidth_Hz / calibUserBwDivider + 9e6);
        if(status != 0) return status;
    }
    Modify_SPI_Reg_bits(LMS7param(MAC), ch);

    //TXTSP
    SetDefaults(TxTSP);
    SetDefaults(TxNCO);
    Modify_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 0x1);
    Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), 1);
    Modify_SPI_Reg_bits(0x0208, 6, 4, 0x7); //GFIR3_BYP 1, GFIR2_BYP 1, GFIR1_BYP 1
    Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_TXTSP), 0);
    Modify_SPI_Reg_bits(LMS7param(CMIX_SC_TXTSP), 1);
    LoadDC_REG_IQ(Tx, (int16_t)0x7FFF, (int16_t)0x8000);
    SetNCOFrequency(LMS7002M::Tx, 0, 9e6);

    //RXTSP
    SetDefaults(RxTSP);
    SetDefaults(RxNCO);
    Modify_SPI_Reg_bits(0x040C, 4, 3, 0x3); //GFIR2_BYP, GFIR1_BYP
    Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), 4);
    if(not useFFT)
    {
        Modify_SPI_Reg_bits(LMS7param(AGC_MODE_RXTSP), 1);
        Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);
        Modify_SPI_Reg_bits(LMS7param(CAPSEL), 0);
        Modify_SPI_Reg_bits(LMS7param(AGC_AVG_RXTSP), 1);
        Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_RXTSP), 0);
        Modify_SPI_Reg_bits(LMS7param(GFIR3_L_RXTSP), 7);
        if(Get_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN)) == 1)
        {
            int clkh_ov = Get_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN));
            int gfir3n = 4 * cgenMultiplier/pow2(clkh_ov);
            gfir3n = pow2(int(log2(gfir3n)));
            Modify_SPI_Reg_bits(LMS7param(GFIR3_N_RXTSP), gfir3n - 1);
        }
        else
        {
            int gfir3n = 4 * cgenMultiplier;
            gfir3n = pow2(int(log2(gfir3n)));
            Modify_SPI_Reg_bits(LMS7param(GFIR3_N_RXTSP), gfir3n - 1);
        }
        SetGFIRCoefficients(Rx, 2, firCoefs, sizeof(firCoefs) / sizeof(int16_t));
    }
    else
    {
        Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), 1);
        Modify_SPI_Reg_bits(LMS7param(AGC_BYP_RXTSP), 1);
        Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);
        Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), 0);
    }
    if (Get_SPI_Reg_bits(LMS7_MASK, true) != 0)
        Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP), 1);

    //CDS
    Modify_SPI_Reg_bits(LMS7param(CDS_TXATSP), 3);
    Modify_SPI_Reg_bits(LMS7param(CDS_TXBTSP), 3);

    //RSSI_DC_CALIBRATION
    SetDefaults(RSSI_DC_CALIBRATION);

    SetNCOFrequency(LMS7002M::Rx, 0, bandwidth_Hz/calibUserBwDivider - offsetNCO);
    //modifications when calibrating channel B
    if(ch == 2)
    {
        Modify_SPI_Reg_bits(LMS7param(MAC), 1);
        Modify_SPI_Reg_bits(LMS7param(EN_NEXTRX_RFE), 1);
        Modify_SPI_Reg_bits(LMS7param(EN_NEXTTX_TRF), 1);
        Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0);
        Modify_SPI_Reg_bits(LMS7param(MAC), ch);
    }

#ifdef ENABLE_CALIBRATION_USING_FFT
    if(useExtLoopback || useFFT)
    {
        //limelight
        Modify_SPI_Reg_bits(LMS7param(LML1_FIDM), 0);
        Modify_SPI_Reg_bits(LMS7param(LML2_FIDM), 0);
        Modify_SPI_Reg_bits(LMS7param(LML1_MODE), 0);
        Modify_SPI_Reg_bits(LMS7param(LML2_MODE), 0);
        if(ch == 1)
        {
            Modify_SPI_Reg_bits(LMS7param(LML2_S0S), 1); //pos0, AQ
            Modify_SPI_Reg_bits(LMS7param(LML2_S1S), 0); //pos1, AI
        }
        else if(ch == 2)
        {
            Modify_SPI_Reg_bits(LMS7param(LML2_S0S), 3); //pos0, BQ
            Modify_SPI_Reg_bits(LMS7param(LML2_S1S), 2); //pos1, BI
        }
        //if decimation/interpolation is 0(2^1) or 7(bypass), interface clocks should not be divided
        int decimation = Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
        float interfaceRx_Hz = GetReferenceClk_TSP(LMS7002M::Rx);
        //need to adjust decimation to fit into USB speed
        float rateLimit_Bps;
        DeviceInfo info = dataPort->GetDeviceInfo();
        if(info.deviceName == GetDeviceName(LMS_DEV_STREAM))
            rateLimit_Bps = 110e6;
        else if(info.deviceName == GetDeviceName(LMS_DEV_LIMESDR))
            rateLimit_Bps = 200e6;
        else if(info.deviceName == GetDeviceName(LMS_DEV_ULIMESDR))
            rateLimit_Bps = 100e6;
        else
            rateLimit_Bps = 100e6;
#ifndef N_DEBUG
        rateLimit_Bps = 50e6; //debug mode is slower to receive data
#endif

        decimation = ceil(log2((interfaceRx_Hz*3/2)/rateLimit_Bps));
        if(decimation < 0)
            decimation = 0;
        if(decimation > 4)
            decimation = 4;
        if (decimation != 7)
            interfaceRx_Hz /= pow(2.0, decimation);
        int interpolation = Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
        float interfaceTx_Hz = GetReferenceClk_TSP(LMS7002M::Tx);
        if (interpolation != 7)
            interfaceTx_Hz /= pow(2.0, interpolation);
        const int channelsCount = 2;
        SetInterfaceFrequency(GetFrequencyCGEN(), interpolation, decimation);

        dataPort->UpdateExternalDataRate(0, interfaceTx_Hz/channelsCount, interfaceRx_Hz/channelsCount);

        GenerateWindowCoefficients(3, gFFTSize, windowF, amplitudeCorr);
        GenerateWindowCoefficients(3, gFFTSize/2, windowG, amplitudeCorrG);

        StreamConfig config;
        config.channelID = ch==1 ? 0 : 1;
        config.format = StreamConfig::STREAM_12_BIT_IN_16;
        config.linkFormat = StreamConfig::STREAM_12_BIT_COMPRESSED;
        config.isTx = false;
        config.bufferLength = 2*gFFTSize;
        config.performanceLatency = 1.0;

        dataPort->SetupStream(streamId, config);
        dataPort->ControlStream(streamId, true);
    }
#endif
    return 0;
}

/** @brief Calibrates Receiver. DC offset, IQ gains, IQ phase correction
    @return 0-success, other-failure
*/
int LMS7002M::CalibrateRx(float_type bandwidth_Hz, bool useExtLoopback)
{
    if(controlPort == nullptr)
        return ReportError(ENODEV, "Device not connected");
#ifdef __cplusplus
    auto beginTime = std::chrono::high_resolution_clock::now();
#endif
#ifndef ENABLE_CALIBRATION_USING_FFT
    if(useExtLoopback)
        return ReportError(EPERM, "External loopback calibration requires FFT module");
#else
    if(useExtLoopback)
        useFFT = true;
    if(mCalibrationByMCU)
    {
        useExtLoopback = false;
        useFFT = false;
    }
#endif // ENABLE_CALIBRATION_USING_FFT
    DeviceInfo info = controlPort->GetDeviceInfo();
    bool useOnBoardLoopback = (info.deviceName == GetDeviceName(LMS_DEV_LIMESDR) && std::stoi(info.hardwareVersion) >= 3);
    int status;
    verbose_printf(cSquaresLine);
    verbose_printf("Rx calibration using %s %s %s loopback\n",
        (useFFT ? "FFT" : "RSSI"),
        (useExtLoopback ? "EXTERNAL" : "INTERNAL"),
        useOnBoardLoopback ? "ON BOARD" : (useExtLoopback ? "CABLE" : "CHIP"));
    Channel ch = this->GetActiveChannel();
    uint32_t boardId = info.boardSerialNumber;
    uint8_t channel = ch == 1 ? 0 : 1;
    uint8_t sel_path_rfe = (uint8_t)Get_SPI_Reg_bits(LMS7param(SEL_PATH_RFE));
    int lna = sel_path_rfe;
    double rxFreq = GetFrequencySX(LMS7002M::Rx);

    const char* lnaName;
    switch(lna)
    {
        case 0: lnaName = "none"; break;
        case 1: lnaName = "LNAH"; break;
        case 2: lnaName = "LNAL"; break;
        case 3: lnaName = "LNAW"; break;
    }
    verbose_printf("Rx ch.%s @ %4g MHz, BW: %g MHz, RF input: %s, PGA: %i, LNA: %i, TIA: %i\n",
                ch == Channel::ChA ? "A" : "B", rxFreq/1e6,
                bandwidth_Hz/1e6, lnaName,
                Get_SPI_Reg_bits(LMS7param(G_PGA_RBB)),
                Get_SPI_Reg_bits(LMS7param(G_LNA_RFE)),
                Get_SPI_Reg_bits(LMS7param(G_TIA_RFE)));

    int16_t dcoffi(0);
    int16_t dcoffq(0);
    uint16_t gcorri(0);
    uint16_t gcorrq(0);
    int16_t phaseOffset(0);

    if(useCache)
    {
        int dcI, dcQ, gainI, gainQ, phOffset;
        bool foundInCache = (mValueCache->GetDC_IQ(boardId, rxFreq, channel, false, lna, &dcI, &dcQ, &gainI, &gainQ, &phOffset) == 0);
        dcoffi = dcI;
        dcoffq = dcQ;
        gcorri = gainI;
        gcorrq = gainQ;
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
            int8_t dcIsigned = (dcI & 0x3f) * (dcI&0x40 ? -1 : 1);
            int8_t dcQsigned = (dcQ & 0x3f) * (dcQ&0x40 ? -1 : 1);
            int16_t phaseSigned = phOffset << 4;
            phaseSigned >>= 4;
            verbose_printf("Rx calibration values found in cache:\n");
            verbose_printf("   | DC  | GAIN | PHASE\n");
            verbose_printf("---+-----+------+------\n");
            verbose_printf("I: | %3i | %4i | %i\n", dcIsigned, gainI, phaseSigned);
            verbose_printf("Q: | %3i | %4i |\n", dcQsigned, gainQ);
            verbose_printf(cSquaresLine);
            return 0;
        }
    }

    verbose_printf("Performed by: %s\n", mCalibrationByMCU ? "MCU" : "PC");
    verbose_printf(cDashLine);
    LMS7002M_SelfCalState state(this);
    auto registersBackup = BackupRegisterMap();
    if(mCalibrationByMCU && not useExtLoopback)
    {
        uint8_t mcuID = mcuControl->ReadMCUProgramID();
        verbose_printf("Current MCU firmware: %i, %s\n", mcuID,
            mcuID == MCU_ID_CALIBRATIONS_SINGLE_IMAGE ? "DC/IQ calibration full" : "unknown");
        if(mcuID != MCU_ID_CALIBRATIONS_SINGLE_IMAGE)
        {
            verbose_printf("Uploading DC/IQ calibration firmware\n");
            status = mcuControl->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, IConnection::MCU_PROG_MODE::SRAM);
            if(status != 0)
                return status;
        }

        //set reference clock parameter inside MCU
        long refClk = GetReferenceClk_SX(false);
        mcuControl->SetParameter(MCU_BD::MCU_REF_CLK, refClk);
        verbose_printf("MCU Ref. clock: %g MHz\n", refClk / 1e6);
        //set bandwidth for MCU to read from register, value is integer stored in MHz
        mcuControl->SetParameter(MCU_BD::MCU_BW, bandwidth_Hz);
        mcuControl->RunProcedure(MCU_FUNCTION_CALIBRATE_RX);
        status = mcuControl->WaitForMCU(1000);
        if(status != 0)
        {
            ReportError("MCU working too long %i", status);
        }
        //need to read back calibration results
        //dcoffi = Get_SPI_Reg_bits(LMS7param(DCOFFI_RFE), true);
        //dcoffq = Get_SPI_Reg_bits(LMS7param(DCOFFQ_RFE), true);
        gcorri = Get_SPI_Reg_bits(LMS7param(GCORRI_RXTSP), true);
        gcorrq = Get_SPI_Reg_bits(LMS7param(GCORRQ_RXTSP), true);
        phaseOffset = Get_SPI_Reg_bits(LMS7param(IQCORR_RXTSP), true);

        if(useCache)
            mValueCache->InsertDC_IQ(boardId, rxFreq, channel, false, lna, /*dcoffi*/0, /*dcoffq*/0, gcorri, gcorrq, phaseOffset);

        return status;
    }

    Log("Rx calibration started", LOG_INFO);
    Log("Saving registers state", LOG_INFO);
    if(Get_SPI_Reg_bits(LMS7_MASK) == 0)
    {
        if((sel_path_rfe == 1 || sel_path_rfe == 0) && not useExtLoopback)
            return ReportError(EINVAL, "Rx Calibration: bad SEL_PATH");
    }
    else
    {
        if((sel_path_rfe == 0) && not useExtLoopback)
            return ReportError(EINVAL, "Rx Calibration: bad SEL_PATH");
    }

    Log("Setup stage", LOG_INFO);
    status = CalibrateRxSetup(bandwidth_Hz, useExtLoopback);
    if(status != 0)
        goto RxCalibrationEndStage;

#ifdef ENABLE_CALIBRATION_USING_FFT
    {
        //calculate NCO offset, that the signal would be in FFT bin
        float_type binWidth = GetSampleRate(LMS7002M::Rx, ch)/gFFTSize;
        offsetNCO = int(0.1e6 / binWidth+0.5)*binWidth+binWidth/2;
    }
    srcBin = gFFTSize*offsetNCO/GetSampleRate(LMS7002M::Rx, ch);
    SelectFFTBin(dataPort, srcBin);
#endif // ENABLE_CALIBRATION_USING_FFT
    Log("Rx DC calibration", LOG_INFO);
    CalibrateRxDCAuto();
    if(useExtLoopback && useOnBoardLoopback)
    {
        status = SetExtLoopback(dataPort, ch, true);
        if(status != 0)
            goto RxCalibrationEndStage;
    }
    else
    {
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
    }

    Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    if (Get_SPI_Reg_bits(LMS7param(PD_LOCH_T2RBUF)) == false)
    {
        Modify_SPI_Reg_bits(LMS7param(PD_LOCH_T2RBUF), 1);
        //TDD MODE
        Modify_SPI_Reg_bits(LMS7param(MAC), 1);
        Modify_SPI_Reg_bits(LMS7param(PD_VCO), 0);
    }
    Modify_SPI_Reg_bits(LMS7param(MAC), ch);

    CheckSaturationRx(bandwidth_Hz, useExtLoopback);

    if (Get_SPI_Reg_bits(LMS7_MASK, true) != 0)
        Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP), 0);
    else
        Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 0);
    SetNCOFrequency(LMS7002M::Rx, 0, bandwidth_Hz/calibUserBwDivider + offsetNCO);

    CalibrateIQImbalance(LMS7002M::Rx, &gcorri, &gcorrq, &phaseOffset);

    dcoffi = Get_SPI_Reg_bits(LMS7param(DCOFFI_RFE), true);
    dcoffq = Get_SPI_Reg_bits(LMS7param(DCOFFQ_RFE), true);
RxCalibrationEndStage:
#ifdef ENABLE_CALIBRATION_USING_FFT
    dataPort->ControlStream(streamId, false);
    dataPort->CloseStream(streamId);
#endif
    Log("Restoring registers state", LOG_INFO);
    RestoreRegisterMap(registersBackup);
    UploadAll();

    if(useExtLoopback && useOnBoardLoopback)
        SetExtLoopback(dataPort, ch, false);

    if (status != 0)
    {
        Log("Rx calibration failed", LOG_WARNING);
        return status;
    }
    if(useCache)
        mValueCache->InsertDC_IQ(boardId, rxFreq, channel, false, lna, dcoffi, dcoffq, gcorri, gcorrq, phaseOffset);

    Modify_SPI_Reg_bits(LMS7param(MAC), ch);

    Modify_SPI_Reg_bits(LMS7param(DCMODE), 1);
    if(ch == 1)
        Modify_SPI_Reg_bits(LMS7param(PD_DCDAC_RXA), 0);
    else
        Modify_SPI_Reg_bits(LMS7param(PD_DCDAC_RXB), 0);

    Modify_SPI_Reg_bits(LMS7param(GCORRI_RXTSP), gcorri);
    Modify_SPI_Reg_bits(LMS7param(GCORRQ_RXTSP), gcorrq);
    Modify_SPI_Reg_bits(LMS7param(IQCORR_RXTSP), phaseOffset);
    Modify_SPI_Reg_bits(0x040C, 2, 0, 0); //DC_BYP 0, GC_BYP 0, PH_BYP 0
    Log("Rx calibration finished", LOG_INFO);
#ifdef LMS_VERBOSE_OUTPUT
    verbose_printf("#####Rx calibration RESULTS:###########################\n");
    verbose_printf("Method: %s %s loopback\n",
        (useFFT ? "FFT" : "RSSI"),
        (useExtLoopback ? "EXTERNAL" : "INTERNAL"));
    verbose_printf("Rx ch.%s @ %4g MHz, BW: %g MHz, RF input: %s, PGA: %i, LNA: %i, TIA: %i\n",
                ch == Channel::ChA ? "A" : "B", rxFreq/1e6,
                bandwidth_Hz/1e6, lnaName,
                Get_SPI_Reg_bits(LMS7param(G_PGA_RBB)),
                Get_SPI_Reg_bits(LMS7param(G_LNA_RFE)),
                Get_SPI_Reg_bits(LMS7param(G_TIA_RFE)));
    {
    int8_t dcIsigned = (dcoffi & 0x3f) * (dcoffi&0x40 ? -1 : 1);
    int8_t dcQsigned = (dcoffq & 0x3f) * (dcoffq&0x40 ? -1 : 1);
    int16_t phaseSigned = phaseOffset << 4;
    phaseSigned >>= 4;
    verbose_printf("   | DC  | GAIN | PHASE\n");
    verbose_printf("---+-----+------+------\n");
    verbose_printf("I: | %3i | %4i | %i\n", dcIsigned, gcorri, phaseSigned);
    verbose_printf("Q: | %3i | %4i |\n", dcQsigned, gcorrq);
    }
#ifdef __cplusplus
    int32_t duration = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::high_resolution_clock::now()-beginTime).count();
    verbose_printf("Duration: %i ms\n", duration);
#endif
    verbose_printf(cSquaresLine);
#endif //LMS_VERBOSE_OUTPUT
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
    GetGFIRCoefficients(LMS7002M::Rx, 2, rxGFIR3_backup, sizeof(rxGFIR3_backup)/sizeof(int16_t));
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
    SetGFIRCoefficients(LMS7002M::Rx, 2, rxGFIR3_backup, sizeof(rxGFIR3_backup)/sizeof(int16_t));
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

int LMS7002M::CheckSaturationRx(const float_type bandwidth_Hz, const bool useExtLoopback)
{
    if (Get_SPI_Reg_bits(LMS7_MASK, true) != 0)
        Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP), 1);
    else
        Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP), 0);
    Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 0);
    SetNCOFrequency(LMS7002M::Rx, 0, bandwidth_Hz / calibUserBwDivider - offsetNCO);

    uint32_t rssi = GetRSSI();
#ifdef ENABLE_CALIBRATION_USING_FFT
    SelectFFTBin(dataPort, srcBin);
    if(useExtLoopback)
    {
        const uint32_t target_rssi = dBFS_2_RSSI(-14.0);
        int loss_main_txpad = Get_SPI_Reg_bits(LMS7param(LOSS_MAIN_TXPAD_TRF));
        while (rssi < target_rssi && loss_main_txpad > 0)
        {
            rssi = GetRSSI();
            if (rssi < target_rssi)
                loss_main_txpad -= 1;
            if (rssi > target_rssi)
                break;
            Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), loss_main_txpad);
        }

        int cg_iamp = Get_SPI_Reg_bits(LMS7param(CG_IAMP_TBB));
        while (rssi < target_rssi && cg_iamp < 39)
        {
            rssi = GetRSSI();
            if (rssi < target_rssi)
                cg_iamp += 2;
            if (rssi > target_rssi)
                break;
            Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), cg_iamp);
        }
        return 0;
    }
#endif
    const uint32_t target_rssi = 0x07000; //0x0B000 = -3 dBFS
    int g_rxloopb_rfe = Get_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE));
    int cg_iamp = 1;
    Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), cg_iamp);
    while (rssi < target_rssi && g_rxloopb_rfe  < 15)
    {
        rssi = GetRSSI();
        if (rssi < target_rssi)
            g_rxloopb_rfe += 2;
        if (rssi > target_rssi)
            break;
        Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), g_rxloopb_rfe);
    }

    while (rssi < 0x01000 && cg_iamp < 63-6)
    {
        rssi = GetRSSI();
        if (rssi < 0x01000)
            cg_iamp += 2;
        if (rssi > 0x01000)
            break;
        Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), cg_iamp);
    }

    while (rssi < target_rssi && cg_iamp < 62)
    {
        rssi = GetRSSI();
        if (rssi < target_rssi)
            cg_iamp += 1;
        Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), cg_iamp);
    }
    return 0;
}

static uint16_t toDCOffset(int16_t offset)
{
    uint16_t valToSend = 0;
    if (offset < 0)
        valToSend |= 0x40;
    valToSend |= labs(offset);
    return valToSend;
}

void LMS7002M::GridSearch(GridSearchParam* args)
{
    const uint16_t DCOFFaddr = 0x010E;
    const bool rxDC = (args->a.param.address == DCOFFaddr || args->b.param.address == DCOFFaddr);
    uint32_t rssi, minRSSI = ~0;
    int16_t minI(0), minQ(0), i, q;

    for(i=args->a.minValue; i<=args->a.maxValue; ++i)
    {
        Modify_SPI_Reg_bits(args->a.param, rxDC ? toDCOffset(i) : i);
        for(q=args->b.minValue; q<=args->b.maxValue; ++q)
        {
            Modify_SPI_Reg_bits(args->b.param, rxDC ? toDCOffset(q) : q);
            rssi = GetRSSI();
            if (rssi < minRSSI)
            {
                minI = i;
                minQ = q;
                minRSSI = rssi;
                args->signalLevel = minRSSI;
            }
#ifdef DRAW_GNU_PLOTS
            if(args->a.target == TX_DC_I || args->b.target == TX_DC_Q)
                gRSSI_TXDC_measurements[q+128][i+128] = RSSI_2_dBFS(rssi);
#endif
        }
    }
    args->a.result = minI;
    args->b.result = minQ;
}

void LMS7002M::BinarySearch(BinSearchParam* args)
{
    const bool rxDC = args->param.address == LMS7param(DCOFFI_RFE).address;
    uint32_t rssiLeft = ~0;
    uint32_t rssiRight;
    int16_t left = args->minValue;
    int16_t right = args->maxValue;
    int16_t step;
#ifdef DRAW_GNU_PLOTS
    bool scan = true;
    RSSI_measurements measurements;
    std::vector<float> scanX;
    std::vector<float> scanY;
    std::vector<float> searchPoints;
#endif
    Modify_SPI_Reg_bits(args->param, rxDC ? toDCOffset(right) : right);
    rssiRight = GetRSSI();
#ifdef DRAW_GNU_PLOTS
    searchPoints.push_back(right); searchPoints.push_back(rssiRight);
#endif

    while(right-left >= 1)
    {
        step = (right-left)/2;
        if(rssiLeft < rssiRight)
        {
            Modify_SPI_Reg_bits(args->param, rxDC ? toDCOffset(right) : right);
            rssiRight = GetRSSI();
#ifdef DRAW_GNU_PLOTS
            searchPoints.push_back(right); searchPoints.push_back(rssiRight);
#endif
        }
        else
        {
            Modify_SPI_Reg_bits(args->param, rxDC ? toDCOffset(left) : left);
            rssiLeft = GetRSSI();
#ifdef DRAW_GNU_PLOTS
            searchPoints.push_back(left); searchPoints.push_back(rssiLeft);
#endif
        }
        if(step == 0)
            break;
        if(rssiLeft < rssiRight)
            right -= step;
        else
            left += step;
    }

    args->result = rssiLeft < rssiRight ? left : right;
    Modify_SPI_Reg_bits(args->param, rxDC ? toDCOffset(args->result) : args->result);
#ifdef DRAW_GNU_PLOTS
    int value = args->result;
    searchPlot.write("plot\
'-' u 1:2 with points ps 4 pt 23 notitle,\
'-' u 1:2:3 with labels offset 0.5,0.5 notitle,\
'-' u 1:2 with points title 'search'");
    if(scan)
        searchPlot.write(",\
'-' u 1:2 with line title 'scan',\
'-' u 1:2 with points notitle");
    searchPlot.write("\n");
    searchPlot.writef("%i %f\ne\n", value, rssiLeft < rssiRight ? RSSI_2_dBFS(rssiLeft) : RSSI_2_dBFS(rssiRight));
    for(uint32_t i=0; i<searchPoints.size()/2; ++i)
        searchPlot.writef("%f %f %i\n", searchPoints[2*i], float(useFFT ? RSSI_2_dBFS(searchPoints[2*i+1]) : searchPoints[2*i+1]), i);
    searchPlot.write("e\n");
    for(uint32_t i=0; i<searchPoints.size()/2; ++i)
        searchPlot.writef("%f %f\n", searchPoints[2*i], float(useFFT ? RSSI_2_dBFS(searchPoints[2*i+1]) : searchPoints[2*i+1]));
    searchPlot.write("e\n");
    if(scan)
    {
        std::vector<float> trendX;
        std::vector<float> trendY;
        for(int i=args->minValue; i<=args->maxValue; )
        {
            Modify_SPI_Reg_bits(args->param, rxDC ? toDCOffset(i) : i);
            scanX.push_back(i);
            uint32_t rssi = GetRSSI();
            scanY.push_back(RSSI_2_dBFS(rssi));
            if( abs(value - i) <= 32)
            {
                trendX.push_back(i);
                trendY.push_back(RSSI_2_dBFS(rssi));
                i += 1;
            }
            else
                i += 5;
        }
        for(int j=0; j<2; ++j)
        {
            for(uint32_t i=0; i<scanX.size(); ++i)
                searchPlot.writef("%f %f\n", scanX[i], scanY[i]);
            searchPlot.write("e\n");
        }

        if(args->target == TX_DC_I || args->target == TX_DC_Q)
        {
            int8_t staticValue = (args->target == TX_DC_I) ? Get_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP), true) : Get_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP), true);
            for(uint32_t i=0; i<scanX.size(); ++i)
            {
                if(args->target == TX_DC_I)
                    gRSSI_TXDC_measurements[staticValue+128][int(scanX[i])+128] = scanY[i];
                else if(args->target == TX_DC_Q)
                    gRSSI_TXDC_measurements[int(scanX[i])+128][staticValue+128] = scanY[i];
            }
        }
    }
    searchPlot.flush();
    Modify_SPI_Reg_bits(args->param, rxDC ? toDCOffset(value) : value);
#endif
}

void LMS7002M::CoarseSearch(const uint16_t addr, const uint8_t msb, const uint8_t lsb, int16_t &value, const uint8_t maxIterations)
{
    const uint16_t DCOFFaddr = 0x010E;
    uint8_t rssi_counter = 0;
    uint32_t rssiUp;
    uint32_t rssiDown;

#ifdef DRAW_GNU_PLOTS
    RSSI_measurements measurements;
    std::vector<float> searchPoints;
    std::vector<float> sweepPoints;
    std::vector<float> fftSweepValues;
    std::vector<float> geortzSweepFloat;
    std::vector<float> geortzSweepFPGA;
    std::vector<float> minM;
    std::vector<float> maxM;
    std::vector< std::vector<float> >avgs;
    avgs.resize(avgCount);
    bool scan = false;
    for(int i=0; i<pow2(maxIterations) && scan; ++i)
    {
        Modify_SPI_Reg_bits(addr, msb, lsb, addr != DCOFFaddr ? value-i : toDCOffset(value-i));
        sweepPoints.push_back(value-i);
        measurements.clear();
        GetRSSI(&measurements);

        fftSweepValues.push_back(measurements.amplitudeFFT[0]);
        geortzSweepFloat.push_back(measurements.amplitudeGeortzelF[0]);
        geortzSweepFPGA.push_back(measurements.amplitudeGeortzelFPGA[0]);
    }
#endif

    Modify_SPI_Reg_bits(addr, msb, lsb, addr != DCOFFaddr ? value : toDCOffset(value));
    for(rssi_counter = 0; rssi_counter < maxIterations - 1; ++rssi_counter)
    {
        rssiUp = GetRSSI();
    #ifdef DRAW_GNU_PLOTS
        searchPoints.push_back(value); searchPoints.push_back(rssiUp);
    #endif
        value -= pow2(maxIterations - rssi_counter);
        Modify_SPI_Reg_bits(addr, msb, lsb, addr != DCOFFaddr ? value : toDCOffset(value));
        rssiDown = GetRSSI();
    #ifdef DRAW_GNU_PLOTS
        searchPoints.push_back(value); searchPoints.push_back(rssiDown);
    #endif
        if(rssiUp >= rssiDown)
            value += pow2(maxIterations - 2 - rssi_counter);
        else
            value = value + pow2(maxIterations - rssi_counter) + pow2(maxIterations - 1 - rssi_counter) - pow2(maxIterations - 2 - rssi_counter);
        Modify_SPI_Reg_bits(addr, msb, lsb, addr != DCOFFaddr ? value : toDCOffset(value));
    }
    value -= pow2(maxIterations - rssi_counter);
    Modify_SPI_Reg_bits(addr, msb, lsb, addr != DCOFFaddr ? value : toDCOffset(value));

    rssiUp = GetRSSI();
#ifdef DRAW_GNU_PLOTS
    searchPoints.push_back(value); searchPoints.push_back(rssiUp);
#endif
    if(addr != DCOFFaddr)
        Modify_SPI_Reg_bits(addr, msb, lsb, value - pow2(maxIterations - rssi_counter));
    else
        Modify_SPI_Reg_bits(addr, msb, lsb, toDCOffset(value - pow2(maxIterations - rssi_counter)));
    rssiDown = GetRSSI();
#ifdef DRAW_GNU_PLOTS
    searchPoints.push_back(value); searchPoints.push_back(rssiDown);
#endif
    if(rssiUp < rssiDown)
        value += 1;
    Modify_SPI_Reg_bits(addr, msb, lsb, addr != DCOFFaddr ? value : toDCOffset(value));
    rssiDown = GetRSSI();
#ifdef DRAW_GNU_PLOTS
    searchPoints.push_back(value); searchPoints.push_back(rssiDown);
#endif

    if(rssiUp < rssiDown)
    {
        value += 1;
        Modify_SPI_Reg_bits(addr, msb, lsb, addr != DCOFFaddr ? value : toDCOffset(value));
#ifdef DRAW_GNU_PLOTS
        searchPoints.push_back(value); searchPoints.push_back(GetRSSI());
#endif
    }
#ifdef DRAW_GNU_PLOTS
    searchPlot.write("plot\
'-' u 1:2:3 with labels offset 0.5,0.5 notitle,\
'-' u 1:2 with points title 'search'");
    if(scan)
        searchPlot.write(",\
'-' u 1:2 with lines title 'geortzFPGA'");
    searchPlot.write("\n");
    for(uint32_t i=0; i<searchPoints.size()/2; ++i)
    {
        printf("%f %f\n", searchPoints[2*i], float(useFFT ? RSSI_2_dBFS(searchPoints[2*i+1]) : searchPoints[2*i+1]));
        searchPlot.writef("%f %f %i\n", searchPoints[2*i], float(useFFT ? RSSI_2_dBFS(searchPoints[2*i+1]) : searchPoints[2*i+1]), i);
    }
    searchPlot.write("e\n");
    for(uint32_t i=0; i<searchPoints.size()/2; ++i)
        searchPlot.writef("%f %f\n", searchPoints[2*i], float(useFFT ? RSSI_2_dBFS(searchPoints[2*i+1]) : searchPoints[2*i+1]));
    searchPlot.write("e\n");
    if(scan)
    {
        for(uint32_t i=0; i<geortzSweepFPGA.size(); ++i)
            searchPlot.writef("%f %f\n", sweepPoints[i], geortzSweepFPGA[i]);
        searchPlot.write("e\n");
    }
    searchPlot.flush();
#endif
}

int LMS7002M::CheckSaturationTxRx(const float_type bandwidth_Hz, const bool useExtLoopback)
{
    verbose_printf(cDashLine);
#ifdef ENABLE_CALIBRATION_USING_FFT
#ifdef DRAW_GNU_PLOTS
    std::vector<float> pgaFirstStage;
    std::vector<float> lnaStage;
    std::vector<float> tiaStage;
    std::vector<float> pgaSecondStage;
#endif
    if(useExtLoopback)
    {
        SetNCOFrequency(LMS7002M::Rx, 0, calibrationSXOffset_Hz - offsetNCO + bandwidth_Hz / calibUserBwDivider);
        uint32_t rssi_prev;
        uint32_t rssi;
        uint32_t target_rssi;
        const float txFreq = GetFrequencySX(LMS7002M::Tx);
        if(txFreq < 1e9)
            target_rssi = dBFS_2_RSSI(-15.0);
        else
            target_rssi = dBFS_2_RSSI(-10.0);
        verbose_printf("Receiver gains search, target level: %3.2f dBFS\n",
                        RSSI_2_dBFS(target_rssi));
        int g_tia = Get_SPI_Reg_bits(LMS7param(G_TIA_RFE));
        int g_lna = Get_SPI_Reg_bits(LMS7param(G_LNA_RFE));
        int g_pga = Get_SPI_Reg_bits(LMS7param(G_PGA_RBB));
        rssi_prev = rssi = GetRSSI();
        verbose_printf("Initial:  PGA: %2i, LNA: %2i, TIA: %2i, %3.2f dbFS\n",
                        g_pga, g_lna, g_tia, RSSI_2_dBFS(rssi));
        {
            float_type pgaDiff_dBFS = RSSI_2_dBFS(target_rssi) - RSSI_2_dBFS(rssi);
            g_pga = pgaDiff_dBFS > 9 ? 9 : int8_t(pgaDiff_dBFS);
            Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), g_pga);
            rssi = GetRSSI();
        }
        while(rssi < target_rssi && g_pga < 9)
        {
            ++g_pga;
            Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), g_pga);
            rssi = GetRSSI();
            verbose_printf("Adjusted: PGA: %2i, LNA: %2i, TIA: %2i, %3.2f dbFS\r",
                            g_pga, g_lna, g_tia, RSSI_2_dBFS(rssi));
#ifdef DRAW_GNU_PLOTS
            pgaFirstStage.push_back(RSSI_2_dBFS(rssi));
#endif
            if(rssi < rssi_prev)
            {
                --g_pga;
                Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), g_pga);
                rssi = rssi_prev;
                break;
            }
            else
                rssi_prev = rssi;
        }

#ifdef DRAW_GNU_PLOTS
        lnaStage.push_back(RSSI_2_dBFS(rssi));
#endif
        {
            float_type lnaDiff_dBFS = RSSI_2_dBFS(target_rssi) - RSSI_2_dBFS(rssi);
            while(lnaDiff_dBFS > 0 && g_lna < 15)
            {
                if(g_lna < 8)
                    lnaDiff_dBFS -= 3.0;
                else
                    lnaDiff_dBFS -= 1.0;
                ++g_lna;
            }
            Modify_SPI_Reg_bits(LMS7param(G_LNA_RFE), g_lna);
            rssi = GetRSSI();
        }
        while(rssi < target_rssi && g_lna < 15)
        {
            ++g_lna;
            Modify_SPI_Reg_bits(LMS7param(G_LNA_RFE), g_lna);
            rssi = GetRSSI();
            verbose_printf("Adjusted: PGA: %2i, LNA: %2i, TIA: %2i, %3.2f dbFS\r",
                            g_pga, g_lna, g_tia, RSSI_2_dBFS(rssi));
#ifdef DRAW_GNU_PLOTS
            lnaStage.push_back(RSSI_2_dBFS(rssi));
#endif
            if(rssi < rssi_prev)
            {
                --g_lna;
                Modify_SPI_Reg_bits(LMS7param(G_LNA_RFE), g_lna);
                rssi = rssi_prev;
                break;
            }
            else
                rssi_prev = rssi;
        }

        if(g_lna > 15)
            g_lna = 15;
        Modify_SPI_Reg_bits(LMS7param(G_LNA_RFE), g_lna);
        verbose_printf("Adjusted: PGA: %2i, LNA: %2i, TIA: %2i, %3.2f dbFS\r",
                            g_pga, g_lna, g_tia, RSSI_2_dBFS(rssi));

#ifdef DRAW_GNU_PLOTS
        tiaStage.push_back(RSSI_2_dBFS(rssi));
#endif
        while(rssi < target_rssi && g_tia < 3)
        {
            ++g_tia;
            Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE), g_tia);
            rssi = GetRSSI();
            verbose_printf("Adjusted: PGA: %2i, LNA: %2i, TIA: %2i, %3.2f dbFS\r",
                            g_pga, g_lna, g_tia, RSSI_2_dBFS(rssi));
#ifdef DRAW_GNU_PLOTS
            tiaStage.push_back(RSSI_2_dBFS(rssi));
#endif
            if(rssi < rssi_prev)
            {
                --g_tia;
                Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE), g_tia);
                rssi = rssi_prev;
                break;
            }
            else
                rssi_prev = rssi;
        }
        if(g_tia > 3)
            g_tia = 3;
        Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE), g_tia);
        verbose_printf("Adjusted: PGA: %2i, LNA: %2i, TIA: %2i, %3.2f dbFS\r",
                            g_pga, g_lna, g_tia, RSSI_2_dBFS(rssi));
#ifdef DRAW_GNU_PLOTS
        pgaSecondStage.push_back(RSSI_2_dBFS(rssi));
#endif
        while(rssi < target_rssi && g_pga < 30)
        {
            const int8_t stepSize = 1;
            g_pga += stepSize;
            Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), g_pga);
            rssi = GetRSSI();
            verbose_printf("Adjusted: PGA: %2i, LNA: %2i, TIA: %2i, %3.2f dbFS\r",
                            g_pga, g_lna, g_tia, RSSI_2_dBFS(rssi));
#ifdef DRAW_GNU_PLOTS
            pgaSecondStage.push_back(RSSI_2_dBFS(rssi));
#endif
            float delta_dBFS = RSSI_2_dBFS(rssi)-RSSI_2_dBFS(rssi_prev);
            if(rssi < rssi_prev || delta_dBFS < 0.92)
            {
                g_pga -= stepSize;
                Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), g_pga);
                break;
            }
            else
                rssi_prev = rssi;
        }
        Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), g_pga);
        rssi = GetRSSI();
#ifdef DRAW_GNU_PLOTS
        pgaSecondStage.push_back(RSSI_2_dBFS(rssi));

        saturationPlot.write("set yrange [:0]\n");
        saturationPlot.write("set title 'Rx gains search'\n");
        saturationPlot.write("set key right bottom\n");
        saturationPlot.write("plot\
'-' u 1:2 with lines title 'PGA',\
'-' u 1:2 with lines title 'LNA',\
'-' u 1:2 with lines title 'TIA',\
'-' u 1:2 with lines title 'PGA',\
'-' u 1:2 with lines title 'target Level'\n");
        int index = 1;
        const std::vector<std::vector<float> *> arrays =
            {&pgaFirstStage, &lnaStage, &tiaStage, &pgaSecondStage};
        for(auto a : arrays)
        {
            --index;
            for(size_t i=0; i<a->size(); ++i)
                saturationPlot.writef("%i %f\n", index++, (*a)[i]);
            saturationPlot.write("e\n");
        }
        saturationPlot.writef("%i %f\n%i %f\ne\n", 0, RSSI_2_dBFS(target_rssi),
                               index, RSSI_2_dBFS(target_rssi));
        saturationPlot.flush();
#endif
        verbose_printf("Adjusted: PGA: %2i, LNA: %2i, TIA: %2i, %3.2f dbFS",
                            g_pga, g_lna, g_tia, RSSI_2_dBFS(rssi));
        if(rssi < dBFS_2_RSSI(-30))
        {
            verbose_printf(" | TOO LOW!!!\n");
            return ReportError(-1,
                "Tx calibration: Rx gains search, RF level too low %.2f",
                RSSI_2_dBFS(rssi));
        }
        verbose_printf("\n");

        Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);
        CalibrateRxDC();
        Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 0);

        SetNCOFrequency(LMS7002M::Rx, 0, calibrationSXOffset_Hz + offsetNCO + bandwidth_Hz / calibUserBwDivider);
        Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP), 1);

        //---------IQ calibration-----------------
        CalibrateIQImbalance(LMS7002M::Rx);
        Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP), 0);
        return 0;
    }
#endif // ENABLE_CALIBRATION_USING_FFT
    //----------------------------------------
    //CalibrateRxDCAuto();
    SetNCOFrequency(LMS7002M::Rx, 0, calibrationSXOffset_Hz - offsetNCO + (bandwidth_Hz / calibUserBwDivider) * 2);
    Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 0);

    uint32_t rssi = GetRSSI();

    int8_t g_pga = Get_SPI_Reg_bits(LMS7param(G_PGA_RBB));
    int8_t g_rxloop = Get_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE));
    uint32_t saturationLevel = 0x05000;
    verbose_printf("Receiver saturation search, target level: ");
#ifdef ENABLE_CALIBRATION_USING_FFT
    if(useFFT)
    {
        saturationLevel = dBFS_2_RSSI(-22.0);
        verbose_printf("%3.2f dBFS\n", RSSI_2_dBFS(saturationLevel));
    }

    else
#endif // ENABLE_CALIBRATION_USING_FFT
        verbose_printf("%i \n", saturationLevel);

    verbose_printf("initial  PGA: %2i, RxLoopback: %2i, %3.2f dbFS\n", g_pga, g_rxloop, RSSI_2_dBFS(rssi));
    while(rssi < saturationLevel && g_rxloop < 15)
    {
        rssi = GetRSSI();
        verbose_printf("adjusted PGA: %2i, RxLoopback: %2i, %3.2f dbFS\r", g_pga, g_rxloop, RSSI_2_dBFS(rssi));
        if(rssi < saturationLevel)
        {
            g_rxloop += 1;
            Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), g_rxloop);
        }
        else
            break;
    }
    rssi = GetRSSI();
    while(g_pga < 18 && g_rxloop == 15 && rssi < saturationLevel)
    {
        g_pga += 1;
        Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), g_pga);
        rssi = GetRSSI();
        verbose_printf("adjusted PGA: %2i, RxLoopback: %2i, %3.2f dbFS\r", g_pga, g_rxloop, RSSI_2_dBFS(rssi));
    }
    verbose_printf("\n");
    Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(DC_BYP_RXTSP), 1);
    return 0;
}

void LMS7002M::CalibrateTxDC(int16_t *dccorri, int16_t *dccorrq)
{
    BinSearchParam argsI;
    BinSearchParam argsQ;
#ifdef DRAW_GNU_PLOTS
    memset(gRSSI_TXDC_measurements, 0, sizeof(gRSSI_TXDC_measurements));
    searchPlot.write("set title 'Tx DC search\n");
    spectrumPlot.write("set title 'Tx DC search\n");
#endif // DRAW_GNU_PLOTS
    verbose_printf("Searching Tx DC...\n");
    Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), 1);
    Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP), 0);
    Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 0);

    int8_t corrI = 0;
    int8_t corrQ = 0;
    //Modify_SPI_Reg_bits(DCCORRI_TXTSP, 0);
    //Modify_SPI_Reg_bits(DCCORRQ_TXTSP, 0);
    Modify_SPI_Reg_bits(0x0204, 15, 0, (corrI << 8 | corrQ));

    //find I
    avgCount = 1;
    argsI.param = LMS7param(DCCORRI_TXTSP);
    argsI.maxValue = corrI+127;
    argsI.minValue = corrI-128;
    argsI.target = TX_DC_I;
    BinarySearch(&argsI);
    corrI = argsI.result;
    //Modify_SPI_Reg_bits(DCCORRI_TXTSP, corrI);

    argsQ.param = LMS7param(DCCORRQ_TXTSP);
    argsQ.maxValue = corrQ+127;
    argsQ.minValue = corrQ-128;
    argsQ.target = TX_DC_Q;
    BinarySearch(&argsQ);
    corrQ = argsQ.result;
    //Modify_SPI_Reg_bits(DCCORRQ_TXTSP, corrQ);

    avgCount = 2;
    argsI.maxValue = corrI+4;
    argsI.minValue = corrI-4;
    BinarySearch(&argsI);
    corrI = argsI.result;
    //Modify_SPI_Reg_bits(DCCORRI_TXTSP, corrI);

    argsQ.maxValue = corrQ+4;
    argsQ.minValue = corrQ-4;
    BinarySearch(&argsQ);
    corrQ = argsQ.result;
    //Modify_SPI_Reg_bits(DCCORRQ_TXTSP, corrQ);
    verbose_printf("coarse: Tx DCCORRI: %i, DCCORRQ: %i\n", corrI, corrQ);

    const int gridRadius = 2;
    GridSearchParam gridArgs;
    argsI.maxValue = corrI+gridRadius;
    argsI.minValue = corrI-gridRadius;
    gridArgs.a = argsI;
    argsQ.maxValue = corrQ+gridRadius;
    argsQ.minValue = corrQ-gridRadius;
    gridArgs.b = argsQ;
    avgCount = 3;
    GridSearch(&gridArgs);
    corrI = gridArgs.a.result;
    corrQ = gridArgs.b.result;
    verbose_printf("GRID 1: Tx DCCORRI: %i, DCCORRQ: %i  | %.3f dbFS\n", corrI, corrQ, RSSI_2_dBFS(gridArgs.signalLevel));

    avgCount = 1;
    //Modify_SPI_Reg_bits(DCCORRI_TXTSP, corrI);
    //Modify_SPI_Reg_bits(DCCORRQ_TXTSP, corrQ);
    Modify_SPI_Reg_bits(0x0204, 15, 0, (corrI << 8 | corrQ));
    if(dccorri)
        *dccorri = corrI;
    if(dccorrq)
        *dccorrq = corrQ;
    verbose_printf("Tx DCCORRI: %i, DCCORRQ: %i | %.3f dbFS\n", corrI, corrQ, RSSI_2_dBFS(gridArgs.signalLevel));
#ifdef DRAW_GNU_PLOT
    char filename[128];
    sprintf(filename, "TxDC_%s.txt", calibrationStart.c_str());
    fstream fout;
    fout.open(filename, ios::out);
    for(int i=0; i<256; ++i)
    {
        for(int q=0; q<256; ++q)
            fout << i-128 << " " << q-128 << " " << (gRSSI_TXDC_measurements[q][i]) << endl;
        fout << endl;
    }
    fout.close();
#endif
}

void LMS7002M::FineSearch(const uint16_t addrI, const uint8_t msbI, const uint8_t lsbI, int16_t &valueI, const uint16_t addrQ, const uint8_t msbQ, const uint8_t lsbQ, int16_t &valueQ, const uint8_t fieldSize)
{
#ifdef LMS_VERBOSE_OUTPUT
    uint32_t **rssiField = new uint32_t*[fieldSize];
    for (int i = 0; i < fieldSize; ++i)
    {
        rssiField[i] = new uint32_t[fieldSize];
        for (int q = 0; q < fieldSize; ++q)
            rssiField[i][q] = ~0;
    }
#endif

    const uint16_t DCOFFaddr = 0x010E;
    uint32_t minRSSI = ~0;
    int16_t minI = 0;
    int16_t minQ = 0;
    uint32_t rssi = ~0;

    for (int i = 0; i < fieldSize; ++i)
    {
        int16_t ival = valueI + (i - fieldSize / 2);
        Modify_SPI_Reg_bits(addrI, msbI, lsbI, addrI != DCOFFaddr ? ival : toDCOffset(ival), true);
        for (int q = 0; q < fieldSize; ++q)
        {
            int16_t qval = valueQ + (q - fieldSize / 2);
            Modify_SPI_Reg_bits(addrQ, msbQ, lsbQ, addrQ != DCOFFaddr ? qval : toDCOffset(qval), true);
            rssi = GetRSSI();
            if (rssi < minRSSI)
            {
                minI = ival;
                minQ = qval;
                minRSSI = rssi;
            }
#ifdef LMS_VERBOSE_OUTPUT
            rssiField[i][q] = rssi;
#endif
        }
    }

#ifdef LMS_VERBOSE_OUTPUT
    printf("     |");
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
#ifdef DRAW_GNU_PLOTS
    GNUPlotPipe pl(false);
    pl.write("set contour surface\n");
    pl.write("plot '-' matrix with image\n");

    for(int i=0; i<fieldSize; ++i)
    {
        for(int q=0; q<fieldSize; ++q)
        {
            pl.writef("%i ", rssiField[i][q]);
        }
        pl.write("\n");
    }

    pl.write("e\n");
    pl.write("e\n");
#endif
    for (int i = 0; i < fieldSize; ++i)
        delete rssiField[i];
    delete[] rssiField;
#endif
    valueI = minI;
    valueQ = minQ;
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
        Modify_SPI_Reg_bits(LMS7param(DC_REG_RXTSP), Q);
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
    const double freq = this->GetFrequencySX(isTx);
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

    return mValueCache->InsertDC_IQ(boardId, freq, idx, isTx, band, dccorri, dccorrq, gcorri, gcorrq, phaseOffset);
}

int LMS7002M::ApplyDigitalCorrections(const bool isTx)
{
    const int idx = this->GetActiveChannelIndex();
    const uint32_t boardId = controlPort->GetDeviceInfo().boardSerialNumber;
    const double freq = this->GetFrequencySX(isTx);
    int band = 0;
    if (isTx)
    {
        uint8_t sel_band1_trf = (uint8_t)Get_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF));
        band = sel_band1_trf ? 0 : 1;
    }
    else
    {
        uint8_t sel_path_rfe = (uint8_t)Get_SPI_Reg_bits(LMS7param(SEL_PATH_RFE));
        band = int(sel_path_rfe);
    }

    int dccorri, dccorrq, gcorri, gcorrq, phaseOffset;
    int rc = mValueCache->GetDC_IQ_Interp(boardId, freq, idx, isTx, band, &dccorri, &dccorrq, &gcorri, &gcorrq, &phaseOffset);
    if (rc != 0) return rc;

    if (isTx)
    {
        Modify_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP), dccorri);
        Modify_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP), dccorrq);
        Modify_SPI_Reg_bits(LMS7param(GCORRI_TXTSP), gcorri);
        Modify_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP), gcorrq);
        Modify_SPI_Reg_bits(LMS7param(IQCORR_TXTSP), phaseOffset);

        Modify_SPI_Reg_bits(LMS7param(DC_BYP_TXTSP), 0);
        Modify_SPI_Reg_bits(LMS7param(PH_BYP_TXTSP), 0);
        Modify_SPI_Reg_bits(LMS7param(GC_BYP_TXTSP), 0);
    }
    else
    {
        Modify_SPI_Reg_bits(LMS7param(GCORRI_RXTSP), gcorri);
        Modify_SPI_Reg_bits(LMS7param(GCORRQ_RXTSP), gcorrq);
        Modify_SPI_Reg_bits(LMS7param(IQCORR_RXTSP), phaseOffset);

        Modify_SPI_Reg_bits(LMS7param(PH_BYP_RXTSP), 0);
        Modify_SPI_Reg_bits(LMS7param(GC_BYP_RXTSP), 0);
    }
    return 0;
}

void LMS7002M::CalibrateIQImbalance(const bool tx, uint16_t *gainI, uint16_t *gainQ, int16_t *phase)
{
    const char *dirName = tx ? "Tx" : "Rx";
#ifdef DRAW_GNU_PLOTS
    searchPlot.writef("set title '%s IQ search\n", dirName);
    spectrumPlot.writef("set title '%s IQ search\n", dirName);
#endif // DRAW_GNU_PLOTS
    verbose_printf("Searching %s IQ imbalance...\n", dirName);
    const auto gcorri = tx ? LMS7param(GCORRI_TXTSP) : LMS7param(GCORRI_RXTSP);
    const auto gcorrq = tx ? LMS7param(GCORRQ_TXTSP) : LMS7param(GCORRQ_RXTSP);
    int16_t gain;
    int16_t phaseOffset;
    BinSearchParam argsPhase;
    BinSearchParam argsGain;
    GridSearchParam gridArgs;

    argsPhase.param = tx ? LMS7param(IQCORR_TXTSP) : LMS7param(IQCORR_RXTSP);
    argsPhase.maxValue = 128;
    argsPhase.minValue = -128;
    BinarySearch(&argsPhase);
    phaseOffset = argsPhase.result;
    verbose_printf("Coarse search %s IQCORR: %i\n", dirName, phaseOffset);

    //coarse gain
    uint32_t rssiIgain;
    uint32_t rssiQgain;
    Modify_SPI_Reg_bits(gcorri, 2047 - 64);
    Modify_SPI_Reg_bits(gcorrq, 2047);
    rssiIgain = GetRSSI();
    Modify_SPI_Reg_bits(gcorri, 2047);
    Modify_SPI_Reg_bits(gcorrq, 2047 - 64);
    rssiQgain = GetRSSI();

    Modify_SPI_Reg_bits(gcorri, 2047);
    Modify_SPI_Reg_bits(gcorrq, 2047);

    if(rssiIgain < rssiQgain)
        argsGain.param = gcorri;
    else
        argsGain.param = gcorrq;
    const char* chName = (argsGain.param.address == gcorri.address ? "I" : "Q");

    argsGain.maxValue = 2047;
    argsGain.minValue = 2047-512;
    BinarySearch(&argsGain);
    gain = argsGain.result;
    verbose_printf("Coarse search %s GAIN_%s: %i\n", dirName, chName, gain);

    argsPhase.maxValue = phaseOffset+8;
    argsPhase.minValue = phaseOffset-8;
    BinarySearch(&argsPhase);
    phaseOffset = argsPhase.result;
    verbose_printf("Coarse search %s IQCORR: %i\n", dirName, phaseOffset);

    const uint8_t gridRadius = 2;
    gridArgs.a = argsGain;
    gridArgs.a.minValue = gain-gridRadius;
    gridArgs.a.maxValue = gain+gridRadius;
    gridArgs.b = argsPhase;
    gridArgs.b.minValue = phaseOffset-gridRadius;
    gridArgs.b.maxValue = phaseOffset+gridRadius;

    GridSearch(&gridArgs);
    gain = gridArgs.a.result;
    phaseOffset = gridArgs.b.result;
    verbose_printf("GRID search %s GAIN_%s: %i, IQCORR: %i | %.3f dBFS\n", dirName, chName, gain, phaseOffset, RSSI_2_dBFS(gridArgs.signalLevel));

    Modify_SPI_Reg_bits(argsGain.param, gain);
    Modify_SPI_Reg_bits(argsPhase.param, phaseOffset);

    if(phase)
        *phase = phaseOffset;
    if(gainI)
        *gainI = (argsGain.param.address == gcorri.address ? gain : 2047);
    if(gainQ)
        *gainQ = (argsGain.param.address == gcorrq.address ? gain : 2047);
}
