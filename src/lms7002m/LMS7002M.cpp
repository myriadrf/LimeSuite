/**
@file LMS7002M.cpp
@author Lime Microsystems (www.limemicro.com)
@brief Implementation of LMS7002M transceiver configuring
*/

#define _USE_MATH_DEFINES
#include <cmath>
#include <ciso646>

#include "LMS7002M.h"
#include <stdio.h>
#include <set>
#include "IConnection.h"
#include "ErrorReporting.h"
#include "INI.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "LMS7002M_RegistersMap.h"
#include <math.h>
#include <assert.h>
#include <chrono>
#include <thread>

#include "MCU_BD.h"
const static uint16_t MCU_PARAMETER_ADDRESS = 0x002D; //register used to pass parameter values to MCU
#define MCU_ID_DC_IQ_CALIBRATIONS 0x01
#define MCU_FUNCTION_CALIBRATE_TX 1
#define MCU_FUNCTION_CALIBRATE_RX 2

using namespace std;
using namespace lime;

#include "MCU_BD.h"

float_type LMS7002M::gVCO_frequency_table[3][2] = { { 3800e6, 5222e6 }, { 4961e6, 6754e6 }, {6306e6, 7714e6} };
float_type LMS7002M::gCGEN_VCO_frequencies[2] = {2000e6, 2700e6};

///define for parameter enumeration if prefix might be needed
#define LMS7param(id) id

//module addresses needs to be sorted in ascending order
const uint16_t LMS7002M::readOnlyRegisters[] =      { 0x002F, 0x008C, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x0123, 0x0209, 0x020A, 0x020B, 0x040E, 0x040F };
const uint16_t LMS7002M::readOnlyRegistersMasks[] = { 0x0000, 0x0FFF, 0x007F, 0x0000, 0x0000, 0x0000, 0x0000, 0x003F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

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

/** @brief Sets connection which is used for data communication with chip
*/
void LMS7002M::SetConnection(IConnection* port, const size_t devIndex)
{
    controlPort = port;
    mdevIndex = devIndex;

    if (controlPort != nullptr)
    {
        addrLMS7002M = controlPort->GetDeviceInfo().addrsLMS7002M.at(devIndex);
        mcuControl->Initialize(port);
    }
}

/** @brief Creates LMS7002M main control object.
It requires IConnection to be set by SetConnection() to communicate with chip
*/
LMS7002M::LMS7002M() :
    controlPort(nullptr),
    addrLMS7002M(-1),
    mdevIndex(0),
    mSelfCalDepth(0),
    mRegistersMap(new LMS7002M_RegistersMap()),
    useCache(0)
{
    mCalibrationByMCU = true;

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
    delete mRegistersMap;
}

void LMS7002M::SetActiveChannel(const Channel ch)
{
    if (ch == this->GetActiveChannel(false)) return;
    this->Modify_SPI_Reg_bits(LMS7param(MAC), int(ch));
}

LMS7002M::Channel LMS7002M::GetActiveChannel(bool fromChip)
{
    auto ch = Get_SPI_Reg_bits(LMS7param(MAC), fromChip);
    return Channel(ch);
}

size_t LMS7002M::GetActiveChannelIndex(bool fromChip)
{
    switch (this->GetActiveChannel(fromChip))
    {
    case ChB: return mdevIndex*2 + 1;
    default: return mdevIndex*2 + 0;
    }
}

int LMS7002M::EnableChannel(const bool isTx, const bool enable)
{
    Channel ch = this->GetActiveChannel();

    //--- LML ---
    if (ch == ChA)
    {
        if (isTx) this->Modify_SPI_Reg_bits(TXEN_A, enable?1:0);
        else      this->Modify_SPI_Reg_bits(RXEN_A, enable?1:0);
    }
    else
    {
        if (isTx) this->Modify_SPI_Reg_bits(TXEN_B, enable?1:0);
        else      this->Modify_SPI_Reg_bits(RXEN_B, enable?1:0);
    }

    //--- ADC/DAC ---
    this->Modify_SPI_Reg_bits(EN_DIR_AFE, 1);
    this->Modify_SPI_Reg_bits(EN_G_AFE, enable?1:0);
    this->Modify_SPI_Reg_bits(PD_AFE, enable?0:1);
    if (ch == ChA)
    {
        if (isTx) this->Modify_SPI_Reg_bits(PD_TX_AFE1, enable?0:1);
        else      this->Modify_SPI_Reg_bits(PD_RX_AFE1, enable?0:1);
    }
    else
    {
        if (isTx) this->Modify_SPI_Reg_bits(PD_TX_AFE2, enable?0:1);
        else      this->Modify_SPI_Reg_bits(PD_RX_AFE2, enable?0:1);
    }

    //--- digital ---
    if (isTx)
    {
        this->Modify_SPI_Reg_bits(EN_TXTSP, enable?1:0);
        this->Modify_SPI_Reg_bits(GFIR3_BYP_TXTSP, 1);
        this->Modify_SPI_Reg_bits(GFIR2_BYP_TXTSP, 1);
        this->Modify_SPI_Reg_bits(GFIR1_BYP_TXTSP, 1);
    }
    else
    {
        this->Modify_SPI_Reg_bits(EN_RXTSP, enable?1:0);
        this->Modify_SPI_Reg_bits(AGC_MODE_RXTSP, 2); //bypass
        this->Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 1);
        this->Modify_SPI_Reg_bits(AGC_BYP_RXTSP, 1);
        this->Modify_SPI_Reg_bits(GFIR3_BYP_RXTSP, 1);
        this->Modify_SPI_Reg_bits(GFIR2_BYP_RXTSP, 1);
        this->Modify_SPI_Reg_bits(GFIR1_BYP_RXTSP, 1);
        this->Modify_SPI_Reg_bits(DC_BYP_RXTSP, 1);
        this->Modify_SPI_Reg_bits(GC_BYP_RXTSP, 1);
        this->Modify_SPI_Reg_bits(PH_BYP_RXTSP, 1);
    }

    //--- baseband ---
    if (isTx)
    {
        this->Modify_SPI_Reg_bits(EN_DIR_TBB, 1);
        this->Modify_SPI_Reg_bits(EN_G_TBB, enable?1:0);
    }
    else
    {
        this->Modify_SPI_Reg_bits(EN_DIR_RBB, 1);
        this->Modify_SPI_Reg_bits(EN_G_RBB, enable?1:0);
        this->Modify_SPI_Reg_bits(PD_PGA_RBB, enable?0:1);
    }

    //--- frontend ---
    if (isTx)
    {
        this->Modify_SPI_Reg_bits(EN_DIR_TRF, 1);
        this->Modify_SPI_Reg_bits(EN_G_TRF, enable?1:0);
        this->Modify_SPI_Reg_bits(PD_TLOBUF_TRF, enable?0:1);
        this->Modify_SPI_Reg_bits(PD_TXPAD_TRF, enable?0:1);
    }
    else
    {
        this->Modify_SPI_Reg_bits(EN_DIR_RFE, 1);
        this->Modify_SPI_Reg_bits(EN_G_RFE, enable?1:0);
        this->Modify_SPI_Reg_bits(PD_MXLOBUF_RFE, enable?0:1);
        this->Modify_SPI_Reg_bits(PD_QGEN_RFE, enable?0:1);
        this->Modify_SPI_Reg_bits(PD_TIA_RFE, enable?0:1);
    }

    //--- synthesizers ---
    if (isTx)
    {
        this->SetActiveChannel(ChSXT);
        this->Modify_SPI_Reg_bits(EN_DIR_SXRSXT, 1);
        this->Modify_SPI_Reg_bits(EN_G, enable?1:0);
        if (ch == ChB) //enable LO to channel B
        {
            this->SetActiveChannel(ChA);
            this->Modify_SPI_Reg_bits(EN_NEXTTX_TRF, 1);
        }
    }
    else
    {
        this->SetActiveChannel(ChSXR);
        this->Modify_SPI_Reg_bits(EN_DIR_SXRSXT, 1);
        this->Modify_SPI_Reg_bits(EN_G, enable?1:0);
        if (ch == ChB) //enable LO to channel B
        {
            this->SetActiveChannel(ChA);
            this->Modify_SPI_Reg_bits(EN_NEXTRX_RFE, 1);
        }
    }
    this->SetActiveChannel(ch);

    return 0;
}

/*!
 * Helpful macro to check the connection before doing SPI work.
 */
#define checkConnection() { \
    if (controlPort == nullptr) return ReportError(ENOTCONN, "no connection object"); \
    if (not controlPort->IsOpen()) return ReportError(ENOTCONN, "connection is not open"); \
}

/** @brief Sends reset signal to chip, after reset enables B channel controls
    @return 0-success, other-failure
*/
int LMS7002M::ResetChip()
{
    checkConnection();

    int status = controlPort->DeviceReset();
    if (status == 0) Modify_SPI_Reg_bits(LMS7param(MIMO_SISO), 0); //enable B channel after reset
    return status;
}

int LMS7002M::SoftReset()
{
    auto reg_0x0020 = this->SPI_read(0x0020, true);
    auto reg_0x002E = this->SPI_read(0x002E, true);
    this->SPI_write(0x0020, 0x0);
    this->SPI_write(0x0020, reg_0x0020);
    this->SPI_write(0x002E, reg_0x002E);//must write
    return 0;
}

int LMS7002M::LoadConfigLegacyFile(const char* filename)
{
    ifstream f(filename);
    if (f.good() == false) //file not found
    {
        f.close();
        return ReportError(ENOENT, "LoadConfigLegacyFile(%s) - file not found", filename);
    }
    f.close();
    uint16_t addr = 0;
    uint16_t value = 0;
    Channel ch = this->GetActiveChannel(); //remember used channel
    int status;
    typedef INI<string, string, string> ini_t;
    ini_t parser(filename, true);
    if (parser.select("FILE INFO") == false)
        return ReportError(EINVAL, "LoadConfigLegacyFile(%s) - invalid format, missing FILE INFO section", filename);

    string type = "";
    type = parser.get("type", "undefined");
    stringstream ss;
    if (type.find("LMS7002 configuration") == string::npos)
    {
        ss << "File " << filename << " not recognized" << endl;
        return ReportError(EINVAL, "LoadConfigLegacyFile(%s) - invalid format, missing LMS7002 configuration", filename);
    }

    int fileVersion = 0;
    fileVersion = parser.get("version", 0);

    vector<uint16_t> addrToWrite;
    vector<uint16_t> dataToWrite;
    if (fileVersion == 1)
    {
        if (parser.select("Reference clocks"))
        {
            this->SetReferenceClk_SX(Rx, parser.get("SXR reference frequency MHz", 30.72) * 1e6);
            this->SetReferenceClk_SX(Tx, parser.get("SXT reference frequency MHz", 30.72) * 1e6);
        }

        if (parser.select("LMS7002 registers ch.A") == true)
        {
            ini_t::sectionsit_t section = parser.sections.find("LMS7002 registers ch.A");

            uint16_t x0020_value = 0;
            this->SetActiveChannel(ChA); //select A channel
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
            if (status != 0 && controlPort != nullptr)
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
            if (status != 0 && controlPort != nullptr)
                return status;
        }

        this->SetActiveChannel(ChB);

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
            this->SetActiveChannel(ChB); //select B channel
            status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size());
            if (status != 0 && controlPort != nullptr)
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
        this->SetActiveChannel(ch);
        return 0;
    }
    return ReportError(EINVAL, "LoadConfigLegacyFile(%s) - invalid format", filename);
}

/** @brief Reads configuration file and uploads registers to chip
    @param filename Configuration source file
    @return 0-success, other-failure
*/
int LMS7002M::LoadConfig(const char* filename)
{
	ifstream f(filename);
    if (f.good() == false) //file not found
    {
        f.close();
        return ReportError(ENOENT, "LoadConfig(%s) - file not found", filename);
    }
    f.close();
    uint16_t addr = 0;
    uint16_t value = 0;
    Channel ch = this->GetActiveChannel(); //remember used channel

    int status;
    typedef INI<string, string, string> ini_t;
    ini_t parser(filename, true);
    if (parser.select("file_info") == false)
    {
        //try loading as legacy format
        status = LoadConfigLegacyFile(filename);
        this->SetActiveChannel(ChA);
        return status;
    }
    string type = "";
    type = parser.get("type", "undefined");
    stringstream ss;
    if (type.find("lms7002m_minimal_config") == string::npos)
    {
        ss << "File " << filename << " not recognized" << endl;
        return ReportError(EINVAL, "LoadConfig(%s) - invalid format, missing lms7002m_minimal_config", filename);
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
            this->SetActiveChannel(ChA); //select A channel
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
            if (status != 0 && controlPort != nullptr)
                return status;
            status = SPI_write(0x0020, x0020_value);
            if (status != 0 && controlPort != nullptr)
                return status;
            this->SetActiveChannel(ChB);
            if (status != 0 && controlPort != nullptr)
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
            this->SetActiveChannel(ChB); //select B channel
            status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size());
            if (status != 0 && controlPort != nullptr)
                return status;
        }
        this->SetActiveChannel(ch);

        parser.select("reference_clocks");
        this->SetReferenceClk_SX(Rx, parser.get("sxr_ref_clk_mhz", 30.72) * 1e6);
        this->SetReferenceClk_SX(Tx, parser.get("sxt_ref_clk_mhz", 30.72) * 1e6);
    }

    this->SetActiveChannel(ChA);
    checkConnection();
    return 0;
}

/** @brief Reads all registers from chip and saves to file
    @param filename destination filename
    @return 0-success, other failure
*/
int LMS7002M::SaveConfig(const char* filename)
{
    int status;
    ofstream fout;
    fout.open(filename);
    fout << "[file_info]" << endl;
    fout << "type=lms7002m_minimal_config" << endl;
    fout << "version=1" << endl;

    char addr[80];
    char value[80];

    Channel ch = this->GetActiveChannel();

    vector<uint16_t> addrToRead;
    for (uint8_t i = 0; i < MEMORY_SECTIONS_COUNT; ++i)
        for (uint16_t addr = MemorySectionAddresses[i][0]; addr <= MemorySectionAddresses[i][1]; ++addr)
            addrToRead.push_back(addr);
    vector<uint16_t> dataReceived;
    dataReceived.resize(addrToRead.size(), 0);

    fout << "[lms7002_registers_a]" << endl;
    this->SetActiveChannel(ChA);
    for (uint16_t i = 0; i < addrToRead.size(); ++i)
    {
        dataReceived[i] = Get_SPI_Reg_bits(addrToRead[i], 15, 0, false);
        sprintf(addr, "0x%04X", addrToRead[i]);
        sprintf(value, "0x%04X", dataReceived[i]);
        fout << addr << "=" << value << endl;
    }

    fout << "[lms7002_registers_b]" << endl;
    addrToRead.clear(); //add only B channel addresses
    for (uint8_t i = 0; i < MEMORY_SECTIONS_COUNT; ++i)
        for (uint16_t addr = MemorySectionAddresses[i][0]; addr <= MemorySectionAddresses[i][1]; ++addr)
            if (addr >= 0x0100)
                addrToRead.push_back(addr);

    this->SetActiveChannel(ChB);
    for (uint16_t i = 0; i < addrToRead.size(); ++i)
    {
        dataReceived[i] = Get_SPI_Reg_bits(addrToRead[i], 15, 0, false);
        sprintf(addr, "0x%04X", addrToRead[i]);
        sprintf(value, "0x%04X", dataReceived[i]);
        fout << addr << "=" << value << endl;
    }

    this->SetActiveChannel(ch); //retore previously used channel

    fout << "[reference_clocks]" << endl;
    fout << "sxt_ref_clk_mhz=" << this->GetReferenceClk_SX(Tx) / 1e6 << endl;
    fout << "sxr_ref_clk_mhz=" << this->GetReferenceClk_SX(Rx) / 1e6 << endl;
    fout.close();
    return 0;
}

int LMS7002M::SetRBBPGA_dB(const float_type value)
{
    int g_pga_rbb = (int)(value + 12.5);
    if (g_pga_rbb > 0x1f) g_pga_rbb = 0x1f;
    if (g_pga_rbb < 0) g_pga_rbb = 0;
    int ret = this->Modify_SPI_Reg_bits(G_PGA_RBB, g_pga_rbb);

    int rcc_ctl_pga_rbb = (430.0*pow(0.65, (g_pga_rbb/10.0))-110.35)/20.4516 + 16;

    int c_ctl_pga_rbb = 0;
    if (0 <= g_pga_rbb && g_pga_rbb < 8) c_ctl_pga_rbb = 3;
    if (8 <= g_pga_rbb && g_pga_rbb < 13) c_ctl_pga_rbb = 2;
    if (13 <= g_pga_rbb && g_pga_rbb < 21) c_ctl_pga_rbb = 1;
    if (21 <= g_pga_rbb) c_ctl_pga_rbb = 0;

    ret |= this->Modify_SPI_Reg_bits(RCC_CTL_PGA_RBB, rcc_ctl_pga_rbb);
    ret |= this->Modify_SPI_Reg_bits(C_CTL_PGA_RBB, c_ctl_pga_rbb);
    return ret;
}

float_type LMS7002M::GetRBBPGA_dB(void)
{
    auto g_pga_rbb = this->Get_SPI_Reg_bits(G_PGA_RBB);
    return g_pga_rbb - 12;
}

int LMS7002M::SetRFELNA_dB(const float_type value)
{
    const double gmax = 30;
    double val = value - gmax;

    int g_lna_rfe = 0;
    if (val >= 0) g_lna_rfe = 15;
    else if (val >= -1) g_lna_rfe = 14;
    else if (val >= -2) g_lna_rfe = 13;
    else if (val >= -3) g_lna_rfe = 12;
    else if (val >= -4) g_lna_rfe = 11;
    else if (val >= -5) g_lna_rfe = 10;
    else if (val >= -6) g_lna_rfe = 9;
    else if (val >= -9) g_lna_rfe = 8;
    else if (val >= -12) g_lna_rfe = 7;
    else if (val >= -15) g_lna_rfe = 6;
    else if (val >= -18) g_lna_rfe = 5;
    else if (val >= -21) g_lna_rfe = 4;
    else if (val >= -24) g_lna_rfe = 3;
    else if (val >= -27) g_lna_rfe = 2;
    else g_lna_rfe = 1;

    return this->Modify_SPI_Reg_bits(G_LNA_RFE, g_lna_rfe);
}

float_type LMS7002M::GetRFELNA_dB(void)
{
    const double gmax = 30;
    auto g_lna_rfe = this->Get_SPI_Reg_bits(G_LNA_RFE);
    switch (g_lna_rfe)
    {
    case 15: return gmax-0;
    case 14: return gmax-1;
    case 13: return gmax-2;
    case 12: return gmax-3;
    case 11: return gmax-4;
    case 10: return gmax-5;
    case 9: return gmax-6;
    case 8: return gmax-9;
    case 7: return gmax-12;
    case 6: return gmax-15;
    case 5: return gmax-18;
    case 4: return gmax-21;
    case 3: return gmax-24;
    case 2: return gmax-27;
    case 1: return gmax-30;
    }
    return 0.0;
}

int LMS7002M::SetRFELoopbackLNA_dB(const float_type gain)
{
    const double gmax = 40;
    double val = gain - gmax;

    int g_rxloopb_rfe = 0;
    if (val >= 0) g_rxloopb_rfe = 15;
    else if (val >= -0.5) g_rxloopb_rfe = 14;
    else if (val >= -1) g_rxloopb_rfe = 13;
    else if (val >= -1.6) g_rxloopb_rfe = 12;
    else if (val >= -2.4) g_rxloopb_rfe = 11;
    else if (val >= -3) g_rxloopb_rfe = 10;
    else if (val >= -4) g_rxloopb_rfe = 9;
    else if (val >= -5) g_rxloopb_rfe = 8;
    else if (val >= -6.2) g_rxloopb_rfe = 7;
    else if (val >= -7.5) g_rxloopb_rfe = 6;
    else if (val >= -9) g_rxloopb_rfe = 5;
    else if (val >= -11) g_rxloopb_rfe = 4;
    else if (val >= -14) g_rxloopb_rfe = 3;
    else if (val >= -17) g_rxloopb_rfe = 2;
    else if (val >= -24) g_rxloopb_rfe = 1;
    else g_rxloopb_rfe = 0;

    return this->Modify_SPI_Reg_bits(G_RXLOOPB_RFE, g_rxloopb_rfe);
}

float_type LMS7002M::GetRFELoopbackLNA_dB(void)
{
    const double gmax = 40;
    auto g_rxloopb_rfe = this->Get_SPI_Reg_bits(G_RXLOOPB_RFE);
    switch (g_rxloopb_rfe)
    {
    case 15: return gmax-0;
    case 14: return gmax-0.5;
    case 13: return gmax-1;
    case 12: return gmax-1.6;
    case 11: return gmax-2.4;
    case 10: return gmax-3;
    case 9: return gmax-4;
    case 8: return gmax-5;
    case 7: return gmax-6.2;
    case 6: return gmax-7.5;
    case 5: return gmax-9;
    case 4: return gmax-11;
    case 3: return gmax-14;
    case 2: return gmax-17;
    case 1: return gmax-24;
    }
    return 0.0;
}

int LMS7002M::SetRFETIA_dB(const float_type value)
{
    const double gmax = 12;
    double val = value - gmax;

    int g_tia_rfe = 0;
    if (val >= 0) g_tia_rfe = 3;
    else if (val >= -3) g_tia_rfe = 2;
    else g_tia_rfe = 1;

    return this->Modify_SPI_Reg_bits(G_TIA_RFE, g_tia_rfe);
}

float_type LMS7002M::GetRFETIA_dB(void)
{
    const double gmax = 12;
    auto g_tia_rfe = this->Get_SPI_Reg_bits(G_TIA_RFE);
    switch (g_tia_rfe)
    {
    case 3: return gmax-0;
    case 2: return gmax-3;
    case 1: return gmax-12;
    }
    return 0.0;
}

int LMS7002M::SetTRFPAD_dB(const float_type value)
{
    const double pmax = 0;
    double loss = pmax-value;

    //different scaling realm
    if (loss > 10) loss = (loss+10)/2;

    //clip
    if (loss > 31) loss = 31;
    if (loss < 0) loss = 0;

    //integer round
    int loss_int = (int)(loss + 0.5);

    int ret = 0;
    ret |= this->Modify_SPI_Reg_bits(LOSS_LIN_TXPAD_TRF, loss_int);
    ret |= this->Modify_SPI_Reg_bits(LOSS_MAIN_TXPAD_TRF, loss_int);
    return ret;
}

float_type LMS7002M::GetTRFPAD_dB(void)
{
    const double pmax = 0;
    auto loss_int = this->Get_SPI_Reg_bits(LOSS_LIN_TXPAD_TRF);
    if (loss_int > 10) return pmax-10-2*(loss_int-10);
    return pmax-loss_int;
}

int LMS7002M::SetTRFLoopbackPAD_dB(const float_type gain)
{
    //there are 4 discrete gain values, use the midpoints
    int val = 0;
    if      (gain >= (-1.4-0)/2)   val = 0;
    else if (gain >= (-1.4-3.3)/2) val = 1;
    else if (gain >= (-3.3-4.3)/2) val = 2;
    else                           val = 3;

    return this->Modify_SPI_Reg_bits(L_LOOPB_TXPAD_TRF, val);
}

float_type LMS7002M::GetTRFLoopbackPAD_dB(void)
{
    switch (this->Get_SPI_Reg_bits(L_LOOPB_TXPAD_TRF))
    {
    case 0: return 0.0;
    case 1: return -1.4;
    case 2: return -3.3;
    case 3: return -4.3;
    }
    return 0.0;
}

int LMS7002M::SetPathRFE(PathRFE path)
{
    int sel_path_rfe = 0;
    switch (path)
    {
    case PATH_RFE_NONE: sel_path_rfe = 0; break;
    case PATH_RFE_LNAH: sel_path_rfe = 1; break;
    case PATH_RFE_LNAL: sel_path_rfe = 2; break;
    case PATH_RFE_LNAW: sel_path_rfe = 3; break;
    case PATH_RFE_LB1: sel_path_rfe = 3; break;
    case PATH_RFE_LB2: sel_path_rfe = 2; break;
    }

    int pd_lna_rfe = 1;
    switch (path)
    {
    case PATH_RFE_LNAH:
    case PATH_RFE_LNAL:
    case PATH_RFE_LNAW: pd_lna_rfe = 0; break;
    }

    int pd_rloopb_1_rfe = (path == PATH_RFE_LB1)?0:1;
    int pd_rloopb_2_rfe = (path == PATH_RFE_LB2)?0:1;
    int en_inshsw_l_rfe = (path == PATH_RFE_LNAL)?0:1;
    int en_inshsw_w_rfe = (path == PATH_RFE_LNAW)?0:1;
    int en_inshsw_lb1_rfe = (path == PATH_RFE_LB1)?0:1;
    int en_inshsw_lb2_rfe = (path == PATH_RFE_LB2)?0:1;

    this->Modify_SPI_Reg_bits(PD_LNA_RFE, pd_lna_rfe);
    this->Modify_SPI_Reg_bits(PD_RLOOPB_1_RFE, pd_rloopb_1_rfe);
    this->Modify_SPI_Reg_bits(PD_RLOOPB_2_RFE, pd_rloopb_2_rfe);
    this->Modify_SPI_Reg_bits(EN_INSHSW_LB1_RFE, en_inshsw_lb1_rfe);
    this->Modify_SPI_Reg_bits(EN_INSHSW_LB2_RFE, en_inshsw_lb2_rfe);
    this->Modify_SPI_Reg_bits(EN_INSHSW_L_RFE, en_inshsw_l_rfe);
    this->Modify_SPI_Reg_bits(EN_INSHSW_W_RFE, en_inshsw_w_rfe);
    this->Modify_SPI_Reg_bits(SEL_PATH_RFE, sel_path_rfe);

    //enable/disable the loopback path
    const bool loopback = (path == PATH_RFE_LB1) or (path == PATH_RFE_LB2);
    this->Modify_SPI_Reg_bits(EN_LOOPB_TXPAD_TRF, loopback?1:0);

    //update external band-selection to match
    this->UpdateExternalBandSelect();

    return 0;
}

LMS7002M::PathRFE LMS7002M::GetPathRFE(void)
{
    if (this->Get_SPI_Reg_bits(EN_INSHSW_LB1_RFE) != 0) return PATH_RFE_LB1;
    if (this->Get_SPI_Reg_bits(EN_INSHSW_LB2_RFE) != 0) return PATH_RFE_LB2;
    if (this->Get_SPI_Reg_bits(EN_INSHSW_L_RFE) != 0) return PATH_RFE_LNAL;
    if (this->Get_SPI_Reg_bits(EN_INSHSW_W_RFE) != 0) return PATH_RFE_LNAW;
    if (this->Get_SPI_Reg_bits(PD_LNA_RFE) != 0) return PATH_RFE_NONE;
    return PATH_RFE_LNAH;
}

int LMS7002M::SetBandTRF(const int band)
{
    this->Modify_SPI_Reg_bits(SEL_BAND1_TRF, (band==1)?1:0);
    this->Modify_SPI_Reg_bits(SEL_BAND2_TRF, (band==2)?1:0);

    //update external band-selection to match
    this->UpdateExternalBandSelect();

    return 0;
}

int LMS7002M::GetBandTRF(void)
{
    if (this->Get_SPI_Reg_bits(SEL_BAND1_TRF) == 1) return 1;
    if (this->Get_SPI_Reg_bits(SEL_BAND2_TRF) == 1) return 2;
    return 0;
}

void LMS7002M::UpdateExternalBandSelect(void)
{
    return controlPort->UpdateExternalBandSelect(
        this->GetActiveChannelIndex(),
        this->GetBandTRF(),
        int(this->GetPathRFE()));
}

void LMS7002M::SetReferenceClk_SX(bool tx, float_type freq_Hz)
{
    if(controlPort == nullptr)
        return; //TODO return error
    if (tx) controlPort->SetTxReferenceClockRate(freq_Hz);
    else controlPort->SetReferenceClockRate(freq_Hz);
}

/**	@brief Returns reference clock in Hz used for SXT or SXR
	@param Tx transmitter or receiver selection
*/
float_type LMS7002M::GetReferenceClk_SX(bool tx)
{
    if(controlPort == nullptr)
        return 30.72e6; //return default reference clock
    return (tx ? controlPort->GetTxReferenceClockRate() : controlPort->GetReferenceClockRate());
}

/**	@return Current CLKGEN frequency in Hz
    Returned frequency depends on reference clock used for Receiver
*/
float_type LMS7002M::GetFrequencyCGEN()
{
    float_type dMul = (GetReferenceClk_SX(Rx)/2.0)/(Get_SPI_Reg_bits(LMS7param(DIV_OUTCH_CGEN))+1); //DIV_OUTCH_CGEN
    uint16_t gINT = Get_SPI_Reg_bits(0x0088, 13, 0); //read whole register to reduce SPI transfers
    uint32_t gFRAC = ((gINT & 0xF) * 65536) | Get_SPI_Reg_bits(0x0087, 15, 0);
    return dMul * (((gINT>>4) + 1 + gFRAC/1048576.0));
}

/** @brief Returns TSP reference frequency
    @param tx TxTSP or RxTSP selection
    @return TSP reference frequency in Hz
*/
float_type LMS7002M::GetReferenceClk_TSP(bool tx)
{
    float_type cgenFreq = GetFrequencyCGEN();
	float_type clklfreq = cgenFreq/pow(2.0, Get_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN)));
    if(Get_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN)) == 0)
        return tx ? clklfreq : cgenFreq/4.0;
    else
        return tx ? cgenFreq : clklfreq/4.0;
}

/** @brief Sets CLKGEN frequency, calculations use receiver'r reference clock
    @param freq_Hz desired frequency in Hz
    @return 0-succes, other-cannot deliver desired frequency
*/
int LMS7002M::SetFrequencyCGEN(const float_type freq_Hz, const bool retainNCOfrequencies)
{
    LMS7002M_SelfCalState state(this);
    float_type dFvco;
    float_type dFrac;
    int16_t iHdiv;

    //remember NCO frequencies
    Channel chBck = this->GetActiveChannel();
    vector<vector<float_type> > rxNCO(2);
    vector<vector<float_type> > txNCO(2);
    bool rxModeNCO = false;
    bool txModeNCO = false;
    if(retainNCOfrequencies)
    {
        rxModeNCO = Get_SPI_Reg_bits(MODE_RX, true);
        txModeNCO = Get_SPI_Reg_bits(MODE_TX, true);
        for (int ch = 0; ch < 2; ++ch)
        {
            this->SetActiveChannel((ch == 0)?ChA:ChB);
            for (int i = 0; i < 16 && rxModeNCO == 0; ++i)
                rxNCO[ch].push_back(GetNCOFrequency(LMS7002M::Rx, i, false));
            for (int i = 0; i < 16 && txModeNCO == 0; ++i)
                txNCO[ch].push_back(GetNCOFrequency(LMS7002M::Tx, i, false));
        }
    }
    //VCO frequency selection according to F_CLKH
    iHdiv = (int16_t)((gCGEN_VCO_frequencies[1]/ 2) / freq_Hz) - 1;
	dFvco = 2*(iHdiv+1) * freq_Hz;

    //Integer division
    uint16_t gINT = (uint16_t)(dFvco/GetReferenceClk_SX(Rx) - 1);

    //Fractional division
    dFrac = dFvco/GetReferenceClk_SX(Rx) - (uint32_t)(dFvco/GetReferenceClk_SX(Rx));
    uint32_t gFRAC = (uint32_t)(dFrac * 1048576);

    Modify_SPI_Reg_bits(LMS7param(INT_SDM_CGEN), gINT); //INT_SDM_CGEN
    Modify_SPI_Reg_bits(0x0087, 15, 0, gFRAC&0xFFFF); //INT_SDM_CGEN[15:0]
    Modify_SPI_Reg_bits(0x0088, 3, 0, gFRAC>>16); //INT_SDM_CGEN[19:16]
    Modify_SPI_Reg_bits(LMS7param(DIV_OUTCH_CGEN), iHdiv); //DIV_OUTCH_CGEN

    //recalculate NCO
    for (int ch = 0; ch < 2 && retainNCOfrequencies; ++ch)
    {
        this->SetActiveChannel((ch == 0)?ChA:ChB);
        for (int i = 0; i < 16 && rxModeNCO == 0; ++i)
            SetNCOFrequency(LMS7002M::Rx, i, rxNCO[ch][i]);
        for (int i = 0; i < 16 && txModeNCO == 0; ++i)
            SetNCOFrequency(LMS7002M::Tx, i, txNCO[ch][i]);
    }
    this->SetActiveChannel(chBck);
#ifndef NDEBUG
    printf("CGEN: Freq=%g MHz, VCO=%g GHz, INT=%i, FRAC=%i, DIV_OUTCH_CGEN=%i\n", freq_Hz/1e6, dFvco/1e9, gINT, gFRAC, iHdiv);
#endif // NDEBUG
    return TuneVCO(VCO_CGEN);
}

bool LMS7002M::GetCGENLocked(void)
{
    return (Get_SPI_Reg_bits(VCO_CMPHO_CGEN.address, 13, 12, true) & 0x3) == 2;
}

bool LMS7002M::GetSXLocked(bool tx)
{
    SetActiveChannel(tx?ChSXT:ChSXR);
    return (Get_SPI_Reg_bits(VCO_CMPHO.address, 13, 12, true) & 0x3) == 2;
}

/** @brief Performs VCO tuning operations for CLKGEN, SXR, SXT modules
    @param module module selection for tuning 0-cgen, 1-SXR, 2-SXT
    @return 0-success, other-failure
*/
int LMS7002M::TuneVCO(VCO_Module module) // 0-cgen, 1-SXR, 2-SXT
{
    const char* moduleName = (module == VCO_CGEN) ? "CGEN" : ((module == VCO_SXR) ? "SXR" : "SXT");
    checkConnection();
	int8_t i;
	uint8_t cmphl; //comparators
	int16_t csw_lowest = -1;
	uint16_t addrVCOpd; // VCO power down address
	uint16_t addrCSW_VCO;
	uint16_t addrCMP; //comparator address
	uint8_t lsb; //SWC lsb index
	uint8_t msb; //SWC msb index

	Channel ch = this->GetActiveChannel(); //remember used channel

	if(module != VCO_CGEN) //set addresses to SX module
	{
        this->SetActiveChannel(Channel(module));
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
    int status = Modify_SPI_Reg_bits (addrVCOpd, 2, 1, 0); //activate VCO and comparator
    if(status != 0)
        return status;
    if (Get_SPI_Reg_bits(addrVCOpd, 2, 1) != 0)
        return ReportError(-1, "TuneVCO(%s) - VCO is powered down", moduleName);
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
        cmphl = (uint8_t)Get_SPI_Reg_bits(addrCMP, 13, 12, true);
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
                if(Get_SPI_Reg_bits(addrCMP, 13, 12, true) == 0)
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
    cmphl = (uint8_t)Get_SPI_Reg_bits(addrCMP, 13, 12, true);
    this->SetActiveChannel(ch); //restore previously used channel

    if(cmphl == 2) return 0;

    return ReportError(EINVAL, "TuneVCO(%s) - failed to lock (cmphl != 2)", moduleName);
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
int LMS7002M::Modify_SPI_Reg_bits(const LMS7Parameter &param, const uint16_t value, bool fromChip)
{
    return Modify_SPI_Reg_bits(param.address, param.msb, param.lsb, value, fromChip);
}

/** @brief Change given parameter value
    @param address register address
    @param value new bits value, the value is shifted left by lsb bits
    @param fromChip read initial value directly from chip
*/
int LMS7002M::Modify_SPI_Reg_bits(const uint16_t address, const uint8_t msb, const uint8_t lsb, const uint16_t value, bool fromChip)
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
int LMS7002M::Modify_SPI_Reg_mask(const uint16_t *addr, const uint16_t *masks, const uint16_t *values, uint8_t start, uint8_t stop)
{
    int status;
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
    if (status != 0)
        return status;
    SPI_write_batch(&addresses[0], &data[0], addresses.size());
    return status;
}

/** @brief Sets SX frequency
    @param Tx Rx/Tx module selection
    @param freq_Hz desired frequency in Hz
    @return 0-success, other-cannot deliver requested frequency
*/
int LMS7002M::SetFrequencySX(bool tx, float_type freq_Hz)
{
    checkConnection();
    const uint8_t sxVCO_N = 2; //number of entries in VCO frequencies
    const float_type m_dThrF = 5500e6; //threshold to enable additional divider
    float_type VCOfreq;
    int8_t div_loch;
    int8_t sel_vco;
    bool canDeliverFrequency = false;
    uint16_t integerPart;
    uint32_t fractionalPart;
    int8_t i;
    int16_t csw_value;
    uint32_t boardId = controlPort->GetDeviceInfo().boardSerialNumber;

    //find required VCO frequency
    for (div_loch = 6; div_loch >= 0; --div_loch)
    {
        VCOfreq = (1 << (div_loch + 1)) * freq_Hz;
        if ((VCOfreq >= gVCO_frequency_table[0][0]) && (VCOfreq <= gVCO_frequency_table[2][sxVCO_N - 1]))
        {
            canDeliverFrequency = true;
            break;
        }
    }
    if (canDeliverFrequency == false)
        return ReportError(ERANGE, "SetFrequencySX%s(%g MHz) - cannot deliver frequency", tx?"T":"R", freq_Hz / 1e6);

    const float_type refClk_Hz = GetReferenceClk_SX(tx);
    integerPart = (uint16_t)(VCOfreq / (refClk_Hz * (1 + (VCOfreq > m_dThrF))) - 4);
    fractionalPart = (uint32_t)((VCOfreq / (refClk_Hz * (1 + (VCOfreq > m_dThrF))) - (uint32_t)(VCOfreq / (refClk_Hz * (1 + (VCOfreq > m_dThrF))))) * 1048576);

    Channel ch = this->GetActiveChannel();
    this->SetActiveChannel(tx?ChSXT:ChSXR);
    Modify_SPI_Reg_bits(LMS7param(INT_SDM), integerPart); //INT_SDM
    Modify_SPI_Reg_bits(0x011D, 15, 0, fractionalPart & 0xFFFF); //FRAC_SDM[15:0]
    Modify_SPI_Reg_bits(0x011E, 3, 0, (fractionalPart >> 16)); //FRAC_SDM[19:16]
    Modify_SPI_Reg_bits(LMS7param(DIV_LOCH), div_loch); //DIV_LOCH
    Modify_SPI_Reg_bits(LMS7param(EN_DIV2_DIVPROG), (VCOfreq > m_dThrF)); //EN_DIV2_DIVPROG

    //find which VCO supports required frequency
    Modify_SPI_Reg_bits(LMS7param(PD_VCO), 0); //
    Modify_SPI_Reg_bits(LMS7param(PD_VCO_COMP), 0); //

    bool foundInCache = false;
    int vco_query;
    int csw_query;
    if(useCache)
    {
        foundInCache = (valueCache.GetVCO_CSW(boardId, freq_Hz, mdevIndex, tx, &vco_query, &csw_query) == 0);
    }
    if(foundInCache)
    {
        printf("SetFrequency using cache values vco:%i, csw:%i\n", vco_query, csw_query);
        sel_vco = vco_query;
        csw_value = csw_query;
    }
    else
    {
        int cswBackup = Get_SPI_Reg_bits(LMS7param(CSW_VCO)); //remember to restore previous tune value
        canDeliverFrequency = false;
        int tuneScore[] = { -128, -128, -128 }; //best is closest to 0
        for (sel_vco = 0; sel_vco < 3; ++sel_vco)
        {
            Modify_SPI_Reg_bits(LMS7param(SEL_VCO), sel_vco);
            int status = TuneVCO(tx ? VCO_SXT : VCO_SXR);
            int csw = Get_SPI_Reg_bits(LMS7param(CSW_VCO), true);
            tuneScore[sel_vco] = -128 + csw;
            if (status == 0)
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
        csw_value = tuneScore[sel_vco] + 128;
    }
    if(useCache && !foundInCache)
    {
        valueCache.InsertVCO_CSW(boardId, freq_Hz, mdevIndex, tx, sel_vco, csw_value);
    }
    Modify_SPI_Reg_bits(LMS7param(SEL_VCO), sel_vco);
    Modify_SPI_Reg_bits(LMS7param(CSW_VCO), csw_value);
    this->SetActiveChannel(ch); //restore used channel

    if (canDeliverFrequency == false)
        return ReportError(EINVAL, "SetFrequencySX%s(%g MHz) - cannot deliver frequency", tx?"T":"R", freq_Hz / 1e6);
    return 0;
}

/**	@brief Returns currently set SXR/SXT frequency
	@return SX frequency Hz
*/
float_type LMS7002M::GetFrequencySX(bool tx)
{
    Channel ch = this->GetActiveChannel(); //remember previously used channel
	float_type dMul;
	this->SetActiveChannel(tx?ChSXT:ChSXR);
	uint16_t gINT = Get_SPI_Reg_bits(0x011E, 13, 0);	// read whole register to reduce SPI transfers
    uint32_t gFRAC = ((gINT&0xF) * 65536) | Get_SPI_Reg_bits(0x011D, 15, 0);

    const float_type refClk_Hz = GetReferenceClk_SX(tx);
    dMul = (float_type)refClk_Hz / (1 << (Get_SPI_Reg_bits(LMS7param(DIV_LOCH)) + 1));
    //Calculate real frequency according to the calculated parameters
    dMul = dMul * ((gINT >> 4) + 4 + (float_type)gFRAC / 1048576.0) * (Get_SPI_Reg_bits(LMS7param(EN_DIV2_DIVPROG)) + 1);
    this->SetActiveChannel(ch); //restore used channel
	return dMul;
}

/** @brief Sets chosen NCO's frequency
    @param tx transmitter or receiver selection
    @param index NCO index from 0 to 15
    @param freq_Hz desired NCO frequency
    @return 0-success, other-failure
*/
int LMS7002M::SetNCOFrequency(bool tx, uint8_t index, float_type freq_Hz)
{
    if(index > 15)
        return ReportError(ERANGE, "SetNCOFrequency(index = %d) - index out of range [0, 15]", int(index));
    float_type refClk_Hz = GetReferenceClk_TSP(tx);
    uint16_t addr = tx ? 0x0240 : 0x0440;
	uint32_t fcw = (uint32_t)((freq_Hz/refClk_Hz)*4294967296);
    SPI_write(addr+2+index*2, (fcw >> 16)); //NCO frequency control word register MSB part.
    SPI_write(addr+3+index*2, fcw); //NCO frequency control word register LSB part.
    return 0;
}

/** @brief Returns chosen NCO's frequency in Hz
    @param tx transmitter or receiver selection
    @param index NCO index from 0 to 15
    @param fromChip read frequency directly from chip or local registers
    @return NCO frequency in Hz
*/
float_type LMS7002M::GetNCOFrequency(bool tx, uint8_t index, bool fromChip)
{
    if(index > 15)
        return ReportError(ERANGE, "GetNCOFrequency_MHz(index = %d) - index out of range [0, 15]", int(index));
    float_type refClk_Hz = GetReferenceClk_TSP(tx);
    uint16_t addr = tx ? 0x0240 : 0x0440;
    uint32_t fcw = 0;
    fcw |= SPI_read(addr + 2 + index * 2, fromChip) << 16; //NCO frequency control word register MSB part.
    fcw |= SPI_read(addr + 3 + index * 2, fromChip); //NCO frequency control word register LSB part.
    return refClk_Hz*(fcw/4294967296.0);
}

/** @brief Sets chosen NCO phase offset angle when memory table MODE is 0
@param tx transmitter or receiver selection
@param angle_deg phase offset angle in degrees
@return 0-success, other-failure
*/
int LMS7002M::SetNCOPhaseOffsetForMode0(bool tx, float_type angle_deg)
{
    uint16_t addr = tx ? 0x0241 : 0x0441;
    uint16_t pho = (uint16_t)(65536 * (angle_deg / 360 ));
    SPI_write(addr, pho);
    return 0;
}

/** @brief Sets chosen NCO's phase offset angle
    @param tx transmitter or receiver selection
    @param index PHO index from 0 to 15
    @param angle_deg phase offset angle in degrees
    @return 0-success, other-failure
*/
int LMS7002M::SetNCOPhaseOffset(bool tx, uint8_t index, float_type angle_deg)
{
    if(index > 15)
        return ReportError(ERANGE, "SetNCOPhaseOffset(index = %d) - index out of range [0, 15]", int(index));
    uint16_t addr = tx ? 0x0244 : 0x0444;
	uint16_t pho = (uint16_t)(65536*(angle_deg / 360));
    SPI_write(addr+index, pho);
    return 0;
}

/** @brief Returns chosen NCO's phase offset angle in radians
    @param tx transmitter or receiver selection
    @param index PHO index from 0 to 15
    @return phase offset angle in degrees
*/
float_type LMS7002M::GetNCOPhaseOffset_Deg(bool tx, uint8_t index)
{
    if(index > 15)
        return ReportError(ERANGE, "GetNCOPhaseOffset_Deg(index = %d) - index out of range [0, 15]", int(index));
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
int LMS7002M::SetGFIRCoefficients(bool tx, uint8_t GFIR_index, const int16_t *coef, uint8_t coefCount)
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
        return ReportError(ERANGE, "SetGFIRCoefficients(coefCount=%d) - exceeds coefLimit=%d", int(coefCount), int(coefLimit));
    vector<uint16_t> addresses;
    for (index = 0; index < coefCount; ++index)
        addresses.push_back(startAddr + index + 24 * (index / 40));
    SPI_write_batch(&addresses[0], (uint16_t*)coef, coefCount);
    return 0;
}

/** @brief Returns currently loaded FIR coefficients
    @param tx Transmitter or receiver selection
    @param GFIR_index GIR index from 0 to 2
    @param coef array of returned coefficients
    @param coefCount number of coefficients to read
    @return 0-success, other-failure
*/
int LMS7002M::GetGFIRCoefficients(bool tx, uint8_t GFIR_index, int16_t *coef, uint8_t coefCount)
{
    checkConnection();

    int status = -1;
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
        return ReportError(ERANGE, "GetGFIRCoefficients(coefCount=%d) - exceeds coefLimit=%d", int(coefCount), int(coefLimit));

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
        status = 0;
    }

    return status;
}

/** @brief Write given data value to whole register
    @param address SPI address
    @param data new register value
    @return 0-succes, other-failure
*/
int LMS7002M::SPI_write(uint16_t address, uint16_t data)
{
    return this->SPI_write_batch(&address, &data, 1);
}

/** @brief Reads whole register value from given address
    @param address SPI address
    @param status operation status(optional)
    @param fromChip read value directly from chip
    @return register value
*/
uint16_t LMS7002M::SPI_read(uint16_t address, bool fromChip, int *status)
{
    if (!controlPort || fromChip == false)
    {
        if (status && !controlPort)
            *status = ReportError(ENOTCONN, "chip not connected");
        int mac = mRegistersMap->GetValue(0, LMS7param(MAC).address) & 0x0003;
        int regNo = (mac == 2)? 1 : 0; //only when MAC is B -> use register space B
        if (address < 0x0100) regNo = 0; //force A when below MAC mapped register space
        return mRegistersMap->GetValue(regNo, address);
    }
    if(controlPort)
    {
        uint16_t data = 0;
        int st = this->SPI_read_batch(&address, &data, 1);
        if (status != nullptr) *status = st;
        return data;
    }
    return 0;
}

/** @brief Batches multiple register writes into least ammount of transactions
    @param spiAddr spi register addresses to be written
    @param spiData registers data to be written
    @param cnt number of registers to write
    @return 0-success, other-failure
*/
int LMS7002M::SPI_write_batch(const uint16_t* spiAddr, const uint16_t* spiData, uint16_t cnt)
{
    int mac = mRegistersMap->GetValue(0, LMS7param(MAC).address) & 0x0003;
    std::vector<uint32_t> data(cnt);
    for (size_t i = 0; i < cnt; ++i)
    {
        data[i] = (1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]; //msbit 1=SPI write

        //write which register cache based on MAC bits
        //or always when below the MAC mapped register space
        bool wr0 = ((mac & 0x1) != 0) or (spiAddr[i] < 0x0100);
        bool wr1 = ((mac & 0x2) != 0) or (spiAddr[i] < 0x0100);

        if (wr0) mRegistersMap->SetValue(0, spiAddr[i], spiData[i]);
        if (wr1) mRegistersMap->SetValue(1, spiAddr[i], spiData[i]);

        //refresh mac, because batch might also change active channel
        if(spiAddr[i] == LMS7param(MAC).address)
            mac = mRegistersMap->GetValue(0, LMS7param(MAC).address) & 0x0003;
    }

    checkConnection();

    return controlPort->TransactSPI(addrLMS7002M, data.data(), nullptr, cnt);
}

/** @brief Batches multiple register reads into least amount of transactions
    @param spiAddr SPI addresses to read
    @param spiData array for read data
    @param cnt number of registers to read
    @return 0-success, other-failure
*/
int LMS7002M::SPI_read_batch(const uint16_t* spiAddr, uint16_t* spiData, uint16_t cnt)
{
    checkConnection();

    std::vector<uint32_t> dataWr(cnt);
    std::vector<uint32_t> dataRd(cnt);
    for (size_t i = 0; i < cnt; ++i)
    {
        dataWr[i] = (uint32_t(spiAddr[i]) << 16);
    }

    int status = controlPort->TransactSPI(addrLMS7002M, dataWr.data(), dataRd.data(), cnt);
    if (status != 0) return status;

    int mac = mRegistersMap->GetValue(0, LMS7param(MAC).address) & 0x0003;

    for (size_t i = 0; i < cnt; ++i)
    {
        spiData[i] = dataRd[i] & 0xffff;

        //write which register cache based on MAC bits
        //or always when below the MAC mapped register space
        bool wr0 = ((mac & 0x1) != 0) or (spiAddr[i] < 0x0100);
        bool wr1 = ((mac & 0x2) != 0) or (spiAddr[i] < 0x0100);

        if (wr0) mRegistersMap->SetValue(0, spiAddr[i], spiData[i]);
        if (wr1) mRegistersMap->SetValue(1, spiAddr[i], spiData[i]);
    }
    return 0;
}

/** @brief Performs registers test by writing known data and confirming readback data
    @return 0-registers test passed, other-failure
*/
int LMS7002M::RegistersTest()
{
    char chex[16];
    checkConnection();

    int status;
    Channel ch = this->GetActiveChannel();

    //backup both channel data for restoration after test
    vector<uint16_t> ch1Addresses;
    for (uint8_t i = 0; i < MEMORY_SECTIONS_COUNT; ++i)
        for (uint16_t addr = MemorySectionAddresses[i][0]; addr <= MemorySectionAddresses[i][1]; ++addr)
            ch1Addresses.push_back(addr);
    vector<uint16_t> ch1Data;
    ch1Data.resize(ch1Addresses.size(), 0);

    //backup A channel
    this->SetActiveChannel(ChA);
    status = SPI_read_batch(&ch1Addresses[0], &ch1Data[0], ch1Addresses.size());
    if (status != 0)
        return status;

    vector<uint16_t> ch2Addresses;
    for (uint8_t i = 0; i < MEMORY_SECTIONS_COUNT; ++i)
        for (uint16_t addr = MemorySectionAddresses[i][0]; addr <= MemorySectionAddresses[i][1]; ++addr)
            if (addr >= 0x0100)
                ch2Addresses.push_back(addr);
    vector<uint16_t> ch2Data;
    ch2Data.resize(ch2Addresses.size(), 0);

    this->SetActiveChannel(ChB);
    status = SPI_read_batch(&ch2Addresses[0], &ch2Data[0], ch2Addresses.size());
    if (status != 0)
        return status;

    //test registers
    ResetChip();
    Modify_SPI_Reg_bits(LMS7param(MIMO_SISO), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0);
    this->SetActiveChannel(ChA);

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
                moduleTestsSuccess &= RegistersTestInterval(startAddr, endAddr, patterns[p], ss) == 0;
        }
        allTestSuccess &= moduleTestsSuccess;
    }

    //restore register values
    this->SetActiveChannel(ChA);
    SPI_write_batch(&ch1Addresses[0], &ch1Data[0], ch1Addresses.size());
    this->SetActiveChannel(ChB);
    SPI_write_batch(&ch2Addresses[0], &ch2Data[0], ch2Addresses.size());
    this->SetActiveChannel(ch);

    fstream fout;
    fout.open("registersTest.txt", ios::out);
    fout << ss.str() << endl;
    fout.close();

    if (allTestSuccess) return 0;
    return ReportError(-1, "RegistersTest() failed - %s", GetLastErrorMessage());
}

/** @brief Performs registers test for given address interval by writing given pattern data
    @param startAddr first register address
    @param endAddr last reigster address
    @param pattern data to be written into registers
    @return 0-register test passed, other-failure
*/
int LMS7002M::RegistersTestInterval(uint16_t startAddr, uint16_t endAddr, uint16_t pattern, stringstream &ss)
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
    int status;
    status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size());
    if (status != 0)
        return status;
    status = SPI_read_batch(&addrToWrite[0], &dataReceived[0], addrToWrite.size());
    if (status != 0)
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
    if (registersMatch) return 0;
    return ReportError(-1, "RegistersTestInterval(startAddr=0x%x, endAddr=0x%x) - failed", startAddr, endAddr);
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

/** @brief Sets given module registers to default values
    @return 0-success, other-failure
*/
int LMS7002M::SetDefaults(MemorySection module)
{
    int status = 0;
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

/** @brief Reads all chip configuration and checks if it matches with local registers copy
*/
bool LMS7002M::IsSynced()
{
    if (!controlPort || controlPort->IsOpen() == false)
        return false;
    bool isSynced = true;
    int status;

    Channel ch = this->GetActiveChannel();

    vector<uint16_t> addrToRead = mRegistersMap->GetUsedAddresses(0);
    vector<uint16_t> dataReceived;
    dataReceived.resize(addrToRead.size(), 0);

    this->SetActiveChannel(ChA);
    status = SPI_read_batch(&addrToRead[0], &dataReceived[0], addrToRead.size());
    if (status != 0)
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

    this->SetActiveChannel(ChB);
    status = SPI_read_batch(&addrToRead[0], &dataReceived[0], addrToRead.size());
    if (status != 0)
        return false;
    //check if local copy matches chip
    for (uint16_t i = 0; i < addrToRead.size(); ++i)
        if (dataReceived[i] != mRegistersMap->GetValue(1, addrToRead[i]))
        {
            isSynced = false;
            break;
        }

isSyncedEnding:
    this->SetActiveChannel(ch); //restore previously used channel
    return isSynced;
}

/** @brief Writes all registers from host to chip

*/
int LMS7002M::UploadAll()
{
    checkConnection();

    Channel ch = this->GetActiveChannel(); //remember used channel

    int status;

    vector<uint16_t> addrToWrite;
    vector<uint16_t> dataToWrite;

    uint16_t x0020_value = mRegistersMap->GetValue(0, 0x0020);
    this->SetActiveChannel(ChA); //select A channel

    addrToWrite = mRegistersMap->GetUsedAddresses(0);
    //remove 0x0020 register from list, to not change MAC
    addrToWrite.erase( find(addrToWrite.begin(), addrToWrite.end(), 0x0020) );
    for (auto address : addrToWrite)
        dataToWrite.push_back(mRegistersMap->GetValue(0, address));

    status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size());
    if (status != 0)
        return status;
    //after all channel A registers have been written, update 0x0020 register value
    status = SPI_write(0x0020, x0020_value);
    if (status != 0)
        return status;
    this->SetActiveChannel(ChB);
    if (status != 0)
        return status;

    addrToWrite = mRegistersMap->GetUsedAddresses(1);
    dataToWrite.clear();
    for (auto address : addrToWrite)
    {
        dataToWrite.push_back(mRegistersMap->GetValue(1, address));
    }
    this->SetActiveChannel(ChB); //select B channel
    status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size());
    if (status != 0)
        return status;
    this->SetActiveChannel(ch); //restore last used channel

    //update external band-selection to match
    this->UpdateExternalBandSelect();

    return 0;
}

/** @brief Reads all registers from the chip to host

*/
int LMS7002M::DownloadAll()
{
    checkConnection();
    int status;
    Channel ch = this->GetActiveChannel(false);

    vector<uint16_t> addrToRead = mRegistersMap->GetUsedAddresses(0);
    vector<uint16_t> dataReceived;
    dataReceived.resize(addrToRead.size(), 0);
    this->SetActiveChannel(ChA);
    status = SPI_read_batch(&addrToRead[0], &dataReceived[0], addrToRead.size());
    if (status != 0)
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

    this->SetActiveChannel(ChB);
    status = SPI_read_batch(&addrToRead[0], &dataReceived[0], addrToRead.size());
    if (status != 0)
        return status;
    for (uint16_t i = 0; i < addrToRead.size(); ++i)
        mRegistersMap->SetValue(1, addrToRead[i], dataReceived[i]);

    this->SetActiveChannel(ch); //retore previously used channel

    //update external band-selection to match
    this->UpdateExternalBandSelect();

    return 0;
}

/** @brief Configures interfaces for desired frequency
    @return 0-success, other-failure
    Sets interpolation and decimation, changes MCLK sources and TSP clock dividers accordingly to selected interpolation and decimation
*/
int LMS7002M::SetInterfaceFrequency(float_type cgen_freq_Hz, const uint8_t interpolation, const uint8_t decimation)
{
    int status = 0;
    LMS7002M_SelfCalState state(this);
    status = Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), decimation);
    if(status != 0)
        return status;
    Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), interpolation);

    //clock rate already set because the readback frequency is pretty-close,
    //dont set the cgen frequency again to save time due to VCO selection
    const auto freqDiff = std::abs(this->GetFrequencyCGEN() - cgen_freq_Hz);
    if (not this->GetCGENLocked() or freqDiff > 10.0)
    {
        status = SetFrequencyCGEN(cgen_freq_Hz);
        if (status != 0) return status;
    }

    int mclk2src = Get_SPI_Reg_bits(LMS7param(MCLK2SRC));
    if (decimation == 7 || decimation == 0) //bypass
    {
        Modify_SPI_Reg_bits(LMS7param(RXTSPCLKA_DIV), 0);
        Modify_SPI_Reg_bits(LMS7param(RXDIVEN), false);
        Modify_SPI_Reg_bits(LMS7param(MCLK2SRC), mclk2src & 1 | 0x2);
    }
    else
    {
        uint8_t divider = (uint8_t)pow(2.0, decimation);
        if (divider > 1)
            Modify_SPI_Reg_bits(LMS7param(RXTSPCLKA_DIV), (divider / 2) - 1);
        else
            Modify_SPI_Reg_bits(LMS7param(RXTSPCLKA_DIV), 0);
        Modify_SPI_Reg_bits(LMS7param(RXDIVEN), true);
        Modify_SPI_Reg_bits(LMS7param(MCLK2SRC), mclk2src & 1);
    }
    int mclk1src = Get_SPI_Reg_bits(LMS7param(MCLK1SRC));
    if (interpolation == 7 || interpolation == 0) //bypass
    {
        Modify_SPI_Reg_bits(LMS7param(TXTSPCLKA_DIV), 0);
        Modify_SPI_Reg_bits(LMS7param(TXDIVEN), false);
        Modify_SPI_Reg_bits(LMS7param(MCLK1SRC), mclk1src & 1 | 0x2);
    }
    else
    {
        uint8_t divider = (uint8_t)pow(2.0, interpolation);
        if (divider > 1)
            Modify_SPI_Reg_bits(LMS7param(TXTSPCLKA_DIV), (divider / 2) - 1);
        else
            Modify_SPI_Reg_bits(LMS7param(TXTSPCLKA_DIV), 0);
        Modify_SPI_Reg_bits(LMS7param(TXDIVEN), true);
        Modify_SPI_Reg_bits(LMS7param(MCLK1SRC), mclk1src & 1);
    }

    return status;
}

float_type LMS7002M::GetSampleRate(bool tx)
{
    //if decimation/interpolation is 0(2^1) or 7(bypass), interface clocks should not be divided
    if (tx)
    {
        int interpolation = Get_SPI_Reg_bits(HBI_OVR_TXTSP);
        float_type interfaceTx_Hz = GetReferenceClk_TSP(LMS7002M::Tx);
        if (interpolation != 7)
            interfaceTx_Hz /= 2*pow(2.0, interpolation);
        return interfaceTx_Hz;
    }
    else
    {
        int decimation = Get_SPI_Reg_bits(HBD_OVR_RXTSP);
        float_type interfaceRx_Hz = GetReferenceClk_TSP(LMS7002M::Rx);
        if (decimation != 7)
            interfaceRx_Hz /= 2*pow(2.0, decimation);
        return interfaceRx_Hz;
    }
}

void LMS7002M::ConfigureLML_RF2BB(
    const LMLSampleSource s0,
    const LMLSampleSource s1,
    const LMLSampleSource s2,
    const LMLSampleSource s3)
{
    //map a sample source to a position
    std::map<LMLSampleSource, int> m;
    m[AI] = 1;
    m[AQ] = 0;
    m[BI] = 3;
    m[BQ] = 2;

    //load the same config on both LMLs
    //only one will get used based on direction
    this->Modify_SPI_Reg_bits(LML1_S3S, m[s3]);
    this->Modify_SPI_Reg_bits(LML1_S2S, m[s2]);
    this->Modify_SPI_Reg_bits(LML1_S1S, m[s1]);
    this->Modify_SPI_Reg_bits(LML1_S0S, m[s0]);

    this->Modify_SPI_Reg_bits(LML2_S3S, m[s3]);
    this->Modify_SPI_Reg_bits(LML2_S2S, m[s2]);
    this->Modify_SPI_Reg_bits(LML2_S1S, m[s1]);
    this->Modify_SPI_Reg_bits(LML2_S0S, m[s0]);
}

void LMS7002M::ConfigureLML_BB2RF(
    const LMLSampleSource s0,
    const LMLSampleSource s1,
    const LMLSampleSource s2,
    const LMLSampleSource s3)
{
    //map a sample source to a position
    std::map<LMLSampleSource, int> m;
    m[s3] = 2;
    m[s2] = 3;
    m[s0] = 1;
    m[s1] = 0;

    //load the same config on both LMLs
    //only one will get used based on direction
    this->Modify_SPI_Reg_bits(LML1_BQP, m[BQ]);
    this->Modify_SPI_Reg_bits(LML1_BIP, m[BI]);
    this->Modify_SPI_Reg_bits(LML1_AQP, m[AQ]);
    this->Modify_SPI_Reg_bits(LML1_AIP, m[AI]);

    this->Modify_SPI_Reg_bits(LML2_BQP, m[BQ]);
    this->Modify_SPI_Reg_bits(LML2_BIP, m[BI]);
    this->Modify_SPI_Reg_bits(LML2_AQP, m[AQ]);
    this->Modify_SPI_Reg_bits(LML2_AIP, m[AI]);
}

int LMS7002M::SetRxDCRemoval(const bool enable)
{
    this->Modify_SPI_Reg_bits(DC_BYP_RXTSP, enable?0:1);
    this->Modify_SPI_Reg_bits(DCCORR_AVG_RXTSP, 0x7);
    return 0;
}

bool LMS7002M::GetRxDCRemoval(void)
{
    return this->Get_SPI_Reg_bits(DC_BYP_RXTSP) == 0;
}

int LMS7002M::SetTxDCOffset(const float_type I, const float_type Q)
{
    const bool bypass = I == 0.0 and Q == 0.0;
    this->Modify_SPI_Reg_bits(DC_BYP_RXTSP, bypass?1:0);
    this->Modify_SPI_Reg_bits(DCCORRI_TXTSP, std::lrint(I*128));
    this->Modify_SPI_Reg_bits(DCCORRQ_TXTSP, std::lrint(Q*128));
	return 0;
}

void LMS7002M::GetTxDCOffset(float_type &I, float_type &Q)
{
    I = int8_t(this->Get_SPI_Reg_bits(DCCORRI_TXTSP))/128.0; //signed 8-bit
    Q = int8_t(this->Get_SPI_Reg_bits(DCCORRQ_TXTSP))/128.0; //signed 8-bit
}

int LMS7002M::SetIQBalance(const bool tx, const float_type phase, const float_type gainI, const float_type gainQ)
{
    const bool bypassPhase = (phase == 0.0);
    const bool bypassGain = ((gainI == 1.0) and (gainQ == 1.0)) or ((gainI == 0.0) and (gainQ == 0.0));
    int iqcorr = std::lrint(2047*(phase/(M_PI/2)));
    int gcorri = std::lrint(2047*gainI);
    int gcorrq = std::lrint(2047*gainQ);

    this->Modify_SPI_Reg_bits(tx?PH_BYP_TXTSP:PH_BYP_RXTSP, bypassPhase?1:0);
    this->Modify_SPI_Reg_bits(tx?GC_BYP_TXTSP:GC_BYP_RXTSP, bypassGain?1:0);
    this->Modify_SPI_Reg_bits(tx?IQCORR_TXTSP:IQCORR_RXTSP, iqcorr);
    this->Modify_SPI_Reg_bits(tx?GCORRI_TXTSP:GCORRI_RXTSP, gcorri);
    this->Modify_SPI_Reg_bits(tx?GCORRQ_TXTSP:GCORRQ_RXTSP, gcorrq);
	return 0;
}

void LMS7002M::GetIQBalance(const bool tx, float_type &phase, float_type &gainI, float_type &gainQ)
{
    int iqcorr = int16_t(this->Get_SPI_Reg_bits(tx?IQCORR_TXTSP:IQCORR_RXTSP) << 4) >> 4; //sign extend 12-bit
    int gcorri = int16_t(this->Get_SPI_Reg_bits(tx?GCORRI_TXTSP:GCORRI_RXTSP)); //unsigned 11-bit
    int gcorrq = int16_t(this->Get_SPI_Reg_bits(tx?GCORRQ_TXTSP:GCORRQ_RXTSP)); //unsigned 11-bit

    phase = (M_PI/2)*iqcorr/2047.0;
    gainI = gcorri/2047.0;
    gainQ = gcorrq/2047.0;
}

void LMS7002M::EnterSelfCalibration(void)
{
    if (controlPort && mSelfCalDepth == 0)
    {
        controlPort->EnterSelfCalibration(this->GetActiveChannelIndex());
    }
    mSelfCalDepth++;
}

void LMS7002M::ExitSelfCalibration(void)
{
    mSelfCalDepth--;
    if (controlPort && mSelfCalDepth == 0)
        controlPort->ExitSelfCalibration(this->GetActiveChannelIndex());
}

LMS7002M_SelfCalState::LMS7002M_SelfCalState(LMS7002M *rfic):
    rfic(rfic)
{
    rfic->EnterSelfCalibration();
}

LMS7002M_SelfCalState::~LMS7002M_SelfCalState(void)
{
    rfic->ExitSelfCalibration();
}

void LMS7002M::EnableValuesCache(bool enabled)
{
    useCache = enabled;
}

bool LMS7002M::IsValuesCacheEnabled()
{
    return useCache;
}

MCU_BD* LMS7002M::GetMCUControls() const
{
    return mcuControl;
}

void LMS7002M::EnableCalibrationByMCU(bool enabled)
{
    mCalibrationByMCU = enabled;
}

float_type LMS7002M::GetTemperature()
{
    auto ch = GetActiveChannel();
    auto regMap = BackupRegisterMap();
    Modify_SPI_Reg_bits(MAC, 1);

    //RFE
    Modify_SPI_Reg_bits(EN_G_RFE, 0);
    //RBB
    Modify_SPI_Reg_bits(EN_G_RBB, 0);
    //AFE
    Modify_SPI_Reg_bits(PD_RX_AFE1, 0);
    Modify_SPI_Reg_bits(MUX_AFE_1, 2);
    Modify_SPI_Reg_bits(EN_G_AFE, 1);
    //BIAS
    Modify_SPI_Reg_bits(MUX_BIAS_OUT, 2);
    //RxTSP
    SetDefaults(RxTSP);
    Modify_SPI_Reg_bits(DC_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(GFIR3_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(GFIR2_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(GFIR1_BYP_RXTSP, 1);
    Modify_SPI_Reg_bits(GCORRI_RXTSP, 0);
    Modify_SPI_Reg_bits(AGC_AVG_RXTSP, 7);
    Modify_SPI_Reg_bits(AGC_MODE_RXTSP, 1);

    //READ ADCQ value
    Modify_SPI_Reg_bits(CAPSEL, 1);
    Modify_SPI_Reg_bits(CAPTURE, 0);
    int negativeCount = 0;
    int signMeasureCount = 19;
    int sign = -1;
    for(int i = 0; i < signMeasureCount; ++i)
    {
        Modify_SPI_Reg_bits(CAPTURE, 1);
        Modify_SPI_Reg_bits(CAPTURE, 0);
        int16_t adcq = SPI_read(0x040F, true);
        if(adcq & 0x200)
            ++negativeCount;
    }
    if(negativeCount > signMeasureCount / 2)
        sign = 1;

    Modify_SPI_Reg_bits(CAPSEL, 0);
    Modify_SPI_Reg_bits(CAPTURE, 0);
    Modify_SPI_Reg_bits(CAPTURE, 1);
    uint32_t rssi = (Get_SPI_Reg_bits(0x040F, 15, 0, true) << 2) | Get_SPI_Reg_bits(0x040E, 1, 0, true);
    double temperature = (rssi / 16.0) * 0.443892 * sign + 40.5;

    RestoreRegisterMap(regMap);
    return temperature;
}
