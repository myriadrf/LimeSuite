#include "LMS7002_REGx51.h"
#include "spi.h"	  
#include "LMS7002M_parameters_compact.h"

uint8_t currentInstruction;
uint8_t lastInstruction;

uint32_t idata results_ram _at_ 0xF8;
uint8_t idata firmwareID _at_ 0xFE;

#define calibrationSXOffset_MHz 4

#define VCO_SXR 1
#define VCO_SXT 2
#define VCO_CGEN 0

#define float_type float

#define LMS7param(pr) pr
#define _Rx 0
#define _Tx 1

#define MCU_PARAMETER_ADDRESS 0x002D //register used to pass parameter values to MCU

float_type mRefClkSXR_MHz = 30.72;
float_type bandwidth_MHz = 5;
float_type calibUserBwDivider = 5;

#define DCCORRI_TXTSP_address 0x0204
#define DCCORRI_TXTSP_msb_lsb 15 << 4 | 8

#define DCCORRQ_TXTSP_address 0x0204
#define DCCORRQ_TXTSP_msb_lsb 7 << 4 | 0

#define GCORRI_TXTSP_address 0x0202
#define GCORRQ_TXTSP_address 0x0201
#define IQCORR_TXTSP_address 0x0203
#define IQCORR_TXTSP_msb_lsb 11 << 4 | 0

#define GCORRI_RXTSP_address 0x0402
#define GCORRQ_RXTSP_address 0x0401
#define IQCORR_RXTSP_address 0x0403
#define IQCORR_RXTSP_msb_lsb 11 << 4 | 0
#define gainMSB_LSB (10 << 4 | 0)

#define DCOFFI_RFE_address 0x010E
#define DCOFFI_RFE_msb 13
#define DCOFFI_RFE_lsb 7

#define DCOFFQ_RFE_address 0x010E
#define DCOFFQ_RFE_msb 6
#define DCOFFQ_RFE_lsb 0

void CoarseSearch(const uint16_t addr, const uint8_t msb_lsb, int16_t *value, const uint8_t maxIterations);
void FineSearch(const uint16_t addrI, const uint8_t msbI_lsbI, int16_t *valueI, const uint16_t addrQ, const uint8_t msbQ_lsbQ, int16_t *valueQ, const uint8_t fieldSize);

uint16_t pow2(const uint8_t power)
{
    return 1 << power;
}

enum
{
	MCU_WORKING = 0xFF,
	MCU_IDLE = 0x80,
	MCU_FAILURE = 0x81,
};

/**	@brief Returns reference clock frequency
	Special case because calibrations use fixed CLKGEN frequency, reference clock is
	deduced from CLKGEN INT coefficient
*/
float_type GetReferenceClock_MHz()
{	
	uint16_t Nint = Get_SPI_Reg_bits(0x0088, 13<<4 | 4);
	if( Nint == 41 || Nint == 48) //52 MHz ref
		mRefClkSXR_MHz = 52.0;
	else
		mRefClkSXR_MHz = 30.72;
	return mRefClkSXR_MHz;
}

/**	@return Current CLKGEN frequency in MHz
    Returned frequency depends on reference clock used for Receiver
*/
float_type GetFrequencyCGEN_MHz()
{
    float_type dMul = (mRefClkSXR_MHz/2.0)/(Get_SPI_Reg_bits(DIV_OUTCH_CGEN)+1); //DIV_OUTCH_CGEN
    uint32_t gINT = Get_SPI_Reg_bits(0x0088, 13<<4 | 0); //read whole register to reduce SPI transfers
    uint32_t gFRAC = ((gINT & 0xF) << 16) | Get_SPI_Reg_bits(0x0087, 15 << 4 | 0);
    return dMul * (((gINT>>4) + 1 + gFRAC/1048576.0));
}

/** @brief Returns TSP reference frequency
    @param tx TxTSP or RxTSP selection
    @return TSP reference frequency in MHz
*/
float_type GetReferenceClk_TSP_MHz(bool tx)
{
    float_type cgenFreq = GetFrequencyCGEN_MHz();
	float_type clklfreq = cgenFreq/pow2(Get_SPI_Reg_bits(CLKH_OV_CLKL_CGEN));
    if(Get_SPI_Reg_bits(EN_ADCCLKH_CLKGN) == 0)
        return tx ? clklfreq : cgenFreq/4.0;
    else
        return tx ? cgenFreq : clklfreq/4.0;
}

/** @brief Sets chosen NCO's frequency
    @param tx transmitter or receiver selection
    @param index NCO index from 0 to 15
    @param freq_MHz desired NCO frequency
    @return 0-success, other-failure
*/
void SetNCOFrequency(bool tx, uint8_t index, float_type freq_MHz)
{
    float_type refClk_MHz = GetReferenceClk_TSP_MHz(tx);
    uint16_t addr = tx ? 0x0240 : 0x0440;
	uint32_t fcw = (uint32_t)((freq_MHz/refClk_MHz)*4294967296.0);
    SPI_write(addr+2+index*2, (fcw >> 16)); //NCO frequency control word register MSB part.
    SPI_write(addr+3+index*2, fcw); //NCO frequency control word register LSB part.
}

/**	@brief Returns currently set SXR/SXT frequency
	@return SX frequency MHz
*/
float_type GetFrequencySX_MHz(bool Tx)
{
	uint32_t gINT;
	uint32_t gFRAC;
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(MAC); //remember previously used channel
	float_type dMul;
	if(Tx)
        Modify_SPI_Reg_bits(MAC, 2); // Rx mac = 1, Tx mac = 2
	else
        Modify_SPI_Reg_bits(MAC, 1); // Rx mac = 1, Tx mac = 2
	gINT = Get_SPI_Reg_bits(0x011E, 13 << 4 | 0);	// read whole register to reduce SPI transfers
    gFRAC = ((gINT&0xF) << 16) | Get_SPI_Reg_bits(0x011D, 15 << 4 | 0);
    dMul = (float_type)mRefClkSXR_MHz / (1 << (Get_SPI_Reg_bits(DIV_LOCH) + 1));
    //Calculate real frequency according to the calculated parameters
    dMul = dMul * ((gINT >> 4) + 4 + ((float_type)gFRAC / 1048576.0)) * (Get_SPI_Reg_bits(EN_DIV2_DIVPROG) + 1);
    Modify_SPI_Reg_bits(MAC, ch); //restore used channel
	return dMul;
}

int16_t labs(const int16_t number)
{
 	if(number < 0)
		return number * -1;
	return number;
}

uint8_t toDCOffset(int8_t offset)
{
    if (offset < 0)
        return labs(offset) | 0x40;
	else
    	return offset;
}

#define x400reg 0x0081
uint32_t GetRSSI()
{
	//flip capture
	SPI_write(0x0400, x400reg);
	SPI_write(0x0400, x400reg | 0x8000);
    return ((uint32_t)(SPI_read(0x040F) << 2)) | (uint32_t)(Get_SPI_Reg_bits(0x040E, 1 << 4 | 0));
}

#define RSSI_READ_COUNT 30
uint32_t ReadRSSI()
{	
	xdata uint16_t ram_buff[RSSI_READ_COUNT];
	uint32_t temp;
	uint8_t i, j;
	Modify_SPI_Reg_bits(CAPSEL, 0);

	//collect rssi values;
	for(i=0; i<RSSI_READ_COUNT; ++i)
	{
		ram_buff[i] = GetRSSI();
	}
	
	//sort values ascending
	for(i = 0; i<RSSI_READ_COUNT-1; ++i)
	{
		for(j = i; j<RSSI_READ_COUNT; ++j)
		{
			if(ram_buff[i] > ram_buff[j])
			{
				temp = ram_buff[i];
				ram_buff[i] = ram_buff[j];
				ram_buff[j] = temp;
			}
		}
	}

	//calculate avg from lowest values
	temp = 0;
	for(i = 0; i<4; ++i)
		temp += ram_buff[i];
	temp /= 4; 
	results_ram = temp;
	return temp;
}

void SetRxDCOFF(const int8_t offsetI, const int8_t offsetQ)
{
    SPI_write(0x010E, toDCOffset(offsetI) << 7 | toDCOffset(offsetQ));
}

/** @brief Performs Rx DC offsets calibration
*/
void CalibrateRxDC_RSSI()
{
	int16_t offsetI = 32;
    int16_t offsetQ = 32;
    Modify_SPI_Reg_bits(DC_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(CAPSEL, 0);
    SetRxDCOFF(offsetI, offsetQ);
    CoarseSearch(DCOFFI_RFE_address, DCOFFI_RFE_msb << 4 | DCOFFI_RFE_lsb, &offsetI, 6);
    CoarseSearch(DCOFFQ_RFE_address, DCOFFQ_RFE_msb << 4 | DCOFFQ_RFE_lsb, &offsetQ, 6);
    CoarseSearch(DCOFFI_RFE_address, DCOFFI_RFE_msb << 4 | DCOFFI_RFE_lsb, &offsetI, 4);
    CoarseSearch(DCOFFQ_RFE_address, DCOFFQ_RFE_msb << 4 | DCOFFQ_RFE_lsb, &offsetQ, 4);
	FineSearch(DCOFFI_RFE_address, DCOFFI_RFE_msb << 4 | DCOFFI_RFE_lsb, &offsetI, DCOFFQ_RFE_address, DCOFFQ_RFE_msb << 4 | DCOFFQ_RFE_lsb, &offsetQ, 7);

	SetRxDCOFF(offsetI, offsetQ);	   
	Modify_SPI_Reg_bits(DC_BYP_RXTSP, 0); // DC_BYP 0
}

#define DCOFFaddr 0x010E
void CoarseSearch(const uint16_t addr, const uint8_t msb_lsb, int16_t *valuePtr, const uint8_t maxIterations)
{
	uint8_t rssi_counter = 0;
	uint32_t rssiUp;
	uint32_t rssiDown;
	int16_t value = *valuePtr;
	Modify_SPI_Reg_bits(addr, msb_lsb, addr != DCOFFaddr ? value : toDCOffset(value));
	for (rssi_counter = 0; rssi_counter < maxIterations-1; ++rssi_counter)
	{
		rssiUp = ReadRSSI();
        value -= pow2(maxIterations - rssi_counter);
        Modify_SPI_Reg_bits(addr, msb_lsb, addr != DCOFFaddr ? value : toDCOffset(value));
		rssiDown = ReadRSSI();

		if (rssiUp >= rssiDown)
			value += pow2(maxIterations - 2 - rssi_counter);
		else
			value = value + pow2(maxIterations - rssi_counter) + pow2(maxIterations - 1 - rssi_counter) - pow2(maxIterations-2 - rssi_counter);
        Modify_SPI_Reg_bits(addr, msb_lsb, addr != DCOFFaddr ? value : toDCOffset(value));
	}
	value -= pow2(maxIterations - rssi_counter);
	rssiUp = ReadRSSI();
    if (addr != DCOFFaddr)
	    Modify_SPI_Reg_bits(addr, msb_lsb, value - pow2(maxIterations - rssi_counter));
    else
        Modify_SPI_Reg_bits(addr, msb_lsb, toDCOffset(value - pow2(maxIterations - rssi_counter)));
	rssiDown = ReadRSSI();
	if (rssiUp < rssiDown)
		value += 1;

    Modify_SPI_Reg_bits(addr, msb_lsb, addr != DCOFFaddr ? value : toDCOffset(value));
	rssiDown = ReadRSSI();
	
	if (rssiUp < rssiDown)
	{
		value += 1;
        Modify_SPI_Reg_bits(addr, msb_lsb, addr != DCOFFaddr ? value : toDCOffset(value));
	}
	*valuePtr = value;
}

void FineSearch(const uint16_t addrI, const uint8_t msbI_lsbI, int16_t *valueI, const uint16_t addrQ, const uint8_t msbQ_lsbQ, int16_t *valueQ, const uint8_t fieldSize)
{	
	xdata uint32_t rssiField[7][7];
	uint32_t minRSSI = ~0;
	int16_t i, q;
	int16_t startI = *valueI;
	int16_t startQ = *valueQ;

	for (i = 0; i < fieldSize; ++i)
	{
		for (q = 0; q < fieldSize; ++q)
		{
			int16_t ival = startI + (i - fieldSize / 2);
			int16_t qval = startQ + (q - fieldSize / 2);
			Modify_SPI_Reg_bits(addrI, msbI_lsbI, addrI != DCOFFaddr ? ival : toDCOffset(ival));
			Modify_SPI_Reg_bits(addrQ, msbQ_lsbQ, addrQ != DCOFFaddr ? qval : toDCOffset(qval));
			rssiField[i][q] = ReadRSSI();
			if (rssiField[i][q] < minRSSI)
			{
				*valueI = ival;
				*valueQ = qval;
				minRSSI = rssiField[i][q];
			}
		}
	}
	
	Modify_SPI_Reg_bits(addrI, msbI_lsbI, addrI != DCOFFaddr ? *valueI : toDCOffset(*valueI));
	Modify_SPI_Reg_bits(addrQ, msbQ_lsbQ, addrQ != DCOFFaddr ? *valueQ : toDCOffset(*valueQ));
}

void CheckSaturationTxRx()
{
	uint32_t rssi;
	int8_t g_pga;
	int8_t g_rxlooop;

	Modify_SPI_Reg_bits(LMS7param(DC_BYP_RXTSP), 0);
	Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 0);
	SetNCOFrequency(_Rx, 0, calibrationSXOffset_MHz - 0.1 + (bandwidth_MHz / calibUserBwDivider) * 2);

	rssi = ReadRSSI();
	g_pga = Get_SPI_Reg_bits(G_PGA_RBB);
	g_rxlooop = Get_SPI_Reg_bits(G_RXLOOPB_RFE);
	while (rssi < 0x0B000 && g_rxlooop < 15)
	{
		rssi = ReadRSSI();
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
}	

void CalibrateTxDC_RSSI()
{
	int16_t corrI = 64;
	int16_t corrQ = 64;

	Modify_SPI_Reg_bits(EN_G_TRF, 1);
	Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
	SetNCOFrequency(_Rx, 0, calibrationSXOffset_MHz - 0.1 + (bandwidth_MHz / calibUserBwDivider));

	Modify_SPI_Reg_bits(DCCORRQ_TXTSP, 0);
	CoarseSearch(DCCORRI_TXTSP_address, DCCORRI_TXTSP_msb_lsb, &corrI, 7);
	CoarseSearch(DCCORRQ_TXTSP_address, DCCORRQ_TXTSP_msb_lsb, &corrQ, 7);
	CoarseSearch(DCCORRI_TXTSP_address, DCCORRI_TXTSP_msb_lsb, &corrI, 4);
	CoarseSearch(DCCORRQ_TXTSP_address, DCCORRQ_TXTSP_msb_lsb, &corrQ, 4);
    FineSearch(DCCORRI_TXTSP_address, DCCORRI_TXTSP_msb_lsb, &corrI, DCCORRQ_TXTSP_address, DCCORRQ_TXTSP_msb_lsb, &corrQ, 7);
}

/** @brief Calibrates Transmitter. DC correction, IQ gains, IQ phase correction
    @return 0-success, other-failure
*/
uint8_t CalibrateTx()
{
    uint16_t gainAddr;
    int16_t phaseOffset;
    int16_t gain = 1983;
	mRefClkSXR_MHz = GetReferenceClock_MHz();
	//get calibration bandwidth, register value is kHz
	bandwidth_MHz = SPI_read(MCU_PARAMETER_ADDRESS);

    CheckSaturationTxRx();

    Modify_SPI_Reg_bits(EN_G_TRF, 0);
       
    CalibrateRxDC_RSSI();    
    CalibrateTxDC_RSSI();

    //TXIQ
    Modify_SPI_Reg_bits(EN_G_TRF, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_TXTSP, 0);

    SetNCOFrequency(_Rx, 0, calibrationSXOffset_MHz - 0.1);

    //coarse gain
    {
        uint32_t rssiIgain;
        uint32_t rssiQgain;
        Modify_SPI_Reg_bits(GCORRI_TXTSP, 2047 - 64);
        Modify_SPI_Reg_bits(GCORRQ_TXTSP, 2047);
        rssiIgain = ReadRSSI();
        Modify_SPI_Reg_bits(GCORRI_TXTSP, 2047);
        Modify_SPI_Reg_bits(GCORRQ_TXTSP, 2047 - 64);
        rssiQgain = ReadRSSI();

        Modify_SPI_Reg_bits(GCORRI_TXTSP, 2047);
        Modify_SPI_Reg_bits(GCORRQ_TXTSP, 2047);

		gainAddr = rssiIgain < rssiQgain ? GCORRI_TXTSP_address : GCORRQ_TXTSP_address;
    }
	CoarseSearch(gainAddr, gainMSB_LSB, &gain, 7);

    //coarse phase offset
    {
        uint32_t rssiUp;
        uint32_t rssiDown;
        Modify_SPI_Reg_bits(IQCORR_TXTSP, 15);
        rssiUp = ReadRSSI();
        Modify_SPI_Reg_bits(IQCORR_TXTSP, -15);
        rssiDown = ReadRSSI();
        if (rssiUp > rssiDown)
            phaseOffset = -64;
        else if (rssiUp < rssiDown)
            phaseOffset = 192;
        else
            phaseOffset = 64;
    }
	CoarseSearch(IQCORR_TXTSP_address, IQCORR_TXTSP_msb_lsb, &phaseOffset, 7);
    CoarseSearch(gainAddr, gainMSB_LSB, &gain, 4);
	FineSearch(gainAddr, gainMSB_LSB, &gain, IQCORR_TXTSP_address, IQCORR_TXTSP_msb_lsb, &phaseOffset, 7);
    return MCU_IDLE;
}	

void CheckSaturation()
{
	uint32_t rssi;
	int16_t cg_iamp = Get_SPI_Reg_bits(CG_IAMP_TBB);
	Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 0);
	Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
	
	SetNCOFrequency(_Rx, 0, bandwidth_MHz/calibUserBwDivider - 0.1);
	rssi = ReadRSSI();

	{
		int16_t g_rxloopb_rfe = Get_SPI_Reg_bits(G_RXLOOPB_RFE);
	    while (rssi < 0x0B000 && g_rxloopb_rfe  < 15)
	    {
	        rssi = GetRSSI();
	        if (rssi < 0x0B000)
	            g_rxloopb_rfe += 2;
	        if (rssi > 0x0B000)
	            break;
	        Modify_SPI_Reg_bits(G_RXLOOPB_RFE, g_rxloopb_rfe);
	    }
	}
	
	while (rssi < 0x01000 && cg_iamp < 63-6)
	{
		rssi = ReadRSSI();
		if (rssi < 0x01000)
			cg_iamp += 4;
		if (rssi > 0x01000)
			break;
		Modify_SPI_Reg_bits(CG_IAMP_TBB, cg_iamp);
	}

	while (rssi < 0x0B000 && cg_iamp < 62)
	{
		rssi = ReadRSSI();
		if (rssi < 0x0B000)
			cg_iamp += 2;
		Modify_SPI_Reg_bits(CG_IAMP_TBB, cg_iamp);
	}
}

/** @brief Calibrates Receiver. DC offset, IQ gains, IQ phase correction
    @return 0-success, other-failure
*/
uint8_t CalibrateRx()
{
	int16_t iqcorr_rx = 0;
    int16_t gain;
    uint16_t gainAddr = 0;
    int16_t phaseOffset;

	uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));
    uint8_t sel_path_rfe = (uint8_t)Get_SPI_Reg_bits(LMS7param(SEL_PATH_RFE));
    if (sel_path_rfe == 1 || sel_path_rfe == 0)
        return MCU_FAILURE;

	//get calibration bandwidth, register value is kHz
	bandwidth_MHz = SPI_read(MCU_PARAMETER_ADDRESS);

	mRefClkSXR_MHz = GetReferenceClock_MHz();
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

	Modify_SPI_Reg_bits(MAC, 2);
    if(Get_SPI_Reg_bits(PD_LOCH_T2RBUF) == false)
	{
		Modify_SPI_Reg_bits(PD_LOCH_T2RBUF, 1);
		//TDD MODE
	    Modify_SPI_Reg_bits(MAC, 1);
	    Modify_SPI_Reg_bits(PD_VCO, 0);
	}
    Modify_SPI_Reg_bits(MAC, ch);

	CheckSaturation();

	Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
    SetNCOFrequency(_Rx, 0, bandwidth_MHz/calibUserBwDivider + 0.1);


	Modify_SPI_Reg_bits(IQCORR_RXTSP, 0);
	Modify_SPI_Reg_bits(GCORRI_RXTSP, 2047);
    Modify_SPI_Reg_bits(GCORRQ_RXTSP, 2047);

    //coarse gain
    {
		uint32_t rssiIgain;
		uint32_t rssiQgain;
        Modify_SPI_Reg_bits(GCORRI_RXTSP, 2047 - 64);
        Modify_SPI_Reg_bits(GCORRQ_RXTSP, 2047);
        rssiIgain = GetRSSI();
        Modify_SPI_Reg_bits(GCORRI_RXTSP, 2047);
        Modify_SPI_Reg_bits(GCORRQ_RXTSP, 2047 - 64);
        rssiQgain = GetRSSI();

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

	CoarseSearch(gainAddr, gainMSB_LSB, &gain, 7);
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
	CoarseSearch(IQCORR_RXTSP_address, IQCORR_RXTSP_msb_lsb, &phaseOffset, 7);
    CoarseSearch(gainAddr, gainMSB_LSB, &gain, 4);
	CoarseSearch(IQCORR_RXTSP_address, IQCORR_RXTSP_msb_lsb, &phaseOffset, 4);

    FineSearch(gainAddr, gainMSB_LSB, &gain, IQCORR_RXTSP_address, IQCORR_RXTSP_msb_lsb, &phaseOffset, 7);
    Modify_SPI_Reg_bits(gainAddr, gainMSB_LSB, gain);
    Modify_SPI_Reg_bits(IQCORR_RXTSP_address, IQCORR_RXTSP_msb_lsb, phaseOffset);

    return MCU_IDLE;
}

/*
	P1[7] : 0-MCU idle, 1-MCU_working
	P1[6:0] : return status (while working = 0x3F)
*/

void main()  //main routine
{
	SP=0xD0; // Set stack pointer
	DIR0=0x00; // ;DIR0 - Configure P0 as all inputs
  	DIR1=0xFF;  // ;DIR1 - Configure P1 as all outputs
  	DIR2=0x07;  // ;DIR2 -  Configure P2_3 is input
	IEN1=0;//0x04;  //EX2=1 enable external interrupt 2

	ucSCLK=0; //repairs external SPI
	ucSEN=1;//

	firmwareID = 0x01;	

	//P1 returns MCU status	
	P1 = MCU_IDLE; 
	lastInstruction = P0;
	while(1) 
	{				
		currentInstruction = P0;
		if(currentInstruction != lastInstruction)
		{
			lastInstruction = currentInstruction;
			if(currentInstruction == 0)
				P1 = MCU_IDLE;
			else if(currentInstruction == 1)  // Tx DC IQ phase
			{
				P1 = MCU_WORKING;
				P1 = CalibrateTx();				
			}
			else if(currentInstruction == 2)  // Rx DC IQ phase
			{
				P1 = MCU_WORKING;				
				P1 = CalibrateRx();
			}
			else if(currentInstruction == 3) // Read RSSI
			{
				P1 = MCU_WORKING;
				ReadRSSI();
				P1 = MCU_IDLE;
			}
			else if(currentInstruction == 255) // return program ID
			{	
				P1 = MCU_WORKING;
				P1 = MCU_IDLE | firmwareID;
			}
		} 		
	}
}

