#include "LimeSDR_X3.h"

#include <fcntl.h>
#include <sstream>

#include "Logger.h"
#include "LitePCIe.h"
#include "limesuite/LMS7002M.h"
#include "lms7002m/LMS7002M_validation.h"
#include "FPGA_common.h"
#include "TRXLooper_PCIE.h"
#include "FPGA_X3.h"
#include "LMS64CProtocol.h"
#include "DSP/Equalizer.h"
#include "limesuite/DeviceNode.h"
#include "CommonFunctions.h"
#include "SlaveSelectShim.h"

#include "mcu_program/common_src/lms7002m_calibrations.h"
#include "mcu_program/common_src/lms7002m_filters.h"
#include "MCU_BD.h"

#include "math.h"

namespace lime {

// X3 board specific subdevice ids
static const uint8_t SPI_LMS7002M_1 = 0;
static const uint8_t SPI_LMS7002M_2 = 1;
static const uint8_t SPI_LMS7002M_3 = 2;
static const uint8_t SPI_FPGA = 3;

static SDRDevice::CustomParameter cp_vctcxo_dac = { "VCTCXO DAC (volatile)", 0, 0, 65535, false };
static SDRDevice::CustomParameter cp_temperature = { "Board Temperature", 1, 0, 65535, true };

static SDRDevice::CustomParameter cp_lms1_tx1dac = { "LMS1 TX1DAC", 2, 0, 65535, false };
static SDRDevice::CustomParameter cp_lms1_tx2dac = { "LMS1 TX2DAC", 3, 0, 65535, false };

static const std::vector<std::pair<uint16_t, uint16_t>> lms1defaultsOverride = { //
    { 0x0022, 0x0FFF },
    { 0x0023, 0x5550 },
    { 0x002B, 0x0038 },
    { 0x002C, 0x0000 },
    { 0x002D, 0x0641 },
    { 0x0086, 0x4101 },
    { 0x0087, 0x5555 },
    { 0x0088, 0x0525 },
    { 0x0089, 0x1078 },
    { 0x008B, 0x218C },
    { 0x008C, 0x267B },
    { 0x00A6, 0x000F },
    { 0x00A9, 0x8000 },
    { 0x00AC, 0x2000 },
    { 0x0108, 0x218C },
    { 0x0109, 0x57C1 },
    { 0x010A, 0x154C },
    { 0x010B, 0x0001 },
    { 0x010C, 0x8865 },
    { 0x010D, 0x011A },
    { 0x010E, 0x0000 },
    { 0x010F, 0x3142 },
    { 0x0110, 0x2B14 },
    { 0x0111, 0x0000 },
    { 0x0112, 0x000C },
    { 0x0113, 0x03C2 },
    { 0x0114, 0x01F0 },
    { 0x0115, 0x000D },
    { 0x0118, 0x418C },
    { 0x0119, 0x5292 },
    { 0x011A, 0x3001 },
    { 0x011C, 0x8941 },
    { 0x011D, 0x0000 },
    { 0x011E, 0x0984 },
    { 0x0120, 0xE6C0 },
    { 0x0121, 0x3638 },
    { 0x0122, 0x0514 },
    { 0x0123, 0x200F },
    { 0x0200, 0x00E1 },
    { 0x0208, 0x017B },
    { 0x020B, 0x4000 },
    { 0x020C, 0x8000 },
    { 0x0400, 0x8081 },
    { 0x0404, 0x0006 },
    { 0x040B, 0x1020 },
    { 0x040C, 0x00FB }
};

static const std::vector<std::pair<uint16_t, uint16_t>> lms2and3defaultsOverride = { //
    { 0x0022, 0x0FFF },
    { 0x0023, 0x5550 },
    { 0x002B, 0x0038 },
    { 0x002C, 0x0000 },
    { 0x002D, 0x0641 },
    { 0x0082, 0x803E }, // Power down AFE ADCs/DACs
    { 0x0086, 0x4101 },
    { 0x0087, 0x5555 },
    { 0x0088, 0x0525 },
    { 0x0089, 0x1078 },
    { 0x008B, 0x218C },
    { 0x008C, 0x267B },
    { 0x00A6, 0x000F },
    { 0x00A9, 0x8000 },
    { 0x00AC, 0x2000 },
    { 0x0108, 0x218C },
    { 0x0109, 0x57C1 },
    { 0x010A, 0xD54C },
    { 0x010B, 0x0001 },
    { 0x010C, 0x8865 },
    { 0x010D, 0x011A },
    { 0x010E, 0x0000 },
    { 0x010F, 0x3142 },
    { 0x0110, 0x2B14 },
    { 0x0111, 0x0000 },
    { 0x0112, 0x000C },
    { 0x0113, 0x03C2 },
    { 0x0114, 0x01F0 },
    { 0x0115, 0x000D },
    { 0x0118, 0x418C },
    { 0x0119, 0xD292 },
    { 0x011A, 0x3001 },
    { 0x011C, 0x8941 },
    { 0x011D, 0x0000 },
    { 0x011E, 0x0984 },
    { 0x0120, 0xE6C0 },
    { 0x0121, 0x3638 },
    { 0x0122, 0x0514 },
    { 0x0123, 0x200F },
    { 0x0200, 0x00E1 },
    { 0x0208, 0x017B },
    { 0x020B, 0x4000 },
    { 0x020C, 0x8000 },
    { 0x0400, 0x8081 },
    { 0x0404, 0x0006 },
    { 0x040B, 0x1020 },
    { 0x040C, 0x00FB }
};

static inline void ValidateChannel(uint8_t channel)
{
    if (channel > 2)
        throw std::logic_error("invalid channel index");
}

// Callback for updating FPGA's interface clocks when LMS7002M CGEN is manually modified
int LimeSDR_X3::LMS1_UpdateFPGAInterface(void* userData)
{
    constexpr int chipIndex = 0;
    assert(userData != nullptr);
    LimeSDR_X3* pthis = static_cast<LimeSDR_X3*>(userData);
    // don't care about cgen changes while doing Config(), to avoid unnecessary fpga updates
    if (pthis->mConfigInProgress)
        return 0;
    LMS7002M* soc = pthis->mLMSChips[chipIndex];
    return UpdateFPGAInterfaceFrequency(*soc, *pthis->mFPGA, chipIndex);
}

// Do not perform any unnecessary configuring to device in constructor, so you
// could read back it's state for debugging purposes
LimeSDR_X3::LimeSDR_X3(std::shared_ptr<IComms> spiLMS7002M,
    std::shared_ptr<IComms> spiFPGA,
    std::vector<std::shared_ptr<LitePCIe>> trxStreams,
    std::shared_ptr<ISerialPort> control)
    : LMS7002M_SDRDevice()
    , mTRXStreamPorts(trxStreams)
    , mfpgaPort(spiFPGA)
    , mSerialPort(control)
    , mConfigInProgress(false)
{
    SDRDevice::Descriptor& desc = mDeviceDescriptor;

    LMS64CProtocol::FirmwareInfo fw;
    LMS64CProtocol::GetFirmwareInfo(*mSerialPort, fw);
    LMS64CProtocol::FirmwareToDescriptor(fw, desc);

    desc.spiSlaveIds = {
        { "LMS7002M_1", SPI_LMS7002M_1 }, { "LMS7002M_2", SPI_LMS7002M_2 }, { "LMS7002M_3", SPI_LMS7002M_3 }, { "FPGA", SPI_FPGA }
    };

    const std::unordered_map<eMemoryRegion, Region> eepromMap = { { eMemoryRegion::VCTCXO_DAC, { 16, 2 } } };

    desc.memoryDevices[MEMORY_DEVICES_TEXT.at(eMemoryDevice::FPGA_FLASH)] =
        std::make_shared<DataStorage>(this, eMemoryDevice::FPGA_FLASH);
    desc.memoryDevices[MEMORY_DEVICES_TEXT.at(eMemoryDevice::EEPROM)] =
        std::make_shared<DataStorage>(this, eMemoryDevice::EEPROM, eepromMap);

    desc.customParameters.push_back(cp_vctcxo_dac);
    desc.customParameters.push_back(cp_temperature);

    mLMS7002Mcomms[0] = std::make_shared<SlaveSelectShim>(spiLMS7002M, SPI_LMS7002M_1);

    mFPGA = new lime::FPGA_X3(spiFPGA, mLMS7002Mcomms[0]);
    FPGA::GatewareInfo gw = mFPGA->GetGatewareInfo();
    FPGA::GatewareToDescriptor(gw, desc);

    mEqualizer = new Equalizer(spiFPGA, SPI_FPGA);

    mClockGeneratorCDCM = new CDCM_Dev(spiFPGA, CDCM2_BASE_ADDR);
    // TODO: read back cdcm values or mClockGeneratorCDCM->Reset(30.72e6, 25e6);

    RFSOCDescriptor soc;

    // LMS#1
    soc.name = "LMS 1";
    soc.channelCount = 2;
    soc.pathNames[TRXDir::Rx] = { "None", "LNAH", "LNAL" };
    soc.pathNames[TRXDir::Tx] = { "None", "Band1", "Band2" };

    soc.samplingRateRange = { 100e3, 61.44e6, 0 };
    soc.frequencyRange = { 100e3, 3.8e9, 0 };

    soc.lowPassFilterRange[TRXDir::Rx] = { 1.4001e6, 130e6 };
    soc.lowPassFilterRange[TRXDir::Tx] = { 5e6, 130e6 };

    soc.antennaRange[TRXDir::Rx]["LNAH"] = { 2e9, 2.6e9 };
    soc.antennaRange[TRXDir::Rx]["LNAL"] = { 700e6, 900e6 };
    soc.antennaRange[TRXDir::Tx]["Band1"] = { 30e6, 1.9e9 };
    soc.antennaRange[TRXDir::Tx]["Band2"] = { 2e9, 2.6e9 };

    SetGainInformationInDescriptor(soc);

    desc.rfSOC.push_back(soc);

    LMS7002M* lms1 = new LMS7002M(mLMS7002Mcomms[0]);
    lms1->ModifyRegistersDefaults(lms1defaultsOverride);
    lms1->SetOnCGENChangeCallback(LMS1_UpdateFPGAInterface, this);
    mLMSChips.push_back(lms1);

    // LMS#2
    soc.name = "LMS 2";
    soc.pathNames[TRXDir::Rx] = { "None", "TDD", "FDD", "Calibration (LMS3)" };
    soc.pathNames[TRXDir::Tx] = { "None", "TDD", "FDD" };
    desc.rfSOC.push_back(soc);
    mLMS7002Mcomms[1] = std::make_shared<SlaveSelectShim>(spiLMS7002M, SPI_LMS7002M_2);
    LMS7002M* lms2 = new LMS7002M(mLMS7002Mcomms[1]);
    lms2->ModifyRegistersDefaults(lms2and3defaultsOverride);
    mLMSChips.push_back(lms2);

    // LMS#3
    soc.name = "LMS 3";
    soc.pathNames[TRXDir::Rx] = { "None", "LNAH", "Calibration (LMS2)" };
    soc.pathNames[TRXDir::Tx] = { "None", "Band1" };
    desc.rfSOC.push_back(soc);
    mLMS7002Mcomms[2] = std::make_shared<SlaveSelectShim>(spiLMS7002M, SPI_LMS7002M_3);
    LMS7002M* lms3 = new LMS7002M(mLMS7002Mcomms[2]);
    lms3->ModifyRegistersDefaults(lms2and3defaultsOverride);
    mLMSChips.push_back(lms3);

    for (size_t i = 0; i < mLMSChips.size(); ++i)
    {
        mLMSChips[i]->SetConnection(mLMS7002Mcomms[i]);
        //iter->SetReferenceClk_SX(false, 30.72e6);
    }

    const int chipCount = mLMSChips.size();
    mStreamers.resize(chipCount, nullptr);

    auto fpgaNode = std::make_shared<DeviceNode>("FPGA", eDeviceNodeClass::FPGA_X3, mFPGA);
    fpgaNode->children.push_back(std::make_shared<DeviceNode>("LMS_1", eDeviceNodeClass::LMS7002M, lms1));
    fpgaNode->children.push_back(std::make_shared<DeviceNode>("LMS_2", eDeviceNodeClass::LMS7002M, lms2));
    fpgaNode->children.push_back(std::make_shared<DeviceNode>("LMS_3", eDeviceNodeClass::LMS7002M, lms3));
    desc.socTree = std::make_shared<DeviceNode>("X3", eDeviceNodeClass::SDRDevice, this);
    desc.socTree->children.push_back(fpgaNode);

    desc.socTree->children.push_back(std::make_shared<DeviceNode>("CDCM6208", eDeviceNodeClass::CDCM6208, mClockGeneratorCDCM));
}

LimeSDR_X3::~LimeSDR_X3()
{
    delete mClockGeneratorCDCM;
    delete mEqualizer;

    for (size_t i = 0; i < mLMSChips.size(); ++i)
    {
        delete mLMSChips[i];
        mLMSChips[i] = nullptr;
    }
}

// Setup default register values specifically for onboard LMS1 chip
int LimeSDR_X3::InitLMS1(bool skipTune)
{
    LMS1_PA_Enable(0, false);
    LMS1_PA_Enable(1, false);

    double dacVal = 65535;
    const std::vector<CustomParameterIO> params{ { cp_lms1_tx1dac.id, dacVal, "" }, { cp_lms1_tx2dac.id, dacVal, "" } };
    CustomParameterWrite(params);

    LMS7002M* lms = mLMSChips[0];
    if (lms->ResetChip() != 0)
        return -1;

    // if(lms->CalibrateTxGain(0,nullptr) != 0)
    //     return -1;

    // EnableChannel(true, 2*i, false);
    // lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2);

    // if(lms->CalibrateTxGain(0,nullptr) != 0)
    //     return -1;

    // EnableChannel(false, 2*i+1, false);
    // EnableChannel(true, 2*i+1, false);

    if (skipTune)
        return 0;

    if (lms->SetFrequencySX(TRXDir::Tx, lms->GetFrequencySX(TRXDir::Tx)) != 0)
        return -1;
    if (lms->SetFrequencySX(TRXDir::Rx, lms->GetFrequencySX(TRXDir::Rx)) != 0)
        return -1;

    // if (SetRate(10e6,2)!=0)
    //     return -1;
    return 0;
}

static void EnableChannelLMS2(LMS7002M* chip, TRXDir dir, const uint8_t channel, const bool enable)
{
    //ChannelScope scope(this, channel);

    auto macBck = chip->GetActiveChannel();
    const LMS7002M::Channel ch = channel > 0 ? LMS7002M::Channel::ChB : LMS7002M::Channel::ChA;
    chip->SetActiveChannel(ch);

    const bool isTx = dir == TRXDir::Tx;
    //--- LML ---
    if (ch == LMS7002M::Channel::ChA)
    {
        if (isTx)
            chip->Modify_SPI_Reg_bits(LMS7param(TXEN_A), enable ? 1 : 0);
        else
            chip->Modify_SPI_Reg_bits(LMS7param(RXEN_A), enable ? 1 : 0);
    }
    else
    {
        if (isTx)
            chip->Modify_SPI_Reg_bits(LMS7param(TXEN_B), enable ? 1 : 0);
        else
            chip->Modify_SPI_Reg_bits(LMS7param(RXEN_B), enable ? 1 : 0);
    }

    //--- ADC/DAC ---
    chip->Modify_SPI_Reg_bits(LMS7param(EN_DIR_AFE), 1);
    chip->Modify_SPI_Reg_bits(isTx ? LMS7_PD_TX_AFE1 : LMS7_PD_RX_AFE1, 1);
    chip->Modify_SPI_Reg_bits(isTx ? LMS7_PD_TX_AFE2 : LMS7_PD_RX_AFE2, 1);

    //int disabledChannels = (chip->Get_SPI_Reg_bits(LMS7_PD_AFE.address,4,1)&0xF);//check if all channels are disabled
    //chip->Modify_SPI_Reg_bits(LMS7param(EN_G_AFE),disabledChannels==0xF ? 0 : 1);
    //chip->Modify_SPI_Reg_bits(LMS7param(PD_AFE), disabledChannels==0xF ? 1 : 0);

    //--- digital --- not used for LMS2
    if (isTx)
    {
        chip->Modify_SPI_Reg_bits(LMS7param(EN_TXTSP), 0);
    }
    else
    {
        chip->Modify_SPI_Reg_bits(LMS7param(EN_RXTSP), 0);
        // chip->Modify_SPI_Reg_bits(LMS7param(AGC_MODE_RXTSP), 2); //bypass
        // chip->Modify_SPI_Reg_bits(LMS7param(AGC_BYP_RXTSP), 1);
        //chip->SPI_write(0x040C, 0x01FF) // bypass all RxTSP
    }

    //--- baseband ---
    if (isTx)
    {
        chip->Modify_SPI_Reg_bits(LMS7param(EN_DIR_TBB), 1);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_G_TBB), enable ? 1 : 0);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_LPFIAMP_TBB), enable ? 0 : 1);
        chip->Modify_SPI_Reg_bits(LMS7param(TSTIN_TBB), 3); // switch to external DAC
    }
    else
    {
        chip->Modify_SPI_Reg_bits(LMS7param(EN_DIR_RBB), 1);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_G_RBB), enable ? 1 : 0);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_PGA_RBB), enable ? 0 : 1);
        chip->Modify_SPI_Reg_bits(LMS7param(OSW_PGA_RBB), 1); // switch external ADC
    }

    //--- frontend ---
    if (isTx)
    {
        chip->Modify_SPI_Reg_bits(LMS7param(EN_DIR_TRF), 1);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), enable ? 1 : 0);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_TLOBUF_TRF), enable ? 0 : 1);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_TXPAD_TRF), enable ? 0 : 1);
    }
    else
    {
        chip->Modify_SPI_Reg_bits(LMS7param(EN_DIR_RFE), 1);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_G_RFE), enable ? 1 : 0);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_MXLOBUF_RFE), enable ? 0 : 1);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_QGEN_RFE), enable ? 0 : 1);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_TIA_RFE), enable ? 0 : 1);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_LNA_RFE), enable ? 0 : 1);
    }

    //--- synthesizers ---
    if (isTx)
    {
        chip->SetActiveChannel(LMS7002M::Channel::ChSXT);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_DIR_SXRSXT), 1);
        //chip->Modify_SPI_Reg_bits(LMS7param(EN_G), (disabledChannels&3) == 3?0:1);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_G), 1);
        if (ch == LMS7002M::Channel::ChB) //enable LO to channel B
        {
            chip->SetActiveChannel(LMS7002M::Channel::ChA);
            chip->Modify_SPI_Reg_bits(LMS7param(EN_NEXTTX_TRF), enable ? 1 : 0);
        }
    }
    else
    {
        chip->SetActiveChannel(LMS7002M::Channel::ChSXR);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_DIR_SXRSXT), 1);
        //chip->Modify_SPI_Reg_bits(LMS7param(EN_G), (disabledChannels&0xC)==0xC?0:1);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_G), 1);
        if (ch == LMS7002M::Channel::ChB) //enable LO to channel B
        {
            chip->SetActiveChannel(LMS7002M::Channel::ChA);
            chip->Modify_SPI_Reg_bits(LMS7param(EN_NEXTRX_RFE), enable ? 1 : 0);
        }
    }
    chip->SetActiveChannel(macBck);
}

// Setup default register values specifically for onboard LMS2 chip
int LimeSDR_X3::InitLMS2(bool skipTune)
{
    LMS2_PA_LNA_Enable(0, false, false);
    LMS2_PA_LNA_Enable(1, false, false);

    struct regVal {
        uint16_t adr;
        uint16_t val;
    };

    LMS7002M* lms = mLMSChips[1];
    if (lms->ResetChip() != 0)
        return -1;

    // if(lms->CalibrateTxGain(0,nullptr) != 0)
    //     return -1;

    // EnableChannel(false, 2*i+1, false);
    // EnableChannel(true, 2*i+1, false);

    if (skipTune)
        return 0;

    if (lms->SetFrequencySX(TRXDir::Tx, lms->GetFrequencySX(TRXDir::Tx)) != 0)
        return -1;
    if (lms->SetFrequencySX(TRXDir::Rx, lms->GetFrequencySX(TRXDir::Rx)) != 0)
        return -1;

    // if (SetRate(10e6,2)!=0)
    //     return -1;
    return 0;
}

// TODO: Setup default register values specifically for onboard LMS3 chip
int LimeSDR_X3::InitLMS3(bool skipTune)
{
    LMS7002M* lms = mLMSChips[2];
    if (lms->ResetChip() != 0)
        return -1;

    if (skipTune)
        return 0;

    if (lms->SetFrequencySX(TRXDir::Tx, lms->GetFrequencySX(TRXDir::Tx)) != 0)
        return -1;
    if (lms->SetFrequencySX(TRXDir::Rx, lms->GetFrequencySX(TRXDir::Rx)) != 0)
        return -1;
    return 0;
}

void LimeSDR_X3::PreConfigure(const SDRConfig& cfg, uint8_t socIndex)
{
    if (socIndex == 0)
    {
        // Turn off PAs before configuring chip to avoid unexpectedly strong signal input
        LMS1_PA_Enable(0, false);
        LMS1_PA_Enable(1, false);
    }
    else if (socIndex == 1)
    {
        LMS2_PA_LNA_Enable(0, false, false);
        LMS2_PA_LNA_Enable(1, false, false);
    }
}

void LimeSDR_X3::PostConfigure(const SDRConfig& cfg, uint8_t socIndex)
{
    // Turn on needed PAs
    for (int c = 0; c < 2; ++c)
    {
        const ChannelConfig& ch = cfg.channel[c];
        switch (socIndex)
        {
        case 0:
            LMS1_UpdateFPGAInterface(this);
            LMS1_PA_Enable(c, ch.tx.enabled);
            break;
        case 1:
            LMS2_PA_LNA_Enable(c, ch.tx.enabled, ch.rx.enabled);
            break;
        }
    }
}

void LimeSDR_X3::Configure(const SDRConfig& cfg, uint8_t socIndex)
{
    std::vector<std::string> errors;
    bool isValidConfig = LMS7002M_Validate(cfg, errors);

    if (!isValidConfig)
    {
        std::stringstream ss;
        for (const auto& err : errors)
            ss << err << std::endl;
        throw std::logic_error(ss.str());
    }

    bool rxUsed = false;
    bool txUsed = false;
    for (int i = 0; i < 2; ++i)
    {
        const ChannelConfig& ch = cfg.channel[i];
        rxUsed |= ch.rx.enabled;
        txUsed |= ch.tx.enabled;
    }

    try
    {
        mConfigInProgress = true;
        PreConfigure(cfg, socIndex);

        // config validation complete, now do the actual configuration
        LMS7002M* chip = mLMSChips.at(socIndex);
        if (!cfg.skipDefaults)
        {
            const bool skipTune = true;
            switch (socIndex)
            {
            case 0:
                InitLMS1(skipTune);
                break;
            case 1:
                InitLMS2(skipTune);
                break;
            case 2:
                InitLMS3(skipTune);
                break;
            }
        }

        if (cfg.referenceClockFreq != 0)
            chip->SetClockFreq(LMS7002M::ClockID::CLK_REFERENCE, cfg.referenceClockFreq, 0);

        const bool tddMode = cfg.channel[0].rx.centerFrequency == cfg.channel[0].tx.centerFrequency;
        if (rxUsed && cfg.channel[0].rx.centerFrequency > 0)
            chip->SetFrequencySX(TRXDir::Rx, cfg.channel[0].rx.centerFrequency);
        if (txUsed && cfg.channel[0].tx.centerFrequency > 0)
            chip->SetFrequencySX(TRXDir::Tx, cfg.channel[0].tx.centerFrequency);
        if (tddMode)
            chip->EnableSXTDD(true);

        if (socIndex == 0)
            chip->Modify_SPI_Reg_bits(LMS7_PD_TX_AFE1, 0); // enabled DAC is required for FPGA to work

        chip->SetActiveChannel(LMS7002M::Channel::ChA);

        double sampleRate = cfg.channel[0].GetDirection(rxUsed ? TRXDir::Rx : TRXDir::Tx).sampleRate;

        if (socIndex == 0 && sampleRate > 0)
        {
            LMS1_SetSampleRate(sampleRate, cfg.channel[0].rx.oversample, cfg.channel[0].tx.oversample);
        }
        else if (socIndex == 1 && sampleRate > 0)
        {
            Equalizer::Config eqCfg;
            for (int i = 0; i < 2; ++i)
            {
                eqCfg.bypassRxEqualizer[i] = true;
                eqCfg.bypassTxEqualizer[i] = true;
                eqCfg.cfr[i].bypass = true;
                eqCfg.cfr[i].sleep = true;
                eqCfg.cfr[i].bypassGain = true;
                eqCfg.cfr[i].interpolation = cfg.channel[0].tx.oversample;
                eqCfg.fir[i].sleep = true;
                eqCfg.fir[i].bypass = true;
            }
            mEqualizer->Configure(eqCfg);
            LMS2_SetSampleRate(sampleRate, cfg.channel[0].tx.oversample);
        }
        else if (socIndex == 2 && sampleRate > 0)
        {
            LMS3_SetSampleRate_ExternalDAC(cfg.channel[0].rx.sampleRate, cfg.channel[1].rx.sampleRate);
        }

        for (int ch = 0; ch < 2; ++ch)
        {
            chip->SetActiveChannel((ch & 1) ? LMS7002M::Channel::ChB : LMS7002M::Channel::ChA);

            if (cfg.channel[ch].rx.testSignal.enabled)
            {
                chip->Modify_SPI_Reg_bits(LMS7_TSGFC_RXTSP, static_cast<uint8_t>(cfg.channel[ch].rx.testSignal.scale));
                chip->Modify_SPI_Reg_bits(LMS7_TSGMODE_RXTSP, cfg.channel[ch].rx.testSignal.dcMode ? 1 : 0);
                chip->SPI_write(0x040C, 0x01FF); // DC.. bypasss
                // chip->LoadDC_REG_IQ(false, 0x1230, 0x4560); // gets reset by starting stream
            }
            chip->Modify_SPI_Reg_bits(LMS7_INSEL_TXTSP, cfg.channel[ch].tx.testSignal.enabled ? 1 : 0);

            ConfigureDirection(TRXDir::Rx, chip, cfg, ch, socIndex);
            ConfigureDirection(TRXDir::Tx, chip, cfg, ch, socIndex);
        }

        if (socIndex == 0)
        {
            // enabled ADC/DAC is required for FPGA to work
            chip->Modify_SPI_Reg_bits(LMS7_PD_RX_AFE1, 0);
            chip->Modify_SPI_Reg_bits(LMS7_PD_TX_AFE1, 0);
        }
        chip->SetActiveChannel(LMS7002M::Channel::ChA);

        // Workaround: Toggle LimeLights transmit port to flush residual value from data interface
        uint16_t txMux = chip->Get_SPI_Reg_bits(LMS7param(TX_MUX));
        chip->Modify_SPI_Reg_bits(LMS7param(TX_MUX), 2);
        chip->Modify_SPI_Reg_bits(LMS7param(TX_MUX), txMux);

        mConfigInProgress = false;
        PostConfigure(cfg, socIndex);
    } //try
    catch (std::logic_error& e)
    {
        printf("LimeSDR_X3 config: %s\n", e.what());
        throw;
    } catch (std::runtime_error& e)
    {
        throw;
    }
}

void LimeSDR_X3::ConfigureDirection(TRXDir dir, LMS7002M* chip, const SDRConfig& cfg, int ch, uint8_t socIndex)
{
    std::string dirName = dir == TRXDir::Rx ? "Rx" : "Tx";
    SDRDevice::ChannelConfig::Direction trx = cfg.channel[ch].GetDirection(dir);

    if (socIndex == 1) // LMS2 uses external ADC/DAC
    {
        EnableChannelLMS2(chip, dir, ch, trx.enabled);
    }
    else
    {
        chip->EnableChannel(dir, ch, trx.enabled);
    }

    SetLMSPath(dir, trx, ch, socIndex);

    if (socIndex == 0)
    {
        if (trx.enabled && chip->SetGFIRFilter(dir, ch, trx.gfir.enabled, trx.gfir.bandwidth) != 0)
        {
            throw std::logic_error(strFormat("%s ch%i GFIR config failed", dirName, ch));
        }
    }

    if (trx.calibrate && trx.enabled)
    {
        SetupCalibrations(chip, trx.sampleRate);

        int status;
        if (dir == TRXDir::Rx)
        {
            status = CalibrateRx(false, false);
        }
        else
        {
            status = CalibrateTx(false);
        }

        if (status != MCU_BD::MCU_NO_ERROR)
        {
            throw std::runtime_error(
                strFormat("%s ch%i DC/IQ calibration failed: %s", dirName, ch, MCU_BD::MCUStatusMessage(status)));
        }
    }

    if (trx.lpf > 0 && trx.enabled)
    {
        SetupCalibrations(chip, trx.sampleRate);
        int status;

        if (dir == TRXDir::Rx)
        {
            status = TuneRxFilter(trx.lpf);
        }
        else
        {
            status = TuneTxFilter(trx.lpf);
        }

        if (status != MCU_BD::MCU_NO_ERROR)
        {
            throw std::runtime_error(
                strFormat("%s ch%i filter calibration failed: %s", dirName, ch, MCU_BD::MCUStatusMessage(status)));
        }
    }
}

void LimeSDR_X3::SetLMSPath(const TRXDir dir, const SDRDevice::ChannelConfig::Direction& trx, const int ch, const uint8_t socIndex)
{
    switch (socIndex)
    {
    case 0:
        LMS1SetPath(dir, ch, trx.path);
        break;
    case 1:
        uint8_t path;

        if (trx.enabled)
        {
            path = trx.path;
        }
        else
        {
            path = (dir == TRXDir::Rx) ? uint8_t(ePathLMS2_Rx::NONE) : uint8_t(ePathLMS2_Tx::NONE);
        }

        LMS2SetPath(dir, ch, path);
        break;
    case 2:
        LMS3SetPath(dir, ch, trx.path);
        break;
    default:
        break;
    }
}

int LimeSDR_X3::Init()
{
    struct regVal {
        uint16_t adr;
        uint16_t val;
    };

    const std::vector<regVal> mFPGAInitVals = {
        { 0x00D1, 0x3357 }, // RF Switches
        //{0x00D2, 0x003C} // PA controls
    };

    for (auto i : mFPGAInitVals)
        mFPGA->WriteRegister(i.adr, i.val);

    mClockGeneratorCDCM->Reset(30.72e6, 25e6);
    const bool skipTune = true;
    InitLMS1(skipTune);
    InitLMS2(skipTune);
    InitLMS3(skipTune);
    return 0;
}

void LimeSDR_X3::Reset()
{
    for (uint32_t i = 0; i < mLMSChips.size(); ++i)
        mLMS7002Mcomms[i]->ResetDevice();
}

double LimeSDR_X3::GetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel)
{
    switch (moduleIndex)
    {
    case 1:
        if (trx == TRXDir::Rx)
        {
            return mClockGeneratorCDCM->GetFrequency(CDCM_Y4); // Rx Ch. A
        }
        else
        {
            const int oversample = mEqualizer->GetOversample();
            return mClockGeneratorCDCM->GetFrequency(CDCM_Y0Y1) / oversample; // Tx Ch. A&B
        }
    case 2:
        if (trx == TRXDir::Rx) // LMS3 Rx uses external ADC
        {
            return mClockGeneratorCDCM->GetFrequency(CDCM_Y6); // Rx Ch. A
        }
        else // LMS3 Tx uses internal ADC
        {
            return LMS7002M_SDRDevice::GetSampleRate(moduleIndex, TRXDir::Tx, channel);
        }
    default:
        return LMS7002M_SDRDevice::GetSampleRate(moduleIndex, trx, channel);
    }
}

void LimeSDR_X3::SetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double sampleRate, uint8_t oversample)
{
    if (moduleIndex == 0 && sampleRate > 0)
    {
        LMS1_SetSampleRate(sampleRate, oversample, oversample);
    }
    else if (moduleIndex == 1 && sampleRate > 0)
    {
        Equalizer::Config eqCfg;
        for (int i = 0; i < 2; ++i)
        {
            eqCfg.bypassRxEqualizer[i] = true;
            eqCfg.bypassTxEqualizer[i] = true;
            eqCfg.cfr[i].bypass = true;
            eqCfg.cfr[i].sleep = true;
            eqCfg.cfr[i].bypassGain = true;
            eqCfg.cfr[i].interpolation = oversample;
            eqCfg.fir[i].sleep = true;
            eqCfg.fir[i].bypass = true;
        }
        mEqualizer->Configure(eqCfg);
        LMS2_SetSampleRate(sampleRate, oversample);
    }
    else if (moduleIndex == 2 && sampleRate > 0)
    {
        LMS3_SetSampleRate_ExternalDAC(sampleRate, sampleRate);
    }
}

double LimeSDR_X3::GetClockFreq(uint8_t clk_id, uint8_t channel)
{
    ValidateChannel(channel);
    LMS7002M* chip = mLMSChips[channel / 2];
    return chip->GetClockFreq(static_cast<LMS7002M::ClockID>(clk_id), channel & 1);
}

void LimeSDR_X3::SetClockFreq(uint8_t clk_id, double freq, uint8_t channel)
{
    ValidateChannel(channel);
    LMS7002M* chip = mLMSChips[channel / 2];
    chip->SetClockFreq(static_cast<LMS7002M::ClockID>(clk_id), freq, channel & 1);
}

int LimeSDR_X3::SPI(uint32_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    switch (chipSelect)
    {
    case SPI_LMS7002M_1:
        return mLMS7002Mcomms[0]->SPI(MOSI, MISO, count);
    case SPI_LMS7002M_2:
        return mLMS7002Mcomms[1]->SPI(MOSI, MISO, count);
    case SPI_LMS7002M_3:
        return mLMS7002Mcomms[2]->SPI(MOSI, MISO, count);
    case SPI_FPGA:
        return mfpgaPort->SPI(MOSI, MISO, count);
    default:
        throw std::logic_error("invalid SPI chip select");
    }
}

int LimeSDR_X3::StreamSetup(const StreamConfig& config, uint8_t moduleIndex)
{
    // Allow multiple setup calls
    if (mStreamers.at(moduleIndex) != nullptr)
    {
        delete mStreamers.at(moduleIndex);
    }

    try
    {
        mStreamers.at(moduleIndex) = new TRXLooper_PCIE(
            mTRXStreamPorts.at(moduleIndex), mTRXStreamPorts.at(moduleIndex), mFPGA, mLMSChips.at(moduleIndex), moduleIndex);
        if (mCallback_logMessage)
            mStreamers[moduleIndex]->SetMessageLogCallback(mCallback_logMessage);
        std::shared_ptr<LitePCIe> trxPort{ mTRXStreamPorts.at(moduleIndex) };
        if (!trxPort->IsOpen())
        {
            int dirFlag = 0;
            if (config.channels.at(lime::TRXDir::Rx).size() > 0 && config.channels.at(lime::TRXDir::Tx).size() > 0)
                dirFlag = O_RDWR;
            else if (config.channels.at(lime::TRXDir::Rx).size() > 0)
                dirFlag = O_RDONLY;
            else if (config.channels.at(lime::TRXDir::Tx).size() > 0)
                dirFlag = O_WRONLY;
            if (trxPort->Open(trxPort->GetPathName(), dirFlag | O_NOCTTY | O_CLOEXEC | O_NONBLOCK) != 0)
            {
                const std::string reason = "Failed to open device in stream start: " + trxPort->GetPathName();
                throw std::runtime_error(reason);
            }
        }
        mStreamers[moduleIndex]->Setup(config);
        mStreamConfig = config;
        return 0;
    } catch (std::logic_error& e)
    {
        printf("LimeSDR_X3::StreamSetup logic_error %s\n", e.what());
        throw;
    } catch (std::runtime_error& e)
    {
        printf("LimeSDR_X3::StreamSetup runtime_error %s\n", e.what());
        throw;
    }
}

void LimeSDR_X3::StreamStop(uint8_t moduleIndex)
{
    LMS7002M_SDRDevice::StreamStop(moduleIndex);
    std::shared_ptr<LitePCIe> trxPort{ mTRXStreamPorts.at(moduleIndex) };
    if (trxPort && trxPort->IsOpen())
        trxPort->Close();
}

void LimeSDR_X3::LMS1_SetSampleRate(double f_Hz, uint8_t rxDecimation, uint8_t txInterpolation)
{
    if (rxDecimation == 0)
        rxDecimation = 2;
    if (txInterpolation == 0)
        txInterpolation = 2;
    assert(rxDecimation > 0);
    assert(txInterpolation > 0);
    if (txInterpolation / rxDecimation > 4)
        throw std::logic_error(
            strFormat("TxInterpolation(%i)/RxDecimation(%i) should not be more than 4", txInterpolation, rxDecimation));
    uint8_t oversample = rxDecimation;
    const bool bypass = (oversample == 1) || (oversample == 0 && f_Hz > 62e6);
    uint8_t hbd_ovr = 7; // decimation ratio is 2^(1+hbd_ovr), HBD_OVR_RXTSP=7 - bypass
    uint8_t hbi_ovr = 7; // interpolation ratio is 2^(1+hbi_ovr), HBI_OVR_TXTSP=7 - bypass
    double cgenFreq = f_Hz * 4; // AI AQ BI BQ
    // TODO:
    // for (uint8_t i = 0; i < GetNumChannels(false) ;i++)
    // {
    //     if (rx_channels[i].cF_offset_nco != 0.0 || tx_channels[i].cF_offset_nco != 0.0)
    //     {
    //         bypass = false;
    //         break;
    //     }
    // }
    if (!bypass)
    {
        if (oversample == 0)
        {
            const int n = lime::LMS7002M::CGEN_MAX_FREQ / (cgenFreq);
            oversample = (n >= 32) ? 32 : (n >= 16) ? 16 : (n >= 8) ? 8 : (n >= 4) ? 4 : 2;
        }

        hbd_ovr = 4;
        if (oversample <= 16)
        {
            const int decTbl[] = { 0, 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3 };
            hbd_ovr = decTbl[oversample];
        }
        cgenFreq *= 2 << hbd_ovr;
        if (txInterpolation >= rxDecimation)
            hbi_ovr = hbd_ovr + std::log2(txInterpolation / rxDecimation);
        else
            throw std::logic_error(
                strFormat("Rx decimation(2^%i) > Tx interpolation(2^%i) currently not supported", hbd_ovr, hbi_ovr));
    }
    lime::info("Sampling rate set(%.3f MHz): CGEN:%.3f MHz, Decim: 2^%i, Interp: 2^%i",
        f_Hz / 1e6,
        cgenFreq / 1e6,
        1 + hbd_ovr,
        1 + hbi_ovr);
    LMS7002M* mLMSChip = mLMSChips[0];
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2 - std::log2(txInterpolation / rxDecimation));
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), hbd_ovr);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), hbi_ovr);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), hbd_ovr);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), hbi_ovr);
    mLMSChip->SetInterfaceFrequency(cgenFreq, hbi_ovr, hbd_ovr);
}

enum // TODO: replace
{
    LMS_PATH_NONE = 0, ///<No active path (RX or TX)
    LMS_PATH_LNAH = 1, ///<RX LNA_H port
    LMS_PATH_LNAL = 2, ///<RX LNA_L port
    LMS_PATH_LNAW = 3, ///<RX LNA_W port
    LMS_PATH_TX1 = 1, ///<TX port 1
    LMS_PATH_TX2 = 2, ///<TX port 2
    LMS_PATH_AUTO = 255, ///<Automatically select port (if supported)
};

void LimeSDR_X3::LMS1_PA_Enable(uint8_t chan, bool enabled)
{
    uint16_t pa_addr = 0x00D2;
    uint16_t pa_val = mFPGA->ReadRegister(pa_addr);

    const int bitMask = 1 << (5 - chan);
    if (enabled)
        pa_val |= bitMask; // PA on
    else
        pa_val &= ~bitMask; // chan 0 = 5; chan 1 = 4
    mFPGA->WriteRegister(pa_addr, pa_val);
}

void LimeSDR_X3::LMS1SetPath(TRXDir dir, uint8_t chan, uint8_t pathId)
{
    const bool tx = dir == TRXDir::Tx;
    uint16_t sw_addr = 0x00D1;
    uint16_t sw_val = mFPGA->ReadRegister(sw_addr);
    lime::LMS7002M* lms = mLMSChips.at(0);

    if (tx)
    {
        uint8_t path;
        switch (ePathLMS1_Tx(pathId))
        {
        case ePathLMS1_Tx::NONE:
            path = LMS_PATH_NONE;
            break;
        case ePathLMS1_Tx::BAND1:
            path = LMS_PATH_TX1;
            break;
        case ePathLMS1_Tx::BAND2:
            path = LMS_PATH_TX2;
            break;
        default:
            throw std::logic_error("Invalid LMS1 Tx path");
        }

        if (path == LMS_PATH_TX1)
            sw_val |= 1 << (13 - chan); // chan 0 = 13; chan 1 = 12
        else if (path == LMS_PATH_TX2)
            sw_val &= ~(1 << (13 - chan));

        mFPGA->WriteRegister(sw_addr, sw_val);
        lms->SetBandTRF(path);
    }
    else
    {
        uint8_t path;
        switch (ePathLMS1_Rx(pathId))
        {
        case ePathLMS1_Rx::NONE:
            path = static_cast<uint8_t>(LMS7002M::PathRFE::NONE);
            break;
        case ePathLMS1_Rx::LNAH:
            path = static_cast<uint8_t>(LMS7002M::PathRFE::LNAH);
            break;
        case ePathLMS1_Rx::LNAL:
            path = static_cast<uint8_t>(LMS7002M::PathRFE::LNAL);
            break;
        //case ePathLMS1_Rx::LNAW : path = LMS7002M::LNAW; break;
        default:
            throw std::logic_error("Invalid LMS1 Rx path");
        }

        if (path == LMS_PATH_LNAW)
            lime::warning("LNAW has no connection to RF ports");
        else if (path == LMS_PATH_LNAH)
            sw_val |= 1 << (11 - chan);
        else if (path == LMS_PATH_LNAL)
            sw_val &= ~(1UL << (11 - chan));

        mFPGA->WriteRegister(sw_addr, sw_val);
        lms->SetPathRFE(lime::LMS7002M::PathRFE(path));
    }
}

void LimeSDR_X3::LMS2_PA_LNA_Enable(uint8_t chan, bool PAenabled, bool LNAenabled)
{
    uint16_t pa_addr = 0x00D2;
    struct RegPA {
        RegPA(uint16_t value)
        {
            lms1PA[0] = value & (1 << 5);
            lms1PA[1] = value & (1 << 4);
            lms2PA[0] = value & (1 << 3);
            lms2PA[1] = value & (1 << 2);
            lms2LNA[0] = !(value & (1 << 1)); // 1=LNA is powered down
            lms2LNA[1] = !(value & (1 << 0));
        }
        uint16_t Value()
        {
            uint16_t value = 0;
            value |= lms1PA[0] << 5;
            value |= lms1PA[1] << 4;
            value |= lms2PA[0] << 3;
            value |= lms2PA[1] << 2;
            value |= (!lms2LNA[0]) << 1;
            value |= (!lms2LNA[1]) << 0;
            return value;
        }
        bool lms1PA[2];
        bool lms2PA[2];
        bool lms2LNA[2];
    };

    RegPA pa(mFPGA->ReadRegister(pa_addr));

    pa.lms2PA[chan] = PAenabled;
    pa.lms2LNA[chan] = LNAenabled;

    mFPGA->WriteRegister(pa_addr, pa.Value());
}

void LimeSDR_X3::LMS2SetPath(TRXDir dir, uint8_t chan, uint8_t path)
{
    const bool tx = dir == TRXDir::Tx;
    uint16_t sw_addr = 0x00D1;
    /*struct RegSW
    {
        RegSW(uint16_t value)
        {
            lms1txBand[0] = value & (1<<13);
            lms1txBand[1] = value & (1<<12);
            lms1rxPath[1] = value & (1<<11);
            lms1rxPath[1] = value & (1<<10);
        }

        bool lms1txBand[2];
        bool lms1rxPath[2];
        bool lms2tx
    }*/

    uint16_t sw_val = mFPGA->ReadRegister(sw_addr);

    uint16_t shift = chan == 0 ? 0 : 2;
    if (path == 0)
    {
    }
    else if (tx && ePathLMS2_Tx(path) == ePathLMS2_Tx::TDD) // TDD_TX
    {
        if (chan == 0)
            sw_val &= ~(1 << 7); // TRX1T to RSFW_TRX1
        else
            sw_val |= 1 << 9; // TRX2T to RSFW_TRX2
        sw_val |= 1 << (6 + shift); // TRX1 or TRX2 to J8 or J10
        sw_val &= ~(1 << (2 + shift)); // RX1C or RX2C to RX1IN or RX2IN (LNA)
        sw_val |= 1 << (3 + shift); // RX1IN or RX2IN to RFSW_TRX1 or RFSW_TRX2
    }
    else if (!tx && ePathLMS2_Rx(path) == ePathLMS2_Rx::TDD) // TDD_RX
    {
        if (chan == 0)
            sw_val |= 1 << 7; // TRX1T to ground
        else
            sw_val &= ~(1 << 9); // TRX2T to ground
        sw_val &= ~(1 << (6 + shift)); // TRX1 or TRX2 to J8 or J10
        sw_val &= ~(1 << (2 + shift)); // RX1C or RX2C to RX1IN or RX2IN (LNA)
        sw_val |= 1 << (3 + shift); // RX1IN or RX2IN to RFSW_TRX1 or RFSW_TRX1
    }
    else if (ePathLMS2_Rx(path) == ePathLMS2_Rx::FDD || ePathLMS2_Tx(path) == ePathLMS2_Tx::FDD) // FDD
    {
        if (chan == 0)
            sw_val &= ~(1 << 7); // TRX1T to RSFW_TRX1
        else
            sw_val |= 1 << 9; // TRX2T to RSFW_TRX2
        sw_val |= 1 << (6 + shift); // TRX1 or TRX2 to J8 or J10
        sw_val &= ~(1 << (2 + shift)); // RX1C or RX2C to RX1IN or RX2IN (LNA)
        sw_val &= ~(1 << (3 + shift)); // RX1IN or RX2In to J9 or  J11
    }
    else if (!tx && ePathLMS2_Rx(path) == ePathLMS2_Rx::CALIBRATION) // Cal
    {
        if (chan == 0)
            sw_val |= 1 << 7; // TRX1T to ground
        else
            sw_val &= ~(1 << 9); // TRX2T to ground
        sw_val |= 1 << (6 + shift); // TRX1 or TRX2 to J8 or J10
        sw_val |= 1 << (2 + shift); // RX1C or RX2C to LMS3 TX1_1 or TX2_1
        sw_val |= 1 << (3 + shift); // RX1IN or RX2IN to RFSW_TRX1 or RFSW_TRX1
    }

    mFPGA->WriteRegister(sw_addr, sw_val);
    lime::LMS7002M* lms = mLMSChips.at(1);
    lms->SetBandTRF(1); // LMS2 uses only BAND1
    lms->SetPathRFE(LMS7002M::PathRFE::LNAH); // LMS2 only uses LNAH
}

void LimeSDR_X3::LMS3SetPath(TRXDir dir, uint8_t chan, uint8_t path)
{
    uint16_t sw_addr = 0x00D1;
    uint16_t sw_val = mFPGA->ReadRegister(sw_addr);
    lime::LMS7002M* lms = mLMSChips.at(0);

    if (dir == TRXDir::Tx)
        lms->SetBandTRF(path);
    else
    {
        if (path == LMS_PATH_NONE || path > 2)
        {
            lms->SetPathRFE(lime::LMS7002M::PathRFE(LMS_PATH_NONE));
            return;
        }
        else if (path == LMS_PATH_LNAH)
            sw_val &= ~(1 << (chan - 4));
        else if (path == 2) // Calibration path
            sw_val |= 1 << (chan - 4);

        mFPGA->WriteRegister(sw_addr, sw_val);
        lms->SetPathRFE(lime::LMS7002M::PathRFE(LMS_PATH_LNAH));
    }
}

void LimeSDR_X3::LMS2_SetSampleRate(double f_Hz, uint8_t oversample)
{
    assert(mClockGeneratorCDCM);
    double txClock = f_Hz;

    // Oversample is available only to Tx for LMS#2
    oversample = std::min(oversample, uint8_t(2));
    if (oversample == 2 || oversample == 0) // 0 is "auto", use max oversample
        txClock *= 2;

    mEqualizer->SetOversample(oversample);

    if (mClockGeneratorCDCM->SetFrequency(CDCM_Y0Y1, txClock, false) != 0) // Tx Ch. A&B
        throw std::runtime_error("Failed to configure CDCM_Y0Y1");
    if (mClockGeneratorCDCM->SetFrequency(CDCM_Y4, f_Hz, false) != 0) // Rx Ch. A
        throw std::runtime_error("Failed to configure CDCM_Y4");
    if (mClockGeneratorCDCM->SetFrequency(CDCM_Y5, f_Hz, true) != 0) // Rx Ch. B
        throw std::runtime_error("Failed to configure CDCM_Y5");

    if (!mClockGeneratorCDCM->IsLocked())
        throw std::runtime_error("CDCM is not locked");
}

void LimeSDR_X3::LMS3_SetSampleRate_ExternalDAC(double chA_Hz, double chB_Hz)
{
    if (mClockGeneratorCDCM->SetFrequency(CDCM_Y6, chA_Hz, false) != 0) // Rx Ch. A
        throw std::runtime_error("Failed to configure CDCM_Y4");
    if (mClockGeneratorCDCM->SetFrequency(CDCM_Y7, chB_Hz, true) != 0) // Rx Ch. B
        throw std::runtime_error("Failed to configure CDCM_Y5");
    if (!mClockGeneratorCDCM->IsLocked())
        throw std::runtime_error("CDCM is not locked");
}

int LimeSDR_X3::CustomParameterWrite(const std::vector<CustomParameterIO>& parameters)
{
    return mfpgaPort->CustomParameterWrite(parameters);
}

int LimeSDR_X3::CustomParameterRead(std::vector<CustomParameterIO>& parameters)
{
    return mfpgaPort->CustomParameterRead(parameters);
}

bool LimeSDR_X3::UploadMemory(
    eMemoryDevice device, uint8_t moduleIndex, const char* data, size_t length, UploadMemoryCallback callback)
{
    int progMode;
    LMS64CProtocol::ProgramWriteTarget target = LMS64CProtocol::ProgramWriteTarget::FPGA;

    switch (device)
    {
    case eMemoryDevice::FPGA_RAM:
        progMode = 0;
        break;
    case eMemoryDevice::FPGA_FLASH:
        progMode = 1;
        break;
    default:
        return false;
    }

    return mfpgaPort->ProgramWrite(data, length, progMode, target, callback);
}

int LimeSDR_X3::MemoryWrite(std::shared_ptr<DataStorage> storage, Region region, const void* data)
{
    if (storage == nullptr || storage->ownerDevice != this || storage->memoryDeviceType != eMemoryDevice::EEPROM)
    {
        return -1;
    }

    return mfpgaPort->MemoryWrite(region.address, data, region.size);
}

int LimeSDR_X3::MemoryRead(std::shared_ptr<DataStorage> storage, Region region, void* data)
{
    if (storage == nullptr || storage->ownerDevice != this || storage->memoryDeviceType != eMemoryDevice::EEPROM)
    {
        return -1;
    }

    return mfpgaPort->MemoryRead(region.address, data, region.size);
}

int LimeSDR_X3::UploadTxWaveform(const StreamConfig& config, uint8_t moduleIndex, const void** samples, uint32_t count)
{
    return TRXLooper_PCIE::UploadTxWaveform(mFPGA, mTRXStreamPorts[moduleIndex], config, moduleIndex, samples, count);
}

} //namespace lime
