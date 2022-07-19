#include "LimeSDR.h"

#include "USBGeneric.h"
#include "LMSBoards.h"
#include "LMS7002M.h"
#include "Si5351C.h"
#include "LMS64CProtocol.h"
#include "Logger.h"
#include "FPGA_common.h"
#include "TRXLooper_USB.h"

#include <assert.h>
#include <memory>
#include <set>
#include <stdexcept>
#include <cmath>

#ifdef __unix__
    #include "libusb.h"
#endif

using namespace lime;

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

const SDRDevice::Descriptor &LimeSDR::GetDescriptor() const
{
    static SDRDevice::Descriptor d;
    d.spiSlaveIds = {{"LMS7002M", spi_LMS7002M}, {"FPGA", spi_FPGA}};
    return d;
}

//control commands to be send via bulk port for boards v1.2 and later
static const std::set<uint8_t> commandsToBulkCtrlHw2 =
{
    CMD_BRDSPI_WR, CMD_BRDSPI_RD,
    CMD_LMS7002_WR, CMD_LMS7002_RD,
    CMD_ANALOG_VAL_WR, CMD_ANALOG_VAL_RD,
    CMD_ADF4002_WR,
    CMD_LMS7002_RST,
    CMD_GPIO_DIR_WR, CMD_GPIO_DIR_RD,
    CMD_GPIO_WR, CMD_GPIO_RD,
};

static inline void ValidateChannel(uint8_t channel)
{
    if (channel > 1)
        throw std::logic_error("invalid channel index");
}

LimeSDR::LimeSDR(lime::USBGeneric *conn)
    : comms(conn), mStreamer(nullptr), rxFIFO(nullptr), txFIFO(nullptr)
{
    const uint8_t spiSlaveId = 0;
    mLMSChip = new LMS7002M(spiSlaveId);
    mLMSChip->SetConnection(this);

    mFPGA = new FPGA(spi_FPGA, spi_LMS7002M);
    mFPGA->SetConnection(this);

    //must configure synthesizer before using LimeSDR
    //if (info.device == LMS_DEV_LIMESDR && info.hardware < 4)
    /*{
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
    if (mStreamer)
        delete mStreamer;
    delete mLMSChip;
    delete comms;
    delete mFPGA;
}

// Verify and configure given settings
// throw logic_error with description why the config is not possible
inline bool InRange(double val, double min, double max)
{
    return val >= min ? val <= max : false;
}

static inline const std::string strFormat(const char *format, ...)
{
    char ctemp[256];

    va_list args;
    va_start(args, format);
    snprintf(ctemp, 256, format, args);
    va_end(args);
    return std::string(ctemp);
}

void LimeSDR::Configure(const SDRConfig cfg)
{
    try {
        // only 2 channels is available on LimeSDR
        for (int i = 2; i < SDRDevice::MAX_CHANNEL_COUNT; ++i)
            if (cfg.channel[i].rxEnabled || cfg.channel[i].txEnabled)
                throw std::logic_error("too many channels enabled, LimeSDR has only 2");

        // MIMO necessary checks
        {
            const ChannelConfig &chA = cfg.channel[0];
            const ChannelConfig &chB = cfg.channel[1];
            const bool rxMIMO = chA.rxEnabled && chB.rxEnabled;
            const bool txMIMO = chA.txEnabled && chB.txEnabled;
            if (rxMIMO || txMIMO) {
                // MIMO sample rates have to match
                if (rxMIMO && chA.rxSampleRate != chB.rxSampleRate)
                    throw std::logic_error("Non matching Rx MIMO channels sampling rate");
                if (txMIMO && chA.txSampleRate != chB.txSampleRate)
                    throw std::logic_error("Non matching Tx MIMO channels sampling rate");

                // LMS7002M MIMO A&B channels share LO, but can be offset by NCO
                // TODO: check if they are withing NCO range
                const double rxLOdiff = chA.rxCenterFrequency - chB.rxCenterFrequency;
                if (rxMIMO && rxLOdiff > 0)
                    throw std::logic_error("MIMO: channels Rx LO too far apart");
                const double txLOdiff = chA.txCenterFrequency - chB.txCenterFrequency;
                if (txMIMO && txLOdiff > 0)
                    throw std::logic_error("MIMO: channels Rx LO too far apart");
            }
        }
        bool rxUsed = false;
        bool txUsed = false;
        // individual channel validation
        const double minLO = 30e6; // LO can be lowest 30e6, 100e3 could be achieved using NCO
        const double maxLO = 3.8e9;
        for (int i = 0; i < 2; ++i) {
            const ChannelConfig &ch = cfg.channel[i];
            rxUsed |= ch.rxEnabled;
            txUsed |= ch.txEnabled;
            if (ch.rxEnabled && not InRange(ch.rxCenterFrequency, minLO, maxLO))
                throw std::logic_error(strFormat("Rx ch%i LO (%g) out of range [%g:%g]", i,
                                                 ch.rxCenterFrequency, minLO, maxLO));
            if (ch.txEnabled && not InRange(ch.txCenterFrequency, minLO, maxLO))
                throw std::logic_error(strFormat("Tx ch%i LO (%g) out of range [%g:%g]", i,
                                                 ch.txCenterFrequency, minLO, maxLO));

            if (ch.rxEnabled && not InRange(ch.rxPath, 0, 3))
                throw std::logic_error(strFormat("Rx ch%i invalid path", i));
            if (ch.txEnabled && not InRange(ch.txPath, 0, 2))
                throw std::logic_error(strFormat("Tx ch%i invalid path", i));
        }

        // config validation complete, now do the actual configuration

        if (cfg.referenceClockFreq != 0)
            mLMSChip->SetClockFreq(LMS7002M::ClockID::CLK_REFERENCE, cfg.referenceClockFreq, 0);

        if (rxUsed)
            mLMSChip->SetFrequencySX(false, cfg.channel[0].rxCenterFrequency);
        if (txUsed)
            mLMSChip->SetFrequencySX(true, cfg.channel[0].rxCenterFrequency);

        for (int i = 0; i < 2; ++i) {
            const ChannelConfig &ch = cfg.channel[i];
            mLMSChip->SetActiveChannel((i & 1) ? LMS7002M::ChB : LMS7002M::ChA);
            mLMSChip->SetPathRFE(static_cast<LMS7002M::PathRFE>(ch.rxPath));
            mLMSChip->SetBandTRF(ch.txPath);
            mLMSChip->EnableChannel(false, i, ch.rxEnabled);
            mLMSChip->EnableChannel(true, i, ch.txEnabled);
            // TODO: set gains, filters...
        }
        mLMSChip->SetActiveChannel(LMS7002M::ChA);
        // sampling rate
        SetSampleRate(10e6, 4);
    } //try
    catch (std::logic_error &e) {
        printf("LimeSDR config: %s", e.what());
        throw;
    }
    catch (std::runtime_error &e) {
        throw;
    }
}

void LimeSDR::SetFPGAInterfaceFreq(uint8_t interp, uint8_t dec, double txPhase, double rxPhase)
{
    assert(mFPGA);
    double fpgaTxPLL = mLMSChip->GetReferenceClk_TSP(lime::LMS7002M::Tx);
    if (interp != 7) {
        uint8_t siso = mLMSChip->Get_SPI_Reg_bits(LMS7_LML1_SISODDR);
        fpgaTxPLL /= std::pow(2, interp + siso);
    }
    double fpgaRxPLL = mLMSChip->GetReferenceClk_TSP(lime::LMS7002M::Rx);
    if (dec != 7) {
        uint8_t siso = mLMSChip->Get_SPI_Reg_bits(LMS7_LML2_SISODDR);
        fpgaRxPLL /= std::pow(2, dec + siso);
    }

    if (std::fabs(rxPhase) > 360 || std::fabs(txPhase) > 360) {
        mFPGA->SetInterfaceFreq(fpgaTxPLL, fpgaRxPLL, 0);
        return;
    }
    else
        mFPGA->SetInterfaceFreq(fpgaTxPLL, fpgaRxPLL, txPhase, rxPhase, 0);
    mLMSChip->ResetLogicregisters();
}

void LimeSDR::SetSampleRate(double f_Hz, uint8_t oversample)
{
    const bool bypass = (oversample == 1) || (oversample == 0 && f_Hz > 62e6);
    uint8_t decimation = 7;     // HBD_OVR_RXTSP=7 - bypass
    uint8_t interpolation = 7;  // HBI_OVR_TXTSP=7 - bypass
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
    if (!bypass) {
        if (oversample == 0) {
            const int n = lime::LMS7002M::CGEN_MAX_FREQ / (f_Hz);
            oversample = (n >= 32) ? 32 : (n >= 16) ? 16 : (n >= 8) ? 8 : (n >= 4) ? 4 : 2;
        }

        decimation = 4;
        if (oversample <= 16) {
            const int decTbl[] = {0, 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3};
            decimation = decTbl[oversample];
        }
        interpolation = decimation;
        cgenFreq *= 2 << decimation;
    }
    lime::info("Sampling rate set(%.3f MHz): CGEN:%.3f MHz, Decim: 2^%i, Interp: 2^%i", f_Hz / 1e6,
               cgenFreq / 1e6, decimation, interpolation);

    mLMSChip->SetFrequencyCGEN(cgenFreq);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), decimation);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), interpolation);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    mLMSChip->SetInterfaceFrequency(mLMSChip->GetFrequencyCGEN(), interpolation, decimation);

    SetFPGAInterfaceFreq(interpolation, decimation, 999, 999); // TODO: default phase
}

int LimeSDR::Init()
{
    struct regVal
    {
        uint16_t adr;
        uint16_t val;
    };

    const std::vector<regVal> initVals = {
        {0x0022, 0x0FFF}, {0x0023, 0x5550}, {0x002B, 0x0038}, {0x002C, 0x0000}, {0x002D, 0x0641},
        {0x0086, 0x4101}, {0x0087, 0x5555}, {0x0088, 0x0525}, {0x0089, 0x1078}, {0x008B, 0x218C},
        {0x008C, 0x267B}, {0x00A6, 0x000F}, {0x00A9, 0x8000}, {0x00AC, 0x2000}, {0x0108, 0x218C},
        {0x0109, 0x57C1}, {0x010A, 0x154C}, {0x010B, 0x0001}, {0x010C, 0x8865}, {0x010D, 0x011A},
        {0x010E, 0x0000}, {0x010F, 0x3142}, {0x0110, 0x2B14}, {0x0111, 0x0000}, {0x0112, 0x000C},
        {0x0113, 0x03C2}, {0x0114, 0x01F0}, {0x0115, 0x000D}, {0x0118, 0x418C}, {0x0119, 0x5292},
        {0x011A, 0x3001}, {0x011C, 0x8941}, {0x011D, 0x0000}, {0x011E, 0x0984}, {0x0120, 0xE6C0},
        {0x0121, 0x3638}, {0x0122, 0x0514}, {0x0123, 0x200F}, {0x0200, 0x00E1}, {0x0208, 0x017B},
        {0x020B, 0x4000}, {0x020C, 0x8000}, {0x0400, 0x8081}, {0x0404, 0x0006}, {0x040B, 0x1020},
        {0x040C, 0x00FB}};

    lime::LMS7002M *lms = mLMSChip;
    // TODO: write GPIO to hard reset the chip
    if (lms->ResetChip() != 0)
        return -1;

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    for (auto i : initVals)
        lms->SPI_write(i.adr, i.val, true);

    // TODO:
    // if(lms->CalibrateTxGain(0,nullptr) != 0)
    //     return -1;

    EnableChannel(SDRDevice::Dir::Rx, 0, false);
    EnableChannel(SDRDevice::Dir::Tx, 0, false);
    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    for (auto i : initVals)
        if (i.adr >= 0x100)
            lms->SPI_write(i.adr, i.val, true);

    // if(lms->CalibrateTxGain(0,nullptr) != 0)
    //     return -1;

    EnableChannel(SDRDevice::Dir::Rx, 1, false);
    EnableChannel(SDRDevice::Dir::Tx, 1, false);

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);

    // if(lms->SetFrequency(SDRDevice::Dir::Tx, 0, lms->GetFrequency(SDRDevice::Dir::Tx, 0)) != 0)
    //     return -1;
    // if(lms->SetFrequency(SDRDevice::Dir::Rx, 0, lms->GetFrequency(SDRDevice::Dir::Rx, 0)) != 0)
    //     return -1;

    // if (SetRate(10e6,2)!=0)
    //     return -1;
    return 0;
}

SDRDevice::DeviceInfo LimeSDR::GetDeviceInfo(void)
{
    assert(comms);
    SDRDevice::DeviceInfo devInfo;
    try
    {
        LMS64CProtocol::LMS64CPacket pkt;
        pkt.cmd = CMD_GET_INFO;
        int sentBytes = comms->ControlTransfer(LIBUSB_REQUEST_TYPE_VENDOR, CTR_W_REQCODE, CTR_W_VALUE, CTR_W_INDEX, (uint8_t*)&pkt, sizeof(pkt), 1000);
        if (sentBytes != sizeof(pkt))
            throw std::runtime_error("LimeSDR::GetDeviceInfo write failed");
        int gotBytes = comms->ControlTransfer(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, CTR_R_REQCODE, CTR_R_VALUE, CTR_R_INDEX, (uint8_t*)&pkt, sizeof(pkt), 1000);
        if (gotBytes != sizeof(pkt))
            throw std::runtime_error("LimeSDR::GetDeviceInfo read failed");

        LMS64CProtocol::LMSinfo info;
        if (pkt.status == STATUS_COMPLETED_CMD && gotBytes >= pkt.headerSize)
        {
            info.firmware = pkt.payload[0];
            info.device = pkt.payload[1] < LMS_DEV_COUNT ? (eLMS_DEV)pkt.payload[1] : LMS_DEV_UNKNOWN;
            info.protocol = pkt.payload[2];
            info.hardware = pkt.payload[3];
            info.expansion = pkt.payload[4] < EXP_BOARD_COUNT ? (eEXP_BOARD)pkt.payload[4] : EXP_BOARD_UNKNOWN;
            info.boardSerialNumber = 0;
            for (int i = 10; i < 18; i++)
            {
                info.boardSerialNumber <<= 8;
                info.boardSerialNumber |= pkt.payload[i];
            }
        }
        else
            return devInfo;
        devInfo.deviceName = GetDeviceName(info.device);
        devInfo.expansionName = GetExpansionBoardName(info.expansion);
        devInfo.firmwareVersion = std::to_string(int(info.firmware));
        devInfo.hardwareVersion = std::to_string(int(info.hardware));
        devInfo.protocolVersion = std::to_string(int(info.protocol));
        devInfo.boardSerialNumber = info.boardSerialNumber;

        LMS64CProtocol::FPGAinfo gatewareInfo;
        const uint32_t addrs[] = {0x0000, 0x0001, 0x0002, 0x0003};
        uint32_t data[4];
        SPI(spi_FPGA, addrs, data, 4);
        gatewareInfo.boardID = (eLMS_DEV)data[0];//(pkt.inBuffer[2] << 8) | pkt.inBuffer[3];
        gatewareInfo.gatewareVersion = data[1];//(pkt.inBuffer[6] << 8) | pkt.inBuffer[7];
        gatewareInfo.gatewareRevision = data[2];//(pkt.inBuffer[10] << 8) | pkt.inBuffer[11];
        gatewareInfo.hwVersion = data[3] & 0x7F;//pkt.inBuffer[15]&0x7F;

        devInfo.gatewareTargetBoard = GetDeviceName(eLMS_DEV(gatewareInfo.boardID));
        devInfo.gatewareVersion = std::to_string(int(gatewareInfo.gatewareVersion));
        devInfo.gatewareRevision = std::to_string(int(gatewareInfo.gatewareRevision));
        devInfo.hardwareVersion = std::to_string(int(gatewareInfo.hwVersion));
        return devInfo;
    }
    catch (...)
    {
        //lime::error("LimeSDR::GetDeviceInfo failed(%s)", e.what());
        devInfo.deviceName = GetDeviceName(LMS_DEV_UNKNOWN);
        devInfo.expansionName = GetExpansionBoardName(EXP_BOARD_UNKNOWN);
    }
    return devInfo;
}

void LimeSDR::Reset()
{
    assert(comms);
    LMS64CProtocol::LMS64CPacket pkt;
    pkt.cmd = CMD_LMS7002_RST;
    pkt.status = STATUS_UNDEFINED;
    pkt.blockCount = 1;
    pkt.periphID = spi_LMS7002M;
    const int LMS_RST_PULSE = 2;
    pkt.payload[0] = LMS_RST_PULSE;

    int sent = comms->BulkTransfer(ctrlBulkOutAddr, (uint8_t *)&pkt, sizeof(pkt), 100);
    if (sent != sizeof(pkt))
        throw std::runtime_error("LMS Reset write failed");
    int recv = comms->BulkTransfer(ctrlBulkInAddr, (uint8_t *)&pkt, sizeof(pkt), 100);

    if (recv >= pkt.headerSize && pkt.status != STATUS_COMPLETED_CMD)
        throw std::runtime_error("LMS Reset read failed");
}

int LimeSDR::EnableChannel(SDRDevice::Dir dir, uint8_t channel, bool enabled)
{
    int ret = mLMSChip->EnableChannel(dir == SDRDevice::Dir::Tx ? LMS7002M::Tx : LMS7002M::Rx,
                                      channel, enabled);
    if (dir == SDRDevice::Dir::Tx) //always enable DAC1, otherwise sample rates <2.5MHz do not work
        mLMSChip->Modify_SPI_Reg_bits(LMS7_PD_TX_AFE1, 0);
    return ret;
}

double LimeSDR::GetRate(SDRDevice::Dir dir, uint8_t channel) const
{
    return mLMSChip->GetSampleRate(dir, channel ? LMS7002M::ChB : LMS7002M::ChA);
}

SDRDevice::Range LimeSDR::GetRateRange(SDRDevice::Dir dir, uint8_t channel) const
{
    return Range(100e3, 61.44e6);
}

std::vector<std::string> LimeSDR::GetPathNames(SDRDevice::Dir dir, uint8_t channel) const
{
    if (dir == SDRDevice::Dir::Tx)
        return {"NONE", "BAND1", "BAND2"};
    else
        return {"NONE", "LNAH", "LNAL", "LNAW", "LB1", "LB2"};
}

uint8_t LimeSDR::GetPath(SDRDevice::Dir dir, uint8_t channel) const
{
    ValidateChannel(channel);
    mLMSChip->SetActiveChannel(channel == 1 ? LMS7002M::ChB : LMS7002M::ChA);
    if (dir == SDRDevice::Dir::Tx)
        return mLMSChip->GetBandTRF();
    return mLMSChip->GetPathRFE();
}

double LimeSDR::GetClockFreq(uint8_t clk_id, uint8_t channel)
{
    return mLMSChip->GetClockFreq(static_cast<LMS7002M::ClockID>(clk_id), channel);
}

void LimeSDR::SetClockFreq(uint8_t clk_id, double freq, uint8_t channel)
{
    mLMSChip->SetClockFreq(static_cast<LMS7002M::ClockID>(clk_id), freq, channel);
}

SDRDevice::Range LimeSDR::GetFrequencyRange(SDRDevice::Dir dir) const
{
    return Range(100e3, 3.8e9);
}

void LimeSDR::Synchronize(bool toChip)
{
    if (toChip) {
        if (mLMSChip->UploadAll() == 0) {
            mLMSChip->Modify_SPI_Reg_bits(LMS7param(MAC), 1, true);
            //ret = SetFPGAInterfaceFreq(-1, -1, -1000, -1000); // TODO: implement
        }
    }
    else
        mLMSChip->DownloadAll();
}

void LimeSDR::EnableCache(bool enable)
{
    mLMSChip->EnableValuesCache(enable);
    if (mFPGA)
        mFPGA->EnableValuesCache(enable);
}

uint16_t LimeSDR::ReadParam(const LMS7Parameter &param, uint8_t channel,
                            bool forceReadFromChip) const
{
    if (channel >= 2)
        throw std::logic_error("LimeSDR::ReadParam - Invalid channel");

    if (param.address >= 0x100)
        mLMSChip->Modify_SPI_Reg_bits(LMS7param(MAC), channel + 1, forceReadFromChip);
    return mLMSChip->Get_SPI_Reg_bits(param, forceReadFromChip);
}

int LimeSDR::WriteParam(const LMS7Parameter &param, uint16_t val, uint8_t channel)
{
    const bool forceReadFromChip = true;
    if (channel >= 2)
        throw std::logic_error("LimeSDR::ReadParam - Invalid channel");

    if (param.address >= 0x100)
        mLMSChip->Modify_SPI_Reg_bits(LMS7param(MAC), channel + 1, forceReadFromChip);
    return mLMSChip->Modify_SPI_Reg_bits(param, val, forceReadFromChip);
}

double LimeSDR::GetTemperature(uint8_t id)
{
    // if (lms->ReadLMSReg(0x2F) == 0x3840)
    // {
    //     lime::error("Feature is not available on this chip revision");
    //     return -1;
    // }
    return mLMSChip->GetTemperature();
}

static void printPacket(const LMS64CProtocol::LMS64CPacket &pkt, uint8_t blockSize,
                        const char *prefix)
{
    printf("%s", prefix);
    int i = 0;
    for (; i < 8; ++i)
        printf("%02X ", ((uint8_t *)&pkt)[i]);
    for (; i < 8 + pkt.blockCount * blockSize; i += blockSize) {
        int j = 0;
        for (; j < blockSize / 2; ++j)
            printf("%02X", ((uint8_t *)&pkt)[i + j]);
        printf(" ");
        for (; j < blockSize; ++j)
            printf("%02X", ((uint8_t *)&pkt)[i + j]);
        printf(" ");
    }
    printf("\n");
}

void LimeSDR::SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, size_t count)
{
    assert(comms);
    assert(MOSI);
    LMS64CProtocol::LMS64CPacket pkt;
    pkt.status = STATUS_UNDEFINED;
    pkt.blockCount = 0;
    pkt.periphID = chipSelect;

    size_t srcIndex = 0;
    size_t destIndex = 0;
    const int maxBlocks = 14;
    while (srcIndex < count) {
        // fill packet with same direction operations
        const bool willDoWrite = MOSI[srcIndex] & (1 << 31);
        for (int i = 0; i < maxBlocks && srcIndex < count; ++i) {
            const bool isWrite = MOSI[srcIndex] & (1 << 31);
            if (isWrite != willDoWrite)
                break; // change between write/read, flush packet

            if (isWrite) {
                switch (chipSelect) {
                case spi_LMS7002M:
                    pkt.cmd = CMD_LMS7002_WR;
                    break;
                case spi_FPGA:
                    pkt.cmd = CMD_BRDSPI_WR;
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
            else {
                switch (chipSelect) {
                case spi_LMS7002M:
                    pkt.cmd = CMD_LMS7002_RD;
                    break;
                case spi_FPGA:
                    pkt.cmd = CMD_BRDSPI_RD;
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
        int sent = comms->BulkTransfer(ctrlBulkOutAddr, (uint8_t*)&pkt, sizeof(pkt), 100);
        if (sent != sizeof(pkt))
            throw std::runtime_error("SPI failed");

        int recv = comms->BulkTransfer(ctrlBulkInAddr, (uint8_t*)&pkt, sizeof(pkt), 100);
        //printPacket(pkt, 4, "Rd:");

        if (recv >= pkt.headerSize + 4 * pkt.blockCount && pkt.status == STATUS_COMPLETED_CMD) {
            for (int i = 0; MISO && i < pkt.blockCount && destIndex < count; ++i) {
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
        pkt.status = STATUS_UNDEFINED;
    }
}

int LimeSDR::I2CWrite(int address, const uint8_t *data, size_t length)
{
    assert(comms);
    LMS64CProtocol::LMS64CPacket pkt;
    int remainingBytes = length;
    const uint8_t* src = data;
    while (remainingBytes > 0)
    {
        pkt.cmd = CMD_I2C_WR;
        pkt.status = STATUS_UNDEFINED;
        pkt.blockCount = remainingBytes > pkt.maxDataLength ? pkt.maxDataLength : remainingBytes;
        pkt.periphID = address;
        memcpy(pkt.payload, src, pkt.blockCount);
        src += pkt.blockCount;
        remainingBytes -= pkt.blockCount;
        int sent = comms->BulkTransfer(ctrlBulkOutAddr, (uint8_t*)&pkt, sizeof(pkt), 100);
        if (sent != sizeof(pkt))
            throw std::runtime_error("I2C write failed");
        int recv = comms->BulkTransfer(ctrlBulkInAddr, (uint8_t*)&pkt, sizeof(pkt), 100);

        if (recv < pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
            throw std::runtime_error("I2C write failed");
    }
    return 0;
}

int LimeSDR::I2CRead(int address, uint8_t *data, size_t length)
{
    assert(comms);
    LMS64CProtocol::LMS64CPacket pkt;
    int remainingBytes = length;
    uint8_t* dest = data;
    while (remainingBytes > 0)
    {
        pkt.cmd = CMD_I2C_RD;
        pkt.status = STATUS_UNDEFINED;
        pkt.blockCount = remainingBytes > pkt.maxDataLength ? pkt.maxDataLength : remainingBytes;
        pkt.periphID = address;

        int sent = comms->BulkTransfer(ctrlBulkOutAddr, (uint8_t*)&pkt, sizeof(pkt), 100);
        if (sent != sizeof(pkt))
            throw std::runtime_error("I2C read failed");
        int recv = comms->BulkTransfer(ctrlBulkInAddr, (uint8_t*)&pkt, sizeof(pkt), 100);

        memcpy(dest, pkt.payload, pkt.blockCount);
        dest += pkt.blockCount;
        remainingBytes -= pkt.blockCount;

        if (recv <= pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
            throw std::runtime_error("I2C read failed");
    }
    return 0;
}

// There might be some leftover samples data still buffered in USB device
// clear the USB buffers before streaming samples to avoid old data
void LimeSDR::ResetUSBFIFO()
{
    LMS64CProtocol::LMS64CPacket pkt;
    pkt.cmd = CMD_USB_FIFO_RST;
    pkt.status = STATUS_UNDEFINED;
    pkt.blockCount = 1;
    pkt.payload[0] = 0;
    int sentBytes = comms->ControlTransfer(LIBUSB_REQUEST_TYPE_VENDOR, CTR_W_REQCODE, CTR_W_VALUE,
                                           CTR_W_INDEX, (uint8_t *)&pkt, sizeof(pkt), 100);
    if (sentBytes != sizeof(pkt))
        throw std::runtime_error("LimeSDR::ResetUSBFIFO write failed");
    int gotBytes =
        comms->ControlTransfer(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, CTR_R_REQCODE,
                               CTR_R_VALUE, CTR_R_INDEX, (uint8_t *)&pkt, sizeof(pkt), 100);
    if (gotBytes != sizeof(pkt))
        throw std::runtime_error("LimeSDR::ResetUSBFIFO read failed");
}

int LimeSDR::StreamStart(const StreamConfig &config)
{
    if (mStreamer)
        return -1; // already running
    try {
        ResetUSBFIFO();
        mStreamer = new TRXLooper_USB(comms, mFPGA, mLMSChip, streamBulkInAddr, streamBulkOutAddr);

        rxFIFO = new PacketsFIFO<FPGA_DataPacket>(512);
        txFIFO = new PacketsFIFO<FPGA_DataPacket>(512);
        mStreamer->AssignFIFO(rxFIFO, txFIFO);
        mStreamer->Start(config);
        mStreamConfig = config;
        return 0;
    }
    catch (std::logic_error &e) {
        return -1;
    }
    catch (std::runtime_error &e) {
        return -1;
    }
}
void LimeSDR::StreamStop()
{
    if (!mStreamer)
        return;

    mStreamer->Stop();
    if (txFIFO)
        delete txFIFO;
    txFIFO = nullptr;
    if (rxFIFO)
        delete rxFIFO;
    rxFIFO = nullptr;
    delete mStreamer;
    mStreamer = nullptr;
}

int LimeSDR::StreamRx(uint8_t channel, void **samples, uint32_t count, StreamMeta *meta)
{
    if (!rxFIFO)
        return 0;

    uint32_t samplesTotal = 0;
    const bool mimo = mStreamConfig.rxCount > 1;
    const bool compressed =
        mStreamConfig.linkFormat == SDRDevice::StreamConfig::DataFormat::FMT_INT12;
    lime::complex16_t *dest[2];
    dest[0] = static_cast<lime::complex16_t *>(samples[0]);
    if (mimo)
        dest[1] = static_cast<lime::complex16_t *>(samples[1]);
    const int mustFitSamples = mimo ? 510 : 1020;
    while (samplesTotal < count) {
        if (count - samplesTotal < mustFitSamples)
            break;
        FPGA_DataPacket *pkt = rxFIFO->pop();
        if (!pkt)
            return samplesTotal;
        int recv = FPGA::FPGAPacketPayload2Samples(pkt->data, sizeof(FPGA_DataPacket::data), mimo,
                                                   compressed, (lime::complex16_t **)dest);
        samplesTotal += recv;
        dest[0] = &static_cast<lime::complex16_t *>(samples[0])[samplesTotal];
        if (mimo)
            dest[1] = &static_cast<lime::complex16_t *>(samples[1])[samplesTotal];

        rxFIFO->release(pkt);
    }
    return samplesTotal;
}

int LimeSDR::StreamTx(uint8_t channel, const void **samples, uint32_t count, const StreamMeta *meta)
{
    if (!txFIFO)
        return 0;
    uint32_t samplesConsumed = 0;
    // while(samplesConsumed < count)
    // {
    //     FPGA_DataPacket* pkt = txFIFO.acquire();
    //     if(meta)
    //     {
    //         pkt.timestamp = meta.timestamp;
    //         pkt.useTimestamp = meta.useTimestamp
    //     }

    // }
    return samplesConsumed;
}

void LimeSDR::StreamStatus(uint8_t channel, SDRDevice::StreamStats &status)
{
    if (rxFIFO) {
        status.rxFIFO_filled = rxFIFO->Usage();
        status.rxDataRate_Bps = mStreamer->GetDataRate(false);
    }
    if (txFIFO) {
        status.txFIFO_filled = txFIFO->Usage();
        status.txDataRate_Bps = mStreamer->GetDataRate(true);
    }
}

void *LimeSDR::GetInternalChip(uint32_t index)
{
    return mLMSChip;
}
