#include "LimeSDR_Mini.h"

#include "USBGeneric.h"
#include "LMSBoards.h"
#include "limesuite/LMS7002M.h"
#include "Si5351C/Si5351C.h"
#include "LMS64CProtocol.h"
#include "Logger.h"
#include "FPGA_Mini.h"
#include "TRXLooper_USB.h"
#include "limesuite/LMS7002M_parameters.h"
#include "lms7002m/LMS7002M_validation.h"
#include "protocols/LMS64CProtocol.h"
#include "limesuite/DeviceNode.h"
#include "FT601/FT601.h"

#include <assert.h>
#include <memory>
#include <set>
#include <stdexcept>
#include <cmath>

#ifdef __unix__
    #include "libusb.h"
#endif

using namespace lime;

static const int STREAM_BULK_WRITE_ADDRESS = 0x03;
static const int STREAM_BULK_READ_ADDRESS = 0x83;

static const int CONTROL_BULK_WRITE_ADDRESS = 0x02;
static const int CONTROL_BULK_READ_ADDRESS = 0x82;

static const uint8_t SPI_LMS7002M = 0;
static const uint8_t SPI_FPGA = 1;

static const SDRDevice::CustomParameter CP_VCTCXO_DAC = { "VCTCXO DAC (runtime)", 0, 0, 255, false };
static const SDRDevice::CustomParameter CP_TEMPERATURE = { "Board Temperature", 1, 0, 65535, true };

LimeSDR_Mini::LimeSDR_Mini(std::shared_ptr<IComms> spiLMS,
    std::shared_ptr<IComms> spiFPGA,
    std::shared_ptr<USBGeneric> streamPort,
    std::shared_ptr<ISerialPort> commsPort)
    : mStreamPort(streamPort)
    , mSerialPort(commsPort)
    , mlms7002mPort(spiLMS)
    , mfpgaPort(spiFPGA)
{
    SDRDevice::Descriptor descriptor = GetDeviceInfo();

    mLMSChips.push_back(new LMS7002M(mlms7002mPort));
    mLMSChips[0]->SetConnection(mlms7002mPort);
    mLMSChips[0]->SetOnCGENChangeCallback(UpdateFPGAInterface, this);

    mFPGA = new FPGA_Mini(spiFPGA, spiLMS);

    double refClk = mFPGA->DetectRefClk();
    mLMSChips[0]->SetReferenceClk_SX(TRXDir::Rx, refClk);

    FPGA::GatewareInfo gw = mFPGA->GetGatewareInfo();
    FPGA::GatewareToDescriptor(gw, descriptor);

    mStreamers.resize(1, nullptr);

    descriptor.customParameters.push_back(CP_VCTCXO_DAC);

    if (descriptor.name == GetDeviceName(LMS_DEV_LIMESDRMINI_V2))
    {
        descriptor.customParameters.push_back(CP_TEMPERATURE);
    }

    descriptor.spiSlaveIds = { { "LMS7002M", SPI_LMS7002M }, { "FPGA", SPI_FPGA } };

    RFSOCDescriptor soc;
    soc.name = "LMS";
    soc.channelCount = 1;
    soc.rxPathNames = { "NONE", "LNAH", "LNAL_NC", "LNAW", "Auto" };
    soc.txPathNames = { "NONE", "BAND1", "BAND2", "Auto" };

    descriptor.rfSOC.push_back(soc);

    std::shared_ptr<DeviceNode> fpgaNode{ std::make_shared<DeviceNode>("FPGA", "FPGA-Mini", mFPGA) };
    fpgaNode->children.push_back(std::shared_ptr<DeviceNode>(std::make_shared<DeviceNode>("LMS", "LMS7002M", mLMSChips[0])));
    descriptor.socTree = std::shared_ptr<DeviceNode>(std::make_shared<DeviceNode>("SDR Mini", "SDRDevice", this));
    descriptor.socTree->children.push_back(fpgaNode);

    mDeviceDescriptor = descriptor;
}

LimeSDR_Mini::~LimeSDR_Mini()
{
    if (mStreamers[0])
    {
        delete mStreamers[0];
        mStreamers[0] = nullptr;
    }

    delete mFPGA;
}

// Verify and configure given settings
// throw logic_error with description why the config is not possible
inline bool InRange(double val, double min, double max)
{
    return val >= min ? val <= max : false;
}

static inline const std::string strFormat(const char* format, ...)
{
    char ctemp[256];

    va_list args;
    va_start(args, format);
    vsnprintf(ctemp, 256, format, args);
    va_end(args);

    return std::string(ctemp);
}

void LimeSDR_Mini::Configure(const SDRConfig& cfg, uint8_t moduleIndex = 0)
{
    try
    {
        std::vector<std::string> errors;
        bool isValidConfig = LMS7002M_Validate(cfg, errors, 1);

        if (!isValidConfig)
        {
            std::stringstream ss;

            for (const auto& err : errors)
            {
                ss << err << std::endl;
            }

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

        // config validation complete, now do the actual configuration

        if (cfg.referenceClockFreq != 0)
        {
            mLMSChips[0]->SetClockFreq(LMS7002M::ClockID::CLK_REFERENCE, cfg.referenceClockFreq, 0);
        }

        if (rxUsed)
        {
            mLMSChips[0]->SetFrequencySX(TRXDir::Rx, cfg.channel[0].rx.centerFrequency);
        }

        if (txUsed)
        {
            mLMSChips[0]->SetFrequencySX(TRXDir::Tx, cfg.channel[0].rx.centerFrequency);
        }

        for (int i = 0; i < 2; ++i)
        {
            const ChannelConfig& ch = cfg.channel[i];
            mLMSChips[0]->SetActiveChannel((i & 1) ? LMS7002M::ChB : LMS7002M::ChA);
            mLMSChips[0]->EnableChannel(TRXDir::Rx, i, ch.rx.enabled);
            mLMSChips[0]->EnableChannel(TRXDir::Tx, i, ch.tx.enabled);

            mLMSChips[0]->SetPathRFE(static_cast<LMS7002M::PathRFE>(ch.rx.path));

            if (ch.rx.path == 4)
            {
                mLMSChips[0]->Modify_SPI_Reg_bits(LMS7_INPUT_CTL_PGA_RBB, 3); // baseband loopback
            }

            mLMSChips[0]->SetBandTRF(ch.tx.path);
            // TODO: set gains, filters...
        }

        mLMSChips[0]->SetActiveChannel(LMS7002M::ChA);
        // sampling rate
        double sampleRate;

        if (rxUsed)
        {
            sampleRate = cfg.channel[0].rx.sampleRate;
        }
        else
        {
            sampleRate = cfg.channel[0].tx.sampleRate;
        }

        SetSampleRate(sampleRate, cfg.channel[0].rx.oversample);
    } //try
    catch (std::logic_error& e)
    {
        printf("LimeSDR_Mini config: %s\n", e.what());
        throw;
    } catch (std::runtime_error& e)
    {
        throw;
    }
}

int LimeSDR_Mini::Init()
{
    struct regVal {
        uint16_t adr;
        uint16_t val;
    };

    const std::vector<regVal> initVals_1v0 = { { 0x0022, 0x0FFF },
        { 0x0023, 0x5550 },
        { 0x002B, 0x0038 },
        { 0x002C, 0x0000 },
        { 0x002D, 0x0641 },
        { 0x0086, 0x4101 },
        { 0x0087, 0x5555 },
        { 0x0088, 0x03F0 },
        { 0x0089, 0x1078 },
        { 0x008B, 0x2100 },
        { 0x008C, 0x267B },
        { 0x0092, 0xFFFF },
        { 0x0093, 0x03FF },
        { 0x00A1, 0x656A },
        { 0x00A6, 0x0001 },
        { 0x00A9, 0x8000 },
        { 0x00AC, 0x2000 },
        { 0x0105, 0x0011 },
        { 0x0108, 0x218C },
        { 0x0109, 0x6100 },
        { 0x010A, 0x1F4C },
        { 0x010B, 0x0001 },
        { 0x010C, 0x8865 },
        { 0x010E, 0x0000 },
        { 0x010F, 0x3142 },
        { 0x0110, 0x2B14 },
        { 0x0111, 0x0000 },
        { 0x0112, 0x942E },
        { 0x0113, 0x03C2 },
        { 0x0114, 0x00D0 },
        { 0x0117, 0x1230 },
        { 0x0119, 0x18D2 },
        { 0x011C, 0x8941 },
        { 0x011D, 0x0000 },
        { 0x011E, 0x0740 },
        { 0x0120, 0xE6C0 },
        { 0x0121, 0x8650 },
        { 0x0123, 0x000F },
        { 0x0200, 0x00E1 },
        { 0x0208, 0x017B },
        { 0x020B, 0x4000 },
        { 0x020C, 0x8000 },
        { 0x0400, 0x8081 },
        { 0x0404, 0x0006 },
        { 0x040B, 0x1020 },
        { 0x040C, 0x00FB } };

    const std::vector<regVal> initVals_1v2 = { { 0x0022, 0x0FFF },
        { 0x0023, 0x5550 },
        { 0x002B, 0x0038 },
        { 0x002C, 0x0000 },
        { 0x002D, 0x0641 },
        { 0x0086, 0x4101 },
        { 0x0087, 0x5555 },
        { 0x0088, 0x03F0 },
        { 0x0089, 0x1078 },
        { 0x008B, 0x2100 },
        { 0x008C, 0x267B },
        { 0x00A1, 0x656A },
        { 0x00A6, 0x0009 },
        { 0x00A7, 0x8A8A },
        { 0x00A9, 0x8000 },
        { 0x00AC, 0x2000 },
        { 0x0105, 0x0011 },
        { 0x0108, 0x218C },
        { 0x0109, 0x6100 },
        { 0x010A, 0x1F4C },
        { 0x010B, 0x0001 },
        { 0x010C, 0x8865 },
        { 0x010E, 0x0000 },
        { 0x010F, 0x3142 },
        { 0x0110, 0x2B14 },
        { 0x0111, 0x0000 },
        { 0x0112, 0x942E },
        { 0x0113, 0x03C2 },
        { 0x0114, 0x00D0 },
        { 0x0117, 0x1230 },
        { 0x0119, 0x18D2 },
        { 0x011C, 0x8941 },
        { 0x011D, 0x0000 },
        { 0x011E, 0x0740 },
        { 0x0120, 0xC5C0 },
        { 0x0121, 0x8650 },
        { 0x0123, 0x000F },
        { 0x0200, 0x00E1 },
        { 0x0208, 0x017B },
        { 0x020B, 0x4000 },
        { 0x020C, 0x8000 },
        { 0x0400, 0x8081 },
        { 0x0404, 0x0006 },
        { 0x040B, 0x1020 },
        { 0x040C, 0x00FB } };

    int hw_version = mFPGA->ReadRegister(3) & 0xF;
    auto& initVals = hw_version >= 2 ? initVals_1v2 : initVals_1v0;

    lime::LMS7002M* lms = mLMSChips[0];

    if (lms->ResetChip() != 0)
    {
        return -1;
    }

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    for (auto i : initVals)
    {
        lms->SPI_write(i.adr, i.val, true);
    }

    if (lms->CalibrateTxGain(0, nullptr) != 0)
    {
        return -1;
    }

    lms->EnableChannel(TRXDir::Tx, 0, false);

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    lms->SPI_write(0x0123, 0x000F); //SXT
    lms->SPI_write(0x0120, 0x80C0); //SXT
    lms->SPI_write(0x011C, 0x8941); //SXT
    lms->EnableChannel(TRXDir::Rx, 0, false);
    lms->EnableChannel(TRXDir::Tx, 0, false);

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);

    /*bool auto_path[2] = {auto_tx_path, auto_rx_path};
    auto_tx_path = false;
    auto_rx_path = false;

    if(SetFrequency(true, 0, GetFrequency(true, 0)) != 0)
    {
        return -1;
    }

    if(SetFrequency(false, 0, GetFrequency(false, 0)) != 0)
    {
        return -1;
    }

    auto_tx_path = auto_path[0];
    auto_rx_path = auto_path[1];

    if (SetRate(15.36e6, 1) != 0)
    {
        return -1;
    }*/

    return 0;
}

void LimeSDR_Mini::Reset()
{
    LMS64CProtocol::DeviceReset(*mSerialPort, 0);
}

double LimeSDR_Mini::GetClockFreq(uint8_t clk_id, uint8_t channel)
{
    return mLMSChips[0]->GetClockFreq(static_cast<LMS7002M::ClockID>(clk_id), channel);
}

void LimeSDR_Mini::SetClockFreq(uint8_t clk_id, double freq, uint8_t channel)
{
    mLMSChips[0]->SetClockFreq(static_cast<LMS7002M::ClockID>(clk_id), freq, channel);
}

void LimeSDR_Mini::Synchronize(bool toChip)
{
    if (toChip)
    {
        if (mLMSChips[0]->UploadAll() == 0)
        {
            mLMSChips[0]->Modify_SPI_Reg_bits(LMS7param(MAC), 1, true);
            //ret = SetFPGAInterfaceFreq(-1, -1, -1000, -1000); // TODO: implement
        }
    }
    else
    {
        mLMSChips[0]->DownloadAll();
    }
}

void LimeSDR_Mini::EnableCache(bool enable)
{
    mLMSChips[0]->EnableValuesCache(enable);

    if (mFPGA)
    {
        mFPGA->EnableValuesCache(enable);
    }
}

void LimeSDR_Mini::SPI(uint32_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    assert(mStreamPort);
    assert(MOSI);
    LMS64CPacket pkt;

    size_t srcIndex = 0;
    size_t destIndex = 0;
    constexpr int maxBlocks = LMS64CPacket::payloadSize / (sizeof(uint32_t) / sizeof(uint8_t)); // = 14

    while (srcIndex < count)
    {
        pkt.status = LMS64CProtocol::STATUS_UNDEFINED;
        pkt.blockCount = 0;
        pkt.periphID = chipSelect;

        // fill packet with same direction operations
        const bool willDoWrite = MOSI[srcIndex] & (1 << 31);

        for (int i = 0; i < maxBlocks && srcIndex < count; ++i)
        {
            const bool isWrite = MOSI[srcIndex] & (1 << 31);

            if (isWrite != willDoWrite)
            {
                break; // change between write/read, flush packet
            }

            if (isWrite)
            {
                switch (chipSelect)
                {
                case SPI_LMS7002M:
                    pkt.cmd = LMS64CProtocol::CMD_LMS7002_WR;
                    break;
                case SPI_FPGA:
                    pkt.cmd = LMS64CProtocol::CMD_BRDSPI_WR;
                    break;
                default:
                    throw std::logic_error("LimeSDR SPI invalid SPI chip select");
                }

                int payloadOffset = pkt.blockCount * 4;
                pkt.payload[payloadOffset + 0] = MOSI[srcIndex] >> 24;
                pkt.payload[payloadOffset + 1] = MOSI[srcIndex] >> 16;
                pkt.payload[payloadOffset + 2] = MOSI[srcIndex] >> 8;
                pkt.payload[payloadOffset + 3] = MOSI[srcIndex];
            }
            else
            {
                switch (chipSelect)
                {
                case SPI_LMS7002M:
                    pkt.cmd = LMS64CProtocol::CMD_LMS7002_RD;
                    break;
                case SPI_FPGA:
                    pkt.cmd = LMS64CProtocol::CMD_BRDSPI_RD;
                    break;
                default:
                    throw std::logic_error("LimeSDR SPI invalid SPI chip select");
                }

                int payloadOffset = pkt.blockCount * 2;
                pkt.payload[payloadOffset + 0] = MOSI[srcIndex] >> 8;
                pkt.payload[payloadOffset + 1] = MOSI[srcIndex];
            }

            ++pkt.blockCount;
            ++srcIndex;
        }

        // flush packet
        //printPacket(pkt, 4, "Wr:");
        int sent = mSerialPort->Write(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
        if (sent != sizeof(pkt))
        {
            throw std::runtime_error("SPI failed");
        }

        int recv = mSerialPort->Read(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
        //printPacket(pkt, 4, "Rd:");

        if (recv >= pkt.headerSize + 4 * pkt.blockCount && pkt.status == LMS64CProtocol::STATUS_COMPLETED_CMD)
        {
            for (int i = 0; MISO && i < pkt.blockCount && destIndex < count; ++i)
            {
                //MISO[destIndex] = 0;
                //MISO[destIndex] = pkt.payload[0] << 24;
                //MISO[destIndex] |= pkt.payload[1] << 16;
                MISO[destIndex] = (pkt.payload[i * 4 + 2] << 8) | pkt.payload[i * 4 + 3];
                ++destIndex;
            }
        }
        else
        {
            throw std::runtime_error("SPI failed");
        }
    }
}

// Callback for updating FPGA's interface clocks when LMS7002M CGEN is manually modified
int LimeSDR_Mini::UpdateFPGAInterface(void* userData)
{
    constexpr int chipIndex = 0;
    assert(userData != nullptr);
    LimeSDR_Mini* pthis = static_cast<LimeSDR_Mini*>(userData);
    LMS7002M* soc = pthis->mLMSChips[chipIndex];
    return UpdateFPGAInterfaceFrequency(*soc, *pthis->mFPGA, chipIndex);
}

void LimeSDR_Mini::SetSampleRate(double f_Hz, uint8_t oversample)
{
    const bool bypass = (oversample <= 1);
    uint8_t decimation = 7; // HBD_OVR_RXTSP=7 - bypass
    uint8_t interpolation = 7; // HBI_OVR_TXTSP=7 - bypass
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

        decimation = 4;
        if (oversample <= 16)
        {
            const int decTbl[] = { 0, 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3 };
            decimation = decTbl[oversample];
        }
        interpolation = decimation;
        cgenFreq *= 2 << decimation;
    }

    if (bypass)
    {
        lime::info("Sampling rate set(%.3f MHz): CGEN:%.3f MHz, Decim: bypass, Interp: bypass", f_Hz / 1e6, cgenFreq / 1e6);
    }
    else
    {
        lime::info("Sampling rate set(%.3f MHz): CGEN:%.3f MHz, Decim: 2^%i, Interp: 2^%i",
            f_Hz / 1e6,
            cgenFreq / 1e6,
            decimation + 1,
            interpolation + 1); // dec/inter ratio is 2^(value+1)
    }

    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7_LML1_SISODDR, 1);
    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7_LML2_SISODDR, 1);
    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7_CDSN_RXALML, !bypass);
    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0);
    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2);
    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), decimation);
    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), interpolation);
    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    if (bypass)
        mLMSChips[0]->SetInterfaceFrequency(f_Hz * 4, 7, 7);
    else
        mLMSChips[0]->SetInterfaceFrequency(cgenFreq, interpolation, decimation);
}

SDRDevice::Descriptor LimeSDR_Mini::GetDeviceInfo(void)
{
    assert(mStreamPort);
    SDRDevice::Descriptor deviceDescriptor;

    LMS64CProtocol::FirmwareInfo info;
    int returnCode = LMS64CProtocol::GetFirmwareInfo(*mSerialPort, info);

    if (returnCode != 0)
    {
        deviceDescriptor.name = GetDeviceName(LMS_DEV_UNKNOWN);
        deviceDescriptor.expansionName = GetExpansionBoardName(EXP_BOARD_UNKNOWN);

        return deviceDescriptor;
    }

    deviceDescriptor.name = GetDeviceName(static_cast<eLMS_DEV>(info.deviceId));
    deviceDescriptor.expansionName = GetExpansionBoardName(static_cast<eEXP_BOARD>(info.expansionBoardId));
    deviceDescriptor.firmwareVersion = std::to_string(int(info.firmware));
    deviceDescriptor.hardwareVersion = std::to_string(int(info.hardware));
    deviceDescriptor.protocolVersion = std::to_string(int(info.protocol));
    deviceDescriptor.serialNumber = info.boardSerialNumber;

    const uint32_t addrs[] = { 0x0000, 0x0001, 0x0002, 0x0003 };
    uint32_t data[4];
    SPI(SPI_FPGA, addrs, data, 4);
    auto boardID = static_cast<eLMS_DEV>(data[0]); //(pkt.inBuffer[2] << 8) | pkt.inBuffer[3];
    auto gatewareVersion = data[1]; //(pkt.inBuffer[6] << 8) | pkt.inBuffer[7];
    auto gatewareRevision = data[2]; //(pkt.inBuffer[10] << 8) | pkt.inBuffer[11];
    auto hwVersion = data[3] & 0x7F; //pkt.inBuffer[15]&0x7F;

    deviceDescriptor.gatewareTargetBoard = GetDeviceName(boardID);
    deviceDescriptor.gatewareVersion = std::to_string(int(gatewareVersion));
    deviceDescriptor.gatewareRevision = std::to_string(int(gatewareRevision));
    deviceDescriptor.hardwareVersion = std::to_string(int(hwVersion));

    return deviceDescriptor;
}

int LimeSDR_Mini::StreamSetup(const StreamConfig& config, uint8_t moduleIndex)
{
    if (mStreamers[0])
    {
        return -1; // already running
    }

    try
    {
        auto connection = std::static_pointer_cast<FT601>(mStreamPort);
        connection->ResetStreamBuffers();

        mStreamers[0] = new TRXLooper_USB(mStreamPort, mFPGA, mLMSChips[0], STREAM_BULK_READ_ADDRESS, STREAM_BULK_WRITE_ADDRESS);
        mStreamers[0]->Setup(config);

        return 0;
    } catch (std::logic_error& e)
    {
        return -1;
    } catch (std::runtime_error& e)
    {
        return -1;
    }
}

void LimeSDR_Mini::StreamStart(uint8_t moduleIndex)
{
    if (mStreamers[0])
    {
        mStreamers[0]->Start();
    }
    else
    {
        throw std::runtime_error("Stream not setup");
    }
}

void LimeSDR_Mini::StreamStop(uint8_t moduleIndex)
{
    if (!mStreamers[0])
    {
        return;
    }

    mStreamers[0]->Stop();

    delete mStreamers[0];
    mStreamers[0] = nullptr;
}

void LimeSDR_Mini::StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats* rx, SDRDevice::StreamStats* tx)
{
    if (rx)
    {
        auto stats = mStreamers[moduleIndex]->GetStats(TRXDir::Rx);
        rx->FIFO_filled = stats.FIFO_filled;
        rx->dataRate_Bps = stats.dataRate_Bps;
    }

    if (tx)
    {
        auto stats = mStreamers[moduleIndex]->GetStats(TRXDir::Tx);
        tx->FIFO_filled = stats.FIFO_filled;
        tx->dataRate_Bps = stats.dataRate_Bps;
    }
}

void* LimeSDR_Mini::GetInternalChip(uint32_t index)
{
    return mLMSChips.at(index);
}

int LimeSDR_Mini::GPIODirRead(uint8_t* buffer, const size_t bufLength)
{
    if (!buffer || bufLength == 0)
    {
        return -1;
    }

    const uint32_t addr = 0xC4;
    uint32_t value;

    int ret = mFPGA->ReadRegisters(&addr, &value, 1);
    buffer[0] = value;

    if (bufLength > 1)
    {
        buffer[1] = (value >> 8);
    }

    return ret;
}

int LimeSDR_Mini::GPIORead(uint8_t* buffer, const size_t bufLength)
{
    if (!buffer || bufLength == 0)
    {
        return -1;
    }

    const uint32_t addr = 0xC2;
    uint32_t value;

    int ret = mFPGA->ReadRegisters(&addr, &value, 1);
    buffer[0] = value;

    if (bufLength > 1)
    {
        buffer[1] = (value >> 8);
    }

    return ret;
}

int LimeSDR_Mini::GPIODirWrite(const uint8_t* buffer, const size_t bufLength)
{
    if (!buffer || bufLength == 0)
    {
        return -1;
    }

    const uint32_t addr = 0xC4;
    const uint32_t value = (bufLength == 1) ? buffer[0] : buffer[0] | (buffer[1] << 8);

    return mFPGA->WriteRegisters(&addr, &value, 1);
}

int LimeSDR_Mini::GPIOWrite(const uint8_t* buffer, const size_t bufLength)
{
    if (!buffer || bufLength == 0)
    {
        return -1;
    }

    const uint32_t addr = 0xC6;
    const uint32_t value = (bufLength == 1) ? buffer[0] : buffer[0] | (buffer[1] << 8);

    return mFPGA->WriteRegisters(&addr, &value, 1);
}

int LimeSDR_Mini::CustomParameterWrite(const int32_t* ids, const double* values, const size_t count, const std::string& units)
{
    return mfpgaPort->CustomParameterWrite(ids, values, count, units);
}

int LimeSDR_Mini::CustomParameterRead(const int32_t* ids, double* values, const size_t count, std::string* units)
{
    return mfpgaPort->CustomParameterRead(ids, values, count, units);
}

int LimeSDR_Mini::ReadFPGARegister(uint32_t address)
{
    return mFPGA->ReadRegister(address);
}

int LimeSDR_Mini::WriteFPGARegister(uint32_t address, uint32_t value)
{
    return mFPGA->WriteRegister(address, value);
}
