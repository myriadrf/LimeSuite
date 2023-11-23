#include "LimeSDR.h"

#include "USBGeneric.h"
#include "LMSBoards.h"
#include "limesuite/LMS7002M.h"
#include "Si5351C/Si5351C.h"
#include "LMS64CProtocol.h"
#include "Logger.h"
#include "FPGA_common.h"
#include "TRXLooper_USB.h"
#include "limesuite/LMS7002M_parameters.h"
#include "lms7002m/LMS7002M_validation.h"
#include "protocols/LMS64CProtocol.h"
#include "limesuite/DeviceNode.h"

#include <assert.h>
#include <memory>
#include <set>
#include <stdexcept>
#include <cmath>

#ifdef __unix__
    #ifdef __GNUC__
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpedantic"
    #endif
    #include <libusb.h>
    #ifdef __GNUC__
        #pragma GCC diagnostic pop
    #endif
#endif

#define CTR_W_REQCODE 0xC1
#define CTR_W_VALUE 0x0000
#define CTR_W_INDEX 0x0000

#define CTR_R_REQCODE 0xC0
#define CTR_R_VALUE 0x0000
#define CTR_R_INDEX 0x0000

using namespace lime;

static constexpr uint8_t ctrlBulkOutAddr = 0x0F;
static constexpr uint8_t ctrlBulkInAddr = 0x8F;

static constexpr uint8_t streamBulkOutAddr = 0x01;
static constexpr uint8_t streamBulkInAddr = 0x81;

static constexpr uint8_t spi_LMS7002M = 0;
static constexpr uint8_t spi_FPGA = 1;

static const SDRDevice::CustomParameter CP_VCTCXO_DAC = { "VCTCXO DAC (volatile)", 0, 0, 65535, false };
static const SDRDevice::CustomParameter CP_TEMPERATURE = { "Board Temperature", 1, 0, 65535, true };

static inline void ValidateChannel(uint8_t channel)
{
    if (channel > 1)
        throw std::logic_error("invalid channel index");
}

LimeSDR::LimeSDR(std::shared_ptr<IComms> spiLMS,
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

    mFPGA = new FPGA(spiFPGA, spiLMS);
    FPGA::GatewareInfo gw = mFPGA->GetGatewareInfo();
    FPGA::GatewareToDescriptor(gw, descriptor);

    mStreamers.resize(1, nullptr);

    descriptor.customParameters.push_back(CP_VCTCXO_DAC);
    descriptor.customParameters.push_back(CP_TEMPERATURE);

    descriptor.spiSlaveIds = { { "LMS7002M", spi_LMS7002M }, { "FPGA", spi_FPGA } };

    RFSOCDescriptor soc;
    soc.name = "LMS";
    soc.channelCount = 2;
    soc.rxPathNames = { "None", "LNAH", "LNAL", "LNAW" };
    soc.txPathNames = { "None", "Band1", "Band2" };
    descriptor.rfSOC.push_back(soc);

    std::shared_ptr<DeviceNode> fpgaNode{ new DeviceNode("FPGA", "FPGA", mFPGA) };
    fpgaNode->children.push_back(std::shared_ptr<DeviceNode>(new DeviceNode("LMS", "LMS7002M", mLMSChips[0])));
    descriptor.socTree = std::shared_ptr<DeviceNode>(new DeviceNode("SDR-USB", "SDRDevice", this));
    descriptor.socTree->children.push_back(fpgaNode);

    mDeviceDescriptor = descriptor;

    //must configure synthesizer before using LimeSDR
    /*if (info.device == LMS_DEV_LIMESDR && info.hardware < 4)
    {
        std::shared_ptr<Si5351C> si5351module(new Si5351C());
        si5351module->Initialize(conn);
        si5351module->SetPLL(0, 25000000, 0);
        si5351module->SetPLL(1, 25000000, 0);
        si5351module->SetClock(0, 27000000, true, false);
        si5351module->SetClock(1, 27000000, true, false);
        for (int i = 2; i < 8; ++i)
            si5351module->SetClock(i, 27000000, false, false);
        Si5351C::Status status = si5351module->ConfigureClocks();
        if (status != Si5351C::SUCCESS)
        {
            lime::warning("Failed to configure Si5351C");
            return;
        }
        status = si5351module->UploadConfiguration();
        if (status != Si5351C::SUCCESS)
            lime::warning("Failed to upload Si5351C configuration");
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); //some settle time
    }*/
}

LimeSDR::~LimeSDR()
{
    if (mStreamers[0])
    {
        delete mStreamers[0];
        mStreamers[0] = nullptr;
    }

    delete mFPGA;
}

void LimeSDR::Configure(const SDRConfig& cfg, uint8_t moduleIndex = 0)
{
    try
    {
        std::vector<std::string> errors;
        bool isValidConfig = LMS7002M_Validate(cfg, errors);

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
            mLMSChips[0]->SetClockFreq(LMS7002M::ClockID::CLK_REFERENCE, cfg.referenceClockFreq, 0);

        if (rxUsed && cfg.channel[0].rx.centerFrequency > 0)
            mLMSChips[0]->SetFrequencySX(TRXDir::Rx, cfg.channel[0].rx.centerFrequency);
        if (txUsed && cfg.channel[0].tx.centerFrequency > 0)
            mLMSChips[0]->SetFrequencySX(TRXDir::Tx, cfg.channel[0].tx.centerFrequency);

        for (int i = 0; i < 2; ++i)
        {
            const ChannelConfig& ch = cfg.channel[i];
            mLMSChips[0]->SetActiveChannel((i & 1) ? LMS7002M::Channel::ChB : LMS7002M::Channel::ChA);
            mLMSChips[0]->EnableChannel(TRXDir::Rx, i, ch.rx.enabled);
            mLMSChips[0]->EnableChannel(TRXDir::Tx, i, ch.tx.enabled);

            mLMSChips[0]->SetPathRFE(static_cast<LMS7002M::PathRFE>(ch.rx.path));
            if (ch.rx.path == 4)
                mLMSChips[0]->Modify_SPI_Reg_bits(LMS7_INPUT_CTL_PGA_RBB, 3); // baseband loopback
            mLMSChips[0]->SetBandTRF(ch.tx.path);
            // TODO: set gains, filters...
        }
        mLMSChips[0]->SetActiveChannel(LMS7002M::Channel::ChA);
        // sampling rate
        double sampleRate;
        if (rxUsed)
            sampleRate = cfg.channel[0].rx.sampleRate;
        else
            sampleRate = cfg.channel[0].tx.sampleRate;
        if (sampleRate > 0)
            SetSampleRate(sampleRate, cfg.channel[0].rx.oversample);
    } //try
    catch (std::logic_error& e)
    {
        printf("LimeSDR config: %s\n", e.what());
        throw;
    } catch (std::runtime_error& e)
    {
        throw;
    }
}

// Callback for updating FPGA's interface clocks when LMS7002M CGEN is manually modified
int LimeSDR::UpdateFPGAInterface(void* userData)
{
    constexpr int chipIndex = 0;
    assert(userData != nullptr);
    LimeSDR* pthis = static_cast<LimeSDR*>(userData);
    LMS7002M* soc = pthis->mLMSChips[chipIndex];
    return UpdateFPGAInterfaceFrequency(*soc, *pthis->mFPGA, chipIndex);
}

void LimeSDR::SetSampleRate(double f_Hz, uint8_t oversample)
{
    const bool bypass = (oversample == 1) || (oversample == 0 && f_Hz > 62e6);
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
        lime::info("Sampling rate set(%.3f MHz): CGEN:%.3f MHz, Decim: bypass, Interp: bypass", f_Hz / 1e6, cgenFreq / 1e6);
    else
        lime::info("Sampling rate set(%.3f MHz): CGEN:%.3f MHz, Decim: 2^%i, Interp: 2^%i",
            f_Hz / 1e6,
            cgenFreq / 1e6,
            decimation + 1,
            interpolation + 1); // dec/inter ratio is 2^(value+1)

    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0);
    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2);
    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), decimation);
    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), interpolation);
    mLMSChips[0]->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    mLMSChips[0]->SetInterfaceFrequency(cgenFreq, interpolation, decimation);
}

int LimeSDR::Init()
{
    struct regVal {
        uint16_t adr;
        uint16_t val;
    };

    const std::vector<regVal> initVals = { { 0x0022, 0x0FFF },
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
        { 0x040C, 0x00FB } };

    lime::LMS7002M* lms = mLMSChips[0];
    // TODO: write GPIO to hard reset the chip
    if (lms->ResetChip() != 0)
        return -1;

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    for (auto i : initVals)
        lms->SPI_write(i.adr, i.val, true);

    // TODO:
    // if(lms->CalibrateTxGain(0,nullptr) != 0)
    //     return -1;

    EnableChannel(TRXDir::Rx, 0, false);
    EnableChannel(TRXDir::Tx, 0, false);
    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    for (auto i : initVals)
        if (i.adr >= 0x100)
            lms->SPI_write(i.adr, i.val, true);

    // if(lms->CalibrateTxGain(0,nullptr) != 0)
    //     return -1;

    EnableChannel(TRXDir::Rx, 1, false);
    EnableChannel(TRXDir::Tx, 1, false);

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);

    // if(lms->SetFrequency(SDRDevice::Dir::Tx, 0, lms->GetFrequency(SDRDevice::Dir::Tx, 0)) != 0)
    //     return -1;
    // if(lms->SetFrequency(SDRDevice::Dir::Rx, 0, lms->GetFrequency(SDRDevice::Dir::Rx, 0)) != 0)
    //     return -1;

    // if (SetRate(10e6,2)!=0)
    //     return -1;
    return 0;
}

SDRDevice::Descriptor LimeSDR::GetDeviceInfo(void)
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
    SPI(spi_FPGA, addrs, data, 4);
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

void LimeSDR::Reset()
{
    LMS64CProtocol::DeviceReset(*mSerialPort, 0);
}

int LimeSDR::EnableChannel(TRXDir dir, uint8_t channel, bool enabled)
{
    int ret = mLMSChips[0]->EnableChannel(dir, channel, enabled);
    if (dir == TRXDir::Tx) //always enable DAC1, otherwise sample rates <2.5MHz do not work
        mLMSChips[0]->Modify_SPI_Reg_bits(LMS7_PD_TX_AFE1, 0);
    return ret;
}
/*
uint8_t LimeSDR::GetPath(SDRDevice::Dir dir, uint8_t channel) const
{
    ValidateChannel(channel);
    mLMSChips[0]->SetActiveChannel(channel == 1 ? LMS7002M::ChB : LMS7002M::ChA);
    if (dir == SDRDevice::Dir::Tx)
        return mLMSChips[0]->GetBandTRF();
    return mLMSChips[0]->GetPathRFE();
}
*/

double LimeSDR::GetClockFreq(uint8_t clk_id, uint8_t channel)
{
    return mLMSChips[0]->GetClockFreq(static_cast<LMS7002M::ClockID>(clk_id), channel);
}

void LimeSDR::SetClockFreq(uint8_t clk_id, double freq, uint8_t channel)
{
    mLMSChips[0]->SetClockFreq(static_cast<LMS7002M::ClockID>(clk_id), freq, channel);
}

void LimeSDR::Synchronize(bool toChip)
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
        mLMSChips[0]->DownloadAll();
}

void LimeSDR::EnableCache(bool enable)
{
    mLMSChips[0]->EnableValuesCache(enable);
    if (mFPGA)
        mFPGA->EnableValuesCache(enable);
}

void LimeSDR::SPI(uint32_t chipSelect, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    assert(mStreamPort);
    assert(MOSI);
    LMS64CPacket pkt;
    pkt.status = LMS64CProtocol::STATUS_UNDEFINED;
    pkt.blockCount = 0;
    pkt.periphID = chipSelect;

    size_t srcIndex = 0;
    size_t destIndex = 0;
    const int maxBlocks = 14;
    while (srcIndex < count)
    {
        // fill packet with same direction operations
        const bool willDoWrite = MOSI[srcIndex] & (1 << 31);
        for (int i = 0; i < maxBlocks && srcIndex < count; ++i)
        {
            const bool isWrite = MOSI[srcIndex] & (1 << 31);
            if (isWrite != willDoWrite)
                break; // change between write/read, flush packet

            if (isWrite)
            {
                switch (chipSelect)
                {
                case spi_LMS7002M:
                    pkt.cmd = LMS64CProtocol::CMD_LMS7002_WR;
                    break;
                case spi_FPGA:
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
                case spi_LMS7002M:
                    pkt.cmd = LMS64CProtocol::CMD_LMS7002_RD;
                    break;
                case spi_FPGA:
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
        int sent = mStreamPort->BulkTransfer(ctrlBulkOutAddr, reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
        if (sent != sizeof(pkt))
            throw std::runtime_error("SPI failed");

        int recv = mStreamPort->BulkTransfer(ctrlBulkInAddr, reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
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
            throw std::runtime_error("SPI failed");
        pkt.blockCount = 0;
        pkt.status = LMS64CProtocol::STATUS_UNDEFINED;
    }
}

/*int LimeSDR::I2CWrite(int address, const uint8_t *data, uint32_t length)
{
    assert(comms);
    LMS64CPacket pkt;
    int remainingBytes = length;
    const uint8_t* src = data;
    while (remainingBytes > 0)
    {
        pkt.cmd = LMS64CProtocol::CMD_I2C_WR;
        pkt.status = LMS64CProtocol::STATUS_UNDEFINED;
        pkt.blockCount = remainingBytes > pkt.payloadSize ? pkt.payloadSize : remainingBytes;
        pkt.periphID = address;
        memcpy(pkt.payload, src, pkt.blockCount);
        src += pkt.blockCount;
        remainingBytes -= pkt.blockCount;
        int sent = comms->BulkTransfer(ctrlBulkOutAddr, reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
        if (sent != sizeof(pkt))
            throw std::runtime_error("I2C write failed");
        int recv = comms->BulkTransfer(ctrlBulkInAddr, reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);

        if (recv < pkt.headerSize || pkt.status != LMS64CProtocol::STATUS_COMPLETED_CMD)
            throw std::runtime_error("I2C write failed");
    }
    return 0;
}*/

/*int LimeSDR::I2CRead(int address, uint8_t *data, uint32_t length)
{
    assert(comms);
    LMS64CPacket pkt;
    int remainingBytes = length;
    uint8_t* dest = data;
    while (remainingBytes > 0)
    {
        pkt.cmd = LMS64CProtocol::CMD_I2C_RD;
        pkt.status = LMS64CProtocol::STATUS_UNDEFINED;
        pkt.blockCount = remainingBytes > pkt.payloadSize ? pkt.payloadSize : remainingBytes;
        pkt.periphID = address;

        int sent = comms->BulkTransfer(ctrlBulkOutAddr, reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
        if (sent != sizeof(pkt))
            throw std::runtime_error("I2C read failed");
        int recv = comms->BulkTransfer(ctrlBulkInAddr, reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);

        memcpy(dest, pkt.payload, pkt.blockCount);
        dest += pkt.blockCount;
        remainingBytes -= pkt.blockCount;

        if (recv <= pkt.headerSize || pkt.status != LMS64CProtocol::STATUS_COMPLETED_CMD)
            throw std::runtime_error("I2C read failed");
    }
    return 0;
}*/

// There might be some leftover samples data still buffered in USB device
// clear the USB buffers before streaming samples to avoid old data
void LimeSDR::ResetUSBFIFO()
{
    LMS64CPacket pkt;
    pkt.cmd = LMS64CProtocol::CMD_USB_FIFO_RST;
    pkt.status = LMS64CProtocol::STATUS_UNDEFINED;
    pkt.blockCount = 1;
    pkt.payload[0] = 0;

    int sentBytes = mStreamPort->ControlTransfer(
        LIBUSB_REQUEST_TYPE_VENDOR, CTR_W_REQCODE, CTR_W_VALUE, CTR_W_INDEX, reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt), 100);
    if (sentBytes != sizeof(pkt))
        throw std::runtime_error("LimeSDR::ResetUSBFIFO write failed");
    int gotBytes = mStreamPort->ControlTransfer(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
        CTR_R_REQCODE,
        CTR_R_VALUE,
        CTR_R_INDEX,
        reinterpret_cast<uint8_t*>(&pkt),
        sizeof(pkt),
        100);
    if (gotBytes != sizeof(pkt))
        throw std::runtime_error("LimeSDR::ResetUSBFIFO read failed");
}

int LimeSDR::StreamSetup(const StreamConfig& config, uint8_t moduleIndex)
{
    if (mStreamers[0])
        return -1; // already running
    try
    {
        mStreamers[0] = new TRXLooper_USB(mStreamPort, mFPGA, mLMSChips[0], streamBulkInAddr, streamBulkOutAddr);
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

void LimeSDR::StreamStart(uint8_t moduleIndex)
{
    if (mStreamers[0])
    {
        ResetUSBFIFO();
        mStreamers[0]->Start();
    }
    else
        throw std::runtime_error("Stream not setup");
}

void LimeSDR::StreamStop(uint8_t moduleIndex)
{
    if (!mStreamers[0])
        return;

    mStreamers[0]->Stop();

    delete mStreamers[0];
    mStreamers[0] = nullptr;
}

void LimeSDR::StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats* rx, SDRDevice::StreamStats* tx)
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

void* LimeSDR::GetInternalChip(uint32_t index)
{
    return mLMSChips.at(index);
}

int LimeSDR::GPIODirRead(uint8_t* buffer, const size_t bufLength)
{
    return mfpgaPort->GPIODirRead(buffer, bufLength);
}

int LimeSDR::GPIORead(uint8_t* buffer, const size_t bufLength)
{
    return mfpgaPort->GPIORead(buffer, bufLength);
}

int LimeSDR::GPIODirWrite(const uint8_t* buffer, const size_t bufLength)
{
    return mfpgaPort->GPIODirWrite(buffer, bufLength);
}

int LimeSDR::GPIOWrite(const uint8_t* buffer, const size_t bufLength)
{
    return mfpgaPort->GPIOWrite(buffer, bufLength);
}

int LimeSDR::CustomParameterWrite(const std::vector<CustomParameterIO>& parameters)
{
    return mfpgaPort->CustomParameterWrite(parameters);
}

int LimeSDR::CustomParameterRead(std::vector<CustomParameterIO>& parameters)
{
    return mfpgaPort->CustomParameterRead(parameters);
}

int LimeSDR::ReadFPGARegister(uint32_t address)
{
    return mFPGA->ReadRegister(address);
}

int LimeSDR::WriteFPGARegister(uint32_t address, uint32_t value)
{
    return mFPGA->WriteRegister(address, value);
}
