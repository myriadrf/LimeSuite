#include "LimeSDR_XTRX.h"

#include <fcntl.h>
#include "math.h"

#include "Logger.h"
#include "LitePCIe.h"
#include "FPGA_common.h"
#include "TRXLooper_PCIE.h"
#include "FPGA_X3.h"
#include "LMS64CProtocol.h"
#include "DSP/Equalizer.h"

#include "mcu_program/common_src/lms7002m_calibrations.h"
#include "mcu_program/common_src/lms7002m_filters.h"
#include "MCU_BD.h"

#include "limesuite/LMS7002M.h"

namespace lime
{

class PCIE_CSR_Pipe : public ISerialPort
{
public:
    explicit PCIE_CSR_Pipe(LitePCIe& port) : port(port) {};
    virtual int Write(const uint8_t* data, size_t length, int timeout_ms) override
    {
        return port.WriteControl(data, length, timeout_ms);
    }
    virtual int Read(uint8_t* data, size_t length, int timeout_ms) override
    {
        return port.ReadControl(data, length, timeout_ms);
    }
protected:
    LitePCIe& port;
};

static constexpr uint8_t spi_LMS7002M = 0;
static constexpr uint8_t spi_FPGA = 1;

static inline void ValidateChannel(uint8_t channel)
{
    if (channel > 5)
        throw std::logic_error("invalid channel index");
}

// Do not perform any unnecessary configuring to device in constructor, so you
// could read back it's state for debugging purposes
LimeSDR_XTRX::LimeSDR_XTRX(lime::LitePCIe* control, lime::LitePCIe* stream)
    : LMS7002M_SDRDevice(), mControlPort(control), mStreamPort(stream)
{
    mFPGA = new lime::FPGA_X3(spi_FPGA, spi_LMS7002M);
    mFPGA->SetConnection(this);

    mLMSChips.push_back(new LMS7002M(spi_LMS7002M));
    for ( auto iter : mLMSChips)
    {
        iter->SetConnection(this);
        iter->SetReferenceClk_SX(false, 26e6);
        iter->SetClockFreq(LMS7002M::ClockID::CLK_REFERENCE, 26e6, 0);
    }

    const int chipCount = mLMSChips.size();
    mStreamers.resize(chipCount, nullptr);
}

LimeSDR_XTRX::~LimeSDR_XTRX()
{
}

inline bool InRange(double val, double min, double max)
{
    return val >= min ? val <= max : false;
}

static inline const std::string strFormat(const char *format, ...)
{
    char ctemp[256];

    va_list args;
    va_start(args, format);
    vsnprintf(ctemp, 256, format, args);
    va_end(args);
    return std::string(ctemp);
}

static int InitLMS1(LMS7002M* lms, bool skipTune = false)
{
    struct regVal
    {
        uint16_t adr;
        uint16_t val;
    };

    const std::vector<regVal> initVals = {
        {0x0022, 0x0FFF}, {0x0023, 0x5550}, {0x002B, 0x0038}, {0x002C, 0x0000},
        {0x002D, 0x0641}, {0x0086, 0x4101}, {0x0087, 0x5555}, {0x0088, 0x0525},
        {0x0089, 0x1078}, {0x008B, 0x218C}, {0x008C, 0x267B}, {0x00A6, 0x000F},
        {0x00A9, 0x8000}, {0x00AC, 0x2000}, {0x0108, 0x218C}, {0x0109, 0x57C1},
        {0x010A, 0x154C}, {0x010B, 0x0001}, {0x010C, 0x8865}, {0x010D, 0x011A},
        {0x010E, 0x0000}, {0x010F, 0x3142}, {0x0110, 0x2B14}, {0x0111, 0x0000},
        {0x0112, 0x000C}, {0x0113, 0x03C2}, {0x0114, 0x01F0}, {0x0115, 0x000D},
        {0x0118, 0x418C}, {0x0119, 0x5292}, {0x011A, 0x3001}, {0x011C, 0x8941},
        {0x011D, 0x0000}, {0x011E, 0x0984}, {0x0120, 0xE6C0}, {0x0121, 0x3638},
        {0x0122, 0x0514}, {0x0123, 0x200F}, {0x0200, 0x00E1}, {0x0208, 0x017B},
        {0x020B, 0x4000}, {0x020C, 0x8000}, {0x0400, 0x8081}, {0x0404, 0x0006},
        {0x040B, 0x1020}, {0x040C, 0x00FB},

        // LDOs
        {0x0092, 0x0D15}, {0x0093, 0x01B1}, {0x00A6, 0x000F},
        // XBUF
        {0x0085, 0x0019}
    };

    if (lms->ResetChip() != 0)
        return -1;

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    for (auto i : initVals)
        lms->SPI_write(i.adr, i.val, true);

    // if(lms->CalibrateTxGain(0,nullptr) != 0)
    //     return -1;

    // EnableChannel(true, 2*i, false);
    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    for (auto i : initVals)
        if (i.adr >= 0x100)
            lms->SPI_write(i.adr, i.val, true);

    // if(lms->CalibrateTxGain(0,nullptr) != 0)
    //     return -1;

    // EnableChannel(false, 2*i+1, false);
    // EnableChannel(true, 2*i+1, false);

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);

    if(skipTune)
        return 0;

    if(lms->SetFrequencySX(true, lms->GetFrequencySX(true))!=0)
        return -1;
    if(lms->SetFrequencySX(false, lms->GetFrequencySX(false))!=0)
        return -1;

    // if (SetRate(10e6,2)!=0)
    //     return -1;
    return 0;
}

void LimeSDR_XTRX::Configure(const SDRConfig cfg, uint8_t socIndex)
{
    try {
        // only 2 channels is available on LMS7002M
        for (int i = 2; i < SDRDevice::MAX_CHANNEL_COUNT; ++i)
            if (cfg.channel[i].rxEnabled || cfg.channel[i].txEnabled)
                throw std::logic_error("too many channels enabled, LMS7002M has only 2");

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

            if (ch.rxEnabled && not InRange(ch.rxPath, 0, 5))
                throw std::logic_error(strFormat("Rx ch%i invalid path", i));
            if(socIndex == 1)
            {
                if (ch.txEnabled && not InRange(ch.txPath, 0, 4))
                    throw std::logic_error(strFormat("Tx ch%i invalid path", i));
            }
            else
            {
                if (ch.txEnabled && not InRange(ch.txPath, 0, 2))
                    throw std::logic_error(strFormat("Tx ch%i invalid path", i));
            }
        }

        LMS7002M* chip = mLMSChips.at(socIndex);
        if (!cfg.skipDefaults)
        {
            const bool skipTune = true;
            InitLMS1(chip, skipTune);
        }

        if (cfg.referenceClockFreq != 0)
            chip->SetClockFreq(LMS7002M::ClockID::CLK_REFERENCE, cfg.referenceClockFreq, 0);

        const bool tddMode = cfg.channel[0].rxCenterFrequency == cfg.channel[0].txCenterFrequency;
        if (rxUsed)
            chip->SetFrequencySX(false, cfg.channel[0].rxCenterFrequency);
        if (txUsed)
            chip->SetFrequencySX(true, cfg.channel[0].txCenterFrequency);
        if(tddMode)
            chip->EnableSXTDD(true);

        for (int i = 0; i < 2; ++i) {
            const ChannelConfig &ch = cfg.channel[i];
            chip->SetActiveChannel((i & 1) ? LMS7002M::ChB : LMS7002M::ChA);
            
            chip->EnableChannel(Rx, i, ch.rxEnabled);
            chip->EnableChannel(Tx, i, ch.txEnabled);

            // enabled DAC is required for FPGA to work
            chip->Modify_SPI_Reg_bits(LMS7_PD_TX_AFE1, 0);
            chip->Modify_SPI_Reg_bits(LMS7_INSEL_RXTSP, ch.rxTestSignal ? 1 : 0);
            if(i == 0 && ch.rxTestSignal)
            {
                chip->Modify_SPI_Reg_bits(LMS7_TSGFC_RXTSP, 1);
                chip->Modify_SPI_Reg_bits(LMS7_TSGMODE_RXTSP, 1);
                chip->SPI_write(0x040C, 0x01FF); // DC.. bypasss
                // chip->LoadDC_REG_IQ(false, 0x1230, 0x4560); // gets reset by starting stream
            }
            else if (i==1 && ch.rxTestSignal)
            {
                chip->Modify_SPI_Reg_bits(LMS7_TSGFC_RXTSP, 1);
                chip->Modify_SPI_Reg_bits(LMS7_TSGMODE_RXTSP, 1);
                chip->SPI_write(0x040C, 0x01FF); // DC.. bypasss
                // chip->LoadDC_REG_IQ(false, 0x789A, 0xABC0); // gets reset by starting stream
            }
            chip->Modify_SPI_Reg_bits(LMS7_INSEL_TXTSP, ch.txTestSignal ? 1 : 0);
            // TODO: set gains, filters...
        }
        chip->SetActiveChannel(LMS7002M::ChA);

        double sampleRate;
        if (rxUsed)
            sampleRate = cfg.channel[0].rxSampleRate;
        else
            sampleRate = cfg.channel[0].txSampleRate;
        LMS1_SetSampleRate(sampleRate, cfg.channel[0].rxOversample, cfg.channel[0].txOversample);

        for (int i = 0; i < 2; ++i) {
            chip->SetActiveChannel(i==0 ? LMS7002M::ChA : LMS7002M::ChB);
            const ChannelConfig &ch = cfg.channel[i];

            if (socIndex == 0)
            {
                if (ch.rxEnabled && chip->SetGFIRFilter(false, i, ch.rxGFIR.enabled, ch.rxGFIR.bandwidth) != 0)
                    throw std::logic_error(strFormat("Rx ch%i GFIR config failed", i));
                if (ch.txEnabled && chip->SetGFIRFilter(true,  i, ch.txGFIR.enabled, ch.txGFIR.bandwidth) != 0)
                    throw std::logic_error(strFormat("Tx ch%i GFIR config failed", i));
            }

            if (ch.rxCalibrate && ch.rxEnabled)
            {
                SetupCalibrations(chip, ch.rxSampleRate);
                int status = CalibrateRx(false, false);
                if(status != MCU_BD::MCU_NO_ERROR)
                    throw std::runtime_error(strFormat("Rx ch%i DC/IQ calibration failed: %s", i, MCU_BD::MCUStatusMessage(status)));
            }
            if (ch.txCalibrate && ch.txEnabled)
            {
                SetupCalibrations(chip, ch.txSampleRate);
                int status = CalibrateTx(false);
                if(status != MCU_BD::MCU_NO_ERROR)
                    throw std::runtime_error(strFormat("Rx ch%i DC/IQ calibration failed: %s", i, MCU_BD::MCUStatusMessage(status)));
            }
            if (ch.rxLPF > 0 && ch.rxEnabled)
            {
                SetupCalibrations(chip, ch.rxSampleRate);
                int status = TuneRxFilter(ch.rxLPF);
                if(status != MCU_BD::MCU_NO_ERROR)
                    throw std::runtime_error(strFormat("Rx ch%i filter calibration failed: %s", i, MCU_BD::MCUStatusMessage(status)));
            }
            if (ch.txLPF > 0 && ch.txEnabled)
            {
                SetupCalibrations(chip, ch.txSampleRate);
                int status = TuneTxFilter(ch.txLPF);
                if(status != MCU_BD::MCU_NO_ERROR)
                    throw std::runtime_error(strFormat("Tx ch%i filter calibration failed: %s", i, MCU_BD::MCUStatusMessage(status)));
            }

            LMS1SetPath(false, i, ch.rxPath);
            LMS1SetPath(true, i, ch.txPath);
        }
        chip->SetActiveChannel(LMS7002M::ChA);

        // Workaround: Toggle LimeLights transmit port to flush residual value from data interface
        uint16_t txMux = chip->Get_SPI_Reg_bits(LMS7param(TX_MUX));
        chip->Modify_SPI_Reg_bits(LMS7param(TX_MUX), 2);
        chip->Modify_SPI_Reg_bits(LMS7param(TX_MUX), txMux);
    } //try
    catch (std::logic_error &e) {
        printf("LimeSDR_XTRX config: %s\n", e.what());
        throw;
    }
    catch (std::runtime_error &e) {
        throw;
    }
}

const SDRDevice::Descriptor &LimeSDR_XTRX::GetDescriptor() const
{
    static SDRDevice::Descriptor d;
    d.spiSlaveIds = {
        {"LMS7002M", spi_LMS7002M},
        {"FPGA", spi_FPGA}
    };

    d.memoryDevices = {
        {"FPGA RAM", (uint32_t)eMemoryDevice::FPGA_RAM},
        {"FPGA FLASH", (uint32_t)eMemoryDevice::FPGA_FLASH},
    };

    if (d.rfSOC.size() != 0) // fill only once
        return d;

    RFSOCDescriptor soc;
    // LMS#1
    soc.channelCount = 2;
    soc.rxPathNames = {"None", "LNAH", "LNAL", "LNAW"};
    soc.txPathNames = {"None", "Band1", "Band2"};
    d.rfSOC.push_back(soc);
    return d;
}

int LimeSDR_XTRX::Init()
{
    struct regVal
    {
        uint16_t adr;
        uint16_t val;
    };

    const std::vector<regVal> mFPGAInitVals = {
        {0x00D1, 0x3357}, // RF Switches
    };

    for (auto i : mFPGAInitVals)
        mFPGA->WriteRegister(i.adr, i.val);

    // uint8_t paramId = 2;
    // double dacVal = 65535;
    // CustomParameterWrite(&paramId,&dacVal,1,"");
    // paramId = 3;
    // CustomParameterWrite(&paramId,&dacVal,1,"");

    const bool skipTune = true;
    InitLMS1(mLMSChips.at(0), skipTune);
    return 0;
}
/*
SDRDevice::DeviceInfo LimeSDR_XTRX::GetDeviceInfo()
{
    assert(mControlPort);
    SDRDevice::DeviceInfo devInfo;
    try
    {
        LMS64CProtocol::LMS64CPacket pkt;
        pkt.cmd = CMD_GET_INFO;
        int sentBytes = mControlPort->WriteControl((uint8_t*)&pkt, sizeof(pkt), 1000);
        if (sentBytes != sizeof(pkt))
            throw std::runtime_error("LimeSDR::GetDeviceInfo write failed");
        int gotBytes = mControlPort->ReadControl((uint8_t*)&pkt, sizeof(pkt), 1000);
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
        gatewareInfo.boardID = (eLMS_DEV)data[0];
        gatewareInfo.gatewareVersion = data[1];
        gatewareInfo.gatewareRevision = data[2];
        gatewareInfo.hwVersion = data[3] & 0x7F;

        devInfo.gatewareTargetBoard = GetDeviceName(eLMS_DEV(gatewareInfo.boardID));
        devInfo.gatewareVersion = std::to_string(int(gatewareInfo.gatewareVersion));
        devInfo.gatewareRevision = std::to_string(int(gatewareInfo.gatewareRevision));
        devInfo.hardwareVersion = std::to_string(int(gatewareInfo.hwVersion));
        return devInfo;
    }
    catch (...)
    {
        devInfo.deviceName = GetDeviceName(LMS_DEV_UNKNOWN);
        devInfo.expansionName = GetExpansionBoardName(EXP_BOARD_UNKNOWN);
    }
    return devInfo;
}
*/
void LimeSDR_XTRX::Reset()
{
    // TODO:
    // for(auto iter : mLMSChips)
    //     iter->Reset();
}

double LimeSDR_XTRX::GetClockFreq(uint8_t clk_id, uint8_t channel)
{
    ValidateChannel(channel);
    LMS7002M* chip = mLMSChips[channel / 2];
    return chip->GetClockFreq(static_cast<LMS7002M::ClockID>(clk_id), channel&1);
}

void LimeSDR_XTRX::SetClockFreq(uint8_t clk_id, double freq, uint8_t channel)
{
    ValidateChannel(channel);
    LMS7002M* chip = mLMSChips[channel / 2];
    chip->SetClockFreq(static_cast<LMS7002M::ClockID>(clk_id), freq, channel&1);
}

void LimeSDR_XTRX::Synchronize(bool toChip)
{
    for (auto iter : mLMSChips)
    {
        if (toChip) {
            if (iter->UploadAll() == 0)
                iter->Modify_SPI_Reg_bits(LMS7param(MAC), 1, true);
            //ret = SetFPGAInterfaceFreq(-1, -1, -1000, -1000); // TODO: implement
        }
        else
            iter->DownloadAll();
    }
}

static void printPacket(const LMS64CPacket &pkt, uint8_t blockSize,
                        const char *prefix)
{
    return;
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

void LimeSDR_XTRX::SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, uint32_t count)
{
    PCIE_CSR_Pipe pipe(*mControlPort);
    switch (chipSelect) {
        case spi_LMS7002M:
            LMS64CProtocol::LMS7002M_SPI(pipe, MOSI, MISO, count);
            return;
        case spi_FPGA:
            LMS64CProtocol::FPGA_SPI(pipe, MOSI, MISO, count);
            return;
        default:
            throw std::logic_error("invalid SPI chip select");
    }
}

int LimeSDR_XTRX::I2CWrite(int address, const uint8_t *data, uint32_t length)
{
    PCIE_CSR_Pipe pipe(*mControlPort);
    return LMS64CProtocol::I2C_Write(pipe, address, data, length);
}

int LimeSDR_XTRX::I2CRead(int address, uint8_t *data, uint32_t length)
{
    PCIE_CSR_Pipe pipe(*mControlPort);
    return LMS64CProtocol::I2C_Read(pipe, address, data, length);
}

int LimeSDR_XTRX::StreamSetup(const StreamConfig &config, uint8_t moduleIndex)
{
    if (mStreamers.at(moduleIndex))
        return -1; // already running
    try {
        mStreamers.at(moduleIndex) = new TRXLooper_PCIE(
            mStreamPort,
            mStreamPort,
            mFPGA, mLMSChips.at(moduleIndex),
            moduleIndex
        );
        if (mCallback_logMessage)
            mStreamers[moduleIndex]->SetMessageLogCallback(mCallback_logMessage);
        LitePCIe* trxPort = mStreamPort;
        if(!trxPort->IsOpen())
        {
            int dirFlag = 0;
            if(config.rxCount > 0 && config.txCount > 0)
                dirFlag = O_RDWR;
            else if(config.rxCount > 0)
                dirFlag = O_RDONLY;
            else if(config.txCount > 0)
                dirFlag = O_WRONLY;
            if (trxPort->Open(trxPort->GetPathName().c_str(), dirFlag | O_NOCTTY | O_CLOEXEC | O_NONBLOCK) != 0)
            {
                char ctemp[128];
                sprintf(ctemp, "Failed to open device in stream start: %s", trxPort->GetPathName().c_str());
                throw std::runtime_error(ctemp);
            }
        }
        mStreamers[moduleIndex]->Setup(config);
        mStreamConfig = config;
        return 0;
    }
    catch (std::logic_error &e) {
        printf("LimeSDR_XTRX::StreamSetup logic_error %s\n", e.what());
        throw;
    }
    catch (std::runtime_error &e) {
        printf("LimeSDR_XTRX::StreamSetup runtime_error %s\n", e.what());
        throw;
    }
}

void LimeSDR_XTRX::StreamStart(uint8_t moduleIndex)
{
    mStreamers.at(moduleIndex)->Start();
}

void LimeSDR_XTRX::StreamStop(uint8_t moduleIndex)
{
    LMS7002M_SDRDevice::StreamStop(moduleIndex);
    LitePCIe* trxPort = mStreamPort;
    if (trxPort && trxPort->IsOpen())
        trxPort->Close();
}

void LimeSDR_XTRX::StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats &status)
{
    // TRXLooper *trx = mStreamers.at(moduleIndex);
    // status.dataRate_Bps = trx->GetDataRate(false);
    // status.txDataRate_Bps = trx->GetDataRate(true);
}

void LimeSDR_XTRX::SetFPGAInterfaceFreq(uint8_t interp, uint8_t dec, double txPhase, double rxPhase)
{
    assert(mFPGA);
    LMS7002M* mLMSChip = mLMSChips[0];
    double fpgaTxPLL = mLMSChip->GetReferenceClk_TSP(Tx);
    if (interp != 7) {
        uint8_t siso = mLMSChip->Get_SPI_Reg_bits(LMS7_LML1_SISODDR);
        fpgaTxPLL /= std::pow(2, interp + siso);
    }
    double fpgaRxPLL = mLMSChip->GetReferenceClk_TSP(Rx);
    if (dec != 7) {
        uint8_t siso = mLMSChip->Get_SPI_Reg_bits(LMS7_LML2_SISODDR);
        fpgaRxPLL /= std::pow(2, dec + siso);
    }

    if (std::fabs(rxPhase) > 360 || std::fabs(txPhase) > 360) {
        if(mFPGA->SetInterfaceFreq(fpgaTxPLL, fpgaRxPLL, 0) != 0)
            throw std::runtime_error("Failed to configure FPGA interface");
        return;
    }
    else
        if(mFPGA->SetInterfaceFreq(fpgaTxPLL, fpgaRxPLL, txPhase, rxPhase, 0) != 0)
            throw std::runtime_error("Failed to configure FPGA interface");
    mLMSChips[0]->ResetLogicregisters();
}

void LimeSDR_XTRX::LMS1_SetSampleRate(double f_Hz, uint8_t rxDecimation, uint8_t txInterpolation)
{
    if(txInterpolation/rxDecimation > 4)
        throw std::logic_error(strFormat("TxInterpolation(%i)/RxDecimation(%i) should not be more than 4", txInterpolation, rxDecimation));
    uint8_t oversample = rxDecimation;
    const bool bypass = (oversample == 1) || (oversample == 0 && f_Hz > 62e6);
    uint8_t hbd_ovr = 7;  // decimation ratio is 2^(1+hbd_ovr), HBD_OVR_RXTSP=7 - bypass
    uint8_t hbi_ovr = 7;  // interpolation ratio is 2^(1+hbi_ovr), HBI_OVR_TXTSP=7 - bypass
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
            const int n = lime::LMS7002M::CGEN_MAX_FREQ / (cgenFreq);
            oversample = (n >= 32) ? 32 : (n >= 16) ? 16 : (n >= 8) ? 8 : (n >= 4) ? 4 : 2;
        }

        hbd_ovr = 4;
        if (oversample <= 16) {
            const int decTbl[] = {0, 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3};
            hbd_ovr = decTbl[oversample];
        }
        cgenFreq *= 2 << hbd_ovr;
        if (txInterpolation >= rxDecimation)
            hbi_ovr = hbd_ovr + std::log2(txInterpolation/rxDecimation);
        else
            throw std::logic_error(strFormat("Rx decimation(2^%i) > Tx interpolation(2^%i) currently not supported", hbd_ovr, hbi_ovr));
    }
    lime::info("Sampling rate set(%.3f MHz): CGEN:%.3f MHz, Decim: 2^%i, Interp: 2^%i", f_Hz / 1e6,
               cgenFreq / 1e6, 1+hbd_ovr, 1+hbi_ovr);
    LMS7002M* mLMSChip = mLMSChips[0];
    mLMSChip->SetFrequencyCGEN(cgenFreq);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2 - std::log2(txInterpolation/rxDecimation));
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), hbd_ovr);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), hbi_ovr);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP), hbd_ovr);
    mLMSChip->Modify_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP), hbi_ovr);
    mLMSChip->SetInterfaceFrequency(cgenFreq, hbi_ovr, hbd_ovr);

    SetFPGAInterfaceFreq(hbi_ovr, hbd_ovr, 999, 999); // TODO: default phase
}

enum // TODO: replace
{
    LMS_PATH_NONE = 0, ///<No active path (RX or TX)
    LMS_PATH_LNAH = 1, ///<RX LNA_H port
    LMS_PATH_LNAL = 2, ///<RX LNA_L port
    LMS_PATH_LNAW = 3, ///<RX LNA_W port
    LMS_PATH_TX1 = 1,  ///<TX port 1
    LMS_PATH_TX2 = 2,   ///<TX port 2
    LMS_PATH_AUTO = 255, ///<Automatically select port (if supported)
};

void LimeSDR_XTRX::LMS1SetPath(bool tx, uint8_t chan, uint8_t pathId)
{
    // RF switch controls are toggled for both channels, use channel 0 as the deciding source.
    if(chan != 0)
        return;

    uint16_t sw_addr = 0x000A;
    uint16_t sw_val = mFPGA->ReadRegister(sw_addr);
    lime::LMS7002M* lms = mLMSChips.at(0);

    if(tx)
    {
        uint8_t path;
        switch(ePathLMS1_Tx(pathId))
        {
            case ePathLMS1_Tx::NONE : path = LMS_PATH_NONE; break;
            case ePathLMS1_Tx::BAND1 : path = LMS_PATH_TX1; break;
            case ePathLMS1_Tx::BAND2 : path = LMS_PATH_TX2; break;
            default: throw std::logic_error("Invalid LMS1 Tx path");
        }
        sw_val &= ~(1 << 4);
        if (path == LMS_PATH_TX1)
            sw_val |= 1 << 4;
        else if (path == LMS_PATH_TX2)
            sw_val &= ~(1 << 4);

        mFPGA->WriteRegister(sw_addr, sw_val);
        lms->SetBandTRF(path);
    }
    else
    {
        uint8_t path;
        switch(ePathLMS1_Rx(pathId))
        {
            case ePathLMS1_Rx::NONE : path = LMS7002M::PATH_RFE_NONE; break;
            case ePathLMS1_Rx::LNAH : path = LMS7002M::PATH_RFE_LNAH; break;
            case ePathLMS1_Rx::LNAL : path = LMS7002M::PATH_RFE_LNAL; break;
            //case ePathLMS1_Rx::LNAW : path = LMS7002M::PATH_RFE_LNAW; break;
            default: throw std::logic_error("Invalid LMS1 Rx path");
        }

        sw_val &= ~(0x3 << 2);
        if(path == LMS_PATH_LNAW)
            sw_val &= ~(0x3 << 2);
        else if (path == LMS_PATH_LNAH)
            sw_val |= 2 << 2;
        else if(path == LMS_PATH_LNAL)
            sw_val |= 1 << 2;
        mFPGA->WriteRegister(sw_addr, sw_val);
        lms->SetPathRFE(lime::LMS7002M::PathRFE(path));
    }
}

int LimeSDR_XTRX::CustomParameterWrite(const int32_t *ids, const double *values, const size_t count, const std::string& units)
{
    PCIE_CSR_Pipe pipe(*mControlPort);
    return LMS64CProtocol::CustomParameterWrite(pipe, ids, values, count, units);
}

int LimeSDR_XTRX::CustomParameterRead(const int32_t *ids, double *values, const size_t count, std::string* units)
{
    PCIE_CSR_Pipe pipe(*mControlPort);
    return LMS64CProtocol::CustomParameterRead(pipe, ids, values, count, units);
}

bool LimeSDR_XTRX::UploadMemory(uint32_t id, const char* data, size_t length, UploadMemoryCallback callback)
{
    PCIE_CSR_Pipe pipe(*mControlPort);
    int progMode;
    LMS64CProtocol::ProgramWriteTarget target;
    target = LMS64CProtocol::ProgramWriteTarget::FPGA;
    if (id == (int)eMemoryDevice::FPGA_RAM)
        progMode = 0;
    if (id == (int)eMemoryDevice::FPGA_FLASH)
        progMode = 1;
    else
        return false;
    return LMS64CProtocol::ProgramWrite(pipe, data, length, progMode, target, callback);
}

} //namespace lime
