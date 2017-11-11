#include "lms7002m_calibrations.h"
#include "LMS7002M_parameters_compact.h"
#include "spi.h"
#include "lms7002m_controls.h"
#include <math.h>

#ifdef __cplusplus
#include <cstdlib>
#define VERBOSE 1
//#define DRAW_GNU_PLOTS

#include <thread>
#include <vector>
#include <chrono>
#include <stdio.h>

typedef struct
{
    int16_t value;
    uint32_t signalLevel;
    std::vector<float> measurements;
} BinSearchResults;
BinSearchResults results;

#include <gnuPlotPipe.h>
GNUPlotPipe saturationPlot;
GNUPlotPipe rxDCPlot;
GNUPlotPipe IQImbalancePlot;
GNUPlotPipe txDCPlot;

///APPROXIMATE conversion
float ChipRSSI_2_dBFS(uint32_t rssi)
{
    uint32_t maxRSSI = 0x15FF4;
    if(rssi == 0)
        rssi = 1;
    return 20*log10((float)(rssi)/maxRSSI);
}

int16_t toSigned(int16_t val, uint8_t msblsb)
{
    val <<= 15-((msblsb >> 4) & 0xF);
    val >>= 15-((msblsb >> 4) & 0xF);
    return val;
}
#else
#define VERBOSE 0
#endif // __cplusplus

float bandwidthRF = 5e6; //Calibration bandwidth
#define calibrationSXOffset_Hz 1e6
#define offsetNCO 0.1e6
#define calibUserBwDivider 5

/*
static uint8_t toDCOffset(const int8_t offset)
{
    if(offset >= 0)
        return offset;
    return (uint8_t)(abs((int)offset) | 0x40);
}
*/

static int16_t clamp(int16_t value, int16_t minBound, int16_t maxBound)
{
    if(value < minBound)
        return minBound;
    if(value > maxBound)
        return maxBound;
    return value;
}

static void FlipRisingEdge(const uint16_t addr, const uint8_t bits)
{
    Modify_SPI_Reg_bits(addr, bits, 0);
    Modify_SPI_Reg_bits(addr, bits, 1);
}

void LoadDC_REG_TX_IQ()
{
    SPI_write(0x020C, 0x7FFF);
    FlipRisingEdge(TSGDCLDI_TXTSP);
    SPI_write(0x020C, 0x8000);
    FlipRisingEdge(TSGDCLDQ_TXTSP);
}

#ifndef __cplusplus
static void Dummy()
{
    uint8_t i;
    volatile uint16_t t=0;
    for(i=200; i; --i)
        ++t;
}
#endif

static void Delay()
{
#ifdef __cplusplus
    std::this_thread::sleep_for(std::chrono::microseconds(50));
#else
    uint8_t i;
    volatile uint16_t t=0;
    for(i=10; i; --i)
        Dummy();
#endif
}

#ifdef __cplusplus
uint32_t rssiCounter = 0;
#endif
uint16_t GetRSSI()
{
    Delay();
    FlipRisingEdge(CAPTURE);
    return ((SPI_read(0x040F) << 2) | (SPI_read(0x040E) & 0x3));
}

static void SetRxGFIR3Coefficients()
{
    //FIR coefficients symmetrical, storing only one half
    ROM const int16_t firCoefs[] =
    {
        8,4,0,-6,-11,-16,-20,-22,-22,-20,-14,-5,6,20,34,46,
        56,61,58,48,29,3,-29,-63,-96,-123,-140,-142,-128,-94,-44,20,
        93,167,232,280,302,291,244,159,41,-102,-258,-409,-539,-628,-658,-614,
        -486,-269,34,413,852,1328,1814,2280,2697,3038,3277,3401,
    };
    uint8_t index = 0;
    for (; index < sizeof(firCoefs)/sizeof(int16_t); ++index)
        SPI_write(0x0500 + index + 24 * (index / 40), firCoefs[index]);
    for (; index < sizeof(firCoefs)/sizeof(int16_t)*2; ++index)
        SPI_write(0x0500 + index + 24 * (index / 40), firCoefs[119-index]);
}

void CheckSaturationTxRx(bool extLoopback)
{
    const uint16_t saturationLevel = 0x05000; //-3dBFS
    uint8_t g_pga;
    uint8_t g_rfe;
    uint16_t rssi;
#ifdef DRAW_GNU_PLOTS
#define PUSH_PLOT_VALUE(vec, data) vec.push_back(data)
    std::vector<float> g_rxLoopbStage;
    std::vector<float> pgaFirstStage;
    std::vector<float> lnaStage;
    std::vector<float> tiaStage;
    std::vector<float> pgaSecondStage;
#else
#define PUSH_PLOT_VALUE(vec, data)
#endif
    Modify_SPI_Reg_bits(DC_BYP_RXTSP, 0);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
    SetNCOFrequency(LMS7002M_Rx, calibrationSXOffset_Hz - offsetNCO + (bandwidthRF / calibUserBwDivider) * 2, 0);

    rssi = GetRSSI();
    PUSH_PLOT_VALUE(g_rxLoopbStage, rssi);

    g_pga = (uint8_t)Get_SPI_Reg_bits(G_PGA_RBB);
    if(extLoopback)
        g_rfe = (uint8_t)Get_SPI_Reg_bits(G_LNA_RFE);
    else
        g_rfe = (uint8_t)Get_SPI_Reg_bits(G_RXLOOPB_RFE);


#if VERBOSE
    printf("Receiver saturation search, target level: %i (%2.3f dBFS)\n", saturationLevel, ChipRSSI_2_dBFS(saturationLevel));
    printf("initial  PGA: %2i, %s: %2i, %3.2f dbFS\n", g_pga, (extLoopback ? "LNA":"RXLOOPB"), g_rfe, ChipRSSI_2_dBFS(rssi));
#endif
    while(rssi < saturationLevel)
    {
        if(g_rfe < 15)
            ++g_rfe;
        else
            break;
        if(extLoopback)
            Modify_SPI_Reg_bits(G_LNA_RFE, g_rfe);
        else
            Modify_SPI_Reg_bits(G_RXLOOPB_RFE, g_rfe);
        rssi = GetRSSI();
        PUSH_PLOT_VALUE(g_rxLoopbStage, rssi);
    }
    PUSH_PLOT_VALUE(pgaFirstStage, rssi);
    while(g_pga < 18 && g_rfe == 15 && rssi < saturationLevel)
    {
        if(g_pga < 18)
            ++g_pga;
        else
            break;
        Modify_SPI_Reg_bits(G_PGA_RBB, g_pga);
        rssi = GetRSSI();
        //rssi_prev = rssi;
        PUSH_PLOT_VALUE(pgaFirstStage, rssi);
    }
#if VERBOSE
    printf("adjusted  PGA: %2i, %s: %2i, %3.2f dbFS\n", g_pga, (extLoopback ? "LNA":"RXLOOPB"), g_rfe, ChipRSSI_2_dBFS(rssi));
#endif
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(DC_BYP_RXTSP, 1);
#ifdef DRAW_GNU_PLOTS
    {
        saturationPlot.write("set yrange [:0]\n");
        saturationPlot.write("set title 'Rx gains search'\n");
        saturationPlot.write("set key right bottom\n");
        saturationPlot.write("set xlabel 'measurement index'\n");
        saturationPlot.write("set ylabel 'RSSI dbFS'\n");
        saturationPlot.write("set grid xtics ytics\n");
        saturationPlot.write("plot\
'-' u 1:2 with lines title 'G_RXLOOPB',\
'-' u 1:2 with lines title 'PGA',\
'-' u 1:2 with lines title 'target Level'\n");
        int index = 1;
        const auto arrays = {&g_rxLoopbStage, &pgaFirstStage};
        for(auto a : arrays)
        {
            --index;
            for(size_t i=0; i<a->size(); ++i)
                saturationPlot.writef("%i %f\n", index++, ChipRSSI_2_dBFS((*a)[i]));
            saturationPlot.write("e\n");
        }
        saturationPlot.writef("%i %f\n%i %f\ne\n", 0, ChipRSSI_2_dBFS(saturationLevel),
                              index, ChipRSSI_2_dBFS(saturationLevel));
        saturationPlot.flush();
    }
#endif
#undef PUSH_PLOT_VALUE
}

typedef struct
{
    LMS7Parameter param;
    int16_t result;
    int16_t minValue;
    int16_t maxValue;
} BinSearchParam;

void BinarySearch(BinSearchParam bdata* args)
{
    uint16_t rssiLeft = ~0;
    uint16_t rssiRight;
    int16_t left = args->minValue;
    int16_t right = args->maxValue;
    int16_t step;
    const uint16_t addr = args->param.address;
    const uint8_t msblsb = args->param.msblsb;
    //const uint16_t regValue = SPI_read(addr);
#ifdef DRAW_GNU_PLOTS
    std::vector<float> searchPoints;
#define PUSH_PLOT_VALUE(vec, param, level) vec.push_back(param);vec.push_back(level)
#else
#define PUSH_PLOT_VALUE(vec, param, level)
#endif
    Modify_SPI_Reg_bits(addr, msblsb, right);
    rssiRight = GetRSSI();
    PUSH_PLOT_VALUE(searchPoints, right, rssiRight);
    while(right-left >= 1)
    {
        step = (right-left)/2;
        if(rssiLeft < rssiRight)
        {
            Modify_SPI_Reg_bits(addr, msblsb, right);
            rssiRight = GetRSSI();
            PUSH_PLOT_VALUE(searchPoints, right, rssiRight);
        }
        else
        {
            Modify_SPI_Reg_bits(addr, msblsb, left);
            rssiLeft = GetRSSI();
            PUSH_PLOT_VALUE(searchPoints, left, rssiLeft);
        }
        if(step <= 0)
            break;
        if(rssiLeft < rssiRight)
            right -= step;
        else
            left += step;
    }
    args->result = rssiLeft < rssiRight ? left : right;
    Modify_SPI_Reg_bits(addr, msblsb, args->result);

#ifdef DRAW_GNU_PLOTS
    results.value = args->result;
    results.signalLevel = rssiLeft < rssiRight ? rssiLeft : rssiRight;
    results.measurements.clear();
    results.measurements = searchPoints;
    for(size_t i=0; i<results.measurements.size(); i+=2)
        for(size_t j=i; j<results.measurements.size(); j+=2)
        {
            if(results.measurements[i] > results.measurements[j])
            {
                float temp = results.measurements[i];
                results.measurements[i] = results.measurements[j];
                results.measurements[j] = temp;
                temp = results.measurements[i+1];
                results.measurements[i+1] = results.measurements[j+1];
                results.measurements[j+1] = temp;
            }
        }
#endif //DRAW_GNU_PLOTS
#undef PUSH_PLOT_VALUE
}

int16_t ReadAnalogDC(const uint16_t addr)
{
    const uint16_t mask = addr < 0x05C7 ? 0x03FF : 0x003F;
    uint16_t value;
    int16_t result;
    SPI_write(addr, 0);
    SPI_write(addr, 0x4000);
    value = SPI_read(addr);
    SPI_write(addr, value & ~0xC000);
    result = (value & mask);
    if(value & (mask+1))
        result *= -1;
    return result;
}

static void WriteAnalogDC(const uint16_t addr, int16_t value)
{
    const uint16_t mask = addr < 0x05C7 ? 0x03FF : 0x003F;
    int16_t regValue = 0;
    if(value < 0)
    {
        regValue |= (mask+1);
        regValue |= (abs(value+mask) & mask);
    }
    else
        regValue |= (abs(value+mask+1) & mask);
    SPI_write(addr, regValue);
    SPI_write(addr, regValue | 0x8000);
}

static void TxDcBinarySearch(BinSearchParam* args)
{
    uint16_t rssiLeft = ~0;
    uint16_t rssiRight;
    int16_t left = args->minValue;
    int16_t right = args->maxValue;
    int16_t step;

    WriteAnalogDC(args->param.address, right);
    rssiRight = GetRSSI();

    while(right-left >= 1)
    {
        step = (right-left)/2;
        if(rssiLeft < rssiRight)
        {
            WriteAnalogDC(args->param.address, right);
            rssiRight = GetRSSI();
        }
        else
        {
            WriteAnalogDC(args->param.address, left);
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
    WriteAnalogDC(args->param.address, args->result);
}

void AdjustAutoDC(const uint16_t address, bool tx)
{
    bool increment;
    uint16_t rssi;
    uint16_t minRSSI;
    int16_t minValue = 0xffff;
    int16_t initVal;
    const uint16_t mask = tx ? 0x03FF : 0x003F;
    const int16_t range = tx ? 1023 : 63;
    SPI_write(address, 0);
    SPI_write(address, 0x4000);
    initVal = SPI_read(address);
    SPI_write(address, 0);
    if(initVal & (mask+1))
    {
        initVal &= mask;
        initVal *= -1;
    }
    else
        initVal &= mask;
    minValue = initVal;

    rssi = GetRSSI();
    minRSSI = rssi;
    {
        int16_t tempValue = clamp(initVal+1, -range, range);
        uint16_t regValue = 0;
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
    increment = GetRSSI() < rssi;
    {
    int8_t iterations = 8;
    while (iterations > 0)
    {
        SPI_write(address, 0);
        if(increment)
            ++initVal;
        else
            --initVal;
        initVal = clamp(initVal, -range, range);
        {
            uint16_t regValue = 0;
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
        rssi = GetRSSI();
        if(rssi < minRSSI)
        {
            minRSSI = rssi;
            minValue = initVal;
        }
        --iterations;
    }
    }
    {
        uint16_t regValue = 0;
        initVal = minValue;
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
    }
}

void CalibrateRxDCAuto()
{
    uint16_t dcRegAddr = 0x5C7;
    const uint8_t ch = Get_SPI_Reg_bits(MAC);
    Modify_SPI_Reg_bits(EN_G_TRF, 0);
    Modify_SPI_Reg_bits(DC_BYP_RXTSP, 1);
#if VERBOSE
    printf("Calibrating Rx DC...\n");
#endif
    //auto calibration

    Modify_SPI_Reg_bits(DCMODE, 1);
    if(ch == 1)
    {
        Modify_SPI_Reg_bits(PD_DCDAC_RXA, 0);
        Modify_SPI_Reg_bits(PD_DCCMP_RXA, 0);
        SPI_write(0x05C2, 0xFF30);
    }
    else
    {
        Modify_SPI_Reg_bits(PD_DCDAC_RXB, 0);
        Modify_SPI_Reg_bits(PD_DCCMP_RXB, 0);
        SPI_write(0x05C2, 0xFFC0);
        dcRegAddr += 2;
    }

    {
        while(SPI_read(0x05C1) & 0xF000);
    }

    //manual adjustments
    Modify_SPI_Reg_bits(GCORRQ_RXTSP.address, GCORRQ_RXTSP.msblsb, 0);
    AdjustAutoDC(dcRegAddr, false);
    Modify_SPI_Reg_bits(GCORRQ_RXTSP.address, GCORRQ_RXTSP.msblsb, 2047);
    AdjustAutoDC(dcRegAddr+1, false);

    Modify_SPI_Reg_bits(DC_BYP_RXTSP, 0); // DC_BYP 0
    Modify_SPI_Reg_bits(EN_G_TRF, 1);
}

void CalibrateTxDCAuto()
{
#if VERBOSE
    printf("Calibrating Tx DC\n");
#endif // VERBOSE
    BinSearchParam iparams;
    BinSearchParam qparams;
    const uint8_t ch = Get_SPI_Reg_bits(MAC);
    Modify_SPI_Reg_bits(EN_G_TRF, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_TXTSP, 0);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);

    Modify_SPI_Reg_bits(DC_BYP_TXTSP, 1);
    //auto calibration
    Modify_SPI_Reg_bits(DCMODE, 1);
    //Modify_SPI_Reg_bits(GCORRI_TXTSP.address, GCORRI_TXTSP.msblsb, 0);
    //Modify_SPI_Reg_bits(GCORRQ_TXTSP.address, GCORRQ_TXTSP.msblsb, 0);
    iparams.param.msblsb = 10<<4 | 0;
    qparams.param.msblsb = 10<<4 | 0;
    if(ch == 1)
    {
        iparams.param.address = 0x5C3;// DC_TXAI;
        qparams.param.address = 0x5C4;// DC_TXAQ;
        Modify_SPI_Reg_bits(PD_DCDAC_TXA, 0);
        Modify_SPI_Reg_bits(PD_DCCMP_TXA, 0);
        //SPI_write(0x05C2, 0x0F03);
    }
    else
    {
        iparams.param.address = 0x5C5;// DC_TXBI;
        qparams.param.address = 0x5C6;// DC_TXBQ;
        Modify_SPI_Reg_bits(PD_DCDAC_TXB, 0);
        Modify_SPI_Reg_bits(PD_DCCMP_TXB, 0);
        //SPI_write(0x05C2, 0x0F0C);
    }

    //wait until finished
    //while(SPI_read(0x05C1) & 0x0F00);

    {
    ROM const int16_t offset[3] = {1023, 128, 8};
    uint8_t i;
    iparams.result = 0; //ReadAnalogDC(iparams.param.address);
    qparams.result = 0; //ReadAnalogDC(qparams.param.address);
    for(i=0; i<3; ++i)
    {
        iparams.minValue = clamp(iparams.result-offset[i], -1024, 1023);
        iparams.maxValue = clamp(iparams.result+offset[i], -1024, 1023);
        qparams.minValue = clamp(qparams.result-offset[i], -1024, 1023);
        qparams.maxValue = clamp(qparams.result+offset[i], -1024, 1023);

        TxDcBinarySearch(&iparams);
        TxDcBinarySearch(&qparams);
    }
    }

    //Modify_SPI_Reg_bits(GCORRI_TXTSP.address, GCORRI_TXTSP.msblsb, 2047);
    //Modify_SPI_Reg_bits(GCORRQ_TXTSP.address, GCORRQ_TXTSP.msblsb, 2047);
#if VERBOSE
    printf("Done\n");
#endif // VERBOSE
}

void CalibrateIQImbalance(bool tx)
{
#ifdef DRAW_GNU_PLOTS
    const char *dirName = tx ? "Tx" : "Rx";
    IQImbalancePlot.writef("set title '%s IQ imbalance'\n", dirName);
    IQImbalancePlot.write("set xlabel 'parameter value'\n");
    IQImbalancePlot.write("set ylabel 'RSSI dBFS'\n");
    IQImbalancePlot.write("set grid ytics xtics\n");
    IQImbalancePlot.write("plot\
'-' w l t 'phase1',\
'-' w l t 'gain1',\
'-' w l t 'phase2'\
\n");
#endif // DRAW_GNU_PLOTS
    BinSearchParam argsPhase;
    BinSearchParam argsGain;
    LMS7Parameter gcorri;
    LMS7Parameter gcorrq;
    if(tx)
    {
        gcorri = GCORRI_TXTSP;
        gcorrq = GCORRQ_TXTSP;
        argsPhase.param = IQCORR_TXTSP;
    }
    else
    {
        gcorri = GCORRI_RXTSP;
        gcorrq = GCORRQ_RXTSP;
        argsPhase.param = IQCORR_RXTSP;
    }

    argsPhase.maxValue = 128;
    argsPhase.minValue = -128;
    BinarySearch(&argsPhase);
#ifdef DRAW_GNU_PLOTS
    for(size_t i=0; i<results.measurements.size(); i+=2)
        IQImbalancePlot.writef("%f %f\n", results.measurements[i], ChipRSSI_2_dBFS(results.measurements[i+1]));
    IQImbalancePlot.write("e\n");
    printf("Coarse search %s IQCORR: %i\n", dirName, argsPhase.result);
#endif

    //coarse gain
    {
        uint16_t rssiIgain;
        uint16_t rssiQgain;
        //Modify_SPI_Reg_bits(gcorri.address, gcorri.msblsb, 2047 - 64);
        //Modify_SPI_Reg_bits(gcorrq.address, gcorrq.msblsb, 2047);
        SPI_write(gcorri.address, 2047 - 64);
        SPI_write(gcorrq.address, 2047);
        rssiIgain = GetRSSI();
        //Modify_SPI_Reg_bits(gcorri.address, gcorri.msblsb, 2047);
        //Modify_SPI_Reg_bits(gcorrq.address, gcorrq.msblsb, 2047 - 64);
        SPI_write(gcorri.address, 2047);
        SPI_write(gcorrq.address, 2047 - 64);
        rssiQgain = GetRSSI();

        if(rssiIgain < rssiQgain)
            argsGain.param = gcorri;
        else
            argsGain.param = gcorrq;
        SPI_write(gcorrq.address, 2047);
    }
    argsGain.maxValue = 2047;
    argsGain.minValue = 2047-512;
    BinarySearch(&argsGain);

#ifdef DRAW_GNU_PLOTS
    const char* chName = (argsGain.param.address == gcorri.address ? "I" : "Q");
    for(size_t i=0; i<results.measurements.size(); i+=2)
        IQImbalancePlot.writef("%f %f\n", results.measurements[i], ChipRSSI_2_dBFS(results.measurements[i+1]));
    IQImbalancePlot.write("e\n");
    printf("Coarse search %s GAIN_%s: %i\n", dirName, chName, argsGain.result);
#endif

    argsPhase.maxValue = argsPhase.result+16;
    argsPhase.minValue = argsPhase.result-16;
    BinarySearch(&argsPhase);
#ifdef DRAW_GNU_PLOTS
    for(size_t i=0; i<results.measurements.size(); i+=2)
        IQImbalancePlot.writef("%f %f\n", results.measurements[i], ChipRSSI_2_dBFS(results.measurements[i+1]));
    IQImbalancePlot.write("e\n");
    printf("Coarse search %s IQCORR: %i\n", dirName, argsPhase.result);
#endif
    //Modify_SPI_Reg_bits(argsGain.param.address, argsGain.param.msblsb, argsGain.result);
    SPI_write(argsGain.param.address, argsGain.result);
    Modify_SPI_Reg_bits(argsPhase.param.address, argsPhase.param.msblsb, argsPhase.result);
}

uint8_t SetupCGEN()
{
    uint8_t status;
    uint8_t cgenMultiplier;
    uint8_t gfir3n;
    cgenMultiplier = (uint8_t)((GetFrequencyCGEN() / 46.08e6) + 0.5);
    if(cgenMultiplier < 2)
        cgenMultiplier = 2;
    if(cgenMultiplier > 13)
        cgenMultiplier = 13;
    //CGEN VCO is powered up in SetFrequencyCGEN/Tune
    status = SetFrequencyCGEN(46.08e6 * cgenMultiplier);

    gfir3n = 4 * cgenMultiplier;
    if(Get_SPI_Reg_bits(EN_ADCCLKH_CLKGN) == 1)
        gfir3n /= pow2(Get_SPI_Reg_bits(CLKH_OV_CLKL_CGEN));
    gfir3n = pow2((uint8_t)(log(gfir3n)/log(2)))-1; //could be log2(gfir3n)
    Modify_SPI_Reg_bits(GFIR3_N_RXTSP, gfir3n);
    return status;
}

uint8_t CalibrateTxSetup(bool extLoopback)
{
    uint8_t status;
    const uint16_t x0020val = SPI_read(0x0020); //remember used channel

    /*BeginBatch("TxSetup");
    //rfe
    //reset RFE to defaults
    SetDefaults(SECTION_RFE);
    Modify_SPI_Reg_bits(G_RXLOOPB_RFE, 7);
    Modify_SPI_Reg_bits(0x010C, 4 << 4 | 3, 0); //PD_MXLOBUF_RFE 0, PD_QGEN_RFE 0
    Modify_SPI_Reg_bits(CCOMP_TIA_RFE, 4);
    Modify_SPI_Reg_bits(CFB_TIA_RFE, 50);

    //RBB
    //reset RBB to defaults
    SetDefaults(SECTION_RBB);
    Modify_SPI_Reg_bits(PD_LPFH_RBB, 0);
    Modify_SPI_Reg_bits(PD_LPFL_RBB, 1);
    Modify_SPI_Reg_bits(G_PGA_RBB, 0);
    Modify_SPI_Reg_bits(INPUT_CTL_PGA_RBB, 1);
    Modify_SPI_Reg_bits(ICT_PGA_OUT_RBB, 12);
    Modify_SPI_Reg_bits(ICT_PGA_IN_RBB, 12);

    //TXTSP
    Modify_SPI_Reg_bits(TSGMODE_TXTSP, 1);
    Modify_SPI_Reg_bits(INSEL_TXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_TXTSP, 0);
    Modify_SPI_Reg_bits(DC_BYP_TXTSP, 0);
    Modify_SPI_Reg_bits(GC_BYP_TXTSP, 0);
    Modify_SPI_Reg_bits(PH_BYP_TXTSP, 0);
    Modify_SPI_Reg_bits(GCORRI_TXTSP.address, GCORRI_TXTSP.msblsb , 2047);
    Modify_SPI_Reg_bits(GCORRQ_TXTSP.address, GCORRQ_TXTSP.msblsb, 2047);
    Modify_SPI_Reg_bits(CMIX_SC_TXTSP, 0);
    Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_TXTSP), 0);
    Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_TXTSP_R3), 0);

    //RXTSP
    SetDefaults(SECTION_RxTSP);
    SetDefaults(SECTION_RxNCO);
    Modify_SPI_Reg_bits(GFIR3_BYP_RXTSP, 0);
    Modify_SPI_Reg_bits(GFIR2_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(GFIR1_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(HBD_OVR_RXTSP, 4); //Decimation HBD ratio
    Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 1);



    Modify_SPI_Reg_bits(AGC_MODE_RXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(AGC_AVG_RXTSP, 0x1);
    Modify_SPI_Reg_bits(GFIR3_L_RXTSP, 7);


    //AFE
    Modify_SPI_Reg_bits(PD_RX_AFE1, 0);
    Modify_SPI_Reg_bits(PD_RX_AFE2, 0);

    //XBUF
    Modify_SPI_Reg_bits(0x0085, 2 << 4 | 0, 1); //PD_XBUF_RX 0, PD_XBUF_TX 0, EN_G_XBUF 1

    //CDS
    Modify_SPI_Reg_bits(CDS_TXATSP, 3);
    Modify_SPI_Reg_bits(CDS_TXBTSP, 3);

    //TRF
    Modify_SPI_Reg_bits(L_LOOPB_TXPAD_TRF, 0);
    Modify_SPI_Reg_bits(EN_LOOPB_TXPAD_TRF, 1);

    //BIAS
    {
        uint16_t backup = Get_SPI_Reg_bits(RP_CALIB_BIAS);
        SetDefaults(SECTION_BIAS);
        Modify_SPI_Reg_bits(RP_CALIB_BIAS, backup);
    }

    EndBatch();*/
    if((x0020val & 0x3) == 1)
        Modify_SPI_Reg_bits(PD_RX_AFE1, 0);
    else
        Modify_SPI_Reg_bits(PD_RX_AFE2, 0);
    {
        ROM const uint16_t TxSetupAddr[] = {0x0084, 0x0085,0x00AE,0x0101,0x0113,0x0200,0x0201,0x0202,0x0208};
        ROM const uint16_t TxSetupData[] = {0x0400, 0x0001,0xF000,0x0001,0x001C,0x000C,0x07FF,0x07FF,0x0000};
        ROM const uint16_t TxSetupMask[] = {0xF8FF, 0x0007,0xF000,0x1801,0x003C,0x000C,0x07FF,0x07FF,0xF10B};
        uint8_t i;
        for(i=sizeof(TxSetupAddr)/sizeof(uint16_t); i; --i)
            SPI_write(TxSetupAddr[i-1], ( SPI_read(TxSetupAddr[i-1]) & ~TxSetupMask[i-1] ) | TxSetupData[i-1]);
    }
    {
        ROM const uint16_t TxSetupAddrWrOnly[] = {0x010C,0x0112,0x0115,0x0116,0x0117,0x0118,0x0119,0x011A,0x0400,0x0401,0x0402,0x0403,0x0404,0x0405,0x0406,0x0407,0x0408,0x0409,0x040A,0x040C,0x0440,0x0442,0x0443};
        ROM const uint16_t TxSetupDataWrOnly[] = {0x88E5,0x4032,0x0005,0x8180,0x280C,0x218C,0x3180,0x2E02,0x0081,0x07FF,0x07FF,0x4000,0x0000,0x0000,0x0000,0x0700,0x0000,0x0000,0x1001,0x2098,0x0020,0x0000,0x0000};

        uint8_t i;
        for(i=sizeof(TxSetupAddrWrOnly)/sizeof(uint16_t); i; --i)
            if(i<=sizeof(TxSetupDataWrOnly)/sizeof(uint16_t))
                SPI_write(TxSetupAddrWrOnly[i-1], TxSetupDataWrOnly[i-1]);
            //else
                //SPI_write(TxSetupAddrWrOnly[i-1], 0);
    }
    SetRxGFIR3Coefficients();
    status = SetupCGEN();
    if(status != 0)
        return status;

    //SXR
    Modify_SPI_Reg_bits(MAC, 1); //switch to ch. A
    //SetDefaults(SECTION_SX);
    SetDefaultsSX();
    {
        const float_type SXRfreq = GetFrequencySX(LMS7002M_Tx) - bandwidthRF/ calibUserBwDivider - calibrationSXOffset_Hz;
        //SX VCO is powered up in SetFrequencySX/Tune
        status = SetFrequencySX(LMS7002M_Rx, SXRfreq);
        if(status != 0)
            return status+0x60;
    }

    //if calibrating ch. B enable buffers
    if(x0020val & 0x2)
    {
        Modify_SPI_Reg_bits(PD_TX_AFE2, 0);
        Modify_SPI_Reg_bits(EN_NEXTRX_RFE, 1);
        Modify_SPI_Reg_bits(EN_NEXTTX_TRF, 1);
    }

    //SXT{
    Modify_SPI_Reg_bits(MAC, 2); //switch to ch. B
    Modify_SPI_Reg_bits(PD_LOCH_T2RBUF, 1);
    SPI_write(0x0020, x0020val); //restore used channel

    LoadDC_REG_TX_IQ();
    SetNCOFrequency(LMS7002M_Tx, bandwidthRF/ calibUserBwDivider, 0);
    {
        const uint8_t sel_band1_2_trf = (uint8_t)Get_SPI_Reg_bits(0x0103, 11 << 4 | 10);
        if(extLoopback)
        {
            if(sel_band1_2_trf != 0x1)
            {
                //printf("Tx Calibration: external calibration is not supported on selected Tx Band");
                return 5;
            }
        }
        else
        {
            if(sel_band1_2_trf != 0x1 && sel_band1_2_trf != 0x2) //BAND1
            {
                //printf("Tx Calibration: band not selected");
                return 5;
            }
            Modify_SPI_Reg_bits(SEL_PATH_RFE, sel_band1_2_trf+1);
            //Modify_SPI_Reg_bits(PD_RLOOPB_1_RFE, 0);
            //Modify_SPI_Reg_bits(PD_RLOOPB_2_RFE, 1);
            Modify_SPI_Reg_bits(0x010C, 6 << 4 | 5, sel_band1_2_trf ^ 0x3);
            //Modify_SPI_Reg_bits(EN_INSHSW_LB1_RFE, 0);
            //Modify_SPI_Reg_bits(EN_INSHSW_LB2_RFE, 1);
            Modify_SPI_Reg_bits(0x010D, 4 << 4 | 3, sel_band1_2_trf ^ 0x3);
        }
    }
    return 0x0;
}

uint8_t CalibrateTx()
{
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(MAC);
#ifdef __cplusplus
    auto beginTime = std::chrono::high_resolution_clock::now();
#endif
#if VERBOSE

    uint8_t sel_band1_trf = (uint8_t)Get_SPI_Reg_bits(SEL_BAND1_TRF);
    printf("Tx ch.%s , BW: %g MHz, RF output: %s, Gain: %i\n",
           ch == 0x1 ? "A" : "B",
           bandwidthRF/1e6,
           sel_band1_trf==1 ? "BAND1" : "BAND2",
           Get_SPI_Reg_bits(CG_IAMP_TBB));
#endif
    uint8_t status;
    //BackupRegisters();
    SaveChipState();
    status = CalibrateTxSetup(0);
    if(status != 0)
        goto TxCalibrationEnd; //go to ending stage to restore registers
    CalibrateRxDCAuto();
    CheckSaturationTxRx(0);
    CalibrateRxDCAuto();

    SetNCOFrequency(LMS7002M_Rx, calibrationSXOffset_Hz - offsetNCO + (bandwidthRF/ calibUserBwDivider), 0);
    CalibrateTxDCAuto();
    SetNCOFrequency(LMS7002M_Rx, calibrationSXOffset_Hz - offsetNCO, 0);
    CalibrateIQImbalance(LMS7002M_Tx);
TxCalibrationEnd:
    if(status != 0)
    {
#if VERBOSE
        printf("Tx calibration failed");
#endif
        RestoreChipState();
        return status;
    }
    {
        //uint16_t dccorri = Get_SPI_Reg_bits(DCCORRI_TXTSP.address, DCCORRI_TXTSP.msblsb);
        //uint16_t dccorrq = Get_SPI_Reg_bits(DCCORRQ_TXTSP.address, DCCORRQ_TXTSP.msblsb);
        uint16_t gcorri = Get_SPI_Reg_bits(GCORRI_TXTSP.address, GCORRI_TXTSP.msblsb);
        uint16_t gcorrq = Get_SPI_Reg_bits(GCORRQ_TXTSP.address, GCORRQ_TXTSP.msblsb);
        uint16_t phaseOffset = Get_SPI_Reg_bits(IQCORR_TXTSP.address, IQCORR_TXTSP.msblsb);
        RestoreChipState();
        Modify_SPI_Reg_bits(MAC, ch);
        //Modify_SPI_Reg_bits(DCCORRI_TXTSP.address, DCCORRI_TXTSP.msblsb, dccorri);
        //Modify_SPI_Reg_bits(DCCORRQ_TXTSP.address, DCCORRQ_TXTSP.msblsb, dccorrq);
        Modify_SPI_Reg_bits(GCORRI_TXTSP.address, GCORRI_TXTSP.msblsb, gcorri);
        Modify_SPI_Reg_bits(GCORRQ_TXTSP.address, GCORRQ_TXTSP.msblsb, gcorrq);
        Modify_SPI_Reg_bits(IQCORR_TXTSP.address, IQCORR_TXTSP.msblsb, phaseOffset);
    }

    Modify_SPI_Reg_bits(DCMODE, 1);
    if(ch == 1)
        Modify_SPI_Reg_bits(PD_DCDAC_TXA, 0);
    else
        Modify_SPI_Reg_bits(PD_DCDAC_TXB, 0);
    Modify_SPI_Reg_bits(DC_BYP_TXTSP, 1);
    Modify_SPI_Reg_bits(0x0208, 1<<4 | 0, 0); //GC_BYP PH_BYP
    LoadDC_REG_TX_IQ();

    //LoadDC_REG_TX_IQ(); //not necessary, just for testing convenience
#if VERBOSE
    //printf("#####Tx calibration RESULTS:###########################\n");
    /*printf("Tx ch.%s, BW: %g MHz, RF output: %s, Gain: %i\n",
                    ch == 1 ? "A" : "B",
                    bandwidthRF/1e6, sel_band1_trf==1 ? "BAND1" : "BAND2",
                    1//Get_SPI_Reg_bits(CG_IAMP_TBB)
                    );*/
    {
        int16_t dcI = ReadAnalogDC(ch==1? 0x5C3 : 0x5C5);
        int16_t dcQ = ReadAnalogDC(ch==1? 0x5C4 : 0x5C6);
        int16_t phaseSigned = toSigned(Get_SPI_Reg_bits(IQCORR_TXTSP.address, IQCORR_TXTSP.msblsb), IQCORR_TXTSP.msblsb);
        uint16_t gcorri = Get_SPI_Reg_bits(GCORRI_TXTSP.address, GCORRI_TXTSP.msblsb);
        uint16_t gcorrq = Get_SPI_Reg_bits(GCORRQ_TXTSP.address, GCORRQ_TXTSP.msblsb);
        printf("   | DC  | GAIN | PHASE\n");
        printf("---+-----+------+------\n");
        printf("I: | %3i | %4i | %i\n", dcI, gcorri, phaseSigned);
        printf("Q: | %3i | %4i |\n", dcQ, gcorrq);
    }
#ifdef __cplusplus
    int32_t duration = std::chrono::duration_cast<std::chrono::milliseconds>
                       (std::chrono::high_resolution_clock::now()-beginTime).count();
    printf("Duration: %i ms\n", duration);
#endif
#endif //LMS_VERBOSE_OUTPUT
    return 0;
}

uint8_t CalibrateTxExternalLoop()
{
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(MAC);
#ifdef __cplusplus
    auto beginTime = std::chrono::high_resolution_clock::now();
#endif
#if VERBOSE

    uint8_t sel_band1_trf = (uint8_t)Get_SPI_Reg_bits(SEL_BAND1_TRF);
    printf("Tx ch.%s , BW: %g MHz, RF output: %s, Gain: %i\n",
           ch == 0x1 ? "A" : "B",
           bandwidthRF/1e6,
           sel_band1_trf==1 ? "BAND1" : "BAND2",
           Get_SPI_Reg_bits(CG_IAMP_TBB));
#endif
    uint8_t status;
    //BackupRegisters();
    SaveChipState();
    status = CalibrateTxSetup(1);
    if(status != 0)
        goto TxCalibrationEnd; //go to ending stage to restore registers
    CalibrateRxDCAuto();
    CheckSaturationTxRx(1);
    CalibrateRxDCAuto();

    SetNCOFrequency(LMS7002M_Rx, calibrationSXOffset_Hz - offsetNCO + (bandwidthRF/ calibUserBwDivider), 0);
    CalibrateTxDCAuto();
    SetNCOFrequency(LMS7002M_Rx, calibrationSXOffset_Hz - offsetNCO, 0);
    CalibrateIQImbalance(LMS7002M_Tx);
TxCalibrationEnd:
    if(status != 0)
    {
#if VERBOSE
        printf("Tx calibration failed");
#endif
        RestoreChipState();
        return status;
    }
    {
        //uint16_t dccorri = Get_SPI_Reg_bits(DCCORRI_TXTSP.address, DCCORRI_TXTSP.msblsb);
        //uint16_t dccorrq = Get_SPI_Reg_bits(DCCORRQ_TXTSP.address, DCCORRQ_TXTSP.msblsb);
        uint16_t gcorri = Get_SPI_Reg_bits(GCORRI_TXTSP.address, GCORRI_TXTSP.msblsb);
        uint16_t gcorrq = Get_SPI_Reg_bits(GCORRQ_TXTSP.address, GCORRQ_TXTSP.msblsb);
        uint16_t phaseOffset = Get_SPI_Reg_bits(IQCORR_TXTSP.address, IQCORR_TXTSP.msblsb);
        RestoreChipState();
        Modify_SPI_Reg_bits(MAC, ch);
        //Modify_SPI_Reg_bits(DCCORRI_TXTSP.address, DCCORRI_TXTSP.msblsb, dccorri);
        //Modify_SPI_Reg_bits(DCCORRQ_TXTSP.address, DCCORRQ_TXTSP.msblsb, dccorrq);
        Modify_SPI_Reg_bits(GCORRI_TXTSP.address, GCORRI_TXTSP.msblsb, gcorri);
        Modify_SPI_Reg_bits(GCORRQ_TXTSP.address, GCORRQ_TXTSP.msblsb, gcorrq);
        Modify_SPI_Reg_bits(IQCORR_TXTSP.address, IQCORR_TXTSP.msblsb, phaseOffset);
    }

    Modify_SPI_Reg_bits(DCMODE, 1);
    if(ch == 1)
        Modify_SPI_Reg_bits(PD_DCDAC_TXA, 0);
    else
        Modify_SPI_Reg_bits(PD_DCDAC_TXB, 0);
    Modify_SPI_Reg_bits(DC_BYP_TXTSP, 1);
    Modify_SPI_Reg_bits(0x0208, 1<<4 | 0, 0); //GC_BYP PH_BYP
    LoadDC_REG_TX_IQ();

    //LoadDC_REG_TX_IQ(); //not necessary, just for testing convenience
#if VERBOSE
    //printf("#####Tx calibration RESULTS:###########################\n");
    /*printf("Tx ch.%s, BW: %g MHz, RF output: %s, Gain: %i\n",
                    ch == 1 ? "A" : "B",
                    bandwidthRF/1e6, sel_band1_trf==1 ? "BAND1" : "BAND2",
                    1//Get_SPI_Reg_bits(CG_IAMP_TBB)
                    );*/
    {
        int16_t dcI = ReadAnalogDC(ch==1? 0x5C3 : 0x5C5);
        int16_t dcQ = ReadAnalogDC(ch==1? 0x5C4 : 0x5C6);
        int16_t phaseSigned = toSigned(Get_SPI_Reg_bits(IQCORR_TXTSP.address, IQCORR_TXTSP.msblsb), IQCORR_TXTSP.msblsb);
        uint16_t gcorri = Get_SPI_Reg_bits(GCORRI_TXTSP.address, GCORRI_TXTSP.msblsb);
        uint16_t gcorrq = Get_SPI_Reg_bits(GCORRQ_TXTSP.address, GCORRQ_TXTSP.msblsb);
        printf("   | DC  | GAIN | PHASE\n");
        printf("---+-----+------+------\n");
        printf("I: | %3i | %4i | %i\n", dcI, gcorri, phaseSigned);
        printf("Q: | %3i | %4i |\n", dcQ, gcorrq);
    }
#ifdef __cplusplus
    int32_t duration = std::chrono::duration_cast<std::chrono::milliseconds>
                       (std::chrono::high_resolution_clock::now()-beginTime).count();
    printf("Duration: %i ms\n", duration);
#endif
#endif //LMS_VERBOSE_OUTPUT
    return 0;
}

#define MSBLSB(x, y) x << 4 | y

uint8_t CalibrateRxSetup(bool extLoopback)
{
    uint8_t status;
    const uint16_t x0020val = SPI_read(0x0020);
    //rfe
    {
        ROM const uint16_t RxSetupAddr[] = {0x0084, 0x0085,0x00AE,0x010C,0x010D,0x0113,0x0115,0x0119};
        ROM const uint16_t RxSetupData[] = {0x0400, 0x0001,0xF000,0x0000,0x0040,0x000C,0x0000,0x0000};
        ROM const uint16_t RxSetupMask[] = {0xF8FF, 0x0007,0xF000,0x001A,0x0040,0x003C,0xC000,0x8000};
        uint8_t i;
        for(i=sizeof(RxSetupAddr)/sizeof(uint16_t); i; --i)
            SPI_write(RxSetupAddr[i-1], ( SPI_read(RxSetupAddr[i-1]) & ~RxSetupMask[i-1] ) | RxSetupData[i-1]);
    }
    {
        ROM const uint16_t RxSetupAddrWrOnly[] = {0x0100,0x0101,0x0102,0x0103,0x0104,0x0105,0x0106,0x0107,0x0108,0x0109,0x010A,0x0200,0x0201,0x0202,0x0208,0x0240,0x0400,0x0401,0x0402,0x0403,0x0407,0x040A,0x040C,0x0440,0x05C0,0x05CB,0x0203,0x0204,0x0205,0x0206,0x0207,0x0241,0x0404,0x0405,0x0406,0x0408,0x0409,0x0441,0x05C1,0x05C2,0x05C3,0x05C4,0x05C5,0x05C6,0x05C7,0x05C8,0x05C9,0x05CA,0x05CC};
        ROM const uint16_t RxSetupDataWrOnly[] = {0x3408,0x6001,0x3180,0x0A12,0x0088,0x0007,0x318C,0x318C,0x0426,0x61C1,0x104C,0x008D,0x07FF,0x07FF,0x2070,0x0020,0x0081,0x07FF,0x07FF,0x4000,0x0700,0x1000,0x2098,0x0020,0x00FF,0x2020};
        uint8_t i;
        for(i=sizeof(RxSetupAddrWrOnly)/sizeof(uint16_t); i; --i)
            if(i<=sizeof(RxSetupDataWrOnly)/sizeof(uint16_t))
                SPI_write(RxSetupAddrWrOnly[i-1], RxSetupDataWrOnly[i-1]);
            else
                SPI_write(RxSetupAddrWrOnly[i-1], 0);
    }/*
    BeginBatch("RxSetup.txt");
    Modify_SPI_Reg_bits(EN_DCOFF_RXFE_RFE, 1);
    Modify_SPI_Reg_bits(G_RXLOOPB_RFE, 3);
    Modify_SPI_Reg_bits(0x010C, 4 << 4 | 3, 0); //PD_MXLOBUF_RFE 0, PD_QGEN_RFE 0
    Modify_SPI_Reg_bits(0x010C, 1 << 4 | 1, 0); //PD_TIA 0
    if(extLoopback)
        Modify_SPI_Reg_bits(0x010C, 7 << 4 | 7, 0); //PD_LNA 0

    //RBB
    Modify_SPI_Reg_bits(0x0115, MSBLSB(15, 14), 0); //Loopback switches disable
    Modify_SPI_Reg_bits(0x0119, MSBLSB(15, 15), 0); //OSW_PGA 0

    //TRF
    //reset TRF to defaults
    SetDefaults(SECTION_TRF);
    Modify_SPI_Reg_bits(L_LOOPB_TXPAD_TRF, 0);
    Modify_SPI_Reg_bits(EN_LOOPB_TXPAD_TRF, 1);
    Modify_SPI_Reg_bits(EN_G_TRF, 0);
    if(extLoopback)
    {
        if(Get_SPI_Reg_bits(SEL_PATH_RFE) == 1)
        {
            Modify_SPI_Reg_bits(SEL_BAND1_TRF, 0);
            Modify_SPI_Reg_bits(SEL_BAND2_TRF, 1);
        }
        Modify_SPI_Reg_bits(LOSS_MAIN_TXPAD_TRF, 15);
    }

    //TBB
    //reset TBB to defaults
    SetDefaults(SECTION_TBB);
    Modify_SPI_Reg_bits(CG_IAMP_TBB, 1);
    Modify_SPI_Reg_bits(ICT_IAMP_FRP_TBB, 1);
    Modify_SPI_Reg_bits(ICT_IAMP_GG_FRP_TBB, 6);

    //XBUF
    Modify_SPI_Reg_bits(0x0085, MSBLSB(2, 0), 1); //PD_XBUF_RX 0, PD_XBUF_TX 0, EN_G_XBUF 1

    //TXTSP
    SetDefaults(SECTION_TxTSP);
    SetDefaults(SECTION_TxNCO);
    Modify_SPI_Reg_bits(TSGFCW_TXTSP, 1);
    Modify_SPI_Reg_bits(TSGMODE_TXTSP, 0x1);
    Modify_SPI_Reg_bits(INSEL_TXTSP, 1);
    Modify_SPI_Reg_bits(0x0208, MSBLSB(6, 4), 0x7); //GFIR3_BYP 1, GFIR2_BYP 1, GFIR1_BYP 1
    Modify_SPI_Reg_bits(CMIX_GAIN_TXTSP, 0);
    Modify_SPI_Reg_bits(CMIX_SC_TXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_TXTSP, 0);

    //RXTSP
    SetDefaults(SECTION_RxTSP);
    SetDefaults(SECTION_RxNCO);
    Modify_SPI_Reg_bits(0x040C, MSBLSB(5, 3), 0x3); //GFIR2_BYP, GFIR1_BYP
    Modify_SPI_Reg_bits(HBD_OVR_RXTSP, 4);

    Modify_SPI_Reg_bits(AGC_MODE_RXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(CAPSEL, 0);
    Modify_SPI_Reg_bits(AGC_AVG_RXTSP, 0);
    Modify_SPI_Reg_bits(CMIX_GAIN_RXTSP, 0);
    Modify_SPI_Reg_bits(GFIR3_L_RXTSP, 7);
    Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 1);

    //CDS
    Modify_SPI_Reg_bits(CDS_TXATSP, 3);
    Modify_SPI_Reg_bits(CDS_TXBTSP, 3);

    //RSSI_DC_CALIBRATION
    SetDefaults(SECTION_RSSI_DC_CALIBRATION);
    EndBatch();
    //BIAS
    {
        uint16_t rp_calib_bias = Get_SPI_Reg_bits(0x0084, MSBLSB(10, 6));
        SetDefaults(SECTION_BIAS);
        Modify_SPI_Reg_bits(0x0084, MSBLSB(10, 6), rp_calib_bias);
    }*/

    /*if(!extLoopback)
    {
        Modify_SPI_Reg_bits(ICT_IAMP_FRP_TBB, 1);
        Modify_SPI_Reg_bits(ICT_IAMP_GG_FRP_TBB, 6);
    }*/

    //AFE
    if((x0020val & 0x3) == 1)
        Modify_SPI_Reg_bits(PD_TX_AFE1, 0);
    else
        Modify_SPI_Reg_bits(PD_TX_AFE2, 0);

    if(!extLoopback)
    {
        switch(Get_SPI_Reg_bits(SEL_PATH_RFE))
        {
        case 2:
            Modify_SPI_Reg_bits(SEL_BAND2_TRF, 1);
            Modify_SPI_Reg_bits(SEL_BAND1_TRF, 0);
            break;
        case 3:
        case 1:
            Modify_SPI_Reg_bits(SEL_BAND2_TRF, 0);
            Modify_SPI_Reg_bits(SEL_BAND1_TRF, 1);
            break;
        default:
            return 1;
        }
    }

    Modify_SPI_Reg_bits(MAC, 2); //Get freq already changes/restores ch

    if(Get_SPI_Reg_bits(PD_LOCH_T2RBUF) == 0) //isTDD
    {
        //in TDD do nothing
        Modify_SPI_Reg_bits(MAC, 1);
        SetDefaultsSX();
        status = SetFrequencySX(LMS7002M_Rx, GetFrequencySX(LMS7002M_Tx) - bandwidthRF/ calibUserBwDivider - 9e6);
    }
    else
    {
        //SXR
        //Modify_SPI_Reg_bits(MAC, 1); //Get freq already changes/restores ch
        const float_type SXRfreqHz = GetFrequencySX(LMS7002M_Rx);

        //SXT
        Modify_SPI_Reg_bits(MAC, 2);
        SetDefaultsSX();
        status = SetFrequencySX(LMS7002M_Tx, SXRfreqHz + bandwidthRF/ calibUserBwDivider + 9e6);
    }
    if(status != 0)
        return status+0x70;
    SPI_write(0x0020, x0020val);

    LoadDC_REG_TX_IQ();

    //CGEN
    // SetDefaults(SECTION_CGEN);
    status = SetupCGEN();
    if(status != 0)
        return status +0x30;
    SetRxGFIR3Coefficients();
    SetNCOFrequency(LMS7002M_Tx, 9e6, 0);
    SetNCOFrequency(LMS7002M_Rx, bandwidthRF/calibUserBwDivider - offsetNCO, 0);
    //modifications when calibrating channel B
    if( (x0020val&0x3) == 2)
    {
        Modify_SPI_Reg_bits(MAC, 1);
        Modify_SPI_Reg_bits(EN_NEXTRX_RFE, 1);
        Modify_SPI_Reg_bits(EN_NEXTTX_TRF, 1);
        Modify_SPI_Reg_bits(PD_TX_AFE2, 0);
        SPI_write(0x0020, x0020val);
    }
    return 0;
}

uint8_t CheckSaturationRx(const float_type bandwidth_Hz, bool extLoopback)
{
    ROM const uint16_t target_rssi = 0x07000; //0x0B000 = -3 dBFS
    uint16_t rssi;
    const uint8_t rxloopbStep = 2;
    const uint8_t lossMainStep = 1;
    //const uint8_t cg_iampStep = 2;
    uint8_t g_rxloopb_rfe = (uint8_t)Get_SPI_Reg_bits(G_RXLOOPB_RFE);
    uint8_t cg_iamp = (uint8_t)Get_SPI_Reg_bits(CG_IAMP_TBB);
    uint8_t g_lossmain = (uint8_t)Get_SPI_Reg_bits(LOSS_MAIN_TXPAD_TRF);
#ifdef DRAW_GNU_PLOTS
    std::vector<uint32_t> firstStage, secondStage;
#define PUSH_PLOT_VALUE(vec, data) vec.push_back(data)
#else
#define PUSH_PLOT_VALUE(vec, data)
#endif
    Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
    SetNCOFrequency(LMS7002M_Rx, bandwidth_Hz / calibUserBwDivider - offsetNCO, 0);

    rssi = GetRSSI();
    PUSH_PLOT_VALUE(firstStage, rssi);

#if VERBOSE
    printf("Initial gains:\tG_RXLOOPB: %2i, CG_IAMP: %2i | %2.3f dbFS\n", g_rxloopb_rfe, cg_iamp, ChipRSSI_2_dBFS(rssi));
#endif

    if(extLoopback)
        while (rssi < target_rssi)
        {
            g_lossmain += lossMainStep;
            if(g_lossmain < 0)
                break;
            Modify_SPI_Reg_bits(LOSS_MAIN_TXPAD_TRF, g_lossmain);
            rssi = GetRSSI();
            PUSH_PLOT_VALUE(firstStage, rssi);
        }
    else
        while (rssi < target_rssi)
        {
            g_rxloopb_rfe += rxloopbStep;
            if(g_rxloopb_rfe > 15)
                break;
            Modify_SPI_Reg_bits(G_RXLOOPB_RFE, g_rxloopb_rfe);
            rssi = GetRSSI();
            PUSH_PLOT_VALUE(firstStage, rssi);
        }

    PUSH_PLOT_VALUE(secondStage, rssi);

    while(rssi < 0x01000)
    {
        cg_iamp += 2;
        if(cg_iamp > 63-6)
            break;
        Modify_SPI_Reg_bits(CG_IAMP_TBB, cg_iamp);
        rssi = GetRSSI();
        PUSH_PLOT_VALUE(secondStage, rssi);
    }

    while(rssi < target_rssi)
    {
        cg_iamp += 1;
        if(cg_iamp > 62)
            break;
        Modify_SPI_Reg_bits(CG_IAMP_TBB, cg_iamp);
        rssi = GetRSSI();
        PUSH_PLOT_VALUE(secondStage, rssi);
    }
#if VERBOSE
    printf("Adjusted gains: G_RXLOOPB: %2i, CG_IAMP: %2i | %2.3f dbFS\n", Get_SPI_Reg_bits(G_RXLOOPB_RFE), Get_SPI_Reg_bits(CG_IAMP_TBB), ChipRSSI_2_dBFS(rssi));
#endif
#ifdef DRAW_GNU_PLOTS
    saturationPlot.write("set yrange [:0]\n");
    saturationPlot.write("set ylabel 'RSSI dbFS'\n");
    saturationPlot.write("set ylabel 'measurement index'\n");
    saturationPlot.write("set title 'Rx saturation check'\n");
    saturationPlot.write("set key right bottom\n");
    saturationPlot.write("set grid ytics xtics\n");
    saturationPlot.write("plot ");

    int index = 0;
    saturationPlot.writef(
        "'-' title '%s' with lines\
, '-' title 'CG_IAMP' with lines\
, '-' title 'target Level' with lines\n", "G_RXLOOPB_RFE");
    for(auto value: firstStage)
        saturationPlot.writef("%i %f\n", index++, ChipRSSI_2_dBFS(value));
    saturationPlot.write("e\n");
    --index;
    for(auto value: secondStage)
        saturationPlot.writef("%i %f\n", index++, ChipRSSI_2_dBFS(value));
    saturationPlot.write("e\n");
    saturationPlot.writef("%i %f\n%i %f\ne\n", 0, ChipRSSI_2_dBFS(target_rssi), index, ChipRSSI_2_dBFS(target_rssi));
    saturationPlot.flush();
#endif
#undef PUSH_PLOT_VALUE
    return 0;
}

uint8_t CalibrateRxExternalLoop()
{
#ifdef __cplusplus
    auto beginTime = std::chrono::high_resolution_clock::now();
#endif
    uint8_t status;
    //uint16_t gcorri;
    //uint16_t gcorrq;
    //int16_t phaseOffset;
    const uint16_t x0020val = SPI_read(0x0020); //remember used channel

#if VERBOSE
    double rxFreq = GetFrequencySX(LMS7002M_Rx);
    const char* lnaName;
    switch(Get_SPI_Reg_bits(SEL_PATH_RFE))
    {
    case 0:
        lnaName = "none";
        break;
    case 1:
        lnaName = "LNAH";
        break;
    case 2:
        lnaName = "LNAW";
        break;
    case 3:
        lnaName = "LNAL";
        break;
    default:
        lnaName = "none";
        break;
    }
    printf("Rx ch.%s @ %4g MHz, BW: %g MHz, RF input: %s, PGA: %i, LNA: %i, TIA: %i\n",
           (x0020val & 0x3) == 1 ? "A" : "B", rxFreq/1e6,
           bandwidthRF/1e6, lnaName,
           Get_SPI_Reg_bits(G_PGA_RBB),
           Get_SPI_Reg_bits(G_LNA_RFE),
           Get_SPI_Reg_bits(G_TIA_RFE));
    printf("Rx calibration started\n");
#endif
    SaveChipState();
    status = CalibrateRxSetup(1);
    if(status != 0)
        goto RxCalibrationEndStage;
    CalibrateRxDCAuto();
    {
        if ((uint8_t)Get_SPI_Reg_bits(SEL_PATH_RFE) == 2)
        {
            Modify_SPI_Reg_bits(PD_RLOOPB_2_RFE, 0);
            Modify_SPI_Reg_bits(EN_INSHSW_LB2_RFE, 0);
        }
        else
        {
            Modify_SPI_Reg_bits(PD_RLOOPB_1_RFE, 0);
            Modify_SPI_Reg_bits(EN_INSHSW_LB1_RFE, 0);
        }
    }

    Modify_SPI_Reg_bits(MAC, 2);
    if (Get_SPI_Reg_bits(PD_LOCH_T2RBUF) == false)
    {
        Modify_SPI_Reg_bits(PD_LOCH_T2RBUF, 1);
        //TDD MODE
        Modify_SPI_Reg_bits(MAC, 1);
        Modify_SPI_Reg_bits(PD_VCO, 0);
    }
    SPI_write(0x0020, x0020val);
    CheckSaturationRx(bandwidthRF, 1);
    Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 0);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
    //Modify_SPI_Reg_bits(GFIR3_BYP_RXTSP, 0);
    SetNCOFrequency(LMS7002M_Rx, bandwidthRF/calibUserBwDivider + offsetNCO, 0);
    CalibrateIQImbalance(LMS7002M_Rx);
RxCalibrationEndStage:
    if (status != 0)
    {
        RestoreChipState();
        //printf("Rx calibration failed", LOG_WARNING);
        return status;
    }
    {
        uint16_t gcorri = Get_SPI_Reg_bits(GCORRI_RXTSP.address, GCORRI_RXTSP.msblsb);
        uint16_t gcorrq = Get_SPI_Reg_bits(GCORRQ_RXTSP.address, GCORRQ_RXTSP.msblsb);
        uint16_t phaseOffset = Get_SPI_Reg_bits(IQCORR_RXTSP.address, IQCORR_RXTSP.msblsb);
        RestoreChipState();
        SPI_write(0x0020, x0020val);
        //Modify_SPI_Reg_bits(DCCORRI_TXTSP.address, DCCORRI_TXTSP.msblsb, dccorri);
        //Modify_SPI_Reg_bits(DCCORRQ_TXTSP.address, DCCORRQ_TXTSP.msblsb, dccorrq);
        Modify_SPI_Reg_bits(GCORRI_RXTSP.address, GCORRI_RXTSP.msblsb, gcorri);
        Modify_SPI_Reg_bits(GCORRQ_RXTSP.address, GCORRQ_RXTSP.msblsb, gcorrq);
        Modify_SPI_Reg_bits(IQCORR_RXTSP.address, IQCORR_RXTSP.msblsb, phaseOffset);
    }
    Modify_SPI_Reg_bits(DCMODE, 1);
    if(x0020val & 0x1)
        Modify_SPI_Reg_bits(PD_DCDAC_RXA, 0);
    else
        Modify_SPI_Reg_bits(PD_DCDAC_RXB, 0);
    Modify_SPI_Reg_bits(0x040C, MSBLSB(2, 0), 0); //DC_BYP 0, GC_BYP 0, PH_BYP 0
    Modify_SPI_Reg_bits(0x040C, MSBLSB(8, 8), 0); //DCLOOP_STOP
    //Log("Rx calibration finished", LOG_INFO);
#if VERBOSE
    printf("#####Rx calibration RESULTS:###########################\n");
    printf("Method: %s %s loopback\n",
           "RSSI",
           "INTERNAL");
    printf("Rx ch.%s @ %4g MHz, BW: %g MHz, RF input: %s, PGA: %i, LNA: %i, TIA: %i\n",
           (x0020val & 3) == 1 ? "A" : "B", rxFreq/1e6,
           bandwidthRF/1e6, lnaName,
           Get_SPI_Reg_bits(G_PGA_RBB),
           Get_SPI_Reg_bits(G_LNA_RFE),
           Get_SPI_Reg_bits(G_TIA_RFE));
    {
        /*int8_t dcIsigned = (dcoffi & 0x3f) * (dcoffi&0x40 ? -1 : 1);
        int8_t dcQsigned = (dcoffq & 0x3f) * (dcoffq&0x40 ? -1 : 1);
        int16_t phaseSigned = phaseOffset << 4;
        phaseSigned >>= 4;
        verbose_printf("   | DC  | GAIN | PHASE\n");
        verbose_printf("---+-----+------+------\n");
        verbose_printf("I: | %3i | %4i | %i\n", dcIsigned, gcorri, phaseSigned);
        verbose_printf("Q: | %3i | %4i |\n", dcQsigned, gcorrq);*/
    }
    int32_t duration = std::chrono::duration_cast<std::chrono::milliseconds>
                       (std::chrono::high_resolution_clock::now()-beginTime).count();
    printf("Duration: %i ms\n", duration);
#endif //LMS_VERBOSE_OUTPUT
    return 0;
}

uint8_t CalibrateRx()
{
#ifdef __cplusplus
    auto beginTime = std::chrono::high_resolution_clock::now();
#endif
    uint8_t status;
    //uint16_t gcorri;
    //uint16_t gcorrq;
    //int16_t phaseOffset;
    const uint16_t x0020val = SPI_read(0x0020); //remember used channel

#if VERBOSE
    double rxFreq = GetFrequencySX(LMS7002M_Rx);
    const char* lnaName;
    switch(Get_SPI_Reg_bits(SEL_PATH_RFE))
    {
    case 0:
        lnaName = "none";
        break;
    case 1:
        lnaName = "LNAH";
        break;
    case 2:
        lnaName = "LNAW";
        break;
    case 3:
        lnaName = "LNAL";
        break;
    default:
        lnaName = "none";
        break;
    }
    printf("Rx ch.%s @ %4g MHz, BW: %g MHz, RF input: %s, PGA: %i, LNA: %i, TIA: %i\n",
           (x0020val & 0x3) == 1 ? "A" : "B", rxFreq/1e6,
           bandwidthRF/1e6, lnaName,
           Get_SPI_Reg_bits(G_PGA_RBB),
           Get_SPI_Reg_bits(G_LNA_RFE),
           Get_SPI_Reg_bits(G_TIA_RFE));
    printf("Rx calibration started\n");
#endif
    SaveChipState();
    status = CalibrateRxSetup(0);
    if(status != 0)
        goto RxCalibrationEndStage;
    CalibrateRxDCAuto();
    {
        if ((uint8_t)Get_SPI_Reg_bits(SEL_PATH_RFE) == 2)
        {
            Modify_SPI_Reg_bits(PD_RLOOPB_2_RFE, 0);
            Modify_SPI_Reg_bits(EN_INSHSW_LB2_RFE, 0);
        }
        else
        {
            Modify_SPI_Reg_bits(PD_RLOOPB_1_RFE, 0);
            Modify_SPI_Reg_bits(EN_INSHSW_LB1_RFE, 0);
        }
    }

    Modify_SPI_Reg_bits(MAC, 2);
    if (Get_SPI_Reg_bits(PD_LOCH_T2RBUF) == false)
    {
        Modify_SPI_Reg_bits(PD_LOCH_T2RBUF, 1);
        //TDD MODE
        Modify_SPI_Reg_bits(MAC, 1);
        Modify_SPI_Reg_bits(PD_VCO, 0);
    }
    SPI_write(0x0020, x0020val);
    CheckSaturationRx(bandwidthRF, 0);
    Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 0);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
    //Modify_SPI_Reg_bits(GFIR3_BYP_RXTSP, 0);
    SetNCOFrequency(LMS7002M_Rx, bandwidthRF/calibUserBwDivider + offsetNCO, 0);
    CalibrateIQImbalance(LMS7002M_Rx);
RxCalibrationEndStage:
    if (status != 0)
    {
        RestoreChipState();
        //printf("Rx calibration failed", LOG_WARNING);
        return status;
    }
    {
        uint16_t gcorri = Get_SPI_Reg_bits(GCORRI_RXTSP.address, GCORRI_RXTSP.msblsb);
        uint16_t gcorrq = Get_SPI_Reg_bits(GCORRQ_RXTSP.address, GCORRQ_RXTSP.msblsb);
        uint16_t phaseOffset = Get_SPI_Reg_bits(IQCORR_RXTSP.address, IQCORR_RXTSP.msblsb);
        RestoreChipState();
        SPI_write(0x0020, x0020val);
        //Modify_SPI_Reg_bits(DCCORRI_TXTSP.address, DCCORRI_TXTSP.msblsb, dccorri);
        //Modify_SPI_Reg_bits(DCCORRQ_TXTSP.address, DCCORRQ_TXTSP.msblsb, dccorrq);
        Modify_SPI_Reg_bits(GCORRI_RXTSP.address, GCORRI_RXTSP.msblsb, gcorri);
        Modify_SPI_Reg_bits(GCORRQ_RXTSP.address, GCORRQ_RXTSP.msblsb, gcorrq);
        Modify_SPI_Reg_bits(IQCORR_RXTSP.address, IQCORR_RXTSP.msblsb, phaseOffset);
    }
    Modify_SPI_Reg_bits(DCMODE, 1);
    if(x0020val & 0x1)
        Modify_SPI_Reg_bits(PD_DCDAC_RXA, 0);
    else
        Modify_SPI_Reg_bits(PD_DCDAC_RXB, 0);
    Modify_SPI_Reg_bits(0x040C, MSBLSB(2, 0), 0); //DC_BYP 0, GC_BYP 0, PH_BYP 0
    Modify_SPI_Reg_bits(0x040C, MSBLSB(8, 8), 0); //DCLOOP_STOP
    //Log("Rx calibration finished", LOG_INFO);
#if VERBOSE
    printf("#####Rx calibration RESULTS:###########################\n");
    printf("Method: %s %s loopback\n",
           "RSSI",
           "INTERNAL");
    printf("Rx ch.%s @ %4g MHz, BW: %g MHz, RF input: %s, PGA: %i, LNA: %i, TIA: %i\n",
           (x0020val & 3) == 1 ? "A" : "B", rxFreq/1e6,
           bandwidthRF/1e6, lnaName,
           Get_SPI_Reg_bits(G_PGA_RBB),
           Get_SPI_Reg_bits(G_LNA_RFE),
           Get_SPI_Reg_bits(G_TIA_RFE));
    {
        /*int8_t dcIsigned = (dcoffi & 0x3f) * (dcoffi&0x40 ? -1 : 1);
        int8_t dcQsigned = (dcoffq & 0x3f) * (dcoffq&0x40 ? -1 : 1);
        int16_t phaseSigned = phaseOffset << 4;
        phaseSigned >>= 4;
        verbose_printf("   | DC  | GAIN | PHASE\n");
        verbose_printf("---+-----+------+------\n");
        verbose_printf("I: | %3i | %4i | %i\n", dcIsigned, gcorri, phaseSigned);
        verbose_printf("Q: | %3i | %4i |\n", dcQsigned, gcorrq);*/
    }
    int32_t duration = std::chrono::duration_cast<std::chrono::milliseconds>
                       (std::chrono::high_resolution_clock::now()-beginTime).count();
    printf("Duration: %i ms\n", duration);
#endif //LMS_VERBOSE_OUTPUT
    return 0;
}
