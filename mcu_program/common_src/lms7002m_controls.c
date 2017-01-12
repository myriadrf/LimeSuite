#include "lms7002m_controls.h"
#include "spi.h"
#include "LMS7002M_parameters_compact.h"
#include "math.h"
#include "typedefs.h"

#ifdef __cplusplus
#include <cmath>
#include <cstdlib>
using namespace std;
#include <chrono>
#include <thread>
#endif

#define VERBOSE 0

//TODO add functions to modify reference clock
float_type RefClk = 30.72e6; //board reference clock

uint16_t pow2(const uint8_t power)
{
    return 1 << power;
}

float_type GetFrequencyCGEN()
{
    const float_type dMul = (RefClk/2.0)/(Get_SPI_Reg_bits(DIV_OUTCH_CGEN)+1); //DIV_OUTCH_CGEN
    const uint16_t gINT = Get_SPI_Reg_bits(0x0088, 13<<4 | 0); //read whole register to reduce SPI transfers
    const uint32_t gFRAC = ((uint32_t)(gINT & 0xF) << 16) | Get_SPI_Reg_bits(0x0087, 15 << 4 | 0);
    return dMul * (((gINT>>4) + 1 + gFRAC/1048576.0));
}

uint8_t SetFrequencyCGEN(float_type freq)
{
    float_type dFvco;
    uint8_t intpart;
    //VCO frequency selection according to F_CLKH
    {
        uint8_t iHdiv;
        for(iHdiv = 255; iHdiv; --iHdiv)
        {
            dFvco = 2 * (iHdiv) * freq;
            if (dFvco >= 2e9 && dFvco <= 2.7e9)
                goto VCO_FOUND;
        }
        return 3;//lime::ReportError(-2, "SetFrequencyCGEN(%g MHz) - cannot deliver requested frequency", freq / 1e6);
VCO_FOUND:
        //dFvco = vcoFreqs[vcoCnt / 2];
        Modify_SPI_Reg_bits(DIV_OUTCH_CGEN, iHdiv - 1);
    }
    //Integer division
    intpart = dFvco/RefClk;
    Modify_SPI_Reg_bits(INT_SDM_CGEN, intpart - 1); //INT_SDM_CGEN
    //Fractional division
    {
        const float_type dFrac = intpart - (uint32_t)(dFvco/RefClk);
        const uint32_t gFRAC = (uint32_t)(dFrac * 1048576);
        Modify_SPI_Reg_bits(0x0087, 15 << 4 | 0, gFRAC&0xFFFF); //INT_SDM_CGEN[15:0]
        Modify_SPI_Reg_bits(0x0088, 3 << 4 | 0, gFRAC>>16); //INT_SDM_CGEN[19:16]
    }

#if VERBOSE
    //printf("CGEN: Freq=%g MHz, VCO=%g GHz, INT=%i, FRAC=%i, DIV_OUTCH_CGEN=%i\n", freq/1e6, dFvco/1e9, gINT, gFRAC, iHdiv);
#endif // NDEBUG
    return TuneVCO(VCO_CGEN);
}

float_type GetReferenceClk_TSP_MHz(bool tx)
{
    const float_type cgenFreq = GetFrequencyCGEN();
    const float_type clklfreq = cgenFreq/pow2(Get_SPI_Reg_bits(CLKH_OV_CLKL_CGEN));
    if(Get_SPI_Reg_bits(EN_ADCCLKH_CLKGN) == 0)
        return tx ? clklfreq : cgenFreq/4;
    else
        return tx ? cgenFreq : clklfreq/4;
}

void SetNCOFrequency(const bool tx, const float freq)
{
    const uint16_t addr = tx ? 0x0242 : 0x0442;
    const uint32_t fcw = (uint32_t)((freq/GetReferenceClk_TSP_MHz(tx))*4294967296.0);
    SPI_write(addr, (fcw >> 16)); //NCO frequency control word register MSB part.
    SPI_write(addr+1, fcw); //NCO frequency control word register LSB part.
}

float_type GetFrequencySX(const bool Tx)
{
    const uint16_t ch = SPI_read(0x0020);//(uint8_t)Get_SPI_Reg_bits(MAC); //remember previously used channel
    Modify_SPI_Reg_bits(MAC, Tx ? 2 : 1); // Rx mac = 1, Tx mac = 2
    {
        const uint16_t gINT = Get_SPI_Reg_bits(0x011E, 13 << 4 | 0);    // read whole register to reduce SPI transfers
        const uint32_t gFRAC = ((uint32_t)(gINT&0xF) << 16) | SPI_read(0x011D);
        const uint8_t enDiv2 = Get_SPI_Reg_bits(EN_DIV2_DIVPROG)+1;
        const uint8_t divLoch = Get_SPI_Reg_bits(DIV_LOCH) + 1;
        SPI_write(0x0020, ch);
        //Calculate real frequency according to the calculated parameters
        return (enDiv2) * (RefClk / pow2(divLoch)) * ((gINT >> 4) + 4 + (gFRAC / 1048576.0));
    }
}

uint8_t SetFrequencySX(const bool tx, const float_type freq_Hz)
{
    const uint16_t macBck = SPI_read(0x0020);
    bool canDeliverFrequency;
    Modify_SPI_Reg_bits(MAC, tx?2:1);
    //find required VCO frequency
    {
        float_type VCOfreq;
        uint32_t fractionalPart;
        float_type temp;
        {
            uint8_t div_loch;
            for (div_loch = 7; div_loch; --div_loch)
            {
                VCOfreq = pow2(div_loch) * freq_Hz;
                if ((VCOfreq >= 3800e6) && (VCOfreq <= 7714e6))
                    break;
            }
            Modify_SPI_Reg_bits(DIV_LOCH, div_loch-1);
        }
        {
            const uint8_t enDiv2 = (VCOfreq > 5500e6) ? 1 : 0;
            Modify_SPI_Reg_bits(EN_DIV2_DIVPROG, enDiv2); //EN_DIV2_DIVPROG
            temp = VCOfreq / (RefClk * (1 + enDiv2));
            fractionalPart = (uint32_t)((temp - (uint32_t)(temp)) * 1048576);

            Modify_SPI_Reg_bits(INT_SDM, (uint16_t)(temp - 4)); //INT_SDM
            Modify_SPI_Reg_bits(0x011E, 3 << 4 | 0, (fractionalPart >> 16)); //FRAC_SDM[19:16]
            SPI_write(0x011D, fractionalPart & 0xFFFF); //FRAC_SDM[15:0]
        }
    }

    canDeliverFrequency = false;
    {
        int8_t sel_vco;
        int8_t tuneScore[3] = { -128, -128, -128 }; //best is closest to 0
        for (sel_vco = 0; sel_vco < 3; ++sel_vco)
        {
            Modify_SPI_Reg_bits(SEL_VCO, sel_vco);
            if( TuneVCO(tx ? VCO_SXT : VCO_SXR) == 0)
            {
                tuneScore[sel_vco] = -128 + Get_SPI_Reg_bits(CSW_VCO);
                canDeliverFrequency = true;
            }
        }
        sel_vco = 2;
        if (abs(tuneScore[0]) < abs(tuneScore[sel_vco]))
            sel_vco = 0;
        if (abs(tuneScore[1]) < abs(tuneScore[sel_vco]))
            sel_vco = 1;

        Modify_SPI_Reg_bits(SEL_VCO, sel_vco);
        Modify_SPI_Reg_bits(CSW_VCO, tuneScore[sel_vco] + 128);
    }
    SPI_write(0x0020, macBck);
    if (canDeliverFrequency == false)
        return 2;//lime::ReportError(EINVAL, "SetFrequencySX%s(%g MHz) - cannot deliver frequency\n%s", tx?"T":"R", freq_Hz / 1e6, ss.str().c_str());
    return 0;
}

typedef struct
{
    int16_t high;
    int16_t low;
} CSWInteval;

static void Delay()
{
#ifdef __cplusplus
    std::this_thread::sleep_for(std::chrono::microseconds(1));
#else
    uint16_t i;
    volatile uint16_t t=0;
    for(i=0; i<200; ++i)
        t <<= 1;
#endif
}

/** @brief Performs VCO tuning operations for CLKGEN, SXR, SXT modules
    @param module module selection for tuning 0-cgen, 1-SXR, 2-SXT
    @return 0-success, other-failure
*/
static uint8_t ReadCMP(const bool SX)
{
    Delay();
    return (uint8_t)Get_SPI_Reg_bits(SX ? 0x0123 : 0x008C, 13 << 4 | 12);
}

uint8_t TuneVCO(uint8_t module) // 0-cgen, 1-SXR, 2-SXT
{
    const bool SX = (module != VCO_CGEN);
    const uint16_t addrCSW_VCO = SX ? 0x0121 : 0x008B;
    const uint8_t msblsb = SX ? (10 << 4 | 3) : (8 << 4 | 1); //CSW msb << 4 | lsb index
    CSWInteval cswSearch[2];

    if(SX) //set addresses to SX module
        //assuming the active channels is already correct
        Modify_SPI_Reg_bits(0x011C, 2 << 4 | 1, 0); //activate VCO and comparator
    else //set addresses to CGEN module
        Modify_SPI_Reg_bits(0x0086, 2 << 4 | 1, 0); //activate VCO and comparator

    //check if lock is within VCO range
    {
        Modify_SPI_Reg_bits(addrCSW_VCO, msblsb, 0);
        if(ReadCMP(SX) == 3) //VCO too high
            return 4;
        Modify_SPI_Reg_bits(addrCSW_VCO, msblsb, 255);
        if(ReadCMP(SX) == 0) //VCO too low
            return 4;
    }

    //search intervals [0-127][128-255]
    {
        uint16_t t;
        for(t=0; t<2; ++t)
        {
            int8_t i;
            cswSearch[t].low = 128*(t+1);
            cswSearch[t].high = 128*t; //search interval lowest value
            Modify_SPI_Reg_bits(addrCSW_VCO, msblsb, cswSearch[t].high);

            for(i=6; i>=0; --i)
            {
                uint8_t cmphl; //comparators
                cswSearch[t].high |= (1 << i); //CSW_VCO<i>=1
                Modify_SPI_Reg_bits(addrCSW_VCO, msblsb, cswSearch[t].high);
                cmphl = ReadCMP(SX);
                if(cmphl == 0x03) // reduce CSW
                    cswSearch[t].high &= (~(((uint16_t)1) << i)); //CSW_VCO<i>=0
                if((cmphl == 0x02) && (cswSearch[t].high <= cswSearch[t].low))
                    cswSearch[t].low = cswSearch[t].high;
            }
            while(cswSearch[t].low <= cswSearch[t].high && cswSearch[t].low > t*128)
            {
                --cswSearch[t].low;
                Modify_SPI_Reg_bits(addrCSW_VCO, msblsb, cswSearch[t].low);
                if(ReadCMP(SX) != 0x2)
                {
                    ++cswSearch[t].low;
                    break;
                }
            }
        }
    }
    //check if the intervals are joined
    {
        int16_t cswHigh, cswLow;
        if(cswSearch[0].high == cswSearch[1].low-1)
        {
            cswHigh = cswSearch[1].high;
            cswLow = cswSearch[0].low;
        }
        else //compare which interval is wider
        {
            const uint8_t intervalIndex = (cswSearch[1].high-cswSearch[1].low > cswSearch[0].high-cswSearch[0].low) ? 1 : 0;
            cswHigh = cswSearch[intervalIndex].high;
            cswLow = cswSearch[intervalIndex].low;
        }

        cswLow = cswLow+(cswHigh-cswLow)/2;
        Modify_SPI_Reg_bits(addrCSW_VCO, msblsb, cswLow);
        if(ReadCMP(SX) != 0x2)
            //just in case high-low==1, if low fails, check if high locks
            Modify_SPI_Reg_bits(addrCSW_VCO, msblsb, cswLow+1);
    }
    if(ReadCMP(SX) != 0x2)
        return 0x20 + module + 1;
    return 0;
}
