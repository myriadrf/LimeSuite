#include "LimeSDR_XTRX.h"

#include <fcntl.h>
#include "math.h"

#include "Logger.h"
#include "LitePCIe.h"
#include "FPGA_common.h"
#include "TRXLooper_PCIE.h"
#include "FPGA_XTRX.h"
#include "LMS64CProtocol.h"
#include "DSP/Equalizer.h"
#include "CommonFunctions.h"

#include "limesuite/DeviceNode.h"
#include "lms7002m/LMS7002M_validation.h"
#include "mcu_program/common_src/lms7002m_calibrations.h"
#include "mcu_program/common_src/lms7002m_filters.h"
#include "MCU_BD.h"

#include "limesuite/LMS7002M.h"

namespace lime {

// XTRX board specific devices ids and data
static const uint8_t SPI_LMS7002M = 0;
static const uint8_t SPI_FPGA = 1;

static SDRDevice::CustomParameter cp_vctcxo_dac = { "VCTCXO DAC (volatile)", 0, 0, 65535, false };

static const std::vector<std::pair<uint16_t, uint16_t>> lms7002defaultsOverrides = {
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
    { 0x040C, 0x00FB },

    // LDOs
    { 0x0092, 0x0D15 },
    { 0x0093, 0x01B1 },
    { 0x00A6, 0x000F },
    // XBUF
    { 0x0085, 0x0019 },
};

static inline void ValidateChannel(uint8_t channel)
{
    if (channel > 2)
        throw std::logic_error("invalid channel index");
}

// Callback for updating FPGA's interface clocks when LMS7002M CGEN is manually modified
OpStatus LimeSDR_XTRX::LMS1_UpdateFPGAInterface(void* userData)
{
    constexpr int chipIndex = 0;
    assert(userData != nullptr);
    LimeSDR_XTRX* pthis = static_cast<LimeSDR_XTRX*>(userData);
    // don't care about cgen changes while doing Config(), to avoid unnecessary fpga updates
    if (pthis->mConfigInProgress)
        return OpStatus::SUCCESS;
    LMS7002M* soc = pthis->mLMSChips[chipIndex];
    return UpdateFPGAInterfaceFrequency(*soc, *pthis->mFPGA, chipIndex);
}

/// @brief Constructs a new LimeSDR_XTRX object
///
/// Do not perform any unnecessary configuring to device in constructor, so you
/// could read back it's state for debugging purposes.
/// @param spiRFsoc The communications port to the LMS7002M chip.
/// @param spiFPGA The communications port to the device's FPGA.
/// @param sampleStream The communications port to send and receive sample data.
/// @param refClk The reference clock of the device.
LimeSDR_XTRX::LimeSDR_XTRX(std::shared_ptr<IComms> spiRFsoc,
    std::shared_ptr<IComms> spiFPGA,
    std::shared_ptr<LitePCIe> sampleStream,
    std::shared_ptr<ISerialPort> control,
    double refClk)
    : LMS7002M_SDRDevice()
    , lms7002mPort(spiRFsoc)
    , fpgaPort(spiFPGA)
    , mStreamPort(sampleStream)
    , mSerialPort(control)
    , mConfigInProgress(false)
{
    SDRDevice::Descriptor& desc = mDeviceDescriptor;
    desc.name = GetDeviceName(LMS_DEV_LIMESDR_XTRX);

    LMS64CProtocol::FirmwareInfo fw;
    LMS64CProtocol::GetFirmwareInfo(*mSerialPort, fw);
    LMS64CProtocol::FirmwareToDescriptor(fw, desc);

    desc.spiSlaveIds = { { "LMS7002M", SPI_LMS7002M }, { "FPGA", SPI_FPGA } };

    // desc.memoryDevices[MEMORY_DEVICES_TEXT.at(eMemoryDevice::FPGA_RAM)] =
    //     std::make_shared<DataStorage>(this, eMemoryDevice::FPGA_RAM);
    const std::unordered_map<eMemoryRegion, Region> flashMap = { { eMemoryRegion::VCTCXO_DAC, { 16, 2 } } };
    desc.memoryDevices[MEMORY_DEVICES_TEXT.at(eMemoryDevice::FPGA_FLASH)] =
        std::make_shared<DataStorage>(this, eMemoryDevice::FPGA_FLASH);

    desc.customParameters.push_back(cp_vctcxo_dac);

    mFPGA = new lime::FPGA_XTRX(spiFPGA, spiRFsoc);
    FPGA::GatewareInfo gw = mFPGA->GetGatewareInfo();
    FPGA::GatewareToDescriptor(gw, desc);

    SDRDevice::Region serialNumberAddr = { 0x01FE0000, sizeof(uint64_t) };
    if (MemoryRead(desc.memoryDevices[MEMORY_DEVICES_TEXT.at(eMemoryDevice::FPGA_FLASH)], serialNumberAddr, &desc.serialNumber) !=
        OpStatus::SUCCESS)
        desc.serialNumber = 0;

    RFSOCDescriptor soc;
    // LMS#1
    soc.name = "LMS7002M";
    soc.channelCount = 2;
    soc.pathNames[TRXDir::Rx] = { "None", "LNAH", "LNAL", "LNAW", "LB1", "LB2" };
    soc.pathNames[TRXDir::Tx] = { "None", "Band1", "Band2" };

    soc.samplingRateRange = { 100e3, 61.44e6, 0 };
    soc.frequencyRange = { 100e3, 3.8e9, 0 };

    soc.lowPassFilterRange[TRXDir::Rx] = { 1.4001e6, 130e6 };
    soc.lowPassFilterRange[TRXDir::Tx] = { 5e6, 130e6 };

    soc.antennaRange[TRXDir::Rx]["LNAH"] = { 2e9, 2.6e9 };
    soc.antennaRange[TRXDir::Rx]["LNAL"] = { 700e6, 900e6 };
    soc.antennaRange[TRXDir::Rx]["LNAW"] = { 700e6, 2.6e9 };
    soc.antennaRange[TRXDir::Rx]["LB1"] = soc.antennaRange[TRXDir::Rx]["LNAL"];
    soc.antennaRange[TRXDir::Rx]["LB2"] = soc.antennaRange[TRXDir::Rx]["LNAW"];
    soc.antennaRange[TRXDir::Tx]["Band1"] = { 30e6, 1.9e9 };
    soc.antennaRange[TRXDir::Tx]["Band2"] = { 2e9, 2.6e9 };

    SetGainInformationInDescriptor(soc);

    desc.rfSOC.push_back(soc);

    LMS7002M* chip = new LMS7002M(spiRFsoc);
    chip->ModifyRegistersDefaults(lms7002defaultsOverrides);
    chip->SetOnCGENChangeCallback(LMS1_UpdateFPGAInterface, this);
    mLMSChips.push_back(chip);
    for (auto iter : mLMSChips)
    {
        iter->SetReferenceClk_SX(TRXDir::Rx, refClk);
        iter->SetClockFreq(LMS7002M::ClockID::CLK_REFERENCE, refClk);
    }

    const int chipCount = mLMSChips.size();
    mStreamers.resize(chipCount, nullptr);

    auto fpgaNode = std::make_shared<DeviceNode>("FPGA", eDeviceNodeClass::FPGA_XTRX, mFPGA);
    fpgaNode->children.push_back(std::make_shared<DeviceNode>("LMS7002M", eDeviceNodeClass::LMS7002M, chip));
    desc.socTree = std::make_shared<DeviceNode>("XTRX", eDeviceNodeClass::SDRDevice, this);
    desc.socTree->children.push_back(fpgaNode);
}

LimeSDR_XTRX::~LimeSDR_XTRX()
{
}

static OpStatus InitLMS1(LMS7002M* lms, bool skipTune = false)
{
    OpStatus status;
    status = lms->ResetChip();
    if (status != OpStatus::SUCCESS)
        return status;
    // lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    // if(lms->CalibrateTxGain(0,nullptr) != 0)
    //     return -1;

    // EnableChannel(true, 2*i, false);
    // lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    // if(lms->CalibrateTxGain(0,nullptr) != 0)
    //     return -1;

    // EnableChannel(false, 2*i+1, false);
    // EnableChannel(true, 2*i+1, false);

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);

    if (skipTune)
        return OpStatus::SUCCESS;

    status = lms->SetFrequencySX(TRXDir::Tx, lms->GetFrequencySX(TRXDir::Tx));
    if (status != OpStatus::SUCCESS)
        return status;

    status = lms->SetFrequencySX(TRXDir::Rx, lms->GetFrequencySX(TRXDir::Rx));
    if (status != OpStatus::SUCCESS)
        return status;

    // if (SetRate(10e6,2)!=0)
    //     return -1;
    return OpStatus::SUCCESS;
}

OpStatus LimeSDR_XTRX::Configure(const SDRConfig& cfg, uint8_t socIndex)
{
    std::vector<std::string> errors;
    bool isValidConfig = LMS7002M_Validate(cfg, errors);

    if (!isValidConfig)
    {
        std::stringstream ss;
        for (const auto& err : errors)
            ss << err << std::endl;
        return ReportError(OpStatus::ERROR, "LimeSDR_XTRX config: %s", ss.str().c_str());
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
        LMS7002M* chip = mLMSChips.at(socIndex);
        if (!cfg.skipDefaults)
        {
            const bool skipTune = true;
            InitLMS1(chip, skipTune);
        }

        LMS7002LOConfigure(chip, cfg);
        for (int i = 0; i < 2; ++i)
        {
            LMS7002ChannelConfigure(chip, cfg.channel[i], i);
            LMS7002TestSignalConfigure(chip, cfg.channel[i], i);
        }

        // enabled ADC/DAC is required for FPGA to work
        chip->Modify_SPI_Reg_bits(LMS7_PD_RX_AFE1, 0);
        chip->Modify_SPI_Reg_bits(LMS7_PD_TX_AFE1, 0);
        chip->SetActiveChannel(LMS7002M::Channel::ChA);

        double sampleRate = cfg.channel[0].GetDirection(rxUsed ? TRXDir::Rx : TRXDir::Tx).sampleRate;

        if (sampleRate > 0)
            LMS1_SetSampleRate(sampleRate, cfg.channel[0].rx.oversample, cfg.channel[0].tx.oversample);

        for (int i = 0; i < 2; ++i)
        {
            const SDRDevice::ChannelConfig& ch = cfg.channel[i];
            LMS1SetPath(false, i, ch.rx.path);
            LMS1SetPath(true, i, ch.tx.path);
            LMS7002ChannelCalibration(chip, ch, i);
        }
        chip->SetActiveChannel(LMS7002M::Channel::ChA);

        // Workaround: Toggle LimeLights transmit port to flush residual value from data interface
        uint16_t txMux = chip->Get_SPI_Reg_bits(LMS7param(TX_MUX));
        chip->Modify_SPI_Reg_bits(LMS7param(TX_MUX), 2);
        chip->Modify_SPI_Reg_bits(LMS7param(TX_MUX), txMux);

        mConfigInProgress = false;
        if (sampleRate > 0)
            LMS1_UpdateFPGAInterface(this);
    } //try
    catch (std::logic_error& e)
    {
        return ReportError(OpStatus::ERROR, "LimeSDR_XTRX config: %s", e.what());
    } catch (std::runtime_error& e)
    {
        return ReportError(OpStatus::ERROR, "LimeSDR_XTRX config: %s", e.what());
    }
    return OpStatus::SUCCESS;
}

OpStatus LimeSDR_XTRX::Init()
{
    struct regVal {
        uint16_t adr;
        uint16_t val;
    };

    const std::vector<regVal> mFPGAInitVals = {
        { 0x00D1, 0x3357 }, // RF Switches
    };

    for (auto i : mFPGAInitVals)
        mFPGA->WriteRegister(i.adr, i.val);

    // uint8_t paramId = 2;
    // double dacVal = 65535;
    // CustomParameterWrite(&paramId,&dacVal,1,"");
    // paramId = 3;
    // CustomParameterWrite(&paramId,&dacVal,1,"");

    const bool skipTune = true;
    return InitLMS1(mLMSChips.at(0), skipTune);
}

OpStatus LimeSDR_XTRX::SetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double sampleRate, uint8_t oversample)
{
    return LMS1_SetSampleRate(sampleRate, oversample, oversample);
}

double LimeSDR_XTRX::GetClockFreq(uint8_t clk_id, uint8_t channel)
{
    ValidateChannel(channel);
    LMS7002M* chip = mLMSChips[channel / 2];
    return chip->GetClockFreq(static_cast<LMS7002M::ClockID>(clk_id));
}

OpStatus LimeSDR_XTRX::SetClockFreq(uint8_t clk_id, double freq, uint8_t channel)
{
    ValidateChannel(channel);
    LMS7002M* chip = mLMSChips[channel / 2];
    return chip->SetClockFreq(static_cast<LMS7002M::ClockID>(clk_id), freq);
}

OpStatus LimeSDR_XTRX::SPI(uint32_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    switch (chipSelect)
    {
    case SPI_LMS7002M:
        return lms7002mPort->SPI(MOSI, MISO, count);
    case SPI_FPGA:
        return fpgaPort->SPI(MOSI, MISO, count);
    default:
        throw std::logic_error("invalid SPI chip select");
    }
}

OpStatus LimeSDR_XTRX::StreamSetup(const StreamConfig& config, uint8_t moduleIndex)
{
    // Allow multiple setup calls
    if (mStreamers.at(moduleIndex) != nullptr)
    {
        delete mStreamers.at(moduleIndex);
    }

    try
    {
        mStreamers.at(moduleIndex) = new TRXLooper_PCIE(mStreamPort, mStreamPort, mFPGA, mLMSChips.at(moduleIndex), moduleIndex);
        if (mCallback_logMessage)
            mStreamers.at(moduleIndex)->SetMessageLogCallback(mCallback_logMessage);
        std::shared_ptr<LitePCIe> trxPort{ mStreamPort };
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
                return ReportError(OpStatus::IO_FAILURE, reason.c_str());
            }
        }
        OpStatus status = mStreamers[moduleIndex]->Setup(config);
        if (status != OpStatus::SUCCESS)
            return status;
        mStreamConfig = config;
        return status;
    } catch (std::logic_error& e)
    {
        lime::error("LimeSDR_XTRX::StreamSetup logic_error %s", e.what());
        throw;
    } catch (std::runtime_error& e)
    {
        lime::error("LimeSDR_XTRX::StreamSetup runtime_error %s", e.what());
        throw;
    }
}

void LimeSDR_XTRX::StreamStop(uint8_t moduleIndex)
{
    LMS7002M_SDRDevice::StreamStop(moduleIndex);
    std::shared_ptr<LitePCIe> trxPort{ mStreamPort };
    if (trxPort && trxPort->IsOpen())
        trxPort->Close();
}

OpStatus LimeSDR_XTRX::LMS1_SetSampleRate(double f_Hz, uint8_t rxDecimation, uint8_t txInterpolation)
{
    if (rxDecimation != 0 && txInterpolation / rxDecimation > 4)
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
            rxDecimation = pow(2, hbd_ovr + 1);
        }
        cgenFreq *= 2 << hbd_ovr;
        if (txInterpolation >= rxDecimation)
        {
            hbi_ovr = hbd_ovr + std::log2(txInterpolation / rxDecimation);
            txInterpolation = pow(2, hbi_ovr + 1);
        }
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
    if (rxDecimation != 0)
        mLMSChip->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2 - std::log2(txInterpolation / rxDecimation));
    else
        mLMSChip->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), hbd_ovr);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), hbi_ovr);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), hbd_ovr);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), hbi_ovr);
    return mLMSChip->SetInterfaceFrequency(cgenFreq, hbi_ovr, hbd_ovr);
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

void LimeSDR_XTRX::LMS1SetPath(bool tx, uint8_t chan, uint8_t pathId)
{
    uint16_t sw_addr = 0x000A;
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
        sw_val &= ~(1 << 4);
        if (path == LMS_PATH_TX1)
            sw_val |= 1 << 4;
        else if (path == LMS_PATH_TX2)
            sw_val &= ~(1 << 4);
        lms->SetBandTRF(path);
    }
    else
    {
        lime::LMS7002M::PathRFE path{ pathId };
        // first configure chip path or loopback
        lms->SetPathRFE(lime::LMS7002M::PathRFE(path));

        // configure rf switches ignoring loopback values
        if (path == LMS7002M::PathRFE::LB1)
            path = LMS7002M::PathRFE::LNAL;
        else if (path == LMS7002M::PathRFE::LB2)
            path = LMS7002M::PathRFE::LNAW;

        sw_val &= ~(0x3 << 2);
        if (path == LMS7002M::PathRFE::LNAW)
            sw_val &= ~(0x3 << 2);
        else if (path == LMS7002M::PathRFE::LNAH)
            sw_val |= 2 << 2;
        else if (path == LMS7002M::PathRFE::LNAL)
            sw_val |= 1 << 2;
    }
    // RF switch controls are toggled for both channels, use channel 0 as the deciding source.
    if (chan == 0)
        mFPGA->WriteRegister(sw_addr, sw_val);
}

OpStatus LimeSDR_XTRX::CustomParameterWrite(const std::vector<CustomParameterIO>& parameters)
{
    return fpgaPort->CustomParameterWrite(parameters);
}

OpStatus LimeSDR_XTRX::CustomParameterRead(std::vector<CustomParameterIO>& parameters)
{
    return fpgaPort->CustomParameterRead(parameters);
}

OpStatus LimeSDR_XTRX::UploadMemory(
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
        return OpStatus::INVALID_VALUE;
    }

    return fpgaPort->ProgramWrite(data, length, progMode, target, callback);
}

OpStatus LimeSDR_XTRX::MemoryWrite(std::shared_ptr<DataStorage> storage, Region region, const void* data)
{
    if (storage == nullptr || storage->ownerDevice != this)
        return OpStatus::INVALID_VALUE;
    return fpgaPort->MemoryWrite(region.address, data, region.size);
}

OpStatus LimeSDR_XTRX::MemoryRead(std::shared_ptr<DataStorage> storage, Region region, void* data)
{
    if (storage == nullptr || storage->ownerDevice != this)
        return OpStatus::INVALID_VALUE;
    return fpgaPort->MemoryRead(region.address, data, region.size);
}

} //namespace lime
