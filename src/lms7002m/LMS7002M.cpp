/**
@file LMS7002M.cpp
@author Lime Microsystems (www.limemicro.com)
@brief Implementation of LMS7002M transceiver configuring
*/

#include "limesuite/LMS7002M.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <ciso646>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <thread>

#include "cpp-feather-ini-parser/INI.h"
#include "limesuite/IComms.h"
#include "limesuite/commonTypes.h"
#include "lms_gfir.h"
#include "LMS7002M_RegistersMap.h"
#include "Logger.h"
#include "mcu_programs.h"
#include "MCU_BD.h"

using namespace lime;

constexpr std::array<std::array<float_type, 2>, 3> LMS7002M::gVCO_frequency_table{
    { { 3800e6, 5222e6 }, { 4961e6, 6754e6 }, { 6306e6, 7714e6 } }
};
constexpr std::array<float_type, 2> LMS7002M::gCGEN_VCO_frequencies{ 1930e6, 2940e6 };

/// Define for parameter enumeration if prefix might be needed
extern std::vector<std::reference_wrapper<const LMS7Parameter>> LMS7parameterList;

// Module addresses needs to be sorted in ascending order
const std::vector<LMS7002M::ReadOnlyRegister> LMS7002M::readOnlyRegisters{
    { 0x002F, 0x0000 },
    { 0x008C, 0x0FFF },
    { 0x00A8, 0x007F },
    { 0x00A9, 0x0000 },
    { 0x00AA, 0x0000 },
    { 0x00AB, 0x0000 },
    { 0x00AC, 0x0000 },
    { 0x0123, 0x003F },
    { 0x0209, 0x0000 },
    { 0x020A, 0x0000 },
    { 0x020B, 0x0000 },
    { 0x040E, 0x0000 },
    { 0x040F, 0x0000 },
};

const std::map<LMS7002M::MemorySection, std::array<uint16_t, 2>> LMS7002M::MemorySectionAddresses{
    { LMS7002M::MemorySection::LimeLight, { 0x0020, 0x002F } },
    { LMS7002M::MemorySection::EN_DIR, { 0x0081, 0x0081 } },
    { LMS7002M::MemorySection::AFE, { 0x0082, 0x0082 } },
    { LMS7002M::MemorySection::BIAS, { 0x0084, 0x0084 } },
    { LMS7002M::MemorySection::XBUF, { 0x0085, 0x0085 } },
    { LMS7002M::MemorySection::CGEN, { 0x0086, 0x008C } },
    { LMS7002M::MemorySection::LDO, { 0x0092, 0x00A7 } },
    { LMS7002M::MemorySection::BIST, { 0x00A8, 0x00AC } },
    { LMS7002M::MemorySection::CDS, { 0x00AD, 0x00AE } },
    { LMS7002M::MemorySection::TRF, { 0x0100, 0x0104 } },
    { LMS7002M::MemorySection::TBB, { 0x0105, 0x010B } },
    { LMS7002M::MemorySection::RFE, { 0x010C, 0x0114 } },
    { LMS7002M::MemorySection::RBB, { 0x0115, 0x011A } },
    { LMS7002M::MemorySection::SX, { 0x011C, 0x0124 } },
    { LMS7002M::MemorySection::TRX_GAIN, { 0x0125, 0x0126 } },
    { LMS7002M::MemorySection::TxTSP, { 0x0200, 0x020C } },
    { LMS7002M::MemorySection::TxNCO, { 0x0240, 0x0261 } },
    { LMS7002M::MemorySection::TxGFIR1, { 0x0280, 0x02A7 } },
    { LMS7002M::MemorySection::TxGFIR2, { 0x02C0, 0x02E7 } },
    { LMS7002M::MemorySection::TxGFIR3a, { 0x0300, 0x0327 } },
    { LMS7002M::MemorySection::TxGFIR3b, { 0x0340, 0x0367 } },
    { LMS7002M::MemorySection::TxGFIR3c, { 0x0380, 0x03A7 } },
    { LMS7002M::MemorySection::RxTSP, { 0x0400, 0x040F } },
    { LMS7002M::MemorySection::RxNCO, { 0x0440, 0x0461 } },
    { LMS7002M::MemorySection::RxGFIR1, { 0x0480, 0x04A7 } },
    { LMS7002M::MemorySection::RxGFIR2, { 0x04C0, 0x04E7 } },
    { LMS7002M::MemorySection::RxGFIR3a, { 0x0500, 0x0527 } },
    { LMS7002M::MemorySection::RxGFIR3b, { 0x0540, 0x0567 } },
    { LMS7002M::MemorySection::RxGFIR3c, { 0x0580, 0x05A7 } },
    { LMS7002M::MemorySection::RSSI_DC_CALIBRATION, { 0x05C0, 0x05CC } },
    { LMS7002M::MemorySection::RSSI_PDET_TEMP_CONFIG, { 0x0600, 0x0606 } },
    { LMS7002M::MemorySection::RSSI_DC_CONFIG, { 0x0640, 0x0641 } },
};

/** @brief Switches LMS7002M SPI to requested channel and restores previous channel when going out of scope */
class ChannelScope
{
  public:
    /**
   * @brief Saves the current channel and restores it at scope exit.
   * @param chip The chip to use.
   * @param useCache Whether to use caching or not.
   */
    ChannelScope(LMS7002M* chip, bool useCache = false)
        : mChip(chip)
        , mStoredValue(chip->GetActiveChannel(!useCache))
        , mNeedsRestore(true)
    {
    }

    /**
      @brief Convenient constructor when using explicit MAC value.
      @param chip The chip to use.
      @param mac The channel to use.
      @param useCache Whether to use caching or not.
     */
    ChannelScope(LMS7002M* chip, LMS7002M::Channel mac, bool useCache = false)
        : mChip(chip)
        , mStoredValue(chip->GetActiveChannel(!useCache))
        , mNeedsRestore(false)
    {
        if (mStoredValue == mac)
            return;

        mChip->SetActiveChannel(mac);
        mNeedsRestore = true;
    }

    /**
      @brief Convenient constructor when using channel index starting from 0.
      @param chip The chip to use.
      @param index The channel index.
      @param useCache Whether to use caching or not.
     */
    ChannelScope(LMS7002M* chip, uint8_t index, bool useCache = false)
        : mChip(chip)
        , mNeedsRestore(false)
    {
        assert(index < 2);
        mStoredValue = chip->GetActiveChannel(!useCache);
        auto expectedChannel = index > 0 ? LMS7002M::Channel::ChB : LMS7002M::Channel::ChA;
        if (mStoredValue == expectedChannel)
            return;

        mChip->SetActiveChannel(expectedChannel);
        mNeedsRestore = true;
    }

    /** @brief Destroy the Channel Scope object and reset the active channel. */
    ~ChannelScope()
    {
        if (mNeedsRestore)
            mChip->SetActiveChannel(mStoredValue);
    }

  private:
    LMS7002M* mChip; ///< The chip to modify
    LMS7002M::Channel mStoredValue; ///< The channel to restore to
    bool mNeedsRestore; ///< Whether the channel needs restoring or not
};

/** @brief Simple logging function to print status messages
    @param text message to print
    @param type message type for filtering specific information
*/
void LMS7002M::Log(const char* text, LogType type)
{
    switch (type)
    {
    case LogType::LOG_INFO:
        lime::info(text);
        if (log_callback)
            log_callback(text, type);
        break;
    case LogType::LOG_WARNING:
        lime::warning(text);
        if (log_callback)
            log_callback(text, type);
        break;
    case LogType::LOG_ERROR:
        lime::error(text);
        if (log_callback)
            log_callback(text, type);
        break;
    case LogType::LOG_DATA:
        lime::debug(text);
        if (log_callback)
            log_callback(text, type);
        break;
    }
}

//Compatibility for vasprintf under MSVC
#ifdef _MSC_VER
int vasprintf(char** strp, const char* fmt, va_list ap)
{
    int r = _vscprintf(fmt, ap);
    if (r < 0)
        return r;
    *strp = (char*)malloc(r + 1);
    return vsprintf_s(*strp, r + 1, fmt, ap);
}
#endif

void LMS7002M::Log(LogType type, const char* format, va_list argList)
{
    char* message = NULL;
    if (vasprintf(&message, format, argList) != -1)
    {
        Log(message, type);
        free(message);
    }
}

/** @brief Sets connection which is used for data communication with chip
*/
void LMS7002M::SetConnection(std::shared_ptr<ISPI> port)
{
    controlPort = port;

    if (controlPort != nullptr)
    {
        unsigned byte_array_size = 0;
        unsigned chipRev = this->Get_SPI_Reg_bits(LMS7_MASK, true);
        if (chipRev >= 1)
            byte_array_size = 1024 * 16;
        else
            byte_array_size = 1024 * 8;
        mcuControl->Initialize(port, byte_array_size);
    }
}

/** @brief Creates LMS7002M main control object.
It requires IConnection to be set by SetConnection() to communicate with chip
*/
LMS7002M::LMS7002M(std::shared_ptr<ISPI> port)
    : mCallback_onCGENChange(nullptr)
    , mCallback_onCGENChange_userData(nullptr)
    , useCache(0)
    , mRegistersMap(new LMS7002M_RegistersMap())
    , controlPort(port)
    , _cachedRefClockRate(30.72e6)
{
    opt_gain_tbb[0] = -1;
    opt_gain_tbb[1] = -1;

    mRegistersMap->InitializeDefaultValues(LMS7parameterList);
    mcuControl = new MCU_BD();
    mcuControl->Initialize(nullptr);
}

LMS7002M::~LMS7002M()
{
    delete mcuControl;
    delete mRegistersMap;
}

void LMS7002M::SetActiveChannel(const Channel ch)
{
    if (ch == this->GetActiveChannel(false))
        return;
    this->Modify_SPI_Reg_bits(LMS7param(MAC), static_cast<uint16_t>(ch));
}

LMS7002M::Channel LMS7002M::GetActiveChannel(bool fromChip)
{
    auto ch = Get_SPI_Reg_bits(LMS7param(MAC), fromChip);
    return static_cast<Channel>(ch);
}

size_t LMS7002M::GetActiveChannelIndex(bool fromChip)
{
    return this->GetActiveChannel(fromChip) == Channel::ChB ? 1 : 0;
}

int LMS7002M::EnableChannel(TRXDir dir, const uint8_t channel, const bool enable)
{
    ChannelScope scope(this, channel, false);

    const Channel ch = channel > 0 ? Channel::ChB : Channel::ChA;

    const bool isTx = dir == TRXDir::Tx;
    //--- LML ---
    if (ch == Channel::ChA)
    {
        if (isTx)
            this->Modify_SPI_Reg_bits(LMS7param(TXEN_A), enable ? 1 : 0);
        else
            this->Modify_SPI_Reg_bits(LMS7param(RXEN_A), enable ? 1 : 0);
    }
    else
    {
        if (isTx)
            this->Modify_SPI_Reg_bits(LMS7param(TXEN_B), enable ? 1 : 0);
        else
            this->Modify_SPI_Reg_bits(LMS7param(RXEN_B), enable ? 1 : 0);
    }

    //--- ADC/DAC ---
    Modify_SPI_Reg_bits(LMS7param(EN_DIR_AFE), 1);

    if (!enable)
    {
        bool disable;
        if (ch == Channel::ChA)
            disable = Get_SPI_Reg_bits(isTx ? LMS7_TXEN_B : LMS7_RXEN_B) == 0;
        else
            disable = Get_SPI_Reg_bits(isTx ? LMS7_TXEN_A : LMS7_RXEN_A) == 0;
        Modify_SPI_Reg_bits(isTx ? LMS7_PD_TX_AFE1 : LMS7_PD_RX_AFE1, disable);
    }
    else
        Modify_SPI_Reg_bits(isTx ? LMS7_PD_TX_AFE1 : LMS7_PD_RX_AFE1, 0);

    if (ch == Channel::ChB)
        Modify_SPI_Reg_bits(isTx ? LMS7_PD_TX_AFE2 : LMS7_PD_RX_AFE2, enable ? 0 : 1);

    int disabledChannels = (Get_SPI_Reg_bits(LMS7_PD_AFE.address, 4, 1) & 0xF); //check if all channels are disabled
    Modify_SPI_Reg_bits(LMS7param(EN_G_AFE), disabledChannels == 0xF ? 0 : 1);
    Modify_SPI_Reg_bits(LMS7param(PD_AFE), disabledChannels == 0xF ? 1 : 0);

    //--- digital ---
    if (isTx)
    {
        this->Modify_SPI_Reg_bits(LMS7param(EN_TXTSP), enable ? 1 : 0);
        this->Modify_SPI_Reg_bits(LMS7param(ISINC_BYP_TXTSP), enable ? 0 : 1);
        this->Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_TXTSP), 1);
        this->Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_TXTSP), 1);
        this->Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_TXTSP), 1);

        if (!enable)
        {
            this->Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP), 1);
            this->Modify_SPI_Reg_bits(LMS7param(DC_BYP_TXTSP), 1);
            this->Modify_SPI_Reg_bits(LMS7param(GC_BYP_TXTSP), 1);
            this->Modify_SPI_Reg_bits(LMS7param(PH_BYP_TXTSP), 1);
        }
    }
    else
    {
        this->Modify_SPI_Reg_bits(LMS7param(EN_RXTSP), enable ? 1 : 0);
        this->Modify_SPI_Reg_bits(LMS7param(DC_BYP_RXTSP), enable ? 0 : 1);
        this->Modify_SPI_Reg_bits(LMS7param(DCLOOP_STOP), enable ? 0 : 1);
        this->Modify_SPI_Reg_bits(LMS7param(AGC_MODE_RXTSP), 2); //bypass
        this->Modify_SPI_Reg_bits(LMS7param(AGC_BYP_RXTSP), 1);
        this->Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), 1);
        this->Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), 1);
        this->Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), 1);
        if (!enable)
        {
            this->Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);
            this->Modify_SPI_Reg_bits(LMS7param(GC_BYP_RXTSP), 1);
            this->Modify_SPI_Reg_bits(LMS7param(PH_BYP_RXTSP), 1);
        }
    }

    //--- baseband ---
    if (isTx)
    {
        this->Modify_SPI_Reg_bits(LMS7param(EN_DIR_TBB), 1);
        this->Modify_SPI_Reg_bits(LMS7param(EN_G_TBB), enable ? 1 : 0);
        this->Modify_SPI_Reg_bits(LMS7param(PD_LPFIAMP_TBB), enable ? 0 : 1);
    }
    else
    {
        this->Modify_SPI_Reg_bits(LMS7param(EN_DIR_RBB), 1);
        this->Modify_SPI_Reg_bits(LMS7param(EN_G_RBB), enable ? 1 : 0);
        this->Modify_SPI_Reg_bits(LMS7param(PD_PGA_RBB), enable ? 0 : 1);
    }

    //--- frontend ---
    if (isTx)
    {
        this->Modify_SPI_Reg_bits(LMS7param(EN_DIR_TRF), 1);
        this->Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), enable ? 1 : 0);
        this->Modify_SPI_Reg_bits(LMS7param(PD_TLOBUF_TRF), enable ? 0 : 1);
        this->Modify_SPI_Reg_bits(LMS7param(PD_TXPAD_TRF), enable ? 0 : 1);
    }
    else
    {
        this->Modify_SPI_Reg_bits(LMS7param(EN_DIR_RFE), 1);
        this->Modify_SPI_Reg_bits(LMS7param(EN_G_RFE), enable ? 1 : 0);
        this->Modify_SPI_Reg_bits(LMS7param(PD_MXLOBUF_RFE), enable ? 0 : 1);
        this->Modify_SPI_Reg_bits(LMS7param(PD_QGEN_RFE), enable ? 0 : 1);
        this->Modify_SPI_Reg_bits(LMS7param(PD_TIA_RFE), enable ? 0 : 1);
        this->Modify_SPI_Reg_bits(LMS7param(PD_LNA_RFE), enable ? 0 : 1);
    }

    //--- synthesizers ---
    if (isTx)
    {
        this->SetActiveChannel(Channel::ChSXT);
        this->Modify_SPI_Reg_bits(LMS7param(EN_DIR_SXRSXT), 1);
        this->Modify_SPI_Reg_bits(LMS7param(EN_G), (disabledChannels & 3) == 3 ? 0 : 1);
        if (ch == Channel::ChB) //enable LO to channel B
        {
            this->SetActiveChannel(Channel::ChA);
            this->Modify_SPI_Reg_bits(LMS7param(EN_NEXTTX_TRF), enable ? 1 : 0);
        }
    }
    else
    {
        this->SetActiveChannel(Channel::ChSXR);
        this->Modify_SPI_Reg_bits(LMS7param(EN_DIR_SXRSXT), 1);
        this->Modify_SPI_Reg_bits(LMS7param(EN_G), (disabledChannels & 0xC) == 0xC ? 0 : 1);
        if (ch == Channel::ChB) //enable LO to channel B
        {
            this->SetActiveChannel(Channel::ChA);
            this->Modify_SPI_Reg_bits(LMS7param(EN_NEXTRX_RFE), enable ? 1 : 0);
        }
    }
    this->SetActiveChannel(ch);

    return 0;
}

int LMS7002M::ResetChip()
{
    int status = 0;

    const std::vector<uint16_t> usedAddresses = mRegistersMap->GetUsedAddresses(0);

    std::vector<uint16_t> addrs;
    addrs.reserve(usedAddresses.size() + 2);
    std::vector<uint16_t> values;
    values.reserve(usedAddresses.size() + 2);

    addrs.push_back(0x0006); // SPISW_CTRL
    values.push_back(0x0000); // ensure baseband is controlling SPI

    addrs.push_back(0x0020);
    uint16_t x0020default = mRegistersMap->GetDefaultValue(0x0020);
    values.push_back(x0020default | 0x3); // enable simultaneous A&B write

    for (uint16_t addr : usedAddresses)
    {
        if (addr == 0x0020) // skip address containing MAC, to continue writing both channels
            continue;
        addrs.push_back(addr);
        values.push_back(mRegistersMap->GetDefaultValue(addr));
    }
    addrs.push_back(0x0020);
    values.push_back((x0020default & ~0x3) | 0x1); // back to A channel only

    status = SPI_write_batch(addrs.data(), values.data(), addrs.size(), true);
    status |= Modify_SPI_Reg_bits(LMS7param(MIMO_SISO), 0); //enable B channel after reset
    return status;
}

int LMS7002M::SoftReset()
{
    auto reg_0x0020 = SPI_read(0x0020, true);
    auto reg_0x002E = SPI_read(0x002E, true);
    int status = SPI_write(0x0020, 0x0);
    status |= SPI_write(0x0020, reg_0x0020);
    status |= SPI_write(0x002E, reg_0x002E); //must write, enables/disabled MIMO channel B
    return status;
}

int LMS7002M::LoadConfigLegacyFile(const std::string& filename)
{
    std::ifstream f(filename);
    if (f.good() == false) //file not found
    {
        f.close();
        return ReportError(ENOENT, "LoadConfigLegacyFile(%s) - file not found", filename.c_str());
    }
    f.close();

    uint16_t addr = 0;
    uint16_t value = 0;
    int status;
    typedef INI<std::string, std::string, std::string> ini_t;
    ini_t parser(filename, true);
    if (parser.select("FILE INFO") == false)
        return ReportError(EINVAL, "LoadConfigLegacyFile(%s) - invalid format, missing FILE INFO section", filename.c_str());

    std::string type = "";
    type = parser.get("type", "undefined");

    if (type.find("LMS7002 configuration") == std::string::npos)
    {
        return ReportError(EINVAL, "LoadConfigLegacyFile(%s) - invalid format, missing LMS7002 configuration", filename.c_str());
    }

    int fileVersion = 0;
    fileVersion = parser.get("version", 0);

    std::vector<uint16_t> addrToWrite;
    std::vector<uint16_t> dataToWrite;
    if (fileVersion == 1)
    {
        ChannelScope scope(this);

        if (parser.select("Reference clocks"))
        {
            this->SetReferenceClk_SX(TRXDir::Rx, parser.get("SXR reference frequency MHz", 30.72) * 1e6);
            this->SetReferenceClk_SX(TRXDir::Tx, parser.get("SXT reference frequency MHz", 30.72) * 1e6);
        }

        if (parser.select("LMS7002 registers ch.A") == true)
        {
            ini_t::sectionsit_t section = parser.sections.find("LMS7002 registers ch.A");

            uint16_t x0020_value = 0;
            this->SetActiveChannel(Channel::ChA); //select A channel
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
            status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size(), true);
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
                        SetNCOFrequency(TRXDir::Rx, i, parser.get(varname, 0.0));
                    }
                }
                else
                {
                    for (int i = 0; i < 16; ++i)
                    {
                        sprintf(varname, "PHO%02i", i);
                        SetNCOPhaseOffset(TRXDir::Rx, i, parser.get(varname, 0.0));
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
                        SetNCOFrequency(TRXDir::Tx, i, parser.get(varname, 0.0));
                    }
                }
                else
                {
                    for (int i = 0; i < 16; ++i)
                    {
                        sprintf(varname, "PHO%02i", i);
                        SetNCOPhaseOffset(TRXDir::Tx, i, parser.get(varname, 0.0));
                    }
                }
            }
            status = SPI_write(0x0020, x0020_value);
            if (status != 0 && controlPort != nullptr)
                return status;
        }

        this->SetActiveChannel(Channel::ChB);

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
            this->SetActiveChannel(Channel::ChB); //select B channel
            status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size(), true);
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
                        SetNCOFrequency(TRXDir::Rx, i, parser.get(varname, 0.0));
                    }
                }
                else
                {
                    for (int i = 0; i < 16; ++i)
                    {
                        sprintf(varname, "PHO%02i", i);
                        SetNCOPhaseOffset(TRXDir::Rx, i, parser.get(varname, 0.0));
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
                        SetNCOFrequency(TRXDir::Tx, i, parser.get(varname, 0.0));
                    }
                }
                else
                {
                    for (int i = 0; i < 16; ++i)
                    {
                        sprintf(varname, "PHO%02i", i);
                        SetNCOPhaseOffset(TRXDir::Tx, i, parser.get(varname, 0.0));
                    }
                }
            }
        }
        return 0;
    }
    return ReportError(EINVAL, "LoadConfigLegacyFile(%s) - invalid format", filename.c_str());
}

int LMS7002M::LoadConfig(const std::string& filename, bool tuneDynamicValues)
{
    std::ifstream f(filename);
    if (f.good() == false) //file not found
    {
        f.close();
        return ReportError(ENOENT, "LoadConfig(%s) - file not found", filename.c_str());
    }
    f.close();

    uint16_t addr = 0;
    uint16_t value = 0;

    int status;
    typedef INI<std::string, std::string, std::string> ini_t;
    ini_t parser(filename, true);
    if (parser.select("file_info") == false)
    {
        //try loading as legacy format
        status = LoadConfigLegacyFile(filename);
        this->SetActiveChannel(Channel::ChA);
        return status;
    }
    std::string type = "";
    type = parser.get("type", "undefined");

    if (type.find("lms7002m_minimal_config") == std::string::npos)
    {
        return ReportError(EINVAL, "LoadConfig(%s) - invalid format, missing lms7002m_minimal_config", filename.c_str());
    }

    int fileVersion = 0;
    fileVersion = parser.get("version", 0);

    std::vector<uint16_t> addrToWrite;
    std::vector<uint16_t> dataToWrite;

    if (fileVersion == 1)
    {
        ChannelScope scope(this);
        if (parser.select("lms7002_registers_a") == true)
        {
            ini_t::sectionsit_t section = parser.sections.find("lms7002_registers_a");

            uint16_t x0020_value = 0;
            this->SetActiveChannel(Channel::ChA); //select A channel
            for (ini_t::keysit_t pairs = section->second->begin(); pairs != section->second->end(); pairs++)
            {
                sscanf(pairs->first.c_str(), "%hx", &addr);
                sscanf(pairs->second.c_str(), "%hx", &value);
                if (addr == LMS7param(MAC).address) //skip register containing channel selection
                {
                    x0020_value = value;
                    continue;
                }

                if (addr >= 0x5C3 && addr <= 0x5CA) //enable analog DC correction
                {
                    addrToWrite.push_back(addr);
                    dataToWrite.push_back(value & 0x3FFF);
                    addrToWrite.push_back(addr);
                    dataToWrite.push_back(value | 0x8000);
                }
                else
                {
                    addrToWrite.push_back(addr);
                    dataToWrite.push_back(value);
                }
            }

            status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size(), true);
            if (status != 0 && controlPort != nullptr)
                return status;
            status = SPI_write(0x0020, x0020_value);
            if (status != 0 && controlPort != nullptr)
                return status;
            this->SetActiveChannel(Channel::ChB);
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
            this->SetActiveChannel(Channel::ChB); //select B channel
            status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size(), true);
            if (status != 0 && controlPort != nullptr)
                return status;
        }

        parser.select("reference_clocks");
        this->SetReferenceClk_SX(TRXDir::Rx, parser.get("sxr_ref_clk_mhz", 30.72) * 1e6);
        this->SetReferenceClk_SX(TRXDir::Tx, parser.get("sxt_ref_clk_mhz", 30.72) * 1e6);
    }

    ResetLogicRegisters();

    if (tuneDynamicValues)
    {
        Modify_SPI_Reg_bits(LMS7param(MAC), 2);
        if (!Get_SPI_Reg_bits(LMS7param(PD_VCO)))
            TuneVCO(VCO_Module::VCO_SXT);
        Modify_SPI_Reg_bits(LMS7param(MAC), 1);
        if (!Get_SPI_Reg_bits(LMS7param(PD_VCO)))
            TuneVCO(VCO_Module::VCO_SXR);
        if (!Get_SPI_Reg_bits(LMS7param(PD_VCO_CGEN)))
        {
            TuneVCO(VCO_Module::VCO_CGEN);
            if (mCallback_onCGENChange)
                return mCallback_onCGENChange(mCallback_onCGENChange_userData);
        }
    }
    this->SetActiveChannel(Channel::ChA);
    return 0;
}

int LMS7002M::ResetLogicRegisters()
{
    const uint16_t x0020_value = SPI_read(0x0020); //reset logic registers
    const uint16_t addr[] = { 0x0020, 0x0020 };
    const uint16_t values[] = { uint16_t(x0020_value & 0x553F), uint16_t(x0020_value | 0xFFC0) };
    //const uint16_t values[] = {x0020_value & 0x55FF, x0020_value | 0xFF00};
    // LRST_TX_B, LRST_TX_A, LRST_RX_B, LRST_RX_A
    return SPI_write_batch(addr, values, 2);
}

int LMS7002M::SaveConfig(const std::string& filename)
{
    std::ofstream fout;
    fout.open(filename);
    fout << "[file_info]" << std::endl;
    fout << "type=lms7002m_minimal_config" << std::endl;
    fout << "version=1" << std::endl;

    char addr[80];
    char value[80];

    ChannelScope scope(this);

    std::vector<uint16_t> addrToRead;
    for (const auto& memorySectionPair : MemorySectionAddresses)
        for (uint16_t addr = memorySectionPair.second[0]; addr <= memorySectionPair.second[1]; ++addr)
            addrToRead.push_back(addr);

    std::vector<uint16_t> dataReceived;
    dataReceived.resize(addrToRead.size(), 0);

    fout << "[lms7002_registers_a]" << std::endl;
    this->SetActiveChannel(Channel::ChA);
    for (uint16_t i = 0; i < addrToRead.size(); ++i)
    {
        if (addrToRead[i] >= 0x5C3 && addrToRead[i] <= 0x5CA)
            SPI_write(addrToRead[i], 0x4000); //perform read-back from DAC
        dataReceived[i] = Get_SPI_Reg_bits(addrToRead[i], 15, 0, false);

        //registers 0x5C3 - 0x53A return inverted value field when DAC value read-back is performed
        if (addrToRead[i] >= 0x5C3 && addrToRead[i] <= 0x5C6 && (dataReceived[i] & 0x400)) //sign bit 10
            dataReceived[i] = 0x400 | (~dataReceived[i] & 0x3FF); //magnitude bits  9:0
        else if (addrToRead[i] >= 0x5C7 && addrToRead[i] <= 0x5CA && (dataReceived[i] & 0x40)) //sign bit 6
            dataReceived[i] = 0x40 | (~dataReceived[i] & 0x3F); //magnitude bits  5:0
        else if (addrToRead[i] == 0x5C2)
            dataReceived[i] &= 0xFF00; //do not save calibration start triggers
        sprintf(addr, "0x%04X", addrToRead[i]);
        sprintf(value, "0x%04X", dataReceived[i]);
        fout << addr << "=" << value << std::endl;
    }

    fout << "[lms7002_registers_b]" << std::endl;
    addrToRead.clear(); //add only B channel addresses
    for (const auto& memorySectionPair : MemorySectionAddresses)
        if (memorySectionPair.first != MemorySection::RSSI_DC_CALIBRATION)
            for (uint16_t addr = memorySectionPair.second[0]; addr <= memorySectionPair.second[1]; ++addr)
                if (addr >= 0x0100)
                    addrToRead.push_back(addr);

    this->SetActiveChannel(Channel::ChB);
    for (uint16_t i = 0; i < addrToRead.size(); ++i)
    {
        dataReceived[i] = Get_SPI_Reg_bits(addrToRead[i], 15, 0, false);
        sprintf(addr, "0x%04X", addrToRead[i]);
        sprintf(value, "0x%04X", dataReceived[i]);
        fout << addr << "=" << value << std::endl;
    }

    fout << "[reference_clocks]" << std::endl;
    fout << "sxt_ref_clk_mhz=" << this->GetReferenceClk_SX(TRXDir::Tx) / 1e6 << std::endl;
    fout << "sxr_ref_clk_mhz=" << this->GetReferenceClk_SX(TRXDir::Rx) / 1e6 << std::endl;
    fout.close();
    return 0;
}

int LMS7002M::SetRBBPGA_dB(const float_type value, const Channel channel)
{
    ChannelScope scope(this, channel);

    int g_pga_rbb = std::clamp(static_cast<int>(std::round(value)) + 12, 0, 31);
    int ret = this->Modify_SPI_Reg_bits(LMS7param(G_PGA_RBB), g_pga_rbb);

    int rcc_ctl_pga_rbb = (430.0 * pow(0.65, (g_pga_rbb / 10.0)) - 110.35) / 20.4516 + 16;

    int c_ctl_pga_rbb = 0;
    if (0 <= g_pga_rbb && g_pga_rbb < 8)
        c_ctl_pga_rbb = 3;
    if (8 <= g_pga_rbb && g_pga_rbb < 13)
        c_ctl_pga_rbb = 2;
    if (13 <= g_pga_rbb && g_pga_rbb < 21)
        c_ctl_pga_rbb = 1;
    if (21 <= g_pga_rbb)
        c_ctl_pga_rbb = 0;

    ret |= this->Modify_SPI_Reg_bits(LMS7param(RCC_CTL_PGA_RBB), rcc_ctl_pga_rbb);
    ret |= this->Modify_SPI_Reg_bits(LMS7param(C_CTL_PGA_RBB), c_ctl_pga_rbb);
    return ret;
}

float_type LMS7002M::GetRBBPGA_dB(const Channel channel)
{
    ChannelScope scope(this, channel);

    auto g_pga_rbb = this->Get_SPI_Reg_bits(LMS7param(G_PGA_RBB));
    return g_pga_rbb - 12;
}

int LMS7002M::SetRFELNA_dB(const float_type value, const Channel channel)
{
    ChannelScope scope(this, channel);

    const double gmax = 30;
    double val = value - gmax;

    int g_lna_rfe = 0;
    if (val >= 0)
        g_lna_rfe = 15;
    else if (val >= -1)
        g_lna_rfe = 14;
    else if (val >= -2)
        g_lna_rfe = 13;
    else if (val >= -3)
        g_lna_rfe = 12;
    else if (val >= -4)
        g_lna_rfe = 11;
    else if (val >= -5)
        g_lna_rfe = 10;
    else if (val >= -6)
        g_lna_rfe = 9;
    else if (val >= -9)
        g_lna_rfe = 8;
    else if (val >= -12)
        g_lna_rfe = 7;
    else if (val >= -15)
        g_lna_rfe = 6;
    else if (val >= -18)
        g_lna_rfe = 5;
    else if (val >= -21)
        g_lna_rfe = 4;
    else if (val >= -24)
        g_lna_rfe = 3;
    else if (val >= -27)
        g_lna_rfe = 2;
    else
        g_lna_rfe = 1;

    return this->Modify_SPI_Reg_bits(LMS7param(G_LNA_RFE), g_lna_rfe);
}

float_type LMS7002M::GetRFELNA_dB(const Channel channel)
{
    ChannelScope scope(this, channel);

    const double gmax = 30;
    auto g_lna_rfe = this->Get_SPI_Reg_bits(LMS7param(G_LNA_RFE));
    switch (g_lna_rfe)
    {
    case 15:
        return gmax - 0;
    case 14:
        return gmax - 1;
    case 13:
        return gmax - 2;
    case 12:
        return gmax - 3;
    case 11:
        return gmax - 4;
    case 10:
        return gmax - 5;
    case 9:
        return gmax - 6;
    case 8:
        return gmax - 9;
    case 7:
        return gmax - 12;
    case 6:
        return gmax - 15;
    case 5:
        return gmax - 18;
    case 4:
        return gmax - 21;
    case 3:
        return gmax - 24;
    case 2:
        return gmax - 27;
    case 1:
        return gmax - 30;
    }
    return 0.0;
}

int LMS7002M::SetRFELoopbackLNA_dB(const float_type gain, const Channel channel)
{
    ChannelScope scope(this, channel);

    const double gmax = 40;
    double val = gain - gmax;

    int g_rxloopb_rfe = 0;
    if (val >= 0)
        g_rxloopb_rfe = 15;
    else if (val >= -0.5)
        g_rxloopb_rfe = 14;
    else if (val >= -1)
        g_rxloopb_rfe = 13;
    else if (val >= -1.6)
        g_rxloopb_rfe = 12;
    else if (val >= -2.4)
        g_rxloopb_rfe = 11;
    else if (val >= -3)
        g_rxloopb_rfe = 10;
    else if (val >= -4)
        g_rxloopb_rfe = 9;
    else if (val >= -5)
        g_rxloopb_rfe = 8;
    else if (val >= -6.2)
        g_rxloopb_rfe = 7;
    else if (val >= -7.5)
        g_rxloopb_rfe = 6;
    else if (val >= -9)
        g_rxloopb_rfe = 5;
    else if (val >= -11)
        g_rxloopb_rfe = 4;
    else if (val >= -14)
        g_rxloopb_rfe = 3;
    else if (val >= -17)
        g_rxloopb_rfe = 2;
    else if (val >= -24)
        g_rxloopb_rfe = 1;
    else
        g_rxloopb_rfe = 0;

    return this->Modify_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE), g_rxloopb_rfe);
}

float_type LMS7002M::GetRFELoopbackLNA_dB(const Channel channel)
{
    ChannelScope scope(this, channel);

    const double gmax = 40;
    auto g_rxloopb_rfe = this->Get_SPI_Reg_bits(LMS7param(G_RXLOOPB_RFE));
    switch (g_rxloopb_rfe)
    {
    case 15:
        return gmax - 0;
    case 14:
        return gmax - 0.5;
    case 13:
        return gmax - 1;
    case 12:
        return gmax - 1.6;
    case 11:
        return gmax - 2.4;
    case 10:
        return gmax - 3;
    case 9:
        return gmax - 4;
    case 8:
        return gmax - 5;
    case 7:
        return gmax - 6.2;
    case 6:
        return gmax - 7.5;
    case 5:
        return gmax - 9;
    case 4:
        return gmax - 11;
    case 3:
        return gmax - 14;
    case 2:
        return gmax - 17;
    case 1:
        return gmax - 24;
    }
    return 0.0;
}

int LMS7002M::SetRFETIA_dB(const float_type value, const Channel channel)
{
    ChannelScope scope(this, channel);

    const double gmax = 12;
    double val = value - gmax;

    int g_tia_rfe = 0;
    if (val >= 0)
        g_tia_rfe = 3;
    else if (val >= -3)
        g_tia_rfe = 2;
    else
        g_tia_rfe = 1;

    return this->Modify_SPI_Reg_bits(LMS7param(G_TIA_RFE), g_tia_rfe);
}

float_type LMS7002M::GetRFETIA_dB(const Channel channel)
{
    ChannelScope scope(this, channel);

    const double gmax = 12;
    auto g_tia_rfe = this->Get_SPI_Reg_bits(LMS7param(G_TIA_RFE));
    switch (g_tia_rfe)
    {
    case 3:
        return gmax - 0;
    case 2:
        return gmax - 3;
    case 1:
        return gmax - 12;
    }
    return 0.0;
}

int LMS7002M::SetTRFPAD_dB(const float_type value, const Channel channel)
{
    ChannelScope scope(this, channel);

    const double pmax = 52;
    int loss_int = std::round(pmax - value);

    //different scaling realm
    if (loss_int > 10)
    {
        loss_int = (loss_int + 10) / 2;
    }

    loss_int = std::clamp(loss_int, 0, 31);

    int ret = 0;
    ret |= this->Modify_SPI_Reg_bits(LMS7param(LOSS_LIN_TXPAD_TRF), loss_int);
    ret |= this->Modify_SPI_Reg_bits(LMS7param(LOSS_MAIN_TXPAD_TRF), loss_int);
    return ret;
}

float_type LMS7002M::GetTRFPAD_dB(const Channel channel)
{
    ChannelScope scope(this, channel);

    const double pmax = 52;
    auto loss_int = this->Get_SPI_Reg_bits(LMS7param(LOSS_LIN_TXPAD_TRF));
    if (loss_int > 10)
        return pmax - 10 - 2 * (loss_int - 10);
    return pmax - loss_int;
}

int LMS7002M::SetTRFLoopbackPAD_dB(const float_type gain, const Channel channel)
{
    ChannelScope scope(this, channel);

    //there are 4 discrete gain values, use the midpoints
    int val = 0;
    if (gain >= (-1.4 - 0) / 2)
        val = 0;
    else if (gain >= (-1.4 - 3.3) / 2)
        val = 1;
    else if (gain >= (-3.3 - 4.3) / 2)
        val = 2;
    else
        val = 3;

    return this->Modify_SPI_Reg_bits(LMS7param(L_LOOPB_TXPAD_TRF), val);
}

float_type LMS7002M::GetTRFLoopbackPAD_dB(const Channel channel)
{
    ChannelScope scope(this, channel);

    switch (this->Get_SPI_Reg_bits(LMS7param(L_LOOPB_TXPAD_TRF)))
    {
    case 0:
        return 0.0;
    case 1:
        return -1.4;
    case 2:
        return -3.3;
    case 3:
        return -4.3;
    }
    return 0.0;
}

int LMS7002M::SetTBBIAMP_dB(const float_type gain, const Channel channel)
{
    ChannelScope scope(this, channel);

    int ind = this->GetActiveChannelIndex() % 2;
    if (opt_gain_tbb[ind] <= 0)
    {
        if (CalibrateTxGain() != 0) //set optimal BB gain
            return -1;
        if (std::fabs(gain) < 0.2) // optimal gain = ~0dB
            return 0;
    }

    int g_iamp = (float_type)opt_gain_tbb[ind] * pow(10.0, gain / 20.0) + 0.4;
    Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), std::clamp(g_iamp, 1, 63), true);

    return 0;
}

float_type LMS7002M::GetTBBIAMP_dB(const Channel channel)
{
    ChannelScope scope(this, channel);

    int g_current = Get_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), true);
    int ind = this->GetActiveChannelIndex() % 2;

    if (opt_gain_tbb[ind] <= 0)
    {
        if (CalibrateTxGain() != 0)
            return 0.0;
        Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), g_current, true); //restore
    }
    return 20.0 * log10((float_type)g_current / (float_type)opt_gain_tbb[ind]);
}

int LMS7002M::SetPathRFE(PathRFE path)
{
    int sel_path_rfe;
    int pd_lb1 = 1;
    int pd_lb2 = 1;
    switch (path)
    {
    case PathRFE::LNAH:
        sel_path_rfe = 1;
        break;
    case PathRFE::LB2:
        pd_lb2 = 0;
    case PathRFE::LNAL:
        sel_path_rfe = 2;
        break;
    case PathRFE::LB1:
        pd_lb1 = 0;
    case PathRFE::LNAW:
        sel_path_rfe = 3;
        break;
    default:
        sel_path_rfe = 0;
        break;
    }

    Modify_SPI_Reg_bits(LMS7param(SEL_PATH_RFE), sel_path_rfe);

    int pd_lna_rfe = (path == PathRFE::LB2 || path == PathRFE::LB1 || sel_path_rfe == 0) ? 1 : 0;
    Modify_SPI_Reg_bits(LMS7param(PD_LNA_RFE), pd_lna_rfe);

    Modify_SPI_Reg_bits(LMS7param(PD_RLOOPB_1_RFE), pd_lb1);
    Modify_SPI_Reg_bits(LMS7param(PD_RLOOPB_2_RFE), pd_lb2);
    Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_LB1_RFE), pd_lb1);
    Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_LB2_RFE), pd_lb2);
    Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_L_RFE), (path == PathRFE::LNAL) ? 0 : 1);
    Modify_SPI_Reg_bits(LMS7param(EN_INSHSW_W_RFE), (path == PathRFE::LNAW) ? 0 : 1);

    //enable/disable the loopback path
    const bool loopback = (path == PathRFE::LB1) or (path == PathRFE::LB2);
    Modify_SPI_Reg_bits(LMS7param(EN_LOOPB_TXPAD_TRF), loopback ? 1 : 0);

    return 0;
}

LMS7002M::PathRFE LMS7002M::GetPathRFE(void)
{
    const int sel_path_rfe = this->Get_SPI_Reg_bits(LMS7param(SEL_PATH_RFE));
    if (this->Get_SPI_Reg_bits(LMS7param(EN_INSHSW_LB1_RFE)) == 0 && sel_path_rfe == 3)
        return PathRFE::LB1;
    if (this->Get_SPI_Reg_bits(LMS7param(EN_INSHSW_LB2_RFE)) == 0 && sel_path_rfe == 2)
        return PathRFE::LB2;
    if (this->Get_SPI_Reg_bits(LMS7param(EN_INSHSW_L_RFE)) == 0 && sel_path_rfe == 2)
        return PathRFE::LNAL;
    if (this->Get_SPI_Reg_bits(LMS7param(EN_INSHSW_W_RFE)) == 0 && sel_path_rfe == 3)
        return PathRFE::LNAW;
    if (sel_path_rfe == 1)
        return PathRFE::LNAH;
    return PathRFE::NONE;
}

int LMS7002M::SetBandTRF(const int band)
{
    this->Modify_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF), (band == 1) ? 1 : 0);
    this->Modify_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF), (band == 2) ? 1 : 0);

    return 0;
}

int LMS7002M::GetBandTRF(void)
{
    if (this->Get_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF)) == 1)
        return 1;
    if (this->Get_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF)) == 1)
        return 2;
    return 0;
}

int LMS7002M::SetPath(TRXDir direction, uint8_t channel, uint8_t path)
{
    ChannelScope scope(this, channel, false);

    if (direction == TRXDir::Tx)
    {
        return SetBandTRF(path);
    }

    return SetPathRFE(lime::LMS7002M::PathRFE(path));
}

int LMS7002M::SetReferenceClk_SX(TRXDir dir, float_type freq_Hz)
{
    _cachedRefClockRate = freq_Hz;
    return 0;
}

float_type LMS7002M::GetReferenceClk_SX(TRXDir dir)
{
    return _cachedRefClockRate;
}

int LMS7002M::SetNCOFrequencies(TRXDir dir, const float_type* freq_Hz, uint8_t count, float_type phaseOffset)
{
    for (uint8_t i = 0; i < 16 && i < count; i++)
    {
        if (SetNCOFrequency(dir, i, freq_Hz[i]))
            return -1;
    }
    return SetNCOPhaseOffsetForMode0(dir, phaseOffset);
}

std::vector<float_type> LMS7002M::GetNCOFrequencies(TRXDir dir, float_type* phaseOffset)
{
    std::vector<float_type> ncos;
    for (int i = 0; i < 16; ++i)
        ncos.push_back(GetNCOFrequency(dir, i, !useCache));
    if (phaseOffset != nullptr)
    {
        uint16_t value = SPI_read(dir == TRXDir::Tx ? 0x0241 : 0x0441);
        *phaseOffset = 360.0 * value / 65536.0;
    }
    return ncos;
}

float_type LMS7002M::GetFrequencyCGEN()
{
    float_type dMul =
        (GetReferenceClk_SX(TRXDir::Rx) / 2.0) / (Get_SPI_Reg_bits(LMS7param(DIV_OUTCH_CGEN), true) + 1); //DIV_OUTCH_CGEN
    uint16_t gINT = Get_SPI_Reg_bits(0x0088, 13, 0, true); //read whole register to reduce SPI transfers
    uint32_t gFRAC = ((gINT & 0xF) * 65536) | Get_SPI_Reg_bits(0x0087, 15, 0, true);
    return dMul * (((gINT >> 4) + 1 + gFRAC / 1048576.0));
}

float_type LMS7002M::GetReferenceClk_TSP(TRXDir dir)
{
    float_type cgenFreq = GetFrequencyCGEN();
    float_type clklfreq = cgenFreq / pow(2.0, Get_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), true));
    if (Get_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), true) == 0)
        return dir == TRXDir::Tx ? clklfreq : cgenFreq / 4.0;
    else
        return dir == TRXDir::Tx ? cgenFreq : clklfreq / 4.0;
}

int LMS7002M::SetFrequencyCGEN(const float_type freq_Hz, const bool retainNCOfrequencies, CGEN_details* output)
{
    if (freq_Hz > CGEN_MAX_FREQ)
        throw std::logic_error("requested CGEN frequency too high");
    float_type dFvco;
    float_type dFrac;

    //remember NCO frequencies
    Channel chBck = this->GetActiveChannel();
    std::vector<std::vector<float_type>> rxNCO(2);
    std::vector<std::vector<float_type>> txNCO(2);
    bool rxModeNCO = false;
    bool txModeNCO = false;
    if (retainNCOfrequencies)
    {
        rxModeNCO = Get_SPI_Reg_bits(LMS7param(MODE_RX), true);
        txModeNCO = Get_SPI_Reg_bits(LMS7param(MODE_TX), true);
        for (int ch = 0; ch < 2; ++ch)
        {
            this->SetActiveChannel((ch == 0) ? Channel::ChA : Channel::ChB);
            for (int i = 0; i < 16 && rxModeNCO == 0; ++i)
                rxNCO[ch].push_back(GetNCOFrequency(TRXDir::Rx, i, false));
            for (int i = 0; i < 16 && txModeNCO == 0; ++i)
                txNCO[ch].push_back(GetNCOFrequency(TRXDir::Tx, i, false));
        }
    }
    //VCO frequency selection according to F_CLKH
    uint16_t iHdiv_high = (gCGEN_VCO_frequencies[1] / 2 / freq_Hz) - 1;
    uint16_t iHdiv_low = (gCGEN_VCO_frequencies[0] / 2 / freq_Hz);
    uint16_t iHdiv = (iHdiv_low + iHdiv_high) / 2;
    iHdiv = iHdiv > 255 ? 255 : iHdiv;
    dFvco = 2 * (iHdiv + 1) * freq_Hz;
    if (dFvco <= gCGEN_VCO_frequencies[0] || dFvco >= gCGEN_VCO_frequencies[1])
        return ReportError(ERANGE, "SetFrequencyCGEN(%g MHz) - cannot deliver requested frequency", freq_Hz / 1e6);
    //Integer division
    uint16_t gINT = (uint16_t)(dFvco / GetReferenceClk_SX(TRXDir::Rx) - 1);

    //Fractional division
    dFrac = dFvco / GetReferenceClk_SX(TRXDir::Rx) - (uint32_t)(dFvco / GetReferenceClk_SX(TRXDir::Rx));
    uint32_t gFRAC = (uint32_t)(dFrac * 1048576);

    Modify_SPI_Reg_bits(LMS7param(INT_SDM_CGEN), gINT); //INT_SDM_CGEN
    Modify_SPI_Reg_bits(0x0087, 15, 0, gFRAC & 0xFFFF); //INT_SDM_CGEN[15:0]
    Modify_SPI_Reg_bits(0x0088, 3, 0, gFRAC >> 16); //INT_SDM_CGEN[19:16]
    Modify_SPI_Reg_bits(LMS7param(DIV_OUTCH_CGEN), iHdiv); //DIV_OUTCH_CGEN

    lime::debug("INT %d, FRAC %d, DIV_OUTCH_CGEN %d", gINT, gFRAC, (uint16_t)iHdiv);
    lime::debug("VCO %.2f MHz, RefClk %.2f MHz", dFvco / 1e6, GetReferenceClk_SX(TRXDir::Rx) / 1e6);

    if (output)
    {
        output->frequency = freq_Hz;
        output->frequencyVCO = dFvco;
        output->referenceClock = GetReferenceClk_SX(TRXDir::Rx);
        output->INT = gINT;
        output->FRAC = gFRAC;
        output->div_outch_cgen = iHdiv;
        output->success = true;
    }

    //recalculate NCO
    for (int ch = 0; ch < 2 && retainNCOfrequencies; ++ch)
    {
        this->SetActiveChannel((ch == 0) ? Channel::ChA : Channel::ChB);
        for (int i = 0; i < 16 && rxModeNCO == 0; ++i)
            SetNCOFrequency(TRXDir::Rx, i, rxNCO[ch][i]);
        for (int i = 0; i < 16 && txModeNCO == 0; ++i)
            SetNCOFrequency(TRXDir::Tx, i, txNCO[ch][i]);
    }
    this->SetActiveChannel(chBck);
    if (TuneVCO(VCO_Module::VCO_CGEN) != 0)
    {
        if (output)
        {
            output->success = false;
            output->csw = Get_SPI_Reg_bits(LMS7param(CSW_VCO_CGEN));
        }
        return ReportError("SetFrequencyCGEN(%g MHz) failed", freq_Hz / 1e6);
    }
    if (output)
        output->csw = Get_SPI_Reg_bits(LMS7param(CSW_VCO_CGEN));

    if (mCallback_onCGENChange)
        return mCallback_onCGENChange(mCallback_onCGENChange_userData);
    return 0;
}

bool LMS7002M::GetCGENLocked(void)
{
    return (Get_SPI_Reg_bits(LMS7param(VCO_CMPHO_CGEN).address, 13, 12, true) & 0x3) == 2;
}

bool LMS7002M::GetSXLocked(TRXDir dir)
{
    SetActiveChannel(dir == TRXDir::Tx ? Channel::ChSXT : Channel::ChSXR);
    return (Get_SPI_Reg_bits(LMS7param(VCO_CMPHO).address, 13, 12, true) & 0x3) == 2;
}

int LMS7002M::TuneCGENVCO()
{
#ifndef NDEBUG
    lime::debug("ICT_VCO_CGEN: %d", Get_SPI_Reg_bits(LMS7param(ICT_VCO_CGEN)));
#endif
    // Initialization activate VCO and comparator
    if (int status = Modify_SPI_Reg_bits(LMS7_PD_VCO_CGEN.address, 2, 1, 0) != 0)
        return status;

    auto checkCSW = [this](int cswVal) {
        Modify_SPI_Reg_bits(LMS7_CSW_VCO_CGEN, cswVal); //write CSW value
        std::this_thread::sleep_for(std::chrono::microseconds(50)); //comparator settling time
        return Get_SPI_Reg_bits(LMS7_VCO_CMPHO_CGEN.address, 13, 12, true); //read comparators
    };
    //find lock
    int csw = 127;
    for (int step = 64; step > 0; step >>= 1)
    {
        auto cmphl = checkCSW(csw);
        if (cmphl == 0)
            csw += step;
        else if (cmphl == 3)
            csw -= step;
        else
            break;
    }
    //search around (+/-7) to determine lock interval
    //number of iterations could be reduced in some cases by narrowing down the search interval in find lock phase
    int cswLow = csw, cswHigh = csw;
    for (int step = 4; step > 0; step >>= 1)
        if (checkCSW(cswLow - step) != 0)
            cswLow = cswLow - step;
    for (int step = 4; step > 0; step >>= 1)
        if (checkCSW(cswHigh + step) == 2)
            cswHigh = cswHigh + step;

    lime::debug("csw %d; interval [%d, %d]", (cswHigh + cswLow) / 2, cswLow, cswHigh);
    auto cmphl = checkCSW((cswHigh + cswLow) / 2);
    if (cmphl == 2)
        return 0;
    lime::error("TuneVCO(CGEN) - failed to lock (cmphl!=%d)", cmphl);
    return -1;
}

int LMS7002M::TuneVCO(VCO_Module module) // 0-cgen, 1-SXR, 2-SXT
{
    if (module == VCO_Module::VCO_CGEN)
        return TuneCGENVCO();
    auto settlingTime = std::chrono::microseconds(50); //can be lower
    struct CSWInteval {
        int16_t high;
        int16_t low;
    };
    CSWInteval cswSearch[2];
    const char* moduleName = (module == VCO_Module::VCO_CGEN) ? "CGEN" : ((module == VCO_Module::VCO_SXR) ? "SXR" : "SXT");
    uint8_t cmphl; //comparators
    uint16_t addrVCOpd; // VCO power down address
    uint16_t addrCSW_VCO;
    uint16_t addrCMP; //comparator address
    uint8_t lsb; //SWC lsb index
    uint8_t msb; //SWC msb index

    ChannelScope scope(this);

    if (module != VCO_Module::VCO_CGEN) //set addresses to SX module
    {
        this->SetActiveChannel(Channel(module));
        addrVCOpd = LMS7param(PD_VCO).address;
        addrCSW_VCO = LMS7param(CSW_VCO).address;
        lsb = LMS7param(CSW_VCO).lsb;
        msb = LMS7param(CSW_VCO).msb;
        addrCMP = LMS7param(VCO_CMPHO).address;
        lime::debug("TuneVCO(%s) ICT_VCO: %d", moduleName, Get_SPI_Reg_bits(LMS7param(ICT_VCO)));
    }
    else //set addresses to CGEN module
    {
        addrVCOpd = LMS7param(PD_VCO_CGEN).address;
        addrCSW_VCO = LMS7param(CSW_VCO_CGEN).address;
        lsb = LMS7param(CSW_VCO_CGEN).lsb;
        msb = LMS7param(CSW_VCO_CGEN).msb;
        addrCMP = LMS7param(VCO_CMPHO_CGEN).address;
        lime::debug("TuneVCO(%s) ICT_VCO_CGEN: %d", moduleName, Get_SPI_Reg_bits(LMS7param(ICT_VCO_CGEN)));
    }
    // Initialization activate VCO and comparator
    if (int status = Modify_SPI_Reg_bits(addrVCOpd, 2, 1, 0) != 0)
        return status;
    if (Get_SPI_Reg_bits(addrVCOpd, 2, 1) != 0)
        return ReportError("TuneVCO(%s) - VCO is powered down", moduleName);

    //check if lock is within VCO range
    {
        Modify_SPI_Reg_bits(addrCSW_VCO, msb, lsb, 0);
        std::this_thread::sleep_for(settlingTime);
        cmphl = (uint8_t)Get_SPI_Reg_bits(addrCMP, 13, 12, true);
        if (cmphl == 3) //VCO too high
        {
            lime::debug("TuneVCO(%s) - attempted VCO too high", moduleName);
            return -1;
        }
        Modify_SPI_Reg_bits(addrCSW_VCO, msb, lsb, 255);
        std::this_thread::sleep_for(settlingTime);
        cmphl = (uint8_t)Get_SPI_Reg_bits(addrCMP, 13, 12, true);
        if (cmphl == 0) //VCO too low
        {
            lime::debug("TuneVCO(%s) - attempted VCO too low", moduleName);
            return -1;
        }
    }

    //search intervals [0-127][128-255]
    for (int t = 0; t < 2; ++t)
    {
        bool hadLock = false;
        // initialize search range with invalid values
        cswSearch[t].low = 128 * (t + 1); // set low to highest possible value
        cswSearch[t].high = 128 * t; // set high to lowest possible value
        lime::debug("TuneVCO(%s) - searching interval [%i:%i]", moduleName, cswSearch[t].high, cswSearch[t].low);
        Modify_SPI_Reg_bits(addrCSW_VCO, msb, lsb, cswSearch[t].high);
        //binary search for and high value, and on the way store approximate low value
        lime::debug("binary search:");
        for (int i = 6; i >= 0; --i)
        {
            cswSearch[t].high |= 1 << i; //CSW_VCO<i>=1
            Modify_SPI_Reg_bits(addrCSW_VCO, msb, lsb, cswSearch[t].high);
            std::this_thread::sleep_for(settlingTime);
            cmphl = (uint8_t)Get_SPI_Reg_bits(addrCMP, 13, 12, true);
            lime::debug("csw=%d\tcmphl=%d", cswSearch[t].high, (int16_t)cmphl);
            if (cmphl & 0x01) // reduce CSW
                cswSearch[t].high &= ~(1 << i); //CSW_VCO<i>=0
            if (cmphl == 2 && cswSearch[t].high < cswSearch[t].low)
            {
                cswSearch[t].low = cswSearch[t].high;
                hadLock = true;
            }
        }
        //linear search to make sure there are no gaps, and move away from edge case
        lime::debug("adjust with linear search:");
        while (cswSearch[t].low <= cswSearch[t].high && cswSearch[t].low > t * 128)
        {
            --cswSearch[t].low;
            Modify_SPI_Reg_bits(addrCSW_VCO, msb, lsb, cswSearch[t].low);
            std::this_thread::sleep_for(settlingTime);
            const uint8_t tempCMPvalue = Get_SPI_Reg_bits(addrCMP, 13, 12, true);
            lime::debug("csw=%d\tcmphl=%d", cswSearch[t].low, (int16_t)tempCMPvalue);
            if (tempCMPvalue != 2)
            {
                ++cswSearch[t].low;
                break;
            }
        }
        if (hadLock)
        {
            lime::debug("CSW: lowest=%d, highest=%d, will use=%d",
                cswSearch[t].low,
                cswSearch[t].high,
                cswSearch[t].low + (cswSearch[t].high - cswSearch[t].low) / 2);
        }
        else
            lime::debug("CSW interval failed to lock");
    }

    //check if the intervals are joined
    int16_t cswHigh, cswLow;
    if (cswSearch[0].high == cswSearch[1].low - 1)
    {
        cswHigh = cswSearch[1].high;
        cswLow = cswSearch[0].low;
        lime::debug("CSW is locking in one continous range: low=%d, high=%d", cswLow, cswHigh);
    }
    //compare which interval is wider
    else
    {
        uint8_t intervalIndex = (cswSearch[1].high - cswSearch[1].low > cswSearch[0].high - cswSearch[0].low);
        cswHigh = cswSearch[intervalIndex].high;
        cswLow = cswSearch[intervalIndex].low;
        lime::debug("choosing wider CSW locking range: low=%d, high=%d", cswLow, cswHigh);
    }

    uint8_t finalCSW = 0;
    if (cswHigh - cswLow == 1)
    {
        lime::debug("TuneVCO(%s) - narrow locking values range detected [%i:%i]. VCO lock status might change with temperature.",
            moduleName,
            cswLow,
            cswHigh);
        //check which of two values really locks
        finalCSW = cswLow;
        Modify_SPI_Reg_bits(addrCSW_VCO, msb, lsb, cswLow);
        std::this_thread::sleep_for(settlingTime);
        cmphl = (uint8_t)Get_SPI_Reg_bits(addrCMP, 13, 12, true);
        if (cmphl != 2)
        {
            finalCSW = cswHigh;
            Modify_SPI_Reg_bits(addrCSW_VCO, msb, lsb, cswHigh);
        }
    }
    else
    {
        finalCSW = cswLow + (cswHigh - cswLow) / 2;
        Modify_SPI_Reg_bits(addrCSW_VCO, msb, lsb, finalCSW);
    }
    std::this_thread::sleep_for(settlingTime);
    cmphl = (uint8_t)Get_SPI_Reg_bits(addrCMP, 13, 12, true);
    if (cmphl == 2)
    {
        lime::debug("TuneVCO(%s) - confirmed lock with final csw=%i, cmphl=%i", moduleName, finalCSW, cmphl);
        return 0;
    }
    lime::debug("TuneVCO(%s) - failed lock with final csw=%i, cmphl=%i", moduleName, finalCSW, cmphl);
    return -1;
}

uint16_t LMS7002M::Get_SPI_Reg_bits(const LMS7Parameter& param, bool fromChip)
{
    return Get_SPI_Reg_bits(param.address, param.msb, param.lsb, fromChip);
}

uint16_t LMS7002M::Get_SPI_Reg_bits(uint16_t address, uint8_t msb, uint8_t lsb, bool fromChip)
{
    return (SPI_read(address, fromChip) & (~(~0u << (msb + 1)))) >> lsb; //shift bits to LSB
}

int LMS7002M::Modify_SPI_Reg_bits(const LMS7Parameter& param, const uint16_t value, bool fromChip)
{
    return Modify_SPI_Reg_bits(param.address, param.msb, param.lsb, value, fromChip);
}

int LMS7002M::Modify_SPI_Reg_bits(const uint16_t address, const uint8_t msb, const uint8_t lsb, const uint16_t value, bool fromChip)
{
    uint16_t spiDataReg = SPI_read(address, fromChip); //read current SPI reg data
    uint16_t spiMask = (~(~0u << (msb - lsb + 1))) << (lsb); // creates bit mask
    spiDataReg = (spiDataReg & (~spiMask)) | ((value << lsb) & spiMask); //clear bits
    return SPI_write(address, spiDataReg); //write modified data back to SPI reg
}

/** @brief Modifies given registers with values applied using masks
    @param addr array of register addresses
    @param masks array of applied masks
    @param values array of values to be written
    @param start starting index of given arrays
    @param stop end index of given arrays
*/
int LMS7002M::Modify_SPI_Reg_mask(const uint16_t* addr, const uint16_t* masks, const uint16_t* values, uint8_t start, uint8_t stop)
{
    int status = 0;
    uint16_t reg_data;
    std::vector<uint16_t> addresses;
    std::vector<uint16_t> data;
    while (start <= stop)
    {
        reg_data = SPI_read(addr[start], true, &status); //read current SPI reg data
        reg_data &= ~masks[start]; //clear bits
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

const LMS7Parameter& LMS7002M::GetParam(const std::string& name)
{
    for (const LMS7Parameter& parameter : LMS7parameterList)
    {
        if (std::string(parameter.name) == name)
        {
            return parameter;
        }
    }

    throw std::logic_error("Parameter " + name + " not found");
}

int LMS7002M::SetFrequencySX(TRXDir dir, float_type freq_Hz, SX_details* output)
{
    static std::map<float_type, int8_t> tuning_cache_sel_vco;
    static std::map<float_type, int16_t> tuning_cache_csw_value;

    assert(freq_Hz > 0);

    const char* vcoNames[] = { "VCOL", "VCOM", "VCOH" };
    const uint8_t sxVCO_N = 2; //number of entries in VCO frequencies
    const float_type m_dThrF = 5500e6; //threshold to enable additional divider
    float_type VCOfreq;
    int8_t div_loch;
    int8_t sel_vco;
    bool canDeliverFrequency = false;
    uint16_t integerPart;
    uint32_t fractionalPart;
    int16_t csw_value;

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
        return ReportError(ERANGE,
            "SetFrequencySX%s(%g MHz) - required VCO frequencies are out of range [%g-%g] MHz",
            dir == TRXDir::Tx ? "T" : "R",
            freq_Hz / 1e6,
            gVCO_frequency_table[0][0] / 1e6,
            gVCO_frequency_table[2][sxVCO_N - 1] / 1e6);

    const float_type refClk_Hz = GetReferenceClk_SX(dir);
    assert(refClk_Hz > 0);
    integerPart = (uint16_t)(VCOfreq / (refClk_Hz * (1 + (VCOfreq > m_dThrF))) - 4);
    fractionalPart = (uint32_t)((VCOfreq / (refClk_Hz * (1 + (VCOfreq > m_dThrF))) -
                                    (uint32_t)(VCOfreq / (refClk_Hz * (1 + (VCOfreq > m_dThrF))))) *
                                1048576);

    ChannelScope scope(this);
    this->SetActiveChannel(dir == TRXDir::Tx ? Channel::ChSXT : Channel::ChSXR);
    Modify_SPI_Reg_bits(LMS7param(EN_INTONLY_SDM), 0);
    Modify_SPI_Reg_bits(LMS7param(INT_SDM), integerPart); //INT_SDM
    Modify_SPI_Reg_bits(0x011D, 15, 0, fractionalPart & 0xFFFF); //FRAC_SDM[15:0]
    Modify_SPI_Reg_bits(0x011E, 3, 0, (fractionalPart >> 16)); //FRAC_SDM[19:16]
    Modify_SPI_Reg_bits(LMS7param(DIV_LOCH), div_loch); //DIV_LOCH
    Modify_SPI_Reg_bits(LMS7param(EN_DIV2_DIVPROG), (VCOfreq > m_dThrF)); //EN_DIV2_DIVPROG

    lime::info("SetFrequencySX%s, (%.3f MHz)INT %d, FRAC %d, DIV_LOCH %d, EN_DIV2_DIVPROG %d",
        dir == TRXDir::Tx ? "T" : "R",
        freq_Hz / 1e6,
        integerPart,
        fractionalPart,
        (int16_t)div_loch,
        (VCOfreq > m_dThrF));
    lime::debug("Expected VCO %.2f MHz, RefClk %.2f MHz", VCOfreq / 1e6, refClk_Hz / 1e6);

    if (output)
    {
        output->frequency = freq_Hz;
        output->frequencyVCO = VCOfreq;
        output->referenceClock = GetReferenceClk_SX(dir);
        output->INT = integerPart;
        output->FRAC = fractionalPart;
        output->en_div2_divprog = (VCOfreq > m_dThrF);
        output->div_loch = div_loch;
    }

    // turn on VCO and comparator
    Modify_SPI_Reg_bits(LMS7param(PD_VCO), 0); //
    Modify_SPI_Reg_bits(LMS7param(PD_VCO_COMP), 0);

    // try setting tuning values from the cache, if it fails perform full tuning
    if (useCache && tuning_cache_sel_vco.count(freq_Hz) > 0)
    {
        sel_vco = tuning_cache_sel_vco[freq_Hz];
        csw_value = tuning_cache_csw_value[freq_Hz];
        Modify_SPI_Reg_bits(LMS7param(SEL_VCO), sel_vco);
        Modify_SPI_Reg_bits(LMS7param(CSW_VCO).address, LMS7param(CSW_VCO).msb, LMS7param(CSW_VCO).lsb, csw_value);
        // probably no need for this as the interface is already very slow..
        std::this_thread::sleep_for(std::chrono::microseconds(50));
        auto cmphl = (uint8_t)Get_SPI_Reg_bits(LMS7param(VCO_CMPHO).address, 13, 12, true);
        if (cmphl == 2)
        {
            lime::info("Fast Tune success; vco=%d value=%d", tuning_cache_sel_vco[freq_Hz], tuning_cache_csw_value[freq_Hz]);
            if (output)
            {
                output->success = true;
                output->sel_vco = sel_vco;
                output->csw = csw_value;
            }
            return 0;
        }
    }

    canDeliverFrequency = false;
    int tuneScore[] = { -128, -128, -128 }; //best is closest to 0
    for (int i = 0; i < 5; i++) //attempt tune multiple times
    {
        for (sel_vco = 0; sel_vco < 3; ++sel_vco)
        {
            lime::debug("Tuning %s :", vcoNames[sel_vco]);
            Modify_SPI_Reg_bits(LMS7param(SEL_VCO), sel_vco);
            int status = TuneVCO(dir == TRXDir::Tx ? VCO_Module::VCO_SXT : VCO_Module::VCO_SXR);
            if (status == 0)
            {
                tuneScore[sel_vco] = -128 + Get_SPI_Reg_bits(LMS7param(CSW_VCO), true);
                canDeliverFrequency = true;
            }
            if (status == 0)
                lime::debug("%s : csw=%d %s", vcoNames[sel_vco], tuneScore[sel_vco] + 128, (status == 0 ? "tune ok" : "tune fail"));
            else
                lime::debug("%s : failed to lock", vcoNames[sel_vco]);
        }
        if (canDeliverFrequency) //tune OK
            break;
        auto bias = Get_SPI_Reg_bits(LMS7param(ICT_VCO));
        if (bias == 255)
            break;
        bias = bias + 32 > 255 ? 255 : bias + 32; //retry with higher bias current
        Modify_SPI_Reg_bits(LMS7param(ICT_VCO), bias);
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
    lime::debug("Selected: %s, CSW_VCO: %i", vcoNames[sel_vco], csw_value);

    if (output)
    {
        if (canDeliverFrequency)
            output->success = true;
        output->sel_vco = sel_vco;
        output->csw = csw_value;
    }
    Modify_SPI_Reg_bits(LMS7param(SEL_VCO), sel_vco);
    Modify_SPI_Reg_bits(LMS7param(CSW_VCO), csw_value);

    // save successful tuning results in cache
    if (useCache && canDeliverFrequency)
    {
        tuning_cache_sel_vco[freq_Hz] = sel_vco;
        tuning_cache_csw_value[freq_Hz] = csw_value;
    }

    if (canDeliverFrequency == false)
        return ReportError("SetFrequencySX%s(%g MHz) - cannot deliver frequency", dir == TRXDir::Tx ? "T" : "R", freq_Hz / 1e6);
    return 0;
}

int LMS7002M::SetFrequencySXWithSpurCancelation(TRXDir dir, float_type freq_Hz, float_type BW)
{
    const float BWOffset = 2e6;
    BW += BWOffset; //offset to avoid ref clock on BW edge
    bool needCancelation = false;
    float_type refClk = GetReferenceClk_SX(TRXDir::Rx);
    int low = (freq_Hz - BW / 2) / refClk;
    int high = (freq_Hz + BW / 2) / refClk;
    if (low != high)
        needCancelation = true;

    int status;
    float newFreq(0);
    if (needCancelation)
    {
        newFreq = (int)(freq_Hz / refClk + 0.5) * refClk;
        TuneRxFilter(BW - BWOffset + 2 * abs(freq_Hz - newFreq));
        status = SetFrequencySX(dir, newFreq);
    }
    else
        status = SetFrequencySX(dir, freq_Hz);
    if (status != 0)
        return status;
    const int ch = Get_SPI_Reg_bits(LMS7param(MAC));
    for (int i = 0; i < 2; ++i)
    {
        Modify_SPI_Reg_bits(LMS7param(MAC), i + 1);
        SetNCOFrequency(TRXDir::Rx, 15, 0);
    }
    if (needCancelation)
    {
        Modify_SPI_Reg_bits(LMS7param(MAC), ch);
        Modify_SPI_Reg_bits(LMS7param(EN_INTONLY_SDM), 1);

        /*uint16_t gINT = Get_SPI_Reg_bits(0x011E, 13, 0);	// read whole register to reduce SPI transfers
        uint32_t gFRAC = ((gINT&0xF) * 65536) | Get_SPI_Reg_bits(0x011D, 15, 0);
        bool upconvert = gFRAC > (1 << 19);
        gINT = gINT >> 4;
        if(upconvert)
        {
            gINT+=;
            Modify_SPI_Reg_bits(LMS7param(INT_SDM), gINT);
        }
        Modify_SPI_Reg_bits(0x011D, 15, 0, 0);
        Modify_SPI_Reg_bits(0x011E, 3, 0, 0);*/
        //const float_type refClk_Hz = GetReferenceClk_SX(dir);
        //float actualFreq = (float_type)refClk_Hz / (1 << (Get_SPI_Reg_bits(LMS7param(DIV_LOCH)) + 1));
        //actualFreq *= (gINT + 4) * (Get_SPI_Reg_bits(LMS7param(EN_DIV2_DIVPROG)) + 1);
        float actualFreq = newFreq;
        float userFreq = freq_Hz;
        bool upconvert = actualFreq > userFreq;
        for (int i = 0; i < 2; ++i)
        {
            Modify_SPI_Reg_bits(LMS7param(MAC), i + 1);
            Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP), !upconvert);
            Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 0);
            Modify_SPI_Reg_bits(LMS7param(SEL_RX), 15);
            Modify_SPI_Reg_bits(LMS7param(CMIX_GAIN_RXTSP), 1);
            SetNCOFrequency(TRXDir::Rx, 14, 0);
            SetNCOFrequency(TRXDir::Rx, 15, abs(actualFreq - userFreq));
        }
    }

    Modify_SPI_Reg_bits(LMS7param(MAC), ch);
    return 0;
}

float_type LMS7002M::GetFrequencySX(TRXDir dir)
{
    ChannelScope(this, dir == TRXDir::Tx ? Channel::ChSXT : Channel::ChSXR);

    float_type dMul;
    uint16_t gINT = Get_SPI_Reg_bits(0x011E, 13, 0); // read whole register to reduce SPI transfers
    uint32_t gFRAC = ((gINT & 0xF) * 65536) | Get_SPI_Reg_bits(0x011D, 15, 0);

    const float_type refClk_Hz = GetReferenceClk_SX(dir);
    dMul = (float_type)refClk_Hz / (1 << (Get_SPI_Reg_bits(LMS7param(DIV_LOCH)) + 1));
    //Calculate real frequency according to the calculated parameters
    dMul = dMul * ((gINT >> 4) + 4 + (float_type)gFRAC / 1048576.0) * (Get_SPI_Reg_bits(LMS7param(EN_DIV2_DIVPROG)) + 1);
    return dMul;
}

int LMS7002M::SetNCOFrequency(TRXDir dir, uint8_t index, float_type freq_Hz)
{
    if (index > 15)
        return ReportError(ERANGE, "SetNCOFrequency(index = %d) - index out of range [0, 15]", int(index));
    float_type refClk_Hz = GetReferenceClk_TSP(dir);
    if (freq_Hz < 0 || freq_Hz / refClk_Hz > 0.5)
        return ReportError(ERANGE,
            "SetNCOFrequency(index = %d) - Frequency(%g MHz) out of range [0-%g) MHz",
            int(index),
            freq_Hz / 1e6,
            refClk_Hz / 2e6);
    uint16_t addr = dir == TRXDir::Tx ? 0x0240 : 0x0440;
    uint32_t fcw = uint32_t((freq_Hz / refClk_Hz) * 4294967296);
    SPI_write(addr + 2 + index * 2, (fcw >> 16)); //NCO frequency control word register MSB part.
    SPI_write(addr + 3 + index * 2, fcw); //NCO frequency control word register LSB part.
    return 0;
}

float_type LMS7002M::GetNCOFrequency(TRXDir dir, uint8_t index, bool fromChip)
{
    if (index > 15)
        return ReportError(ERANGE, "GetNCOFrequency_MHz(index = %d) - index out of range [0, 15]", int(index));
    float_type refClk_Hz = GetReferenceClk_TSP(dir);
    uint16_t addr = dir == TRXDir::Tx ? 0x0240 : 0x0440;
    uint32_t fcw = 0;
    fcw |= SPI_read(addr + 2 + index * 2, fromChip) << 16; //NCO frequency control word register MSB part.
    fcw |= SPI_read(addr + 3 + index * 2, fromChip); //NCO frequency control word register LSB part.
    return refClk_Hz * (fcw / 4294967296.0);
}

int LMS7002M::SetNCOPhaseOffsetForMode0(TRXDir dir, float_type angle_deg)
{
    uint16_t addr = dir == TRXDir::Tx ? 0x0241 : 0x0441;
    uint16_t pho = (uint16_t)(65536 * (angle_deg / 360));
    SPI_write(addr, pho);
    return 0;
}

int LMS7002M::SetNCOPhaseOffset(TRXDir dir, uint8_t index, float_type angle_deg)
{
    if (index > 15)
        return ReportError(ERANGE, "SetNCOPhaseOffset(index = %d) - index out of range [0, 15]", int(index));
    uint16_t addr = dir == TRXDir::Tx ? 0x0244 : 0x0444;
    uint16_t pho = (uint16_t)(65536 * (angle_deg / 360));
    SPI_write(addr + index, pho);
    return 0;
}

int LMS7002M::SetNCOPhases(TRXDir dir, const float_type* angles_deg, uint8_t count, float_type frequencyOffset)
{
    if (SetNCOFrequency(dir, 0, frequencyOffset) != 0)
        return -1;

    if (angles_deg != nullptr)
    {
        for (uint8_t i = 0; i < 16 && i < count; i++)
            if (SetNCOPhaseOffset(dir, i, angles_deg[i]) != 0)
                return -1;
        if (Modify_SPI_Reg_bits(dir == TRXDir::Tx ? LMS7_SEL_TX : LMS7_SEL_RX, 0) != 0)
            return -1;
    }
    return 0;
}

std::vector<float_type> LMS7002M::GetNCOPhases(TRXDir dir, float_type* frequencyOffset)
{
    std::vector<float_type> angles_deg;
    return angles_deg;
}

float_type LMS7002M::GetNCOPhaseOffset_Deg(TRXDir dir, uint8_t index)
{
    if (index > 15)
        return ReportError(ERANGE, "GetNCOPhaseOffset_Deg(index = %d) - index out of range [0, 15]", int(index));
    uint16_t addr = dir == TRXDir::Tx ? 0x0244 : 0x0444;
    uint16_t pho = SPI_read(addr + index);
    float_type angle = 360 * pho / 65536.0;
    return angle;
}

int LMS7002M::SetGFIRCoefficients(TRXDir dir, uint8_t gfirIndex, const float_type* coef, uint8_t coefCount)
{
    if (gfirIndex > 2)
    {
        lime::warning("SetGFIRCoefficients: Invalid GFIR index(%i). Will configure GFIR[2].", gfirIndex);
        gfirIndex = 2;
    }

    const uint16_t startAddr = 0x0280 + (gfirIndex * 0x40) + (dir == TRXDir::Tx ? 0 : 0x0200);
    const uint8_t maxCoefCount = gfirIndex < 2 ? 40 : 120;
    const uint8_t bankCount = gfirIndex < 2 ? 5 : 15;

    if (coefCount > maxCoefCount)
    {
        return ReportError(ERANGE,
            "SetGFIRCoefficients: too many coefficients(%i), GFIR[%i] can have only %i",
            coefCount,
            gfirIndex,
            maxCoefCount);
    }

    uint16_t addrs[120];
    int16_t words[120];
    // actual used coefficients count is multiple of 'bankCount'
    // if coefCount is not multiple, extra '0' coefficients will be written
    const uint8_t bankLength = ceil((float)coefCount / bankCount);
    const int16_t actualCoefCount = bankLength * bankCount;
    assert(actualCoefCount <= maxCoefCount);

    for (int i = 0; i < actualCoefCount; ++i)
    {
        uint8_t bank = i / bankLength;
        uint8_t bankRow = i % bankLength;
        addrs[i] = startAddr + (bank * 8) + bankRow;
        addrs[i] += 24 * (bank / 5);

        if (i < coefCount)
        {
            words[i] = coef[i] * 32767;

            if (coef[i] < -1 || coef[i] > 1)
            {
                lime::warning("Coefficient %f is outside of range [-1:1], incorrect value will be written.", coef[i]);
            }
        }
        else
        {
            words[i] = 0;
        }
    }
    LMS7Parameter gfirL_param = LMS7param(GFIR1_L_TXTSP);
    gfirL_param.address += gfirIndex + (dir == TRXDir::Tx ? 0 : 0x0200);
    Modify_SPI_Reg_bits(gfirL_param, bankLength - 1);

    return SPI_write_batch(addrs, (const uint16_t*)words, actualCoefCount, true);
}

int LMS7002M::GetGFIRCoefficients(TRXDir dir, uint8_t gfirIndex, float_type* coef, uint8_t coefCount)
{
    int status = -1;

    if (gfirIndex > 2)
    {
        lime::warning("GetGFIRCoefficients: Invalid GFIR index(%i). Will read GFIR[2].", gfirIndex);
        gfirIndex = 2;
    }

    const uint16_t startAddr = 0x0280 + (gfirIndex * 0x40) + (dir == TRXDir::Tx ? 0 : 0x0200);
    const uint8_t coefLimit = gfirIndex < 2 ? 40 : 120;

    if (coefCount > coefLimit)
    {
        return ReportError(ERANGE, "GetGFIRCoefficients(coefCount=%d) - exceeds coefLimit=%d", coefCount, coefLimit);
    }

    std::vector<uint16_t> addresses;
    for (uint8_t index = 0; index < coefCount; ++index)
    {
        addresses.push_back(startAddr + index + 24 * (index / 40));
    }

    int16_t spiData[120];
    std::memset(spiData, 0, 120 * sizeof(int16_t));
    if (controlPort)
    {
        status = SPI_read_batch(&addresses[0], reinterpret_cast<uint16_t*>(spiData), coefCount);
        for (uint8_t index = 0; index < coefCount; ++index)
        {
            coef[index] = spiData[index] / 32768.0;
        }
    }
    else
    {
        const int channel = Get_SPI_Reg_bits(LMS7param(MAC), false) > 1 ? 1 : 0;
        for (uint8_t index = 0; index < coefCount; ++index)
        {
            uint16_t value = mRegistersMap->GetValue(channel, addresses[index]);
            coef[index] = *reinterpret_cast<int16_t*>(&value) / 32768.0;
        }
        status = 0;
    }

    return status;
}

int LMS7002M::SPI_write(uint16_t address, uint16_t data, bool toChip)
{
    if (address == 0x0640 || address == 0x0641)
    {
        MCU_BD* mcu = GetMCUControls();
        mcu->RunProcedure(MCU_FUNCTION_GET_PROGRAM_ID);
        if (mcu->WaitForMCU(100) != MCU_ID_CALIBRATIONS_SINGLE_IMAGE)
            mcu->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, MCU_BD::MCU_PROG_MODE::SRAM);
        SPI_write(0x002D, address);
        SPI_write(0x020C, data);
        mcu->RunProcedure(7);
        mcu->WaitForMCU(50);
        return SPI_read(0x040B) == data ? 0 : -1;
    }
    else
        return this->SPI_write_batch(&address, &data, 1, toChip);
}

uint16_t LMS7002M::SPI_read(uint16_t address, bool fromChip, int* status)
{
    fromChip |= !useCache;
    //registers containing read only registers, which values can change
    static const std::unordered_set<uint16_t> volatileRegs = {
        0x0000,
        0x0001,
        0x0002,
        0x0003,
        0x0004,
        0x0005,
        0x0006,
        0x002F,
        0x008C,
        0x00A8,
        0x00A9,
        0x00AA,
        0x00AB,
        0x00AC,
        0x0123,
        0x0209,
        0x020A,
        0x020B,
        0x040E,
        0x040F,
        0x05C3,
        0x05C4,
        0x05C5,
        0x05C6,
        0x05C7,
        0x05C8,
        0x05C9,
        0x05CA,
    };
    if (volatileRegs.find(address) != volatileRegs.end())
        fromChip = true;

    if (!controlPort || fromChip == false)
    {
        if (status && !controlPort)
            *status = ReportError("chip not connected");
        uint8_t mac = mRegistersMap->GetValue(0, LMS7param(MAC).address) & 0x0003;
        uint8_t channel = (mac == 2) ? 1 : 0; //only when MAC is B -> use register space B
        if (address < 0x0100)
            channel = 0; //force A when below MAC mapped register space
        return mRegistersMap->GetValue(channel, address);
    }
    if (controlPort)
    {
        uint16_t data = 0;
        int st;
        if (address == 0x0640 || address == 0x0641)
        {
            MCU_BD* mcu = GetMCUControls();
            mcu->RunProcedure(MCU_FUNCTION_GET_PROGRAM_ID);
            if (mcu->WaitForMCU(100) != MCU_ID_CALIBRATIONS_SINGLE_IMAGE)
                mcu->Program_MCU(mcu_program_lms7_dc_iq_calibration_bin, MCU_BD::MCU_PROG_MODE::SRAM);
            SPI_write(0x002D, address);
            mcu->RunProcedure(8);
            mcu->WaitForMCU(50);
            uint16_t rdVal = SPI_read(0x040B, true, status);
            return rdVal;
        }
        else
            st = this->SPI_read_batch(&address, &data, 1);
        if (status != nullptr)
            *status = st;
        return data;
    }
    return 0;
}

int LMS7002M::SPI_write_batch(const uint16_t* spiAddr, const uint16_t* spiData, uint16_t cnt, bool toChip)
{
    toChip |= !useCache;
    int mac = mRegistersMap->GetValue(0, LMS7param(MAC).address) & 0x0003;
    std::vector<uint32_t> data;
    for (size_t i = 0; i < cnt; ++i)
    {
        //write which register cache based on MAC bits
        //or always when below the MAC mapped register space
        bool wr0 = ((mac & 0x1) != 0) || (spiAddr[i] < 0x0100);
        bool wr1 = ((mac & 0x2) != 0) && (spiAddr[i] >= 0x0100);

        if (!toChip)
        {
            if (wr0 && (mRegistersMap->GetValue(0, spiAddr[i]) == spiData[i]))
                wr0 = false;
            if (wr1 && (mRegistersMap->GetValue(1, spiAddr[i]) == spiData[i]))
                wr1 = false;
            if (!(wr0 || wr1))
                continue;
        }

        data.push_back((1 << 31) | (uint32_t(spiAddr[i]) << 16) | spiData[i]); //msbit 1=SPI write
        if (wr0)
            mRegistersMap->SetValue(0, spiAddr[i], spiData[i]);
        if (wr1)
            mRegistersMap->SetValue(1, spiAddr[i], spiData[i]);

        //refresh mac, because batch might also change active channel
        if (spiAddr[i] == LMS7param(MAC).address)
            mac = mRegistersMap->GetValue(0, LMS7param(MAC).address) & 0x0003;
    }

    if (data.size() == 0)
        return 0;
    if (!controlPort)
    {
        if (useCache)
            return 0;
        lime::error("No device connected");
        return -1;
    }
    controlPort->SPI(data.data(), nullptr, data.size());
    return 0;
}

int LMS7002M::SPI_read_batch(const uint16_t* spiAddr, uint16_t* spiData, uint16_t cnt)
{
    if (!controlPort)
    {
        lime::error("No device connected");
        return -1;
    }

    std::vector<uint32_t> dataWr(cnt);
    std::vector<uint32_t> dataRd(cnt);
    for (size_t i = 0; i < cnt; ++i)
    {
        dataWr[i] = (uint32_t)(spiAddr[i]);
    }

    controlPort->SPI(dataWr.data(), dataRd.data(), cnt);

    int mac = mRegistersMap->GetValue(0, LMS7param(MAC).address) & 0x0003;

    for (size_t i = 0; i < cnt; ++i)
    {
        spiData[i] = dataRd[i] & 0xffff;

        //write which register cache based on MAC bits
        //or always when below the MAC mapped register space
        bool wr0 = ((mac & 0x1) != 0) or (spiAddr[i] < 0x0100);
        bool wr1 = ((mac & 0x2) != 0) and (spiAddr[i] >= 0x0100);

        if (wr0)
            mRegistersMap->SetValue(0, spiAddr[i], spiData[i]);
        if (wr1)
            mRegistersMap->SetValue(1, spiAddr[i], spiData[i]);
    }
    return 0;
}

int LMS7002M::RegistersTest(const std::string& fileName)
{
    char chex[16];
    if (!controlPort)
    {
        lime::error("No device connected");
        return -1;
    }

    int status;
    ChannelScope scope(this);

    //backup both channel data for restoration after test
    std::vector<uint16_t> ch1Addresses;
    for (const auto& memorySectionPair : MemorySectionAddresses)
        for (uint16_t addr = memorySectionPair.second[0]; addr <= memorySectionPair.second[1]; ++addr)
            ch1Addresses.push_back(addr);
    std::vector<uint16_t> ch1Data;
    ch1Data.resize(ch1Addresses.size(), 0);

    //backup A channel
    this->SetActiveChannel(Channel::ChA);
    status = SPI_read_batch(&ch1Addresses[0], &ch1Data[0], ch1Addresses.size());
    if (status != 0)
        return status;

    std::vector<uint16_t> ch2Addresses;
    for (const auto& memorySectionPair : MemorySectionAddresses)
        for (uint16_t addr = memorySectionPair.second[0]; addr <= memorySectionPair.second[1]; ++addr)
            if (addr >= 0x0100)
                ch2Addresses.push_back(addr);
    std::vector<uint16_t> ch2Data;
    ch2Data.resize(ch2Addresses.size(), 0);

    this->SetActiveChannel(Channel::ChB);
    status = SPI_read_batch(&ch2Addresses[0], &ch2Data[0], ch2Addresses.size());
    if (status != 0)
        return status;

    //test registers
    ResetChip();
    Modify_SPI_Reg_bits(LMS7param(MIMO_SISO), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_TX_AFE2), 0);
    this->SetActiveChannel(Channel::ChA);

    std::stringstream ss;

    //check single channel memory sections
    std::vector<MemorySection> modulesToCheck = {
        MemorySection::AFE,
        MemorySection::BIAS,
        MemorySection::XBUF,
        MemorySection::CGEN,
        MemorySection::BIST,
        MemorySection::CDS,
        MemorySection::TRF,
        MemorySection::TBB,
        MemorySection::RFE,
        MemorySection::RBB,
        MemorySection::SX,
        MemorySection::TxTSP,
        MemorySection::TxNCO,
        MemorySection::TxGFIR1,
        MemorySection::TxGFIR2,
        MemorySection::TxGFIR3a,
        MemorySection::TxGFIR3b,
        MemorySection::TxGFIR3c,
        MemorySection::RxTSP,
        MemorySection::RxNCO,
        MemorySection::RxGFIR1,
        MemorySection::RxGFIR2,
        MemorySection::RxGFIR3a,
        MemorySection::RxGFIR3b,
        MemorySection::RxGFIR3c,
        MemorySection::LimeLight,
        MemorySection::LDO,
    };
    const std::string moduleNames[] = {
        "AFE",
        "BIAS",
        "XBUF",
        "CGEN",
        "BIST",
        "CDS",
        "TRF",
        "TBB",
        "RFE",
        "RBB",
        "SX",
        "TxTSP",
        "TxNCO",
        "TxGFIR1",
        "TxGFIR2",
        "TxGFIR3a",
        "TxGFIR3b",
        "TxGFIR3c",
        "RxTSP",
        "RxNCO",
        "RxGFIR1",
        "RxGFIR2",
        "RxGFIR3a",
        "RxGFIR3b",
        "RxGFIR3c",
        "LimeLight",
        "LDO",
    };

    const uint16_t patterns[] = { 0xAAAA, 0x5555 };
    const uint8_t patternsCount = 2;

    bool allTestSuccess = true;

    for (unsigned i = 0; i < modulesToCheck.size(); ++i)
    {
        bool moduleTestsSuccess = true;
        uint16_t startAddr = MemorySectionAddresses.at(modulesToCheck[i]).at(0);
        uint16_t endAddr = MemorySectionAddresses.at(modulesToCheck[i]).at(1);
        uint8_t channelCount = startAddr >= 0x0100 ? 2 : 1;
        for (int cc = 1; cc <= channelCount; ++cc)
        {
            Modify_SPI_Reg_bits(LMS7param(MAC), cc);
            sprintf(chex, "0x%04X", startAddr);
            ss << moduleNames[i] << "  [" << chex << ":";
            sprintf(chex, "0x%04X", endAddr);
            ss << chex << "]";
            if (startAddr >= 0x0100)
            {
                ss << " Ch." << (cc == 1 ? "A" : "B");
            }
            ss << std::endl;
            for (uint8_t p = 0; p < patternsCount; ++p)
                moduleTestsSuccess &= RegistersTestInterval(startAddr, endAddr, patterns[p], ss) == 0;
        }
        allTestSuccess &= moduleTestsSuccess;
    }

    //restore register values
    this->SetActiveChannel(Channel::ChA);
    SPI_write_batch(&ch1Addresses[0], &ch1Data[0], ch1Addresses.size(), true);
    this->SetActiveChannel(Channel::ChB);
    SPI_write_batch(&ch2Addresses[0], &ch2Data[0], ch2Addresses.size(), true);

    if (!fileName.empty())
    {
        std::fstream fout;
        fout.open(fileName, std::ios::out);
        fout << ss.str() << std::endl;
        fout.close();
    }

    if (allTestSuccess)
        return 0;
    lime::error("RegistersTest() failed");
    return -1;
}

/** @brief Performs registers test for given address interval by writing given pattern data
    @param startAddr first register address
    @param endAddr last reigster address
    @param pattern data to be written into registers
    @param ss stringstream to use
    @return 0-register test passed, other-failure
*/
int LMS7002M::RegistersTestInterval(uint16_t startAddr, uint16_t endAddr, uint16_t pattern, std::stringstream& ss)
{
    std::vector<uint16_t> addrToWrite;
    std::vector<uint16_t> dataToWrite;
    std::vector<uint16_t> dataReceived;
    std::vector<uint16_t> dataMasks;

    for (uint16_t addr = startAddr; addr <= endAddr; ++addr)
    {
        addrToWrite.push_back(addr);
    }
    dataMasks.resize(addrToWrite.size(), 0xFFFF);
    for (std::size_t j = 0; j < readOnlyRegisters.size(); ++j)
    {
        for (std::size_t k = 0; k < addrToWrite.size(); ++k)
        {
            if (readOnlyRegisters[j].address == addrToWrite[k])
            {
                dataMasks[k] = readOnlyRegisters[j].mask;
                break;
            }
        }
    }

    dataToWrite.clear();
    dataReceived.clear();
    for (uint16_t j = 0; j < addrToWrite.size(); ++j)
    {
        if (addrToWrite[j] == 0x00A6)
            dataToWrite.push_back(0x1 | (pattern & ~0x2));
        else if (addrToWrite[j] == 0x0084)
            dataToWrite.push_back(pattern & ~0x19);
        else
            dataToWrite.push_back(pattern & dataMasks[j]);
    }
    dataReceived.resize(addrToWrite.size(), 0);
    int status;
    status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size(), true);
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
            ss << ctemp << std::endl;
        }
    }
    if (registersMatch)
    {
        sprintf(ctemp, "0x%04X", pattern);
        ss << "\tRegisters OK (" << ctemp << ")\n";
    }
    if (registersMatch)
        return 0;
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

int LMS7002M::SetDefaults(MemorySection module)
{
    int status = 0;
    std::vector<uint16_t> addrs;
    std::vector<uint16_t> values;
    for (uint16_t address = MemorySectionAddresses.at(module).at(0); address <= MemorySectionAddresses.at(module).at(1); ++address)
    {
        addrs.push_back(address);
        values.push_back(mRegistersMap->GetDefaultValue(address));
    }
    status = SPI_write_batch(&addrs[0], &values[0], addrs.size());
    return status;
}

void LMS7002M::ModifyRegistersDefaults(const std::vector<std::pair<uint16_t, uint16_t>>& registerValues)
{
    for (const auto& addrValuePair : registerValues)
        mRegistersMap->SetDefaultValue(addrValuePair.first, addrValuePair.second);
}

bool LMS7002M::IsSynced()
{
    if (!controlPort)
        return false;

    ChannelScope scope(this);

    std::vector<uint16_t> addrToRead = mRegistersMap->GetUsedAddresses(0);
    std::vector<uint16_t> dataReceived;
    dataReceived.resize(addrToRead.size(), 0);

    this->SetActiveChannel(Channel::ChA);
    std::vector<uint32_t> dataWr(addrToRead.size());
    std::vector<uint32_t> dataRd(addrToRead.size());
    for (size_t i = 0; i < addrToRead.size(); ++i)
        dataWr[i] = (uint32_t(addrToRead[i]) << 16);
    controlPort->SPI(dataWr.data(), dataRd.data(), dataWr.size());

    for (size_t i = 0; i < addrToRead.size(); ++i)
        dataReceived[i] = dataRd[i] & 0xFFFF;

    //check if local copy matches chip
    for (uint16_t i = 0; i < addrToRead.size(); ++i)
    {
        uint16_t regValue = mRegistersMap->GetValue(0, addrToRead[i]);
        if (addrToRead[i] <= readOnlyRegisters[readOnlyRegisters.size() - 1].address &&
            addrToRead[i] >= readOnlyRegisters[0].address)
        {
            //mask out readonly bits
            for (std::size_t j = 0; j < readOnlyRegisters.size(); ++j)
            {
                if (readOnlyRegisters[j].address == addrToRead[i])
                {
                    dataReceived[i] &= readOnlyRegisters[j].mask;
                    regValue &= readOnlyRegisters[j].mask;
                    break;
                }
            }
        }
        if (dataReceived[i] != regValue)
        {
            lime::debug("Addr: 0x%04X  gui: 0x%04X  chip: 0x%04X", addrToRead[i], regValue, dataReceived[i]);
            return false;
        }
    }

    addrToRead.clear(); //add only B channel addresses
    addrToRead = mRegistersMap->GetUsedAddresses(1);
    dataWr.resize(addrToRead.size());
    dataRd.resize(addrToRead.size());
    for (size_t i = 0; i < addrToRead.size(); ++i)
        dataWr[i] = (uint32_t(addrToRead[i]) << 16);
    controlPort->SPI(dataWr.data(), dataRd.data(), dataWr.size());
    for (size_t i = 0; i < addrToRead.size(); ++i)
        dataReceived[i] = dataRd[i] & 0xFFFF;
    this->SetActiveChannel(Channel::ChB);

    //check if local copy matches chip
    for (uint16_t i = 0; i < addrToRead.size(); ++i)
    {
        uint16_t regValue = mRegistersMap->GetValue(1, addrToRead[i]);
        if (addrToRead[i] <= readOnlyRegisters[readOnlyRegisters.size() - 1].address &&
            addrToRead[i] >= readOnlyRegisters[0].address)
        {
            //mask out readonly bits
            for (std::size_t j = 0; j < readOnlyRegisters.size(); ++j)
            {
                if (readOnlyRegisters[j].address == addrToRead[i])
                {
                    dataReceived[i] &= readOnlyRegisters[j].mask;
                    regValue &= readOnlyRegisters[j].mask;
                    break;
                }
            }
        }
        if (dataReceived[i] != regValue)
        {
            lime::debug("Addr: 0x%04X  gui: 0x%04X  chip: 0x%04X", addrToRead[i], regValue, dataReceived[i]);
            return false;
        }
    }

    return true;
}

int LMS7002M::UploadAll()
{
    if (!controlPort)
    {
        lime::error("No device connected");
        return -1;
    }

    ChannelScope scope(this);

    int status;

    std::vector<uint16_t> addrToWrite;
    std::vector<uint16_t> dataToWrite;

    uint16_t x0020_value = mRegistersMap->GetValue(0, 0x0020);
    this->SetActiveChannel(Channel::ChA); //select A channel

    addrToWrite = mRegistersMap->GetUsedAddresses(0);
    //remove 0x0020 register from list, to not change MAC
    addrToWrite.erase(std::find(addrToWrite.begin(), addrToWrite.end(), 0x0020));
    for (auto address : addrToWrite)
        dataToWrite.push_back(mRegistersMap->GetValue(0, address));

    status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size(), true);
    if (status != 0)
        return status;
    //after all channel A registers have been written, update 0x0020 register value
    status = SPI_write(0x0020, x0020_value);
    if (status != 0)
        return status;
    this->SetActiveChannel(Channel::ChB);
    if (status != 0)
        return status;

    addrToWrite = mRegistersMap->GetUsedAddresses(1);
    dataToWrite.clear();
    for (auto address : addrToWrite)
    {
        dataToWrite.push_back(mRegistersMap->GetValue(1, address));
    }
    this->SetActiveChannel(Channel::ChB); //select B channel
    status = SPI_write_batch(&addrToWrite[0], &dataToWrite[0], addrToWrite.size(), true);
    if (status != 0)
        return status;

    return 0;
}

int LMS7002M::DownloadAll()
{
    if (!controlPort)
    {
        lime::error("No device connected");
        return -1;
    }
    int status;
    ChannelScope scope(this, true);

    std::vector<uint16_t> addrToRead = mRegistersMap->GetUsedAddresses(0);
    std::vector<uint16_t> dataReceived;
    dataReceived.resize(addrToRead.size(), 0);
    this->SetActiveChannel(Channel::ChA);
    status = SPI_read_batch(&addrToRead[0], &dataReceived[0], addrToRead.size());
    if (status != 0)
        return status;

    for (uint16_t i = 0; i < addrToRead.size(); ++i)
    {
        mRegistersMap->SetValue(0, addrToRead[i], dataReceived[i]);
    }

    addrToRead.clear(); //add only B channel addresses
    addrToRead = mRegistersMap->GetUsedAddresses(1);
    dataReceived.resize(addrToRead.size(), 0);

    this->SetActiveChannel(Channel::ChB);
    status = SPI_read_batch(&addrToRead[0], &dataReceived[0], addrToRead.size());
    if (status != 0)
        return status;
    for (uint16_t i = 0; i < addrToRead.size(); ++i)
        mRegistersMap->SetValue(1, addrToRead[i], dataReceived[i]);

    return 0;
}

int LMS7002M::SetInterfaceFrequency(float_type cgen_freq_Hz, const uint8_t interpolation, const uint8_t decimation)
{
    int status = 0;
    status = Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), decimation);
    if (status != 0)
        return status;
    Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), interpolation);

    auto siso = Get_SPI_Reg_bits(LMS7_LML2_SISODDR);
    int mclk2src = Get_SPI_Reg_bits(LMS7param(MCLK2SRC));
    if (decimation == 7 || (decimation == 0 && siso == 0)) //bypass
    {
        Modify_SPI_Reg_bits(LMS7param(RXTSPCLKA_DIV), 0);
        Modify_SPI_Reg_bits(LMS7param(RXDIVEN), false);
        Modify_SPI_Reg_bits(LMS7param(MCLK2SRC), (mclk2src & 1) | 0x2);
    }
    else
    {
        uint8_t divider = (uint8_t)pow(2.0, decimation + siso);
        if (divider > 1)
            Modify_SPI_Reg_bits(LMS7param(RXTSPCLKA_DIV), (divider / 2) - 1);
        else
            Modify_SPI_Reg_bits(LMS7param(RXTSPCLKA_DIV), 0);
        Modify_SPI_Reg_bits(LMS7param(RXDIVEN), true);
        Modify_SPI_Reg_bits(LMS7param(MCLK2SRC), mclk2src & 1);
    }

    if (Get_SPI_Reg_bits(LMS7param(RX_MUX)) == 0)
    {
        bool mimoBypass = (decimation == 7) && (siso == 0);
        Modify_SPI_Reg_bits(LMS7param(RXRDCLK_MUX), mimoBypass ? 3 : 1);
        Modify_SPI_Reg_bits(LMS7param(RXWRCLK_MUX), mimoBypass ? 1 : 2);
    }

    siso = Get_SPI_Reg_bits(LMS7_LML1_SISODDR);
    int mclk1src = Get_SPI_Reg_bits(LMS7param(MCLK1SRC));
    if (interpolation == 7 || (interpolation == 0 && siso == 0)) //bypass
    {
        Modify_SPI_Reg_bits(LMS7param(TXTSPCLKA_DIV), 0);
        Modify_SPI_Reg_bits(LMS7param(TXDIVEN), false);
        status = Modify_SPI_Reg_bits(LMS7param(MCLK1SRC), (mclk1src & 1) | 0x2);
    }
    else
    {
        uint8_t divider = (uint8_t)pow(2.0, interpolation + siso);
        if (divider > 1)
            Modify_SPI_Reg_bits(LMS7param(TXTSPCLKA_DIV), (divider / 2) - 1);
        else
            Modify_SPI_Reg_bits(LMS7param(TXTSPCLKA_DIV), 0);
        Modify_SPI_Reg_bits(LMS7param(TXDIVEN), true);
        status = Modify_SPI_Reg_bits(LMS7param(MCLK1SRC), mclk1src & 1);
    }

    if (Get_SPI_Reg_bits(LMS7param(TX_MUX)) == 0)
    {
        bool mimoBypass = (interpolation == 7) && (siso == 0);
        Modify_SPI_Reg_bits(LMS7param(TXRDCLK_MUX), mimoBypass ? 0 : 2);
        Modify_SPI_Reg_bits(LMS7param(TXWRCLK_MUX), 0);
    }

    //clock rate already set because the readback frequency is pretty-close,
    //dont set the cgen frequency again to save time due to VCO selection
    // const auto freqDiff = std::abs(this->GetFrequencyCGEN() - cgen_freq_Hz);
    // if (not this->GetCGENLocked() or freqDiff > 10.0)
    {
        status = SetFrequencyCGEN(cgen_freq_Hz);
        if (status != 0)
            return status;
    }
    return status;
}

void LMS7002M::ConfigureLML_RF2BB(
    const LMLSampleSource s0, const LMLSampleSource s1, const LMLSampleSource s2, const LMLSampleSource s3)
{
    //map a sample source to a position
    const std::map<LMLSampleSource, uint16_t> m{
        { LMLSampleSource::AI, 1 },
        { LMLSampleSource::AQ, 0 },
        { LMLSampleSource::BI, 3 },
        { LMLSampleSource::BQ, 2 },
    };

    //load the same config on both LMLs
    //only one will get used based on direction
    this->Modify_SPI_Reg_bits(LMS7param(LML1_S3S), m.at(s3));
    this->Modify_SPI_Reg_bits(LMS7param(LML1_S2S), m.at(s2));
    this->Modify_SPI_Reg_bits(LMS7param(LML1_S1S), m.at(s1));
    this->Modify_SPI_Reg_bits(LMS7param(LML1_S0S), m.at(s0));

    this->Modify_SPI_Reg_bits(LMS7param(LML2_S3S), m.at(s3));
    this->Modify_SPI_Reg_bits(LMS7param(LML2_S2S), m.at(s2));
    this->Modify_SPI_Reg_bits(LMS7param(LML2_S1S), m.at(s1));
    this->Modify_SPI_Reg_bits(LMS7param(LML2_S0S), m.at(s0));
}

void LMS7002M::ConfigureLML_BB2RF(
    const LMLSampleSource s0, const LMLSampleSource s1, const LMLSampleSource s2, const LMLSampleSource s3)
{
    //map a sample source to a position
    const std::map<LMLSampleSource, uint16_t> m{
        { s3, 2 },
        { s2, 3 },
        { s0, 1 },
        { s1, 0 },
    };

    //load the same config on both LMLs
    //only one will get used based on direction
    this->Modify_SPI_Reg_bits(LMS7param(LML1_BQP), m.at(LMLSampleSource::BQ));
    this->Modify_SPI_Reg_bits(LMS7param(LML1_BIP), m.at(LMLSampleSource::BI));
    this->Modify_SPI_Reg_bits(LMS7param(LML1_AQP), m.at(LMLSampleSource::AQ));
    this->Modify_SPI_Reg_bits(LMS7param(LML1_AIP), m.at(LMLSampleSource::AI));

    this->Modify_SPI_Reg_bits(LMS7param(LML2_BQP), m.at(LMLSampleSource::BQ));
    this->Modify_SPI_Reg_bits(LMS7param(LML2_BIP), m.at(LMLSampleSource::BI));
    this->Modify_SPI_Reg_bits(LMS7param(LML2_AQP), m.at(LMLSampleSource::AQ));
    this->Modify_SPI_Reg_bits(LMS7param(LML2_AIP), m.at(LMLSampleSource::AI));
}

int LMS7002M::SetRxDCRemoval(const bool enable)
{
    this->Modify_SPI_Reg_bits(LMS7param(DC_BYP_RXTSP), enable ? 0 : 1);
    this->Modify_SPI_Reg_bits(LMS7param(DCCORR_AVG_RXTSP), 0x7);
    return 0;
}

int LMS7002M::EnableSXTDD(bool tdd)
{
    Modify_SPI_Reg_bits(LMS7_MAC, 2);
    Modify_SPI_Reg_bits(LMS7_PD_LOCH_T2RBUF, tdd ? 0 : 1);
    Modify_SPI_Reg_bits(LMS7_MAC, 1);
    return Modify_SPI_Reg_bits(LMS7_PD_VCO, tdd ? 1 : 0);
}

bool LMS7002M::GetRxDCRemoval(void)
{
    return this->Get_SPI_Reg_bits(LMS7param(DC_BYP_RXTSP)) == 0;
}

int LMS7002M::SetDCOffset(TRXDir dir, const float_type I, const float_type Q)
{
    const bool bypass = I == 0.0 and Q == 0.0;
    if (dir == TRXDir::Tx)
    {
        this->Modify_SPI_Reg_bits(LMS7param(DC_BYP_TXTSP), bypass ? 1 : 0);
        this->Modify_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP), std::lrint(I * 127));
        this->Modify_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP), std::lrint(Q * 127));
    }
    else
    {
        Modify_SPI_Reg_bits(LMS7param(EN_DCOFF_RXFE_RFE), bypass ? 0 : 1);
        unsigned val = std::lrint(std::abs(I * 63)) + (I < 0 ? 64 : 0);
        Modify_SPI_Reg_bits(LMS7param(DCOFFI_RFE), val);
        val = std::lrint(std::abs(Q * 63)) + (Q < 0 ? 64 : 0);
        Modify_SPI_Reg_bits(LMS7param(DCOFFQ_RFE), val);
    }
    return 0;
}

void LMS7002M::GetDCOffset(TRXDir dir, float_type& I, float_type& Q)
{
    if (dir == TRXDir::Tx)
    {
        I = int8_t(this->Get_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP))) / 127.0; //signed 8-bit
        Q = int8_t(this->Get_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP))) / 127.0; //signed 8-bit
    }
    else
    {
        auto i = Get_SPI_Reg_bits(LMS7param(DCOFFI_RFE));
        I = ((i & 0x40) ? -1.0 : 1.0) * (i & 0x3F) / 63.0;
        auto q = Get_SPI_Reg_bits(LMS7param(DCOFFQ_RFE));
        Q = ((q & 0x40) ? -1.0 : 1.0) * (q & 0x3F) / 63.0;
    }
}

int LMS7002M::SetIQBalance(const TRXDir dir, const float_type phase, const float_type gainI, const float_type gainQ)
{
    const bool bypassPhase = (phase == 0.0);
    const bool bypassGain = ((gainI == 1.0) and (gainQ == 1.0)) or ((gainI == 0.0) and (gainQ == 0.0));
    int iqcorr = std::lrint(2047 * (phase / (M_PI / 2)));
    int gcorri = std::lrint(2047 * gainI);
    int gcorrq = std::lrint(2047 * gainQ);

    this->Modify_SPI_Reg_bits(dir == TRXDir::Tx ? LMS7param(PH_BYP_TXTSP) : LMS7param(PH_BYP_RXTSP), bypassPhase ? 1 : 0);
    this->Modify_SPI_Reg_bits(dir == TRXDir::Tx ? LMS7param(GC_BYP_TXTSP) : LMS7param(GC_BYP_RXTSP), bypassGain ? 1 : 0);
    this->Modify_SPI_Reg_bits(dir == TRXDir::Tx ? LMS7param(IQCORR_TXTSP) : LMS7param(IQCORR_RXTSP), iqcorr);
    this->Modify_SPI_Reg_bits(dir == TRXDir::Tx ? LMS7param(GCORRI_TXTSP) : LMS7param(GCORRI_RXTSP), gcorri);
    this->Modify_SPI_Reg_bits(dir == TRXDir::Tx ? LMS7param(GCORRQ_TXTSP) : LMS7param(GCORRQ_RXTSP), gcorrq);
    return 0;
}

void LMS7002M::GetIQBalance(const TRXDir dir, float_type& phase, float_type& gainI, float_type& gainQ)
{
    int iqcorr = int16_t(this->Get_SPI_Reg_bits(dir == TRXDir::Tx ? LMS7param(IQCORR_TXTSP) : LMS7param(IQCORR_RXTSP)) << 4) >>
                 4; //sign extend 12-bit
    int gcorri =
        int16_t(this->Get_SPI_Reg_bits(dir == TRXDir::Tx ? LMS7param(GCORRI_TXTSP) : LMS7param(GCORRI_RXTSP))); //unsigned 11-bit
    int gcorrq =
        int16_t(this->Get_SPI_Reg_bits(dir == TRXDir::Tx ? LMS7param(GCORRQ_TXTSP) : LMS7param(GCORRQ_RXTSP))); //unsigned 11-bit

    phase = (M_PI / 2) * iqcorr / 2047.0;
    gainI = gcorri / 2047.0;
    gainQ = gcorrq / 2047.0;
}

void LMS7002M::EnableValuesCache(bool enabled)
{
    useCache = enabled;
}

bool LMS7002M::IsValuesCacheEnabled() const
{
    return useCache;
}

MCU_BD* LMS7002M::GetMCUControls() const
{
    return mcuControl;
}

float_type LMS7002M::GetTemperature()
{
    if (CalibrateInternalADC(32) != 0)
        return 0;
    Modify_SPI_Reg_bits(LMS7_RSSI_PD, 0);
    Modify_SPI_Reg_bits(LMS7_RSSI_RSSIMODE, 0);
    uint16_t biasMux = Get_SPI_Reg_bits(LMS7_MUX_BIAS_OUT);
    Modify_SPI_Reg_bits(LMS7_MUX_BIAS_OUT, 2);

    std::this_thread::sleep_for(std::chrono::microseconds(250));
    const uint16_t reg606 = SPI_read(0x0606, true);
    float Vtemp = (reg606 >> 8) & 0xFF;
    Vtemp *= 1.84;
    float Vptat = reg606 & 0xFF;
    Vptat *= 1.84;
    float Vdiff = Vptat - Vtemp;
    Vdiff /= 1.05;
    float temperature = 45.0 + Vdiff;
    Modify_SPI_Reg_bits(LMS7_MUX_BIAS_OUT, biasMux);
    lime::debug("Vtemp 0x%04X, Vptat 0x%04X, Vdiff = %.2f, temp= %.3f", (reg606 >> 8) & 0xFF, reg606 & 0xFF, Vdiff, temperature);
    return temperature;
}

void LMS7002M::SetLogCallback(std::function<void(const char*, LogType)> callback)
{
    log_callback = callback;
}

int LMS7002M::CopyChannelRegisters(const Channel src, const Channel dest, const bool copySX)
{
    ChannelScope scope(this);

    std::vector<uint16_t> addrToWrite;
    addrToWrite = mRegistersMap->GetUsedAddresses(1);
    if (!copySX)
    {
        const auto& SXMemoryAddresses = MemorySectionAddresses.at(MemorySection::SX);
        for (uint32_t address = SXMemoryAddresses.at(0); address <= SXMemoryAddresses.at(1); ++address)
            addrToWrite.erase(std::find(addrToWrite.begin(), addrToWrite.end(), address));
    }
    for (auto address : addrToWrite)
    {
        uint16_t data = mRegistersMap->GetValue(src == Channel::ChA ? 0 : 1, address);
        mRegistersMap->SetValue(dest == Channel::ChA ? 0 : 1, address, data);
    }
    if (controlPort)
        UploadAll();

    return 0;
}

int LMS7002M::CalibrateAnalogRSSI_DC_Offset()
{
    Modify_SPI_Reg_bits(LMS7_EN_INSHSW_W_RFE, 1);
    CalibrateInternalADC(0);
    Modify_SPI_Reg_bits(LMS7param(PD_RSSI_RFE), 0);
    Modify_SPI_Reg_bits(LMS7param(PD_TIA_RFE), 0);

    /*Modify_SPI_Reg_bits(LMS7param(RSSIDC_RSEL), 22);
    Modify_SPI_Reg_bits(LMS7param(RSSIDC_HYSCMP), 0);
    Modify_SPI_Reg_bits(LMS7param(RSSIDC_PD), 0);*/
    SPI_write(0x0640, 22 << 4);

    Modify_SPI_Reg_bits(LMS7param(RSSIDC_DCO2), 0);

    int value = -63;
    uint8_t wrValue = abs(value);
    if (value < 0)
        wrValue |= 0x40;
    Modify_SPI_Reg_bits(LMS7param(RSSIDC_DCO1), wrValue, true);
    uint8_t cmp = Get_SPI_Reg_bits(LMS7param(RSSIDC_CMPSTATUS), true);
    uint8_t cmpPrev = cmp;
    std::vector<int8_t> edges;
    for (value = -63; value < 64; ++value)
    {
        wrValue = abs(value);
        if (value < 0)
            wrValue |= 0x40;
        Modify_SPI_Reg_bits(LMS7param(RSSIDC_DCO1), wrValue, true);
        std::this_thread::sleep_for(std::chrono::microseconds(5));
        cmp = Get_SPI_Reg_bits(LMS7param(RSSIDC_CMPSTATUS), true);
        if (cmp != cmpPrev)
        {
            edges.push_back(value);
            cmpPrev = cmp;
        }
        if (edges.size() > 1)
            break;
    }
    if (edges.size() != 2)
    {
        lime::debug("Not found");
        return ReportError(EINVAL, "Failed to find value");
    }
    int8_t found = (edges[0] + edges[1]) / 2;
    wrValue = abs(found);
    if (found < 0)
        wrValue |= 0x40;
    Modify_SPI_Reg_bits(LMS7param(RSSIDC_DCO1), wrValue, true);
    lime::debug("Found %i", found);
    Modify_SPI_Reg_bits(LMS7_EN_INSHSW_W_RFE, 0);
    return 0;
}

double LMS7002M::GetClockFreq(ClockID clk_id)
{
    switch (clk_id)
    {
    case ClockID::CLK_REFERENCE:
        return GetReferenceClk_SX(TRXDir::Rx);
    case ClockID::CLK_SXR:
        return GetFrequencySX(TRXDir::Rx);
    case ClockID::CLK_SXT:
        return GetFrequencySX(TRXDir::Tx);
    case ClockID::CLK_CGEN:
        return GetFrequencyCGEN();
    case ClockID::CLK_RXTSP:
        return GetReferenceClk_TSP(TRXDir::Rx);
    case ClockID::CLK_TXTSP:
        return GetReferenceClk_TSP(TRXDir::Tx);
    default:
        lime::ReportError(EINVAL, "Invalid clock ID.");
        return 0;
    }
}

void LMS7002M::SetClockFreq(ClockID clk_id, double freq)
{
    switch (clk_id)
    {
    case ClockID::CLK_REFERENCE:
        // TODO: recalculate CGEN,SXR/T
        break;
    case ClockID::CLK_CGEN:
        SetFrequencyCGEN(freq, true, nullptr);
        break;
    case ClockID::CLK_SXR:
        SetFrequencySX(TRXDir::Rx, freq, nullptr);
        break;
    case ClockID::CLK_SXT:
        SetFrequencySX(TRXDir::Rx, freq, nullptr);
        break;
    case ClockID::CLK_RXTSP:
    case ClockID::CLK_TXTSP:
        throw std::logic_error("RxTSP/TxTSP Clocks are read only");
    default:
        throw std::logic_error("LMS7002M::SetClockFreq Unknown clock id");
    }
}

float_type LMS7002M::GetSampleRate(TRXDir dir, Channel ch)
{
    ChannelScope scope(this, ch);
    return GetSampleRate(dir);
}

float_type LMS7002M::GetSampleRate(TRXDir dir)
{
    const auto& parameter = dir == TRXDir::Tx ? LMS7_HBI_OVR_TXTSP : LMS7_HBD_OVR_RXTSP;

    uint16_t ratio = Get_SPI_Reg_bits(parameter);

    double interface_Hz = GetReferenceClk_TSP(dir);

    // If decimation/interpolation is 0 (2^1) or 7 (bypass), interface clocks should not be divided
    if (ratio != 7)
    {
        interface_Hz /= 2 * pow(2.0, ratio);
    }

    return interface_Hz;
}

int LMS7002M::SetGFIRFilter(TRXDir dir, Channel ch, bool enabled, double bandwidth)
{
    ChannelScope scope(this, ch);
    const bool bypassFIR = !enabled;
    if (dir == TRXDir::Tx)
    {
        Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_TXTSP), bypassFIR);
        Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_TXTSP), bypassFIR);
        Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_TXTSP), bypassFIR);
    }
    else
    {
        Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), bypassFIR);
        Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), bypassFIR);
        Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), bypassFIR);
        const bool sisoDDR = Get_SPI_Reg_bits(LMS7_LML1_SISODDR);
        const bool clockIsNotInverted = !(enabled | sisoDDR);
        if (ch == LMS7002M::Channel::ChB)
        {
            Modify_SPI_Reg_bits(LMS7param(CDSN_RXBLML), clockIsNotInverted);
            Modify_SPI_Reg_bits(LMS7param(CDS_RXBLML), enabled ? 3 : 0);
        }
        else
        {
            Modify_SPI_Reg_bits(LMS7param(CDSN_RXALML), clockIsNotInverted);
            Modify_SPI_Reg_bits(LMS7param(CDS_RXALML), enabled ? 3 : 0);
        }
    }
    if (!enabled)
        return 0;

    if (bandwidth <= 0)
        return -1;

    double w, w2;
    int L;
    int div = 1;

    bandwidth /= 1e6;
    double interface_MHz;
    int ratio;
    if (dir == TRXDir::Tx)
    {
        ratio = Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
    }
    else
    {
        ratio = Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
    }

    interface_MHz = GetReferenceClk_TSP(dir) / 1e6;
    if (ratio != 7)
        div = (2 << (ratio));

    w = (bandwidth / 2) / (interface_MHz / div);
    L = div > 8 ? 8 : div;
    div -= 1;

    w2 = w * 1.1;
    if (w2 > 0.495)
    {
        w2 = w * 1.05;
        if (w2 > 0.495)
        {
            lime::error("GFIR LPF cannot be set to the requested bandwidth (%f)", bandwidth);
            return -1;
        }
    }

    double coef[120];
    double coef2[40];

    GenerateFilter(L * 15, w, w2, 1.0, 0, coef);
    GenerateFilter(L * 5, w, w2, 1.0, 0, coef2);

    if (dir == TRXDir::Tx)
    {
        Modify_SPI_Reg_bits(LMS7param(GFIR1_N_TXTSP), div);
        Modify_SPI_Reg_bits(LMS7param(GFIR2_N_TXTSP), div);
        Modify_SPI_Reg_bits(LMS7param(GFIR3_N_TXTSP), div);
    }
    else
    {
        Modify_SPI_Reg_bits(LMS7param(GFIR1_N_RXTSP), div);
        Modify_SPI_Reg_bits(LMS7param(GFIR2_N_RXTSP), div);
        Modify_SPI_Reg_bits(LMS7param(GFIR3_N_RXTSP), div);
    }

    if ((SetGFIRCoefficients(dir, 0, coef2, L * 5) != 0) || (SetGFIRCoefficients(dir, 1, coef2, L * 5) != 0) ||
        (SetGFIRCoefficients(dir, 2, coef, L * 15) != 0))
        return -1;

    std::stringstream ss;
    ss << "LMS " << (dir == TRXDir::Tx ? "Tx" : "Rx") << " GFIR coefficients (BW: " << bandwidth << " MHz):\n";
    ss << "GFIR1 = GFIR2:";
    for (int i = 0; i < L * 5; ++i)
        ss << " " << coef2[i];
    ss << std::endl;
    ss << "GFIR3:";
    for (int i = 0; i < L * 15; ++i)
        ss << " " << coef[i];
    ss << std::endl;
    lime::info(ss.str());

    return ResetLogicRegisters();
}

void LMS7002M::SetOnCGENChangeCallback(CGENChangeCallbackType callback, void* userData)
{
    mCallback_onCGENChange = callback;
    mCallback_onCGENChange_userData = userData;
}
