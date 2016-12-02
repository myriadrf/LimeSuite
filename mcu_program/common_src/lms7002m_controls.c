#include "lms7002m_controls.h"
#include "spi.h"
#include "LMS7002M_parameters_compact.h"
#include "math.h"
#include "typedefs.h"

#ifdef __cplusplus
#include <cmath>
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
    float_type dMul = (RefClk/2.0)/(Get_SPI_Reg_bits(DIV_OUTCH_CGEN)+1); //DIV_OUTCH_CGEN
    uint16_t gINT = Get_SPI_Reg_bits(0x0088, 13<<4 | 0); //read whole register to reduce SPI transfers
    uint32_t gFRAC = ((uint32_t)(gINT & 0xF) << 16) | Get_SPI_Reg_bits(0x0087, 15 << 4 | 0);
    return dMul * (((gINT>>4) + 1 + gFRAC/1048576.0));
}

uint8_t SetFrequencyCGEN(float_type freq)
{
    float_type dFvco;
    int16_t iHdiv;

    //VCO frequency selection according to F_CLKH
	{
	    uint8_t vcoCnt = 0;
	    float_type vcoFreqs[3];
	    for (iHdiv = 0; iHdiv < 256 && vcoCnt < 3; ++iHdiv)
	    {
	        dFvco = 2 * (iHdiv + 1) * freq;
	        if (dFvco >= 2e9 && dFvco <= 2.7e9)
	            vcoFreqs[vcoCnt++] = dFvco;
	    }
	    if (vcoCnt == 0)
	        return 3;//lime::ReportError(-2, "SetFrequencyCGEN(%g MHz) - cannot deliver requested frequency", freq / 1e6);
		dFvco = vcoFreqs[vcoCnt / 2];
    	iHdiv = dFvco / freq / 2 - 1;
		Modify_SPI_Reg_bits(DIV_OUTCH_CGEN, iHdiv);
	}
    //Integer division
   	Modify_SPI_Reg_bits(INT_SDM_CGEN, (uint16_t)(dFvco/RefClk - 1)); //INT_SDM_CGEN

    //Fractional division
	{
    float_type dFrac = dFvco/RefClk - (uint32_t)(dFvco/RefClk);
    uint32_t gFRAC = (uint32_t)(dFrac * 1048576);
	Modify_SPI_Reg_bits(0x0087, 15 << 4 | 0, gFRAC&0xFFFF); //INT_SDM_CGEN[15:0]
    Modify_SPI_Reg_bits(0x0088, 3 << 4 | 0, gFRAC>>16); //INT_SDM_CGEN[19:16]
	}

#if VERBOSE
    printf("CGEN: Freq=%g MHz, VCO=%g GHz, INT=%i, FRAC=%i, DIV_OUTCH_CGEN=%i\n", freq/1e6, dFvco/1e9, gINT, gFRAC, iHdiv);
#endif // NDEBUG
    return TuneVCO(VCO_CGEN);
}

float_type GetReferenceClk_TSP_MHz(bool tx)
{
    const float_type cgenFreq = GetFrequencyCGEN();
	const float_type clklfreq = cgenFreq/pow2(Get_SPI_Reg_bits(CLKH_OV_CLKL_CGEN));
    if(Get_SPI_Reg_bits(EN_ADCCLKH_CLKGN) == 0)
        return tx ? clklfreq : cgenFreq/4.0;
    else
        return tx ? cgenFreq : clklfreq/4.0;
}

void SetNCOFrequency(bool tx, float freq)
{
    const uint16_t addr = tx ? 0x0242 : 0x0442;
    uint32_t fcw = (uint32_t)((freq/GetReferenceClk_TSP_MHz(tx))*4294967296.0);
    SPI_write(addr, (fcw >> 16)); //NCO frequency control word register MSB part.
    SPI_write(addr+1, fcw); //NCO frequency control word register LSB part.
}

float_type GetFrequencySX(bool Tx)
{
	uint16_t gINT;
	uint32_t gFRAC;
    const uint16_t ch = SPI_read(0x0020);//(uint8_t)Get_SPI_Reg_bits(MAC); //remember previously used channel
	float_type dMul;
	if(Tx)
        Modify_SPI_Reg_bits(MAC, 2); // Rx mac = 1, Tx mac = 2
	else
        Modify_SPI_Reg_bits(MAC, 1); // Rx mac = 1, Tx mac = 2
	gINT = Get_SPI_Reg_bits(0x011E, 13 << 4 | 0);	// read whole register to reduce SPI transfers
    gFRAC = ((uint32_t)(gINT&0xF) << 16) | Get_SPI_Reg_bits(0x011D, 15 << 4 | 0);
    dMul = (float_type)RefClk/ (1 << (Get_SPI_Reg_bits(DIV_LOCH) + 1));
    //Calculate real frequency according to the calculated parameters
    dMul = dMul * ((gINT >> 4) + 4 + ((float_type)gFRAC / 1048576.0)) * (Get_SPI_Reg_bits(EN_DIV2_DIVPROG) + 1);
    //Modify_SPI_Reg_bits(MAC, ch); //restore used channel
    SPI_write(0x0020, ch);
	return dMul;
}

uint8_t SetFrequencySX(bool tx, const float_type freq_Hz)
{
    const uint16_t macBck = SPI_read(0x0020);
    bool canDeliverFrequency;
    Modify_SPI_Reg_bits(MAC, tx?2:1);
    //find required VCO frequency
	{
		float_type VCOfreq;
		int8_t div_loch;
		float_type temp;
		uint32_t fractionalPart;
	    for (div_loch = 6; div_loch >= 0; --div_loch)
	    {
	        VCOfreq = (1 << (div_loch + 1)) * freq_Hz;
	        if ((VCOfreq >= 3800e6) && (VCOfreq <= 7714e6))
	            break;
	    }
		Modify_SPI_Reg_bits(DIV_LOCH, div_loch);

    	temp = VCOfreq / (RefClk * (1 + (VCOfreq > 5500e6)));
	    fractionalPart = (uint32_t)((temp - (uint32_t)(temp)) * 1048576);

		Modify_SPI_Reg_bits(INT_SDM, (uint16_t)(temp - 4)); //INT_SDM
	    Modify_SPI_Reg_bits(0x011D, 15 << 4 | 0, fractionalPart & 0xFFFF); //FRAC_SDM[15:0]
	    Modify_SPI_Reg_bits(0x011E, 3 << 4 | 0, (fractionalPart >> 16)); //FRAC_SDM[19:16]
		Modify_SPI_Reg_bits(EN_DIV2_DIVPROG, (VCOfreq > 5500e6)); //EN_DIV2_DIVPROG
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

    //Modify_SPI_Reg_bits(MAC, macBck);
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

void Delay()
{
#ifdef __cplusplus
    std::this_thread::sleep_for(std::chrono::microseconds(1));
#else
    uint16_t i;
	volatile uint8_t t=0;
	for(i=0; i<400; ++i)
		t <<= 1;
#endif
}

/** @brief Performs VCO tuning operations for CLKGEN, SXR, SXT modules
    @param module module selection for tuning 0-cgen, 1-SXR, 2-SXT
    @return 0-success, other-failure
*/
uint8_t TuneVCO(uint8_t module) // 0-cgen, 1-SXR, 2-SXT
{
    CSWInteval cswSearch[2];
    uint16_t addrCSW_VCO;
    uint16_t addrCMP; //comparator address
    uint8_t msblsb; //SWC msb << 4 | lsb index
    uint16_t cswReg;

    if(module != VCO_CGEN) //set addresses to SX module
    {
        //assuming the active channels is already correct
        //this->SetActiveChannel(Channel(module));
        Modify_SPI_Reg_bits(0x011C, 2 << 4 | 1, 0); //activate VCO and comparator
        addrCSW_VCO = 0x0121;
        msblsb = 10 << 4 | 3;
        addrCMP = 0x0123;
    }
    else //set addresses to CGEN module
    {
        Modify_SPI_Reg_bits(0x0086, 2 << 4 | 1, 0); //activate VCO and comparator
        addrCSW_VCO = 0x008B;
        msblsb = 8 << 4 | 1;
        addrCMP = 0x008C;
    }
    cswReg = SPI_read(addrCSW_VCO);
    //check if lock is within VCO range
    {
        Modify_SPI_Reg_bits_WrOnly(addrCSW_VCO, msblsb, 0, cswReg);
        Delay();
        if(Get_SPI_Reg_bits(addrCMP, 13 << 4 | 12) == 3) //VCO too high
            return 4;
        Modify_SPI_Reg_bits_WrOnly(addrCSW_VCO, msblsb, 255, cswReg);
        Delay();
        if(Get_SPI_Reg_bits(addrCMP, 13 << 4 | 12) == 0) //VCO too low
            return 4;
    }

    //search intervals [0-127][128-255]
    {
    int8_t t;
    for(t=0; t<2; ++t)
    {
        int8_t i;
        cswSearch[t].low = 128*(t+1);
        cswSearch[t].high = 128*t; //search interval lowest value
        Modify_SPI_Reg_bits_WrOnly(addrCSW_VCO, msblsb, cswSearch[t].high, cswReg);

        for(i=6; i>=0; --i)
        {
            uint8_t cmphl; //comparators
            cswSearch[t].high |= (((uint16_t)1) << i); //CSW_VCO<i>=1
            Modify_SPI_Reg_bits_WrOnly(addrCSW_VCO, msblsb, cswSearch[t].high, cswReg);
            Delay();
            cmphl = (uint8_t)Get_SPI_Reg_bits(addrCMP, 13 << 4 | 12);
            if(cmphl == 0x03) // reduce CSW
                cswSearch[t].high &= (~(((uint16_t)1) << i)); //CSW_VCO<i>=0
            if((cmphl == 0x02) && (cswSearch[t].high <= cswSearch[t].low))
                cswSearch[t].low = cswSearch[t].high;
        }
		while(cswSearch[t].low <= cswSearch[t].high && cswSearch[t].low > t*128)
        {
            --cswSearch[t].low;
            Modify_SPI_Reg_bits_WrOnly(addrCSW_VCO, msblsb, cswSearch[t].low, cswReg);
            Delay();
            if(Get_SPI_Reg_bits(addrCMP, 13 << 4 | 12) != 2)
            {
                ++cswSearch[t].low;
                break;
            }
        }
    }
    }
	//Modify_SPI_Reg_bits_WrOnly(addrCSW_VCO, msblsb, cswSearch[0].low+(cswSearch[0].high-cswSearch[0].low)/2, cswReg);
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

        Modify_SPI_Reg_bits_WrOnly(addrCSW_VCO, msblsb, cswLow+(cswHigh-cswLow)/2, cswReg);
        Delay();
        if(Get_SPI_Reg_bits(addrCMP, 13 << 4 | 12) != 0x2)
            //just in case high-low==1, if low fails, check if high locks
            Modify_SPI_Reg_bits_WrOnly(addrCSW_VCO, msblsb, cswLow+(cswHigh-cswLow)/2+1, cswReg);
        else
            return 0;
	}
	Delay();
	if(Get_SPI_Reg_bits(addrCMP, 13 << 4 | 12) != 0x2)
		return 0x20 + module + 1;
    return 0;
}
