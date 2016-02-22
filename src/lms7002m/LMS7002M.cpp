/**
@file LMS7002M.cpp
@author Lime Microsystems (www.limemicro.com)
@brief Implementation of LMS7002M transceiver configuring
*/

#include "LMS7002M.h"
#include <stdio.h>
#include <set>
#include "lmsComms.h"
#include "INI.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "LMS7002M_RegistersMap.h"
#include <math.h>

#include <chrono>
#include <thread>

#include "MCU_BD.h"

using namespace std;

//#define USE_MCU
#define RSSI_FROM_MCU

#define LMS_VERBOSE_OUTPUT

const float_type CGEN_FREQ_CALIBRATIONS = 368.64;

inline uint16_t pow2(const uint8_t power)
{
    assert(power >= 0 && power < 16);
    return 1 << power;
}

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

bool sign(const int number)
{
	return number < 0;
}

float_type LMS7002M::gVCO_frequency_table[3][2] = { { 3800, 5222 }, { 4961, 6754 }, {6306, 7714} };
float_type LMS7002M::gCGEN_VCO_frequencies[2] = {2000, 2700};

///define for parameter enumeration if prefix might be needed
#define LMS7param(id) id

//module addresses needs to be sorted in ascending order
const uint16_t LMS7002M::readOnlyRegisters[] =      { 0x002F, 0x008C, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x0123, 0x0209, 0x020A, 0x020B, 0x040E, 0x040F };
const uint16_t LMS7002M::readOnlyRegistersMasks[] = { 0x0000, 0x0FFF, 0x007F, 0x0000, 0x0000, 0x0000, 0x0000, 0x003F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

float_type calibUserBwDivider = 5;

/** @brief Simple logging function to print status messages
    @param text message to print
    @param type message type for filtering specific information
*/
void LMS7002M::Log(const char* text, LogType type)
{
    switch(type)
    {
    case LOG_INFO:
        printf("%s\n", text);
        break;
    case LOG_WARNING:
        printf("Warning: %s\n", text);
        break;
    case LOG_ERROR:
        printf("ERROR: %s\n", text);
        break;
    case LOG_DATA:
        printf("DATA: %s\n", text);
        break;
    }
}

LMS7002M::LMS7002M() : controlPort(NULL), mRegistersMap(new LMS7002M_RegistersMap())
{
    mRefClkSXR_MHz = 30.72;
    mRefClkSXT_MHz = 30.72;
    mcuControl = new MCU_BD();
}

/** @brief Creates LMS7002M main control object, it requires LMScomms to communicate with chip
    @param controlPort data connection for controlling LMS7002 chip registers
*/
LMS7002M::LMS7002M(LMScomms* controlPort) :
    controlPort(controlPort), mRegistersMap(new LMS7002M_RegistersMap())
{
    mRefClkSXR_MHz = 30.72;
    mRefClkSXT_MHz = 30.72;

    //memory intervals for registers tests and calibration algorithms
    MemorySectionAddresses[LimeLight][0] = 0x0020;
    MemorySectionAddresses[LimeLight][1] = 0x002F;
    MemorySectionAddresses[EN_DIR][0] = 0x0081;
    MemorySectionAddresses[EN_DIR][1] = 0x0081;
    MemorySectionAddresses[AFE][0] = 0x0082;
    MemorySectionAddresses[AFE][1] = 0x0082;
    MemorySectionAddresses[BIAS][0] = 0x0084;
    MemorySectionAddresses[BIAS][1] = 0x0084;
    MemorySectionAddresses[XBUF][0] = 0x0085;
    MemorySectionAddresses[XBUF][1] = 0x0085;
    MemorySectionAddresses[CGEN][0] = 0x0086;
    MemorySectionAddresses[CGEN][1] = 0x008C;
    MemorySectionAddresses[LDO][0] = 0x0092;
    MemorySectionAddresses[LDO][1] = 0x00A7;
    MemorySectionAddresses[BIST][0] = 0x00A8;
    MemorySectionAddresses[BIST][1] = 0x00AC;
    MemorySectionAddresses[CDS][0] = 0x00AD;
    MemorySectionAddresses[CDS][1] = 0x00AE;
    MemorySectionAddresses[TRF][0] = 0x0100;
    MemorySectionAddresses[TRF][1] = 0x0104;
    MemorySectionAddresses[TBB][0] = 0x0105;
    MemorySectionAddresses[TBB][1] = 0x010A;
    MemorySectionAddresses[RFE][0] = 0x010C;
    MemorySectionAddresses[RFE][1] = 0x0114;
    MemorySectionAddresses[RBB][0] = 0x0115;
    MemorySectionAddresses[RBB][1] = 0x011A;
    MemorySectionAddresses[SX][0] = 0x011C;
    MemorySectionAddresses[SX][1] = 0x0124;
    MemorySectionAddresses[TxTSP][0] = 0x0200;
    MemorySectionAddresses[TxTSP][1] = 0x020C;
    MemorySectionAddresses[TxNCO][0] = 0x0240;
    MemorySectionAddresses[TxNCO][1] = 0x0261;
    MemorySectionAddresses[TxGFIR1][0] = 0x0280;
    MemorySectionAddresses[TxGFIR1][1] = 0x02A7;
    MemorySectionAddresses[TxGFIR2][0] = 0x02C0;
    MemorySectionAddresses[TxGFIR2][1] = 0x02E7;
    MemorySectionAddresses[TxGFIR3a][0] = 0x0300;
    MemorySectionAddresses[TxGFIR3a][1] = 0x0327;
    MemorySectionAddresses[TxGFIR3b][0] = 0x0340;
    MemorySectionAddresses[TxGFIR3b][1] = 0x0367;
    MemorySectionAddresses[TxGFIR3c][0] = 0x0380;
    MemorySectionAddresses[TxGFIR3c][1] = 0x03A7;
    MemorySectionAddresses[RxTSP][0] = 0x0400;
    MemorySectionAddresses[RxTSP][1] = 0x040F;
    MemorySectionAddresses[RxNCO][0] = 0x0440;
    MemorySectionAddresses[RxNCO][1] = 0x0461;
    MemorySectionAddresses[RxGFIR1][0] = 0x0480;
    MemorySectionAddresses[RxGFIR1][1] = 0x04A7;
    MemorySectionAddresses[RxGFIR2][0] = 0x04C0;
    MemorySectionAddresses[RxGFIR2][1] = 0x04E7;
    MemorySectionAddresses[RxGFIR3a][0] = 0x0500;
    MemorySectionAddresses[RxGFIR3a][1] = 0x0527;
    MemorySectionAddresses[RxGFIR3b][0] = 0x0540;
    MemorySectionAddresses[RxGFIR3b][1] = 0x0567;
    MemorySectionAddresses[RxGFIR3c][0] = 0x0580;
    MemorySectionAddresses[RxGFIR3c][1] = 0x05A7;

    mRegistersMap->InitializeDefaultValues(LMS7parameterList);
    mcuControl = new MCU_BD();
    mcuControl->Initialize(controlPort);
}

LMS7002M::~LMS7002M()
{
    delete mcuControl;
}

/** @brief Sends reset signal to chip, after reset enables B channel controls
    @return 0-success, other-failure
*/
liblms7_status LMS7002M::ResetChip()
{
    if (controlPort == NULL)
        return LIBLMS7_NO_CONNECTION_MANAGER;
    if (controlPort->IsOpen() == false)
        return LIBLMS7_NOT_CONNECTED;

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_RST;
    pkt.outBuffer.push_back(LMS_RST_PULSE);
    controlPort->TransferPacket(pkt);
    if (pkt.status == STATUS_COMPLETED_CMD)
    {
        Modify_SPI_Reg_bits(LMS7param(MIMO_SISO), 0); //enable B channel after reset
        return LIBLMS7_SUCCESS;
    }
    else
        return LIBLMS7_FAILURE;
}

liblms7_status LMS7002M::LoadConfigLegacyFile(const char* filename)
{
    ifstream f(filename);
    if (f.good() == false) //file not found
    {
        f.close();
        return LIBLMS7_FILE_NOT_FOUND;
    }
    f.close();
    uint16_t addr = 0;
    uint16_t value = 0;
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC)); //remember used channel
    liblms7_status status;
    typedef INI<string, string, string> ini_t;
    ini_t parser(filename, true);
    if (parser.select("FILE INFO") == false)
        return LIBLMS7_FILE_INVALID_FORMAT;

    string type = "";
    type = parser.get("type", "undefined");
    stringstream ss;
    if (type.find("LMS7002 configuration") == string::npos)
    {
        ss << "File " << filename << " not recognized" << endl;
        return LIBLMS7_FILE_INVALID_FORMAT;
    }

    int fileVersion = 0;
    fileVersion = parser.get("version", 0);

    vector<uint16_t> addrToWrite;
    vector<uint16_t> dataToWrite;
    if (fileVersion == 1)
    {
        if (parser.select("Reference clocks"))
        {
            mRefClkSXR_MHz = parser.get("SXR reference frequency MHz", 30.72);
            mRefClkSXT_MHz = parser.get("SXT reference frequency MHz", 30.72);
        }

        if (parser.select("LMS7002 registers ch.A") == true)
        {
            ini_t::sectionsit_t section = parser.sections.find("LMS7002 registers ch.A");

            uint16_t x0020_value = 0;
            Modify_SPI_Reg_bits(LMS7param(MAC), 1); //select A channel
            for (ini_t::keysit_t pairs = section->second->begin(); pairs != section->second->end(); pairs++)
            {
                sscanf(pairs->first.c_str(), "%hx", &addr);
                sscanf(pairs->second.c_str(), "%hx", &value);
                if (addr == LMS7param(MAC).address) //skip register containing channel selection
                {
                    x0020_value = value;
                    continue;
                }
                addrToWrite.push_back(addr);
                dataToWrite.push_back(value);
            }
            status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size());
            if (status != LIBLMS7_SUCCESS && status != LIBLMS7_NOT_CONNECTED)
                return status;

            //parse FCW or PHO
            if (parser.select("NCO Rx ch.A") == true)
            {
                char varname[64];
                int mode = Get_SPI_Reg_bits(LMS7param(MODE_RX));
                if (mode == 0) //FCW
                {
                    for (int i = 0; i < 16; ++i)
                    {
                        sprintf(varname, "FCW%02i", i);
                        SetNCOFrequency(LMS7002M::Rx, i, parser.get(varname, 0.0));
                    }
                }
                else
                {
                    for (int i = 0; i < 16; ++i)
                    {
                        sprintf(varname, "PHO%02i", i);
                        SetNCOPhaseOffset(LMS7002M::Rx, i, parser.get(varname, 0.0));
                    }
                }
            }
            if (parser.select("NCO Tx ch.A") == true)
            {
                char varname[64];
                int mode = Get_SPI_Reg_bits(LMS7param(MODE_TX));
                if (mode == 0) //FCW
                {
                    for (int i = 0; i < 16; ++i)
                    {
                        sprintf(varname, "FCW%02i", i);
                        SetNCOFrequency(LMS7002M::Tx, i, parser.get(varname, 0.0));
                    }
                }
                else
                {
                    for (int i = 0; i < 16; ++i)
                    {
                        sprintf(varname, "PHO%02i", i);
                        SetNCOPhaseOffset(LMS7002M::Tx, i, parser.get(varname, 0.0));
                    }
                }
            }
            status = SPI_write(0x0020, x0020_value);
            if (status != LIBLMS7_SUCCESS && status != LIBLMS7_NOT_CONNECTED)
                return status;
        }

        Modify_SPI_Reg_bits(LMS7param(MAC), 2);

        if (parser.select("LMS7002 registers ch.B") == true)
        {
            addrToWrite.clear();
            dataToWrite.clear();
            ini_t::sectionsit_t section = parser.sections.find("LMS7002 registers ch.B");
            for (ini_t::keysit_t pairs = section->second->begin(); pairs != section->second->end(); pairs++)
            {
                sscanf(pairs->first.c_str(), "%hx", &addr);
                sscanf(pairs->second.c_str(), "%hx", &value);
                addrToWrite.push_back(addr);
                dataToWrite.push_back(value);
            }
            Modify_SPI_Reg_bits(LMS7param(MAC), 2); //select B channel
            status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size());
            if (status != LIBLMS7_SUCCESS && status != LIBLMS7_NOT_CONNECTED)
                return status;

            //parse FCW or PHO
            if (parser.select("NCO Rx ch.B") == true)
            {
                char varname[64];
                int mode = Get_SPI_Reg_bits(LMS7param(MODE_RX));
                if (mode == 0) //FCW
                {
                    for (int i = 0; i < 16; ++i)
                    {
                        sprintf(varname, "FCW%02i", i);
                        SetNCOFrequency(LMS7002M::Rx, i, parser.get(varname, 0.0));
                    }
                }
                else
                {
                    for (int i = 0; i < 16; ++i)
                    {
                        sprintf(varname, "PHO%02i", i);
                        SetNCOPhaseOffset(LMS7002M::Rx, i, parser.get(varname, 0.0));
                    }
                }
            }
            if (parser.select("NCO Tx ch.A") == true)
            {
                char varname[64];
                int mode = Get_SPI_Reg_bits(LMS7param(MODE_TX));
                if (mode == 0) //FCW
                {
                    for (int i = 0; i < 16; ++i)
                    {
                        sprintf(varname, "FCW%02i", i);
                        SetNCOFrequency(LMS7002M::Tx, i, parser.get(varname, 0.0));
                    }
                }
                else
                {
                    for (int i = 0; i < 16; ++i)
                    {
                        sprintf(varname, "PHO%02i", i);
                        SetNCOPhaseOffset(LMS7002M::Tx, i, parser.get(varname, 0.0));
                    }
                }
            }
        }
        Modify_SPI_Reg_bits(LMS7param(MAC), ch);
        return LIBLMS7_SUCCESS;
    }
    else
        return LIBLMS7_FILE_INVALID_FORMAT;
    return LIBLMS7_FAILURE;
}

/** @brief Reads configuration file and uploads registers to chip
    @param filename Configuration source file
    @return 0-success, other-failure
*/
liblms7_status LMS7002M::LoadConfig(const char* filename)
{
	ifstream f(filename);
    if (f.good() == false) //file not found
    {
        f.close();
        return LIBLMS7_FILE_NOT_FOUND;
    }
    f.close();
    uint16_t addr = 0;
    uint16_t value = 0;
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC)); //remember used channel

    liblms7_status status;
    typedef INI<string, string, string> ini_t;
    ini_t parser(filename, true);
    if (parser.select("file_info") == false)
    {
        //try loading as legacy format
        status = LoadConfigLegacyFile(filename);
        Modify_SPI_Reg_bits(MAC, 1);
        return status;
    }
    string type = "";
    type = parser.get("type", "undefined");
    stringstream ss;
    if (type.find("lms7002m_minimal_config") == string::npos)
    {
        ss << "File " << filename << " not recognized" << endl;
        return LIBLMS7_FILE_INVALID_FORMAT;
    }

    int fileVersion = 0;
    fileVersion = parser.get("version", 0);

    vector<uint16_t> addrToWrite;
    vector<uint16_t> dataToWrite;

    if (fileVersion == 1)
    {
        if(parser.select("lms7002_registers_a") == true)
        {
            ini_t::sectionsit_t section = parser.sections.find("lms7002_registers_a");

            uint16_t x0020_value = 0;
            Modify_SPI_Reg_bits(LMS7param(MAC), 1); //select A channel
            for (ini_t::keysit_t pairs = section->second->begin(); pairs != section->second->end(); pairs++)
            {
                sscanf(pairs->first.c_str(), "%hx", &addr);
                sscanf(pairs->second.c_str(), "%hx", &value);
                if (addr == LMS7param(MAC).address) //skip register containing channel selection
                {
                    x0020_value = value;
                    continue;
                }
                addrToWrite.push_back(addr);
                dataToWrite.push_back(value);
            }
            status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size());
            if (status != LIBLMS7_SUCCESS && status != LIBLMS7_NOT_CONNECTED)
                return status;
            status = SPI_write(0x0020, x0020_value);
            if (status != LIBLMS7_SUCCESS && status != LIBLMS7_NOT_CONNECTED)
                return status;
            Modify_SPI_Reg_bits(LMS7param(MAC), 2);
            if (status != LIBLMS7_SUCCESS && status != LIBLMS7_NOT_CONNECTED)
                return status;
        }

        if (parser.select("lms7002_registers_b") == true)
        {
            addrToWrite.clear();
            dataToWrite.clear();
            ini_t::sectionsit_t section = parser.sections.find("lms7002_registers_b");
            for (ini_t::keysit_t pairs = section->second->begin(); pairs != section->second->end(); pairs++)
            {
                sscanf(pairs->first.c_str(), "%hx", &addr);
                sscanf(pairs->second.c_str(), "%hx", &value);
                addrToWrite.push_back(addr);
                dataToWrite.push_back(value);
            }
            Modify_SPI_Reg_bits(LMS7param(MAC), 2); //select B channel
            status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size());
            if (status != LIBLMS7_SUCCESS && status != LIBLMS7_NOT_CONNECTED)
                return status;
        }
        Modify_SPI_Reg_bits(LMS7param(MAC), ch);

        parser.select("reference_clocks");
        mRefClkSXR_MHz = parser.get("sxr_ref_clk_mhz", 30.72);
        mRefClkSXT_MHz = parser.get("sxt_ref_clk_mhz", 30.72);
    }

    Modify_SPI_Reg_bits(MAC, 1);
    if (controlPort == NULL)
        return LIBLMS7_NO_CONNECTION_MANAGER;
    if (controlPort->IsOpen() == false)
        return LIBLMS7_NOT_CONNECTED;
    return LIBLMS7_SUCCESS;
}

/** @brief Reads all registers from chip and saves to file
    @param filename destination filename
    @return 0-success, other failure
*/
liblms7_status LMS7002M::SaveConfig(const char* filename)
{
    liblms7_status status;
    typedef INI<> ini_t;
    ini_t parser(filename, true);
    parser.create("file_info");
    parser.set("type", "lms7002m_minimal_config");
    parser.set("version", 1);

    char addr[80];
    char value[80];

    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));

    vector<uint16_t> addrToRead;
    for (uint8_t i = 0; i < MEMORY_SECTIONS_COUNT; ++i)
        for (uint16_t addr = MemorySectionAddresses[i][0]; addr <= MemorySectionAddresses[i][1]; ++addr)
            addrToRead.push_back(addr);
    vector<uint16_t> dataReceived;
    dataReceived.resize(addrToRead.size(), 0);

    parser.create("lms7002_registers_a");
    Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    for (uint16_t i = 0; i < addrToRead.size(); ++i)
    {
        dataReceived[i] = Get_SPI_Reg_bits(addrToRead[i], 15, 0, false);
        sprintf(addr, "0x%04X", addrToRead[i]);
        sprintf(value, "0x%04X", dataReceived[i]);
        parser.set(addr, value);
    }

    parser.create("lms7002_registers_b");
    addrToRead.clear(); //add only B channel addresses
    for (uint8_t i = 0; i < MEMORY_SECTIONS_COUNT; ++i)
        for (uint16_t addr = MemorySectionAddresses[i][0]; addr <= MemorySectionAddresses[i][1]; ++addr)
            if (addr >= 0x0100)
                addrToRead.push_back(addr);

    Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    for (uint16_t i = 0; i < addrToRead.size(); ++i)
    {
        dataReceived[i] = Get_SPI_Reg_bits(addrToRead[i], 15, 0, false);
        sprintf(addr, "0x%04X", addrToRead[i]);
        sprintf(value, "0x%04X", dataReceived[i]);
        parser.set(addr, value);
    }

    Modify_SPI_Reg_bits(LMS7param(MAC), ch); //retore previously used channel

    parser.create("reference_clocks");
    parser.set("sxt_ref_clk_mhz", mRefClkSXT_MHz);
    parser.set("sxr_ref_clk_mhz", mRefClkSXR_MHz);
    parser.save(filename);
    return LIBLMS7_SUCCESS;
}

/**	@brief Returns reference clock in MHz used for SXT or SXR
	@param Tx transmitter or receiver selection
*/
float_type LMS7002M::GetReferenceClk_SX(bool tx)
{
	return tx ? mRefClkSXT_MHz : mRefClkSXR_MHz;
}

/**	@return Current CLKGEN frequency in MHz
    Returned frequency depends on reference clock used for Receiver
*/
float_type LMS7002M::GetFrequencyCGEN_MHz()
{
    float_type dMul = (mRefClkSXR_MHz/2.0)/(Get_SPI_Reg_bits(LMS7param(DIV_OUTCH_CGEN))+1); //DIV_OUTCH_CGEN
    uint16_t gINT = Get_SPI_Reg_bits(0x0088, 13, 0); //read whole register to reduce SPI transfers
    uint32_t gFRAC = ((gINT & 0xF) * 65536) | Get_SPI_Reg_bits(0x0087, 15, 0);
    return dMul * (((gINT>>4) + 1 + gFRAC/1048576.0));
}

/** @brief Returns TSP reference frequency
    @param tx TxTSP or RxTSP selection
    @return TSP reference frequency in MHz
*/
float_type LMS7002M::GetReferenceClk_TSP_MHz(bool tx)
{
    float_type cgenFreq = GetFrequencyCGEN_MHz();
	float_type clklfreq = cgenFreq/pow(2.0, Get_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN)));
    if(Get_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN)) == 0)
        return tx ? clklfreq : cgenFreq/4.0;
    else
        return tx ? cgenFreq : clklfreq/4.0;
}

/** @brief Sets CLKGEN frequency, calculations use receiver'r reference clock
    @param freq_MHz desired frequency in MHz
    @return 0-succes, other-cannot deliver desired frequency
*/
liblms7_status LMS7002M::SetFrequencyCGEN(const float_type freq_MHz)
{
    float_type dFvco;
    float_type dFrac;
    int16_t iHdiv;

    //VCO frequency selection according to F_CLKH
    iHdiv = (int16_t)((gCGEN_VCO_frequencies[1]/ 2) / freq_MHz) - 1;
	dFvco = 2*(iHdiv+1) * freq_MHz;

    //Integer division
    uint16_t gINT = (uint16_t)(dFvco/mRefClkSXR_MHz - 1);

    //Fractional division
    dFrac = dFvco/mRefClkSXR_MHz - (uint32_t)(dFvco/mRefClkSXR_MHz);
    uint32_t gFRAC = (uint32_t)(dFrac * 1048576);

    Modify_SPI_Reg_bits(LMS7param(INT_SDM_CGEN), gINT); //INT_SDM_CGEN
    Modify_SPI_Reg_bits(0x0087, 15, 0, gFRAC&0xFFFF); //INT_SDM_CGEN[15:0]
    Modify_SPI_Reg_bits(0x0088, 3, 0, gFRAC>>16); //INT_SDM_CGEN[19:16]
    Modify_SPI_Reg_bits(LMS7param(DIV_OUTCH_CGEN), iHdiv); //DIV_OUTCH_CGEN

    return TuneVCO(VCO_CGEN);
}

/** @brief Performs VCO tuning operations for CLKGEN, SXR, SXT modules
    @param module module selection for tuning 0-cgen, 1-SXR, 2-SXT
    @return 0-success, other-failure
*/
liblms7_status LMS7002M::TuneVCO(VCO_Module module) // 0-cgen, 1-SXR, 2-SXT
{
    if (controlPort == NULL)
        return LIBLMS7_NO_CONNECTION_MANAGER;
    if (controlPort->IsOpen() == false)
        return LIBLMS7_NOT_CONNECTED;
	int8_t i;
	uint8_t cmphl; //comparators
	int16_t csw_lowest = -1;
	uint16_t addrVCOpd; // VCO power down address
	uint16_t addrCSW_VCO;
	uint16_t addrCMP; //comparator address
	uint8_t lsb; //SWC lsb index
	uint8_t msb; //SWC msb index

	uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC)); //remember used channel

	if(module != VCO_CGEN) //set addresses to SX module
	{
        Modify_SPI_Reg_bits(LMS7param(MAC), module);
        addrVCOpd = LMS7param(PD_VCO).address;
        addrCSW_VCO = LMS7param(CSW_VCO).address;
        lsb = LMS7param(CSW_VCO).lsb;
        msb = LMS7param(CSW_VCO).msb;
        addrCMP = LMS7param(VCO_CMPHO).address;
	}
	else //set addresses to CGEN module
    {
        addrVCOpd = LMS7param(PD_VCO_CGEN).address;
        addrCSW_VCO = LMS7param(CSW_VCO_CGEN).address;
        lsb = LMS7param(CSW_VCO_CGEN).lsb;
        msb = LMS7param(CSW_VCO_CGEN).msb;
        addrCMP = LMS7param(VCO_CMPHO_CGEN).address;
    }
	// Initialization
	Modify_SPI_Reg_bits (addrVCOpd, 2, 1, 0); //activate VCO and comparator
    if (Get_SPI_Reg_bits(addrVCOpd, 2, 1) != 0)
        return LIBLMS7_VCO_IS_POWERED_DOWN;
	if(module == VCO_CGEN)
        Modify_SPI_Reg_bits(LMS7param(SPDUP_VCO_CGEN), 1); //SHORT_NOISEFIL=1 SPDUP_VCO_ Short the noise filter resistor to speed up the settling time
	else
        Modify_SPI_Reg_bits(LMS7param(SPDUP_VCO), 1); //SHORT_NOISEFIL=1 SPDUP_VCO_ Short the noise filter resistor to speed up the settling time
	Modify_SPI_Reg_bits (addrCSW_VCO , msb, lsb , 0); //Set SWC_VCO<7:0>=<00000000>

	i=7;
	while(i>=0)
	{
        Modify_SPI_Reg_bits (addrCSW_VCO, lsb + i, lsb + i, 1); // CSW_VCO<i>=1
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        cmphl = (uint8_t)Get_SPI_Reg_bits(addrCMP, 13, 12);
        if ( (cmphl&0x01) == 1) // reduce CSW
            Modify_SPI_Reg_bits (addrCSW_VCO, lsb + i, lsb + i, 0); // CSW_VCO<i>=0
        if( cmphl == 2 && csw_lowest < 0)
            csw_lowest = Get_SPI_Reg_bits(addrCSW_VCO, msb, lsb);
		--i;
	}
	if(csw_lowest >= 0)
    {
        uint16_t csw_highest = Get_SPI_Reg_bits(addrCSW_VCO, msb, lsb);
        if(csw_lowest == csw_highest)
        {
            while(csw_lowest>=0)
            {
                Modify_SPI_Reg_bits(addrCSW_VCO, msb, lsb, csw_lowest);
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                if(Get_SPI_Reg_bits(addrCMP, 13, 12) == 0)
                {
                    ++csw_lowest;
                    break;
                }
                else
                    --csw_lowest;
            }
        }
        Modify_SPI_Reg_bits(addrCSW_VCO, msb, lsb, csw_lowest+(csw_highest-csw_lowest)/2);
    }
    if (module == VCO_CGEN)
        Modify_SPI_Reg_bits(LMS7param(SPDUP_VCO_CGEN), 0); //SHORT_NOISEFIL=1 SPDUP_VCO_ Short the noise filter resistor to speed up the settling time
    else
        Modify_SPI_Reg_bits(LMS7param(SPDUP_VCO), 0); //SHORT_NOISEFIL=1 SPDUP_VCO_ Short the noise filter resistor to speed up the settling time
	cmphl = (uint8_t)Get_SPI_Reg_bits(addrCMP, 13, 12);
    Modify_SPI_Reg_bits(LMS7param(MAC), ch); //restore previously used channel
	if(cmphl == 2)
        return LIBLMS7_SUCCESS;
    else
        return LIBLMS7_FAILURE;
}

/** @brief Returns given parameter value from chip register
    @param param LMS7002M control parameter
    @param fromChip read directly from chip
    @return parameter value
*/
uint16_t LMS7002M::Get_SPI_Reg_bits(const LMS7Parameter &param, bool fromChip)
{
	return Get_SPI_Reg_bits(param.address, param.msb, param.lsb, fromChip);
}

/** @brief Returns given parameter value from chip register
    @param address register address
    @param msb most significant bit index
    @param lsb least significant bit index
    @param fromChip read directly from chip
    @return register bits from selected interval, shifted to right by lsb bits
*/
uint16_t LMS7002M::Get_SPI_Reg_bits(uint16_t address, uint8_t msb, uint8_t lsb, bool fromChip)
{
    return (SPI_read(address, fromChip) & (~(~0<<(msb+1)))) >> lsb; //shift bits to LSB
}

/** @brief Change given parameter value
    @param param LMS7002M control parameter
    @param fromChip read initial value directly from chip
    @param value new parameter value
*/
liblms7_status LMS7002M::Modify_SPI_Reg_bits(const LMS7Parameter &param, const uint16_t value, bool fromChip)
{
    return Modify_SPI_Reg_bits(param.address, param.msb, param.lsb, value, fromChip);
}

/** @brief Change given parameter value
    @param address register address
    @param value new bits value, the value is shifted left by lsb bits
    @param fromChip read initial value directly from chip
*/
liblms7_status LMS7002M::Modify_SPI_Reg_bits(const uint16_t address, const uint8_t msb, const uint8_t lsb, const uint16_t value, bool fromChip)
{
    uint16_t spiDataReg = SPI_read(address, fromChip); //read current SPI reg data
    uint16_t spiMask = (~(~0 << (msb - lsb + 1))) << (lsb); // creates bit mask
    spiDataReg = (spiDataReg & (~spiMask)) | ((value << lsb) & spiMask);//clear bits
    return SPI_write(address, spiDataReg); //write modified data back to SPI reg
}

/** @brief Modifies given registers with values applied using masks
    @param addr array of register addresses
    @param masks array of applied masks
    @param values array of values to be written
    @param start starting index of given arrays
    @param stop end index of given arrays
*/
liblms7_status LMS7002M::Modify_SPI_Reg_mask(const uint16_t *addr, const uint16_t *masks, const uint16_t *values, uint8_t start, uint8_t stop)
{
    liblms7_status status;
    uint16_t reg_data;
    vector<uint16_t> addresses;
    vector<uint16_t> data;
    while (start <= stop)
    {
        reg_data = SPI_read(addr[start], true, &status); //read current SPI reg data
        reg_data &= ~masks[start];//clear bits
        reg_data |= (values[start] & masks[start]);
        addresses.push_back(addr[start]);
        data.push_back(reg_data);
        ++start;
    }
    if (status != LIBLMS7_SUCCESS)
        return status;
    SPI_write_batch(&addresses[0], &data[0], addresses.size());
    return status;
}

/** @brief Sets SX frequency
    @param Tx Rx/Tx module selection
    @param freq_MHz desired frequency in MHz
	@param refClk_MHz reference clock in MHz
    @return 0-success, other-cannot deliver requested frequency
*/
liblms7_status LMS7002M::SetFrequencySX(bool tx, float_type freq_MHz, float_type refClk_MHz)
{
	if (controlPort == NULL)
        return LIBLMS7_NO_CONNECTION_MANAGER;
    if (controlPort->IsOpen() == false)
        return LIBLMS7_NOT_CONNECTED;
    const uint8_t sxVCO_N = 2; //number of entries in VCO frequencies
    const float_type m_dThrF = 5500; //threshold to enable additional divider
    uint8_t ch; //remember used channel
    float_type VCOfreq;
    int8_t div_loch;
    int8_t sel_vco;
    bool canDeliverFrequency = false;
    uint16_t integerPart;
    uint32_t fractionalPart;
    int8_t i;

    //find required VCO frequency
    for (div_loch = 6; div_loch >= 0; --div_loch)
    {
        VCOfreq = (1 << (div_loch + 1)) * freq_MHz;
        if ((VCOfreq >= gVCO_frequency_table[0][0]) && (VCOfreq <= gVCO_frequency_table[2][sxVCO_N - 1]))
        {
            canDeliverFrequency = true;
            break;
        }
    }
    if (canDeliverFrequency == false)
        return LIBLMS7_CANNOT_DELIVER_FREQUENCY;

    integerPart = (uint16_t)(VCOfreq / (refClk_MHz * (1 + (VCOfreq > m_dThrF))) - 4);
    fractionalPart = (uint32_t)((VCOfreq / (refClk_MHz * (1 + (VCOfreq > m_dThrF))) - (uint32_t)(VCOfreq / (refClk_MHz * (1 + (VCOfreq > m_dThrF))))) * 1048576);

    ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));
    Modify_SPI_Reg_bits(LMS7param(MAC), tx + 1);
    Modify_SPI_Reg_bits(LMS7param(INT_SDM), integerPart); //INT_SDM
    Modify_SPI_Reg_bits(0x011D, 15, 0, fractionalPart & 0xFFFF); //FRAC_SDM[15:0]
    Modify_SPI_Reg_bits(0x011E, 3, 0, (fractionalPart >> 16)); //FRAC_SDM[19:16]
    Modify_SPI_Reg_bits(LMS7param(DIV_LOCH), div_loch); //DIV_LOCH
    Modify_SPI_Reg_bits(LMS7param(EN_DIV2_DIVPROG), (VCOfreq > m_dThrF)); //EN_DIV2_DIVPROG

    //find which VCO supports required frequency
    Modify_SPI_Reg_bits(LMS7param(PD_VCO), 0); //
    Modify_SPI_Reg_bits(LMS7param(PD_VCO_COMP), 0); //
    int cswBackup = Get_SPI_Reg_bits(LMS7param(CSW_VCO)); //remember to restore previous tune value
    canDeliverFrequency = false;
    int tuneScore[] = { -128, -128, -128 }; //best is closest to 0
    for (sel_vco = 0; sel_vco < 3; ++sel_vco)
    {
        Modify_SPI_Reg_bits(LMS7param(SEL_VCO), sel_vco);
        liblms7_status status = TuneVCO(tx ? VCO_SXT : VCO_SXR);
        int csw = Get_SPI_Reg_bits(LMS7param(CSW_VCO), true);
        tuneScore[sel_vco] = -128 + csw;
        if (status == LIBLMS7_SUCCESS)
            canDeliverFrequency = true;
    }
    if (abs(tuneScore[0]) < abs(tuneScore[1]))
    {
        if (abs(tuneScore[0]) < abs(tuneScore[2]))
            sel_vco = 0;
        else
            sel_vco = 2;
    }
    else
    {
        if (abs(tuneScore[1]) < abs(tuneScore[2]))
            sel_vco = 1;
        else
            sel_vco = 2;
    }
    Modify_SPI_Reg_bits(LMS7param(SEL_VCO), sel_vco);
    Modify_SPI_Reg_bits(LMS7param(CSW_VCO), cswBackup);
    Modify_SPI_Reg_bits(LMS7param(MAC), ch); //restore used channel
    if (tx)
        mRefClkSXT_MHz = refClk_MHz;
    else
        mRefClkSXR_MHz = refClk_MHz;
    if (canDeliverFrequency == false)
        return LIBLMS7_CANNOT_DELIVER_FREQUENCY;
    return TuneVCO( tx ? VCO_SXT : VCO_SXR); //Rx-1, Tx-2
}

/**	@brief Returns currently set SXR/SXT frequency
	@return SX frequency MHz
*/
float_type LMS7002M::GetFrequencySX_MHz(bool Tx, float_type refClk_MHz)
{
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC)); //remember previously used channel
	float_type dMul;
	if(Tx)
        Modify_SPI_Reg_bits(LMS7param(MAC), 2); // Rx mac = 1, Tx max = 2
	else
        Modify_SPI_Reg_bits(LMS7param(MAC), 1); // Rx mac = 1, Tx max = 2
	uint16_t gINT = Get_SPI_Reg_bits(0x011E, 13, 0);	// read whole register to reduce SPI transfers
    uint32_t gFRAC = ((gINT&0xF) * 65536) | Get_SPI_Reg_bits(0x011D, 15, 0);

    dMul = (float_type)refClk_MHz / (1 << (Get_SPI_Reg_bits(LMS7param(DIV_LOCH)) + 1));
    //Calculate real frequency according to the calculated parameters
    dMul = dMul * ((gINT >> 4) + 4 + (float_type)gFRAC / 1048576.0) * (Get_SPI_Reg_bits(LMS7param(EN_DIV2_DIVPROG)) + 1);
    Modify_SPI_Reg_bits(LMS7param(MAC), ch); //restore used channel
	return dMul;
}

/** @brief Loads given DC_REG values into registers
    @param tx TxTSP or RxTSP selection
    @param I DC_REG I value
    @param Q DC_REG Q value
*/
liblms7_status LMS7002M::LoadDC_REG_IQ(bool tx, int16_t I, int16_t Q)
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
        Modify_SPI_Reg_bits(LMS7param(DC_BYP_TXTSP), 0); //DC_BYP
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
        Modify_SPI_Reg_bits(LMS7param(DC_BYP_RXTSP), 0); //DC_BYP
    }
    return LIBLMS7_SUCCESS;
}

/** @brief Sets chosen NCO's frequency
    @param tx transmitter or receiver selection
    @param index NCO index from 0 to 15
    @param freq_MHz desired NCO frequency
    @return 0-success, other-failure
*/
liblms7_status LMS7002M::SetNCOFrequency(bool tx, uint8_t index, float_type freq_MHz)
{
    if(index > 15)
        return LIBLMS7_INDEX_OUT_OF_RANGE;
    float_type refClk_MHz = GetReferenceClk_TSP_MHz(tx);
    uint16_t addr = tx ? 0x0240 : 0x0440;
	uint32_t fcw = (uint32_t)((freq_MHz/refClk_MHz)*4294967296);
    SPI_write(addr+2+index*2, (fcw >> 16)); //NCO frequency control word register MSB part.
    SPI_write(addr+3+index*2, fcw); //NCO frequency control word register LSB part.
    return LIBLMS7_SUCCESS;
}

/** @brief Returns chosen NCO's frequency in MHz
    @param tx transmitter or receiver selection
    @param index NCO index from 0 to 15
    @param fromChip read frequency directly from chip or local registers
    @return NCO frequency in MHz
*/
float_type LMS7002M::GetNCOFrequency_MHz(bool tx, uint8_t index, const float_type refClk_MHz, bool fromChip)
{
    if(index > 15)
        return LIBLMS7_INDEX_OUT_OF_RANGE;
    uint16_t addr = tx ? 0x0240 : 0x0440;
    uint32_t fcw = 0;
    fcw |= SPI_read(addr + 2 + index * 2, fromChip) << 16; //NCO frequency control word register MSB part.
    fcw |= SPI_read(addr + 3 + index * 2, fromChip); //NCO frequency control word register LSB part.
    return refClk_MHz*(fcw/4294967296.0);
}

/** @brief Sets chosen NCO phase offset angle when memory table MODE is 0
@param tx transmitter or receiver selection
@param angle_deg phase offset angle in degrees
@return 0-success, other-failure
*/
liblms7_status LMS7002M::SetNCOPhaseOffsetForMode0(bool tx, float_type angle_deg)
{
    uint16_t addr = tx ? 0x0241 : 0x0441;
    uint16_t pho = (uint16_t)(65536 * (angle_deg / 360 ));
    SPI_write(addr, pho);
    return LIBLMS7_SUCCESS;
}

/** @brief Sets chosen NCO's phase offset angle
    @param tx transmitter or receiver selection
    @param index PHO index from 0 to 15
    @param angle_deg phase offset angle in degrees
    @return 0-success, other-failure
*/
liblms7_status LMS7002M::SetNCOPhaseOffset(bool tx, uint8_t index, float_type angle_deg)
{
    if(index > 15)
        return LIBLMS7_INDEX_OUT_OF_RANGE;
    uint16_t addr = tx ? 0x0244 : 0x0444;
	uint16_t pho = (uint16_t)(65536*(angle_deg / 360));
    SPI_write(addr+index, pho);
    return LIBLMS7_SUCCESS;
}

/** @brief Returns chosen NCO's phase offset angle in radians
    @param tx transmitter or receiver selection
    @param index PHO index from 0 to 15
    @return phase offset angle in degrees
*/
float_type LMS7002M::GetNCOPhaseOffset_Deg(bool tx, uint8_t index)
{
    uint16_t addr = tx ? 0x0244 : 0x0444;
    uint16_t pho = SPI_read(addr+index);
    float_type angle = 360*pho/65536.0;
    return angle;
}

/** @brief Uploads given FIR coefficients to chip
    @param tx Transmitter or receiver selection
    @param GFIR_index GIR index from 0 to 2
    @param coef array of coefficients
    @param coefCount number of coefficients
    @return 0-success, other-failure

    This function does not change GFIR*_L or GFIR*_N parameters, they have to be set manually
*/
liblms7_status LMS7002M::SetGFIRCoefficients(bool tx, uint8_t GFIR_index, const int16_t *coef, uint8_t coefCount)
{
    uint8_t index;
    uint8_t coefLimit;
    uint16_t startAddr;
    if (GFIR_index == 0)
        startAddr = 0x0280;
    else if (GFIR_index == 1)
        startAddr = 0x02C0;
    else
        startAddr = 0x0300;

    if (tx == false)
        startAddr += 0x0200;
    if (GFIR_index < 2)
        coefLimit = 40;
    else
        coefLimit = 120;
    if (coefCount > coefLimit)
        return LIBLMS7_TOO_MANY_VALUES;
    vector<uint16_t> addresses;
    for (index = 0; index < coefCount; ++index)
        addresses.push_back(startAddr + index + 24 * (index / 40));
    SPI_write_batch(&addresses[0], (uint16_t*)coef, coefCount);
    return LIBLMS7_SUCCESS;
}

/** @brief Returns currently loaded FIR coefficients
    @param tx Transmitter or receiver selection
    @param GFIR_index GIR index from 0 to 2
    @param coef array of returned coefficients
    @param coefCount number of coefficients to read
    @return 0-success, other-failure
*/
liblms7_status LMS7002M::GetGFIRCoefficients(bool tx, uint8_t GFIR_index, int16_t *coef, uint8_t coefCount)
{
    liblms7_status status = LIBLMS7_FAILURE;
    uint8_t index;
    uint8_t coefLimit;
    uint16_t startAddr;
    if(GFIR_index == 0)
        startAddr = 0x0280;
    else if (GFIR_index == 1)
        startAddr = 0x02C0;
    else
        startAddr = 0x0300;

    if (tx == false)
        startAddr += 0x0200;
    if (GFIR_index < 2)
        coefLimit = 40;
    else
        coefLimit = 120;
    if (coefCount > coefLimit)
        return LIBLMS7_TOO_MANY_VALUES;

    std::vector<uint16_t> addresses;
    for (index = 0; index < coefCount; ++index)
        addresses.push_back(startAddr + index + 24 * (index / 40));
    uint16_t spiData[120];
    memset(spiData, 0, 120 * sizeof(uint16_t));
    if (controlPort->IsOpen())
    {
        status = SPI_read_batch(&addresses[0], spiData, coefCount);
        for (index = 0; index < coefCount; ++index)
            coef[index] = spiData[index];
    }
    else
    {
        const int channel = Get_SPI_Reg_bits(LMS7param(MAC), false) > 1 ? 1 : 0;
        for (index = 0; index < coefCount; ++index)
            coef[index] = mRegistersMap->GetValue(channel, addresses[index]);
        status = LIBLMS7_SUCCESS;
    }

    return status;
}

/** @brief Write given data value to whole register
    @param address SPI address
    @param data new register value
    @return 0-succes, other-failure
*/
liblms7_status LMS7002M::SPI_write(uint16_t address, uint16_t data)
{
    if (controlPort == NULL)
        return LIBLMS7_NO_CONNECTION_MANAGER;

    if ((mRegistersMap->GetValue(0, LMS7param(MAC).address) & 0x0003) > 1 && address >= 0x0100)
        mRegistersMap->SetValue(1, address, data);
    else
        mRegistersMap->SetValue(0, address, data);

    if (controlPort->IsOpen() == false)
        return LIBLMS7_NOT_CONNECTED;

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_WR;
    pkt.outBuffer.push_back(address >> 8);
    pkt.outBuffer.push_back(address & 0xFF);
    pkt.outBuffer.push_back(data >> 8);
    pkt.outBuffer.push_back(data & 0xFF);
    controlPort->TransferPacket(pkt);
    if (pkt.status == STATUS_COMPLETED_CMD)
        return LIBLMS7_SUCCESS;
    else
        return LIBLMS7_FAILURE;
}

/** @brief Reads whole register value from given address
    @param address SPI address
    @param status operation status(optional)
    @param fromChip read value directly from chip
    @return register value
*/
uint16_t LMS7002M::SPI_read(uint16_t address, bool fromChip, liblms7_status *status)
{
    if (controlPort == NULL)
    {
        if (status)
            *status = LIBLMS7_NO_CONNECTION_MANAGER;
    }
    if (controlPort->IsOpen() == false || fromChip == false)
    {
        if ((mRegistersMap->GetValue(0, LMS7param(MAC).address) & 0x0003) > 1 && address >= 0x0100)
            return mRegistersMap->GetValue(1, address);
        else
            return mRegistersMap->GetValue(0, address);
    }

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_RD;
    pkt.outBuffer.push_back(address >> 8);
    pkt.outBuffer.push_back(address & 0xFF);
    if (controlPort->TransferPacket(pkt) == LMScomms::TRANSFER_SUCCESS)
    {
        if (status)
            *status = (pkt.status == STATUS_COMPLETED_CMD ? LIBLMS7_SUCCESS : LIBLMS7_FAILURE);
        return (pkt.inBuffer[2] << 8) | pkt.inBuffer[3];
    }
    else
        return 0;
}

/** @brief Batches multiple register writes into least ammount of transactions
    @param spiAddr spi register addresses to be written
    @param spiData registers data to be written
    @param cnt number of registers to write
    @return 0-success, other-failure
*/
liblms7_status LMS7002M::SPI_write_batch(const uint16_t* spiAddr, const uint16_t* spiData, uint16_t cnt)
{
    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_WR;
    uint32_t index = 0;
    for (uint32_t i = 0; i < cnt; ++i)
    {   
        pkt.outBuffer.push_back(spiAddr[i] >> 8);
        pkt.outBuffer.push_back(spiAddr[i] & 0xFF);
        pkt.outBuffer.push_back(spiData[i] >> 8);
        pkt.outBuffer.push_back(spiData[i] & 0xFF);

        if ((mRegistersMap->GetValue(0, LMS7param(MAC).address) & 0x0003) > 1)
            mRegistersMap->SetValue(1, spiAddr[i], spiData[i]);
        else
            mRegistersMap->SetValue(0, spiAddr[i], spiData[i]);

    }
    if (controlPort == NULL)
        return LIBLMS7_NO_CONNECTION_MANAGER;
    if (controlPort->IsOpen() == false)
        return LIBLMS7_NOT_CONNECTED;

    controlPort->TransferPacket(pkt);
    if (pkt.status == STATUS_COMPLETED_CMD)
        return LIBLMS7_SUCCESS;
    else
        return LIBLMS7_FAILURE;
}

/** @brief Batches multiple register reads into least amount of transactions
    @param spiAddr SPI addresses to read
    @param spiData array for read data
    @param cnt number of registers to read
    @return 0-success, other-failure
*/
liblms7_status LMS7002M::SPI_read_batch(const uint16_t* spiAddr, uint16_t* spiData, uint16_t cnt)
{
    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_RD;
    uint32_t index = 0;
    for (uint32_t i = 0; i < cnt; ++i)
    {
        pkt.outBuffer.push_back(spiAddr[i] >> 8);
        pkt.outBuffer.push_back(spiAddr[i] & 0xFF);
    }
    if (controlPort == NULL)
        return LIBLMS7_NO_CONNECTION_MANAGER;
    if (controlPort->IsOpen() == false)
        return LIBLMS7_NOT_CONNECTED;

    LMScomms::TransferStatus status = controlPort->TransferPacket(pkt);
    if (status != LMScomms::TRANSFER_SUCCESS)
        return LIBLMS7_FAILURE;

    for (uint32_t i = 0; i < cnt; ++i)
    {
        spiData[i] = (pkt.inBuffer[2*sizeof(uint16_t)*i + 2] << 8) | pkt.inBuffer[2*sizeof(uint16_t)*i + 3];
        if ((mRegistersMap->GetValue(0, LMS7param(MAC).address) & 0x0003) > 1)
            mRegistersMap->SetValue(1, spiAddr[i], spiData[i]);
        else
            mRegistersMap->SetValue(0, spiAddr[i], spiData[i]);
    }
    return pkt.status == STATUS_COMPLETED_CMD ? LIBLMS7_SUCCESS : LIBLMS7_FAILURE;
    /*
    for(int i=0; i<cnt; ++i)
    {
        spiData[i] = Get_SPI_Reg_bits(spiAddr[i], 15, 0);
        if ((mRegistersMap->GetValue(0, LMS7param(MAC).address) & 0x0003) > 1)
            mRegistersMap->SetValue(1, spiAddr[i], spiData[i]);
        else
            mRegistersMap->SetValue(0, spiAddr[i], spiData[i]);
    }
    return LIBLMS7_SUCCESS;*/
}

/** @brief Performs registers test by writing known data and confirming readback data
    @return 0-registers test passed, other-failure
*/
liblms7_status LMS7002M::RegistersTest()
{
    char chex[16];
    if (controlPort == NULL)
        return LIBLMS7_NO_CONNECTION_MANAGER;
    if (controlPort->IsOpen() == false)
        return LIBLMS7_NOT_CONNECTED;

    liblms7_status status;
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));

    //backup both channel data for restoration after test
    vector<uint16_t> ch1Addresses;
    for (uint8_t i = 0; i < MEMORY_SECTIONS_COUNT; ++i)
        for (uint16_t addr = MemorySectionAddresses[i][0]; addr <= MemorySectionAddresses[i][1]; ++addr)
            ch1Addresses.push_back(addr);
    vector<uint16_t> ch1Data;
    ch1Data.resize(ch1Addresses.size(), 0);

    //backup A channel
    Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    status = SPI_read_batch(&ch1Addresses[0], &ch1Data[0], ch1Addresses.size());
    if (status != LIBLMS7_SUCCESS)
        return status;

    vector<uint16_t> ch2Addresses;
    for (uint8_t i = 0; i < MEMORY_SECTIONS_COUNT; ++i)
        for (uint16_t addr = MemorySectionAddresses[i][0]; addr <= MemorySectionAddresses[i][1]; ++addr)
            if (addr >= 0x0100)
                ch2Addresses.push_back(addr);
    vector<uint16_t> ch2Data;
    ch2Data.resize(ch2Addresses.size(), 0);

    Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    status = SPI_read_batch(&ch2Addresses[0], &ch2Data[0], ch2Addresses.size());
    if (status != LIBLMS7_SUCCESS)
        return status;

    //test registers
    ResetChip();
    Modify_SPI_Reg_bits(LMS7param(MIMO_SISO), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0);
    Modify_SPI_Reg_bits(LMS7param(MAC), 1);

    stringstream ss;

    //check single channel memory sections
    vector<MemorySection> modulesToCheck = { AFE, BIAS, XBUF, CGEN, LDO, BIST, CDS, TRF, TBB, RFE, RBB, SX,
        TxTSP, TxNCO, TxGFIR1, TxGFIR2, TxGFIR3a, TxGFIR3b, TxGFIR3c,
        RxTSP, RxNCO, RxGFIR1, RxGFIR2, RxGFIR3a, RxGFIR3b, RxGFIR3c, LimeLight };
    const char* moduleNames[] = { "AFE", "BIAS", "XBUF", "CGEN", "LDO", "BIST", "CDS", "TRF", "TBB", "RFE", "RBB", "SX",
        "TxTSP", "TxNCO", "TxGFIR1", "TxGFIR2", "TxGFIR3a", "TxGFIR3b", "TxGFIR3c",
        "RxTSP", "RxNCO", "RxGFIR1", "RxGFIR2", "RxGFIR3a", "RxGFIR3b", "RxGFIR3c", "LimeLight" };

    const uint16_t patterns[] = { 0xAAAA, 0x5555 };
    const uint8_t patternsCount = 2;

    bool allTestSuccess = true;

    for (unsigned i = 0; i < modulesToCheck.size(); ++i)
    {
        bool moduleTestsSuccess = true;
        uint16_t startAddr = MemorySectionAddresses[modulesToCheck[i]][0];
        uint16_t endAddr = MemorySectionAddresses[modulesToCheck[i]][1];
        uint8_t channelCount = startAddr >= 0x0100 ? 2 : 1;
        for (int cc = 1; cc <= channelCount; ++cc)
        {
            Modify_SPI_Reg_bits(LMS7param(MAC), cc);
            sprintf(chex, "0x%04X", startAddr);
            ss << moduleNames[i] << "  [" << chex << ":";
            sprintf(chex, "0x%04X", endAddr);
            ss << chex << "]";
            if (startAddr >= 0x0100)
                ss << " Ch." << (cc == 1 ? "A" : "B");
                ss << endl;
            for (uint8_t p = 0; p < patternsCount; ++p)
                moduleTestsSuccess &= RegistersTestInterval(startAddr, endAddr, patterns[p], ss) == LIBLMS7_SUCCESS;
        }
        allTestSuccess &= moduleTestsSuccess;
    }

    //restore register values
    Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    SPI_write_batch(&ch1Addresses[0], &ch1Data[0], ch1Addresses.size());
    Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    SPI_write_batch(&ch2Addresses[0], &ch2Data[0], ch2Addresses.size());
    Modify_SPI_Reg_bits(LMS7param(MAC), ch);

    fstream fout;
    fout.open("registersTest.txt", ios::out);
    fout << ss.str() << endl;
    fout.close();

    return allTestSuccess ? LIBLMS7_SUCCESS : LIBLMS7_FAILURE;
}

/** @brief Performs registers test for given address interval by writing given pattern data
    @param startAddr first register address
    @param endAddr last reigster address
    @param pattern data to be written into registers
    @return 0-register test passed, other-failure
*/
liblms7_status LMS7002M::RegistersTestInterval(uint16_t startAddr, uint16_t endAddr, uint16_t pattern, stringstream &ss)
{
    vector<uint16_t> addrToWrite;
    vector<uint16_t> dataToWrite;
    vector<uint16_t> dataReceived;
    vector<uint16_t> dataMasks;

    for (uint16_t addr = startAddr; addr <= endAddr; ++addr)
    {
        addrToWrite.push_back(addr);
    }
    dataMasks.resize(addrToWrite.size(), 0xFFFF);
    for (uint16_t j = 0; j < sizeof(readOnlyRegisters)/sizeof(uint16_t); ++j)
        for (uint16_t k = 0; k < addrToWrite.size(); ++k)
            if (readOnlyRegisters[j] == addrToWrite[k])
            {
                dataMasks[k] = readOnlyRegistersMasks[j];
                break;
            }

    dataToWrite.clear();
    dataReceived.clear();
    for (uint16_t j = 0; j < addrToWrite.size(); ++j)
    {
        if (addrToWrite[j] == 0x00A6)
            dataToWrite.push_back(0x1 | pattern & ~0x2);
        else if (addrToWrite[j] == 0x0084)
            dataToWrite.push_back(pattern & ~0x19);
        else
            dataToWrite.push_back(pattern & dataMasks[j]);
    }
    dataReceived.resize(addrToWrite.size(), 0);
    liblms7_status status;
    status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size());
    if (status != LIBLMS7_SUCCESS)
        return status;
    status = SPI_read_batch(&addrToWrite[0], &dataReceived[0], addrToWrite.size());
    if (status != LIBLMS7_SUCCESS)
        return status;
    bool registersMatch = true;
    char ctemp[16];
    for (uint16_t j = 0; j < dataToWrite.size(); ++j)
    {
        if (dataToWrite[j] != (dataReceived[j] & dataMasks[j]))
        {
            registersMatch = false;
            sprintf(ctemp, "0x%04X", addrToWrite[j]);
            ss << "\t" << ctemp << "(wr/rd): ";
            sprintf(ctemp, "0x%04X", dataToWrite[j]);
            ss << ctemp << "/";
            sprintf(ctemp, "0x%04X", dataReceived[j]);
            ss << ctemp << endl;
        }
    }
    if (registersMatch)
    {
        sprintf(ctemp, "0x%04X", pattern);
        ss << "\tRegisters OK (" << ctemp << ")\n";
    }
    return registersMatch ? LIBLMS7_SUCCESS : LIBLMS7_FAILURE;
}

/** @brief Parameters setup instructions for Tx calibration
    @return 0-success, other-failure
*/
liblms7_status LMS7002M::CalibrateTxSetup(float_type bandwidth_MHz, bool useTSGsource)
{
	//Stage 2
	uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));
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
		return LIBLMS7_BAND_NOT_SELECTED;

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
	//reset AFE to defaults
	uint8_t isel_dac_afe = (uint8_t)Get_SPI_Reg_bits(LMS7param(ISEL_DAC_AFE));
	SetDefaults(AFE);
	Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0); //PD_RX_AFE2 0
	
	Modify_SPI_Reg_bits(LMS7param(ISEL_DAC_AFE), isel_dac_afe);

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

    if (SetFrequencyCGEN(CGEN_FREQ_CALIBRATIONS) != LIBLMS7_SUCCESS)
        return LIBLMS7_FAILURE;
    if (TuneVCO(VCO_CGEN) != LIBLMS7_SUCCESS)
        return LIBLMS7_FAILURE;

    //SXR
    Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    SetDefaults(SX);
    Modify_SPI_Reg_bits(LMS7param(PD_VCO), 0);
    {
        float_type SXTfreqMHz = GetFrequencySX_MHz(Tx, mRefClkSXT_MHz);
        float_type SXRfreqMHz = SXTfreqMHz - bandwidth_MHz / calibUserBwDivider - 1;
        if (SetFrequencySX(Rx, SXRfreqMHz, mRefClkSXR_MHz) != LIBLMS7_SUCCESS)
            return LIBLMS7_FAILURE;
        if (TuneVCO(VCO_SXR) != LIBLMS7_SUCCESS)
            return LIBLMS7_FAILURE;
    }
	Modify_SPI_Reg_bits(LMS7param(MAC), ch);

    //SXT
	//do nothing

    //TXTSP
    SetDefaults(TxTSP);
	Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), useTSGsource ? 1 : 0);
    Modify_SPI_Reg_bits(0x0208, 6, 4, 0x7); //GFIR3_BYP 1, GFIR2_BYP 1, GFIR1_BYP 1
	Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_TXTSP), 0);
    LoadDC_REG_IQ(Tx, (int16_t)0x7FFF, (int16_t)0x8000);

    SetNCOFrequency(Tx, 0, bandwidth_MHz / calibUserBwDivider);

    //RXTSP
    SetDefaults(RxTSP);
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

	if (ch == 2)
	{
		Modify_SPI_Reg_bits(MAC, 1);
		Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0);
		Modify_SPI_Reg_bits(LMS7param(EN_NEXTRX_RFE), 1); // EN_NEXTTX_RFE 1
		Modify_SPI_Reg_bits(LMS7param(EN_NEXTTX_TRF), 1); //EN_NEXTTX_TRF 1
		Modify_SPI_Reg_bits(MAC, ch);
	}

    return LIBLMS7_SUCCESS;
}

/** @brief Flips the CAPTURE bit and returns digital RSSI value
*/
uint32_t gRSSI = ~0;
uint32_t badRSSI = 0;

uint32_t LMS7002M::GetRSSI()
{
#ifndef RSSI_FROM_MCU
    Modify_SPI_Reg_bits(LMS7param(CAPTURE), 0);
    Modify_SPI_Reg_bits(LMS7param(CAPTURE), 1);
    return (Get_SPI_Reg_bits(0x040F, 15, 0) << 2) | Get_SPI_Reg_bits(0x040E, 1, 0);
#else
	uint32_t rssiAvg = 0;
	for (int i = 0; i < 2; ++i)
	{
		mcuControl->CallMCU(1);
		int status = mcuControl->WaitForMCU();
		if (status == 0)
			//something wrong
		{
			printf("MCU working too long\n");
		}
        
		mcuControl->DebugModeSet_MCU(1, 0);
		//read result value from MCU RAM
		rssiAvg = 0;
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
		rssiAvg = 0;
		rssiAvg |= (ramData[0] & 0xFF) << 16;
		rssiAvg |= (ramData[1] & 0xFF) << 8;
		rssiAvg |= (ramData[2] & 0xFF);
		mcuControl->DebugModeExit_MCU(1, 0);
	}
    return rssiAvg;
#endif
}

/** @brief Sets Rx Dc offsets by converting two's complementary numbers to sign and magnitude
*/
void LMS7002M::SetRxDCOFF(int8_t offsetI, int8_t offsetQ)
{
    uint16_t valToSend = 0;
    if (offsetI < 0)
        valToSend |= 0x40;
    valToSend |= labs(offsetI);
    valToSend = valToSend << 7;
    if (offsetQ < 0)
        valToSend |= 0x40;
    valToSend |= labs(offsetQ);
    SPI_write(0x010E, valToSend);
}

/** @brief Calibrates Transmitter. DC correction, IQ gains, IQ phase correction
    @return 0-success, other-failure
*/
liblms7_status LMS7002M::CalibrateTx(float_type bandwidth_MHz, bool useTSGsource)
{
    uint16_t gainAddr;
    uint16_t gcorri;
    uint16_t gcorrq;
    uint16_t dccorri;
    uint16_t dccorrq;
    int16_t phaseOffset;
    int16_t gain = 1983;
    const uint16_t gainMSB = 10;
    const uint16_t gainLSB = 0;

    liblms7_status status;
    Log("Tx calibration started", LOG_INFO);
    BackupAllRegisters();

    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));
    Log("Setup stage", LOG_INFO);
    status = CalibrateTxSetup(bandwidth_MHz, useTSGsource);
    if (status != LIBLMS7_SUCCESS)
        goto TxCalibrationEnd; //go to ending stage to restore registers
#ifdef USE_MCU
    mcuControl->CallMCU(12);
    auto statusMcu = mcuControl->WaitForMCU();
    if (statusMcu == 0)
    {
        printf("MCU working too long %i\n", statusMcu);
    }
#else
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
        Modify_SPI_Reg_bits(GCORRI_TXTSP, 2047 - 15);
        Modify_SPI_Reg_bits(GCORRQ_TXTSP, 2047);
        rssiIgain = GetRSSI();
        Modify_SPI_Reg_bits(GCORRI_TXTSP, 2047);
        Modify_SPI_Reg_bits(GCORRQ_TXTSP, 2047 - 15);
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
#endif   
	dccorri = Get_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP));
    dccorrq = Get_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP));
    gcorri = Get_SPI_Reg_bits(LMS7param(GCORRI_TXTSP));
    gcorrq = Get_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP));
    phaseOffset = Get_SPI_Reg_bits(LMS7param(IQCORR_TXTSP));

TxCalibrationEnd:
    Log("Restoring registers state", LOG_INFO);
    Modify_SPI_Reg_bits(LMS7param(MAC), ch);
    RestoreAllRegisters();
    if (status != LIBLMS7_SUCCESS)
    {
        Log("Tx calibration failed", LOG_WARNING);
        return status;
    }

    Modify_SPI_Reg_bits(LMS7param(MAC), ch);
    Modify_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP), dccorri);
    Modify_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP), dccorrq);
    Modify_SPI_Reg_bits(LMS7param(GCORRI_TXTSP), gcorri);
    Modify_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP), gcorrq);
    Modify_SPI_Reg_bits(LMS7param(IQCORR_TXTSP), phaseOffset);

    Modify_SPI_Reg_bits(LMS7param(DC_BYP_TXTSP), 0); //DC_BYP
    Modify_SPI_Reg_bits(0x0208, 1, 0, 0); //GC_BYP PH_BYP
    Log("Tx calibration finished", LOG_INFO);
    return LIBLMS7_SUCCESS;
}

/** @brief Performs Rx DC offsets calibration
*/
/** @brief Performs Rx DC offsets calibration
*/
void LMS7002M::CalibrateRxDC_RSSI()
{
#ifdef USE_MCU
    auto t1 = chrono::high_resolution_clock::now();
    mcuControl->CallMCU(2);
    auto statusMcu = mcuControl->WaitForMCU();
    if (statusMcu == 0)
        //something wrong
    {
        printf("MCU working too long\n");
    }
    auto t2 = chrono::high_resolution_clock::now();
    long timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    cout << "Rx DC calibration duration " << timePeriod << " ms" << endl;
    return;
#endif
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
liblms7_status LMS7002M::FixRXSaturation()
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
    return GetRSSI() < rssi_saturation_level ? LIBLMS7_SUCCESS : LIBLMS7_FAILURE;
}

uint32_t LMS7002M::FindMinRSSI(const LMS7Parameter &param, const int16_t startValue, int16_t *result, const uint8_t scanWidth, const uint8_t twoCompl, int8_t stepMult)
{
    return FindMinRSSI(param.address, param.msb, param.lsb, startValue, result, scanWidth, twoCompl, stepMult);
}

/** @brief Searches for minimal RSSI value while changing given address bits
@param addr address of parameter being changed
@param msb most significant bit index
@param lsb least significant bit index
@param startValue initial value where to start search
@param result found minimal parameter value will be set here
@param twoCompl varying parameter value is treated as two's complement
@return found minimal RSSI value
*/
uint32_t LMS7002M::FindMinRSSI(const uint16_t addr, const uint8_t msb, const uint8_t lsb, const int16_t startValue, int16_t *result, const uint8_t scanWidth, const uint8_t twoCompl, int8_t stepMult)
{
    if (scanWidth < 1)
        return ~0;
    int minI;
    int min = startValue;
    int globMin = 0;
    uint32_t minRSSI = ~0;
    unsigned int *rssiField = new unsigned int[scanWidth];
    int minRSSIindex;
    int i;
    int maxVal;
    int minVal = 0;
    if (twoCompl)
    {
        maxVal = (~(~0x0 << (msb - lsb + 1))) / 2;
        minVal = -(~(~0x0 << (msb - lsb + 1))) / 2 - 1;
    }
    else
        maxVal = (~(~0x0 << (msb - lsb + 1)));

    Modify_SPI_Reg_bits(addr, msb, lsb, startValue);

    Modify_SPI_Reg_bits(LMS7param(AGC_MODE_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(CAPSEL), 0);

    minRSSIindex = 0;
    for (i = 0; i<scanWidth; ++i)
    {
        short currentValue = min + (i - scanWidth / 2)*stepMult;
        if (currentValue < minVal)
            currentValue = minVal;
        else if (currentValue > maxVal)
            currentValue = maxVal;
        if (twoCompl == 2)
        {
            uint16_t valToSend = 0;
            if (currentValue < 0)
                valToSend |= 0x40;
            valToSend |= labs(currentValue);
            Modify_SPI_Reg_bits(addr, msb, lsb, valToSend);
        }
        else
            Modify_SPI_Reg_bits(addr, msb, lsb, currentValue);

        rssiField[i] = GetRSSI();
    }
    minI = min;
    minRSSIindex = 0;
    for (i = 0; i<scanWidth; ++i)
        if (rssiField[i] < minRSSI)
        {
            minRSSI = rssiField[i];
            minRSSIindex = i;
            minI = min + (i - scanWidth / 2)*stepMult;
            if (minI > maxVal)
                minI = maxVal;
            else if (minI < minVal)
                minI = minVal;
            globMin = minI;
        }
    min = minI;

    Modify_SPI_Reg_bits(addr, msb, lsb, min);

    *result = min;
    return minRSSI;
}

/** @brief Sets given module registers to default values
    @return 0-success, other-failure
*/
liblms7_status LMS7002M::SetDefaults(MemorySection module)
{
    liblms7_status status = LIBLMS7_SUCCESS;
    vector<uint16_t> addrs;
    vector<uint16_t> values;
    for(uint32_t address = MemorySectionAddresses[module][0]; address <= MemorySectionAddresses[module][1]; ++address)
    {
        addrs.push_back(address);
        values.push_back(mRegistersMap->GetDefaultValue(address));
    }
    status = SPI_write_batch(&addrs[0], &values[0], addrs.size());
    return status;
}

/** @brief Parameters setup instructions for Rx calibration
    @param bandwidth_MHz filter bandwidth in MHz
    @return 0-success, other-failure
*/
liblms7_status LMS7002M::CalibrateRxSetup(float_type bandwidth_MHz, bool useTSGsource)
{
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));

    //rfe
    Modify_SPI_Reg_bits(LMS7param(EN_DCOFF_RXFE_RFE), 1);
    Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), 15); //G_RXLOOPB_RFE 15
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
            return LIBLMS7_BAD_SEL_PATH; //todo restore settings
    }

    //TBB
    //reset TBB to defaults
    SetDefaults(TBB);
    Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), 2);
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_FRP_TBB), 1); //ICT_IAMP_FRP_TBB 1
    Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_GG_FRP_TBB), 6); //ICT_IAMP_GG_FRP_TBB 6

    //AFE
    //reset AFE to defaults
    SetDefaults(AFE);
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

    liblms7_status status = SetFrequencyCGEN(CGEN_FREQ_CALIBRATIONS);
    if (status != LIBLMS7_SUCCESS)
        return status;

    //SXR
    Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    float_type SXRfreqMHz = GetFrequencySX_MHz(Rx, mRefClkSXR_MHz);

    //SXT
    Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    SetDefaults(SX);
    Modify_SPI_Reg_bits(LMS7param(PD_VCO), 0);
    status = SetFrequencySX(Tx, SXRfreqMHz + bandwidth_MHz / calibUserBwDivider, mRefClkSXT_MHz);
    if ( status != LIBLMS7_SUCCESS)
        return status;
    Modify_SPI_Reg_bits(LMS7param(MAC), ch);

    //TXTSP
    SetDefaults(TxTSP);
	Modify_SPI_Reg_bits(CMIX_BYP_TXTSP, 1);
    Modify_SPI_Reg_bits(TSGMODE_TXTSP, 0x1); //TSGMODE 1
	Modify_SPI_Reg_bits(INSEL_TXTSP, useTSGsource ? 1 : 0);
    //Modify_SPI_Reg_bits(0x0208, 6, 4, 0xFFFF); //GFIR3_BYP 1, GFIR2_BYP 1, GFIR1_BYP 1
    Modify_SPI_Reg_bits(0x0208, 6, 6, 1); //GFIR3_BYP 1, GFIR2_BYP 1, GFIR1_BYP 1
    Modify_SPI_Reg_bits(0x0208, 5, 5, 1); //GFIR3_BYP 1, GFIR2_BYP 1, GFIR1_BYP 1
    Modify_SPI_Reg_bits(0x0208, 4, 4, 1); //GFIR3_BYP 1, GFIR2_BYP 1, GFIR1_BYP 1
	Modify_SPI_Reg_bits(CMIX_GAIN_TXTSP, 1);  
    LoadDC_REG_IQ(Tx, (int16_t)0x7FFF, (int16_t)0x8000);
    SetNCOFrequency(Tx, 0, 0);

    //RXTSP
    SetDefaults(RxTSP);
    Modify_SPI_Reg_bits(LMS7param(AGC_MODE_RXTSP), 1); //AGC_MODE 1
    Modify_SPI_Reg_bits(0x040C, 7, 7, 0x1); //CMIX_BYP 1
    Modify_SPI_Reg_bits(0x040C, 4, 4, 1); //
    Modify_SPI_Reg_bits(0x040C, 3, 3, 1); //

    Modify_SPI_Reg_bits(LMS7param(CAPSEL), 0);
    Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), 4);
    Modify_SPI_Reg_bits(LMS7param(AGC_AVG_RXTSP), 1); //agc_avg iq corr
    Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_RXTSP), 1);
    Modify_SPI_Reg_bits(LMS7param(GFIR3_L_RXTSP), 7);
    Modify_SPI_Reg_bits(LMS7param(GFIR3_N_RXTSP), 31);

    SetGFIRCoefficients(Rx, 2, firCoefs, sizeof(firCoefs) / sizeof(int16_t));

    //modifications when calibrating channel B
    if (ch == 2)
    {
		Modify_SPI_Reg_bits(MAC, 1);
        Modify_SPI_Reg_bits(LMS7param(EN_NEXTRX_RFE), 1);
        Modify_SPI_Reg_bits(EN_NEXTTX_TRF, 1);
        Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0);
		Modify_SPI_Reg_bits(MAC, ch);
    }
    return LIBLMS7_SUCCESS;
}

/** @brief Calibrates Receiver. DC offset, IQ gains, IQ phase correction
    @return 0-success, other-failure
*/
liblms7_status LMS7002M::CalibrateRx(float_type bandwidth_MHz, bool useTSGsource)
{
	liblms7_status status;
	int16_t iqcorr_rx = 0;
    int16_t dcoffi;
	int16_t dcoffq;
    int16_t gain;
    uint16_t gainAddr = 0;
    const uint8_t gainMSB = 10;
    const uint8_t gainLSB = 0;
    int16_t phaseOffset;
    uint16_t mingcorri;
    uint16_t mingcorrq;

	Log("Rx calibration started", LOG_INFO);
	uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));
	Log("Saving registers state", LOG_INFO);
    BackupAllRegisters();    
    uint8_t sel_path_rfe = (uint8_t)Get_SPI_Reg_bits(LMS7param(SEL_PATH_RFE));
    if (sel_path_rfe == 1 || sel_path_rfe == 0)
        return LIBLMS7_BAD_SEL_PATH;

	Log("Setup stage", LOG_INFO);
    status = CalibrateRxSetup(bandwidth_MHz, useTSGsource);
	if (status != LIBLMS7_SUCCESS)
		goto RxCalibrationEndStage;

#ifdef USE_MCU
    mcuControl->CallMCU(11);
    auto statusMcu = mcuControl->WaitForMCU();
    if (statusMcu == 0)
    {
        printf("MCU working too long %i\n", statusMcu);
    }
#else
	Log("Rx DC calibration", LOG_INFO);

    CalibrateRxDC_RSSI();

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
	CheckSaturation();

    SetGFIRCoefficients(Rx, 2, firCoefs, sizeof(firCoefs) / sizeof(int16_t));

	Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
    {
        const float_type RxFreq = GetFrequencySX_MHz(LMS7002M::Rx, mRefClkSXR_MHz);
        const float_type TxFreq = GetFrequencySX_MHz(LMS7002M::Tx, mRefClkSXT_MHz);
        SetNCOFrequency(LMS7002M::Rx, 0, TxFreq - RxFreq + 0.1);
    }
    
	Modify_SPI_Reg_bits(IQCORR_RXTSP, 0);
	Modify_SPI_Reg_bits(GCORRI_RXTSP, 2047);
    Modify_SPI_Reg_bits(GCORRQ_RXTSP, 2047);

    //coarse gain
    {
        Modify_SPI_Reg_bits(GCORRI_RXTSP, 2047 - 15);
        Modify_SPI_Reg_bits(GCORRQ_RXTSP, 2047);
        uint32_t rssiIgain = GetRSSI();
        Modify_SPI_Reg_bits(GCORRI_RXTSP, 2047);
        Modify_SPI_Reg_bits(GCORRQ_RXTSP, 2047 - 15);
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

	CoarseSearch(gainAddr, gainMSB, gainLSB, gain, 4);
#ifdef LMS_VERBOSE_OUTPUT
    printf("Coarse search Rx GAIN_%s: %i\n", gainAddr == GCORRI_RXTSP.address ? "I" : "Q", gain);
#endif

	CoarseSearch(IQCORR_RXTSP.address, IQCORR_RXTSP.msb, IQCORR_RXTSP.lsb, phaseOffset, 4);
#ifdef LMS_VERBOSE_OUTPUT
    printf("Coarse search Rx IQCORR: %i\n", phaseOffset);
#endif

#endif //USE_MCU
	mingcorri = Get_SPI_Reg_bits(GCORRI_RXTSP);
	mingcorrq = Get_SPI_Reg_bits(GCORRQ_RXTSP);
	dcoffi = Get_SPI_Reg_bits(DCOFFI_RFE);
	dcoffq = Get_SPI_Reg_bits(DCOFFQ_RFE);
    phaseOffset = Get_SPI_Reg_bits(IQCORR_RXTSP);

RxCalibrationEndStage:
    Log("Restoring registers state", LOG_INFO);
    RestoreAllRegisters();
    if (status != LIBLMS7_SUCCESS)
    {
        Log("Rx calibration failed", LOG_WARNING);
        return status;
    }

    Modify_SPI_Reg_bits(LMS7param(MAC), ch);
    SetRxDCOFF((int8_t)dcoffi, (int8_t)dcoffq);
    Modify_SPI_Reg_bits(LMS7param(EN_DCOFF_RXFE_RFE), 1);
    Modify_SPI_Reg_bits(LMS7param(GCORRI_RXTSP), mingcorri);
    Modify_SPI_Reg_bits(LMS7param(GCORRQ_RXTSP), mingcorrq);
    Modify_SPI_Reg_bits(LMS7param(IQCORR_RXTSP), phaseOffset);
    Modify_SPI_Reg_bits(0x040C, 2, 0, 0); //DC_BYP 0, GC_BYP 0, PH_BYP 0
    Modify_SPI_Reg_bits(0x0110, 4, 0, 31); //ICT_LO_RFE 31
    Log("Rx calibration finished", LOG_INFO);
    return LIBLMS7_SUCCESS;
}

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

/** @brief Stores chip current registers state into memory for later restoration
*/
void LMS7002M::BackupAllRegisters()
{
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));
    SPI_read_batch(backupAddrs, backupRegs, sizeof(backupAddrs) / sizeof(uint16_t));
    Modify_SPI_Reg_bits(LMS7param(MAC), 1); // channel A
    SPI_read_batch(backupSXAddr, backupRegsSXR, sizeof(backupRegsSXR) / sizeof(uint16_t));
    //backup GFIR3 coefficients
    GetGFIRCoefficients(LMS7002M::Rx, 2, rxGFIR3_backup, 105);
    //EN_NEXTRX_RFE could be modified in channel A
    backup0x010D = SPI_read(0x010D);
    //EN_NEXTTX_TRF could be modified in channel A
    backup0x0100 = SPI_read(0x0100);
    Modify_SPI_Reg_bits(LMS7param(MAC), 2); // channel B
    SPI_read_batch(backupSXAddr, backupRegsSXT, sizeof(backupRegsSXR) / sizeof(uint16_t));
    Modify_SPI_Reg_bits(LMS7param(MAC), ch);
}

/** @brief Sets chip registers to state that was stored in memory using BackupAllRegisters()
*/
void LMS7002M::RestoreAllRegisters()
{
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));
    SPI_write_batch(backupAddrs, backupRegs, sizeof(backupAddrs) / sizeof(uint16_t));
    //restore GFIR3
    SetGFIRCoefficients(LMS7002M::Rx, 2, rxGFIR3_backup, 105);
    Modify_SPI_Reg_bits(LMS7param(MAC), 1); // channel A    
    SPI_write(0x010D, backup0x010D); //restore EN_NEXTRX_RFE
    SPI_write(0x0100, backup0x0100); //restore EN_NEXTTX_TRF
    SPI_write_batch(backupSXAddr, backupRegsSXR, sizeof(backupRegsSXR) / sizeof(uint16_t));
    Modify_SPI_Reg_bits(LMS7param(MAC), 2); // channel B
    SPI_write_batch(backupSXAddr, backupRegsSXT, sizeof(backupRegsSXR) / sizeof(uint16_t));
    Modify_SPI_Reg_bits(LMS7param(MAC), ch);
}

/** @brief Searches for minimal digital RSSI value by changing given gain parameter
    @param param LMS7002M gain correction parameter
    @param foundValue returns value which achieved minimal RSSI
    @return minimal found RSSI value
*/
uint32_t LMS7002M::FindMinRSSI_Gain(const LMS7Parameter &param, uint16_t *foundValue)
{
    uint32_t RSSI = ~0 - 2;
    uint32_t prevRSSI = RSSI + 1;
    uint8_t decrement = 2;
    uint16_t gcorr = 2047;
    while (gcorr > 1024)
    {
        Modify_SPI_Reg_bits(param, gcorr);
        RSSI = GetRSSI();
        if (RSSI < prevRSSI)
        {
            prevRSSI = RSSI;
            *foundValue = gcorr;
            gcorr -= decrement;
            decrement *= 2;
        }
        else
        {
            if (decrement == 2)
                break;
            gcorr -= decrement;
            decrement = 2;
        }
    }
    return prevRSSI;
}

/** @brief Reads all chip configuration and checks if it matches with local registers copy
*/
bool LMS7002M::IsSynced()
{
    if (controlPort->IsOpen() == false)
        return false;
    bool isSynced = true;
    liblms7_status status;

    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC));

    vector<uint16_t> addrToRead = mRegistersMap->GetUsedAddresses(0);
    vector<uint16_t> dataReceived;
    dataReceived.resize(addrToRead.size(), 0);

    Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    status = SPI_read_batch(&addrToRead[0], &dataReceived[0], addrToRead.size());
    if (status != LIBLMS7_SUCCESS)
    {
        isSynced = false;
        goto isSyncedEnding;
    }

    //mask out readonly bits
    for (uint16_t j = 0; j < sizeof(readOnlyRegisters) / sizeof(uint16_t); ++j)
        for (uint16_t k = 0; k < addrToRead.size(); ++k)
            if (readOnlyRegisters[j] == addrToRead[k])
            {
                dataReceived[k] &= readOnlyRegistersMasks[j];
                break;
            }

    //check if local copy matches chip
    for (uint16_t i = 0; i < addrToRead.size(); ++i)
    {
        if (dataReceived[i] != mRegistersMap->GetValue(0, addrToRead[i]))
        {
            isSynced = false;
            goto isSyncedEnding;
        }
    }

    addrToRead.clear(); //add only B channel addresses
    addrToRead = mRegistersMap->GetUsedAddresses(1);

    //mask out readonly bits
    for (uint16_t j = 0; j < sizeof(readOnlyRegisters) / sizeof(uint16_t); ++j)
        for (uint16_t k = 0; k < addrToRead.size(); ++k)
            if (readOnlyRegisters[j] == addrToRead[k])
            {
                dataReceived[k] &= readOnlyRegistersMasks[j];
                break;
            }

    Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    status = SPI_read_batch(&addrToRead[0], &dataReceived[0], addrToRead.size());
    if (status != LIBLMS7_SUCCESS)
        return false;
    //check if local copy matches chip
    for (uint16_t i = 0; i < addrToRead.size(); ++i)
        if (dataReceived[i] != mRegistersMap->GetValue(1, addrToRead[i]))
        {
            isSynced = false;
            break;
        }

isSyncedEnding:
    Modify_SPI_Reg_bits(LMS7param(MAC), ch); //restore previously used channel
    return isSynced;
}

/** @brief Writes all registers from host to chip

When used on Novena board, also changes gpios to match rx path and tx band selections
*/
liblms7_status LMS7002M::UploadAll()
{
    if (controlPort == NULL)
        return LIBLMS7_NO_CONNECTION_MANAGER;
    if (controlPort->IsOpen() == false)
        return LIBLMS7_NOT_CONNECTED;

    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC)); //remember used channel

    liblms7_status status;

    vector<uint16_t> addrToWrite;
    vector<uint16_t> dataToWrite;

    uint16_t x0020_value = mRegistersMap->GetValue(0, 0x0020);
    Modify_SPI_Reg_bits(LMS7param(MAC), 1); //select A channel

    addrToWrite = mRegistersMap->GetUsedAddresses(0);
    //remove 0x0020 register from list, to not change MAC
    addrToWrite.erase( find(addrToWrite.begin(), addrToWrite.end(), 0x0020) );
    for (auto address : addrToWrite)
        dataToWrite.push_back(mRegistersMap->GetValue(0, address));

    status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size());
    status = LIBLMS7_SUCCESS;
    if (status != LIBLMS7_SUCCESS)
        return status;
    //after all channel A registers have been written, update 0x0020 register value
    status = SPI_write(0x0020, x0020_value);
    if (status != LIBLMS7_SUCCESS)
        return status;
    Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    if (status != LIBLMS7_SUCCESS)
        return status;

    addrToWrite = mRegistersMap->GetUsedAddresses(1);
    dataToWrite.clear();
    for (auto address : addrToWrite)
    {
        dataToWrite.push_back(mRegistersMap->GetValue(1, address));
    }
    Modify_SPI_Reg_bits(LMS7param(MAC), 2); //select B channel
    status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size());
    if (status != LIBLMS7_SUCCESS)
        return status;
    Modify_SPI_Reg_bits(LMS7param(MAC), ch); //restore last used channel

    //in case of Novena board, need to update GPIO
    if(controlPort->GetInfo().device == LMS_DEV_NOVENA)
    {
        uint16_t regValue = SPI_read(0x0706) & 0xFFF8;
        //lms_gpio2 - tx output selection:
		//		0 - TX1_A and TX1_B (Band 1),
		//		1 - TX2_A and TX2_B (Band 2)
        regValue |= Get_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF)) << 2; //gpio2
        //RX active paths
        //lms_gpio0 | lms_gpio1      	RX_A		RX_B
        //  0 			0       =>  	no active path
        //  1   		0 		=>	LNAW_A  	LNAW_B
        //  0			1		=>	LNAH_A  	LNAH_B
        //  1			1		=>	LNAL_A 	 	LNAL_B
        switch(Get_SPI_Reg_bits(LMS7param(SEL_PATH_RFE)))
        {
            //set gpio1:gpio0
            case 0: regValue |= 0x0; break;
            case 1: regValue |= 0x2; break;
            case 2: regValue |= 0x3; break;
            case 3: regValue |= 0x1; break;
        }
        SPI_write(0x0706, regValue);
    }
    return LIBLMS7_SUCCESS;
}

/** @brief Reads all registers from the chip to host

When used on Novena board, also updates gpios to match rx path and tx band selections
*/
liblms7_status LMS7002M::DownloadAll()
{
    if (controlPort == nullptr)
        return LIBLMS7_NO_CONNECTION_MANAGER;
    if (controlPort->IsOpen() == false)
        return LIBLMS7_NOT_CONNECTED;
    liblms7_status status;
    uint8_t ch = (uint8_t)Get_SPI_Reg_bits(LMS7param(MAC), false);

    vector<uint16_t> addrToRead = mRegistersMap->GetUsedAddresses(0);
    vector<uint16_t> dataReceived;
    dataReceived.resize(addrToRead.size(), 0);
    Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    status = SPI_read_batch(&addrToRead[0], &dataReceived[0], addrToRead.size());
    if (status != LIBLMS7_SUCCESS)
        return status;

    for (uint16_t i = 0; i < addrToRead.size(); ++i)
    {
        uint16_t adr = addrToRead[i];
        uint16_t val = dataReceived[i];
        mRegistersMap->SetValue(0, addrToRead[i], dataReceived[i]);
    }

    addrToRead.clear(); //add only B channel addresses
    addrToRead = mRegistersMap->GetUsedAddresses(1);
    dataReceived.resize(addrToRead.size(), 0);

    Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    status = SPI_read_batch(&addrToRead[0], &dataReceived[0], addrToRead.size());
    if (status != LIBLMS7_SUCCESS)
        return status;
    for (uint16_t i = 0; i < addrToRead.size(); ++i)
        mRegistersMap->SetValue(1, addrToRead[i], dataReceived[i]);

    Modify_SPI_Reg_bits(LMS7param(MAC), ch); //retore previously used channel

    //in case of Novena board, update GPIO
    if(controlPort->GetInfo().device == LMS_DEV_NOVENA)
    {
        uint16_t regValue = SPI_read(0x0706) & 0xFFF8;
        //lms_gpio2 - tx output selection:
		//		0 - TX1_A and TX1_B (Band 1),
		//		1 - TX2_A and TX2_B (Band 2)
        regValue |= Get_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF)) << 2; //gpio2
        //RX active paths
        //lms_gpio0 | lms_gpio1      	RX_A		RX_B
        //  0 			0       =>  	no active path
        //  1   		0 		=>	LNAW_A  	LNAW_B
        //  0			1		=>	LNAH_A  	LNAH_B
        //  1			1		=>	LNAL_A 	 	LNAL_B
        switch(Get_SPI_Reg_bits(LMS7param(SEL_PATH_RFE)))
        {
            //set gpio1:gpio0
            case 0: regValue |= 0x0; break;
            case 1: regValue |= 0x2; break;
            case 2: regValue |= 0x3; break;
            case 3: regValue |= 0x1; break;
        }
        SPI_write(0x0706, regValue);
    }

    return LIBLMS7_SUCCESS;
}

/** @brief Configures interfaces for desired frequency
    Sets interpolation and decimation, changes MCLK sources and TSP clock dividers accordingly to selected interpolation and decimation
*/
liblms7_status LMS7002M::SetInterfaceFrequency(float_type cgen_freq_MHz, const uint8_t interpolation, const uint8_t decimation)
{
    Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), decimation);
    Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), interpolation);
    liblms7_status status = SetFrequencyCGEN(cgen_freq_MHz);
    if (status != LIBLMS7_SUCCESS)
        return status;

    if (decimation == 7 || decimation == 0) //bypass
    {
        Modify_SPI_Reg_bits(LMS7param(RXTSPCLKA_DIV), 0);
        Modify_SPI_Reg_bits(LMS7param(RXDIVEN), false);
        Modify_SPI_Reg_bits(LMS7param(MCLK2SRC), 3);
    }
    else
    {
        uint8_t divider = (uint8_t)pow(2.0, decimation);
        if (divider > 1)
            Modify_SPI_Reg_bits(LMS7param(RXTSPCLKA_DIV), (divider / 2) - 1);
        else
            Modify_SPI_Reg_bits(LMS7param(RXTSPCLKA_DIV), 0);
        Modify_SPI_Reg_bits(LMS7param(RXDIVEN), true);
        Modify_SPI_Reg_bits(LMS7param(MCLK2SRC), 1);
    }
    if (interpolation == 7 || interpolation == 0) //bypass
    {
        Modify_SPI_Reg_bits(LMS7param(TXTSPCLKA_DIV), 0);
        Modify_SPI_Reg_bits(LMS7param(TXDIVEN), false);
        Modify_SPI_Reg_bits(LMS7param(MCLK1SRC), 2);
    }
    else
    {
        uint8_t divider = (uint8_t)pow(2.0, interpolation);
        if (divider > 1)
            Modify_SPI_Reg_bits(LMS7param(TXTSPCLKA_DIV), (divider / 2) - 1);
        else
            Modify_SPI_Reg_bits(LMS7param(TXTSPCLKA_DIV), 0);
        Modify_SPI_Reg_bits(LMS7param(TXDIVEN), true);
        Modify_SPI_Reg_bits(LMS7param(MCLK1SRC), 0);
    }
    return status;
}

liblms7_status LMS7002M::CheckSaturation()
{
	Modify_SPI_Reg_bits(CMIX_SC_RXTSP, 0);
	Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 0);
	const float_type RxFreq = GetFrequencySX_MHz(LMS7002M::Rx, mRefClkSXR_MHz);
	const float_type TxFreq = GetFrequencySX_MHz(LMS7002M::Tx, mRefClkSXT_MHz);
	SetNCOFrequency(LMS7002M::Rx, 0, TxFreq - RxFreq - 0.1);

	uint32_t rssi = GetRSSI();

	int cg_iamp = Get_SPI_Reg_bits(CG_IAMP_TBB);
	while (rssi < 0x06000 && cg_iamp < 63-6)
	{
		rssi = GetRSSI();
		if (rssi < 0x06000)
			cg_iamp += 6;
		if (rssi > 0x06000)
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

	return LIBLMS7_SUCCESS;
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

uint16_t testAddr = 0x002F;

void LMS7002M::CoarseSearch(const uint16_t addr, const uint8_t msb, const uint8_t lsb, int16_t &value, const uint8_t maxIterations)
{	
    const uint16_t DCOFFaddr = 0x010E;
#ifdef USE_MCU
    Modify_SPI_Reg_bits(addr, msb, lsb, addr != DCOFFaddr ? value : toDCOffset(value));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto t1 = chrono::high_resolution_clock::now();
    int mcu_gain_prog = 0;
    if (addr == GCORRI_RXTSP.address)
        mcu_gain_prog = 3;
    if (addr == GCORRQ_RXTSP.address)
        mcu_gain_prog = 4;
    if (addr == GCORRI_RXTSP.address && maxIterations != 7)
        mcu_gain_prog = 5;
    if (addr == GCORRQ_RXTSP.address && maxIterations != 7)
        mcu_gain_prog = 6;
    if (addr == IQCORR_RXTSP.address)
        mcu_gain_prog = 7;
    if (addr == IQCORR_RXTSP.address && maxIterations != 7)
        mcu_gain_prog = 8;
    
    mcuControl->CallMCU(mcu_gain_prog);
    auto statusMcu = mcuControl->WaitForMCU();
    if (statusMcu == 0)
        //something wrong
    {
        printf("MCU working too long\n");
    }
    auto t2 = chrono::high_resolution_clock::now();
    auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    value = Get_SPI_Reg_bits(addr, msb, lsb);
    if (addr == GCORRQ_RXTSP.address || addr == GCORRI_RXTSP.address)
        value = bin2complementry(value, msb - lsb, 0);
    else
        value = bin2complementry(value, msb - lsb, 1);
    cout << "Coarse Search duration " << timePeriod << " ms, value = " << value << endl;
    return;
#endif
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

liblms7_status LMS7002M::CheckSaturationTxRx(const float_type bandwidth_MHz)
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
	//rssi = GetRSSI();
	while (g_pga < 18 && g_rxlooop == 15)
	{
		g_pga += 1;
		Modify_SPI_Reg_bits(G_PGA_RBB, g_pga);
	}
	Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 1);
	Modify_SPI_Reg_bits(DC_BYP_RXTSP, 1);
	return LIBLMS7_SUCCESS;
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
    printf("     |");
    for (int i = 0; i < fieldSize; ++i)
        printf("%6i|", valueQ - fieldSize / 2 + i);
    printf("\n");
    for (int i = 0; i < fieldSize + 1; ++i)
        printf("-----+");
    printf("\n");
    for (int i = 0; i < fieldSize; ++i)
    {
        printf("%4i |", valueI + (i - fieldSize / 2));
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

MCU_BD* LMS7002M::GetMCUControls() const
{
    return mcuControl;
}