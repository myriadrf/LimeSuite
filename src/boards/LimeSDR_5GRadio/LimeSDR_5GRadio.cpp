#include "LimeSDR_5GRadio.h"

#include <fcntl.h>

#include "Logger.h"
#include "LitePCIe.h"
#include "LMS7002M.h"
#include "FPGA_common.h"
#include "TRXLooper_PCIE.h"
#include "FPGA_5G.h"
#include "LMS64CProtocol.h"
#include "DSP/Equalizer.h"

#include "mcu_program/common_src/lms7002m_calibrations.h"
#include "mcu_program/common_src/lms7002m_filters.h"
#include "MCU_BD.h"

#include "math.h"

namespace lime
{

static constexpr uint8_t spi_LMS7002M_1 = 0;
static constexpr uint8_t spi_LMS7002M_2 = 1;
static constexpr uint8_t spi_LMS7002M_3 = 2;
static constexpr uint8_t spi_FPGA = 3;

static inline void ValidateChannel(uint8_t channel)
{
    if (channel > 5)
        throw std::logic_error("invalid channel index");
}

// Do not perform any unnecessary configuring to device in constructor, so you
// could read back it's state for debugging purposes
LimeSDR_5GRadio::LimeSDR_5GRadio(lime::LitePCIe* control,
    std::vector<lime::LitePCIe*> rxStreams, std::vector<lime::LitePCIe*> txStreams)
    : mControlPort(control), mRXStreamPorts(rxStreams), mTXStreamPorts(txStreams)
{
    mFPGA = new lime::FPGA_5G(spi_FPGA, spi_LMS7002M_1);
    mFPGA->SetConnection(this);

    mEqualizer = new Equalizer(this, spi_FPGA);

    cdcm[0] = new CDCM_Dev(mFPGA, CDCM2_BASE_ADDR);
    // TODO: read back cdcm values or cdcm[0]->Reset(30.72e6, 25e6);
    // cdcm[1] = new CDCM_Dev(mFPGA, CDCM2_BASE_ADDR);

    mLMSChips.push_back(new LMS7002M(spi_LMS7002M_1));
    mLMSChips.push_back(new LMS7002M(spi_LMS7002M_2));
    mLMSChips.push_back(new LMS7002M(spi_LMS7002M_3));
    for ( auto iter : mLMSChips)
    {
        iter->SetConnection(this);
        //iter->SetReferenceClk_SX(false, 30.72e6);
    }

    const int chipCount = mLMSChips.size();
    mStreamers.resize(chipCount, nullptr);
}

LimeSDR_5GRadio::~LimeSDR_5GRadio()
{
    delete cdcm[0];
    delete mEqualizer;
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

// Setup default register values specifically for onboard LMS1 chip
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
        {0x040B, 0x1020}, {0x040C, 0x00FB}
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

static void EnableChannelLMS2(LMS7002M* chip, Dir dir, const uint8_t channel, const bool enable)
{
    //ChannelScope scope(this, channel);

    auto macBck = chip->GetActiveChannel();
    const LMS7002M::Channel ch = channel > 0 ? LMS7002M::ChB : LMS7002M::ChA;
    chip->SetActiveChannel(ch);

    const bool isTx = dir == Tx;
    //--- LML ---
    if (ch == LMS7002M::ChA)
    {
        if (isTx) chip->Modify_SPI_Reg_bits(LMS7param(TXEN_A), enable?1:0);
        else      chip->Modify_SPI_Reg_bits(LMS7param(RXEN_A), enable?1:0);
    }
    else
    {
        if (isTx) chip->Modify_SPI_Reg_bits(LMS7param(TXEN_B), enable?1:0);
        else      chip->Modify_SPI_Reg_bits(LMS7param(RXEN_B), enable?1:0);
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
        chip->Modify_SPI_Reg_bits(LMS7param(EN_G_TBB), enable?1:0);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_LPFIAMP_TBB), enable?0:1);
        chip->Modify_SPI_Reg_bits(LMS7param(TSTIN_TBB), 3); // switch to external DAC
    }
    else
    {
        chip->Modify_SPI_Reg_bits(LMS7param(EN_DIR_RBB), 1);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_G_RBB), enable?1:0);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_PGA_RBB), enable?0:1);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), enable?0:1);
        chip->Modify_SPI_Reg_bits(LMS7param(OSW_PGA_RBB), 1); // switch external ADC
    }

    //--- frontend ---
    if (isTx)
    {
        chip->Modify_SPI_Reg_bits(LMS7param(EN_DIR_TRF), 1);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_G_TRF), enable?1:0);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_TLOBUF_TRF), enable?0:1);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_TXPAD_TRF), enable?0:1);
    }
    else
    {
        chip->Modify_SPI_Reg_bits(LMS7param(EN_DIR_RFE), 1);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_G_RFE), enable?1:0);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_MXLOBUF_RFE), enable?0:1);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_QGEN_RFE), enable?0:1);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_TIA_RFE), enable?0:1);
        chip->Modify_SPI_Reg_bits(LMS7param(PD_LNA_RFE), enable?0:1);
    }

    //--- synthesizers ---
    if (isTx)
    {
        chip->SetActiveChannel(LMS7002M::ChSXT);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_DIR_SXRSXT), 1);
        //chip->Modify_SPI_Reg_bits(LMS7param(EN_G), (disabledChannels&3) == 3?0:1);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_G), 1);
        if (ch == LMS7002M::ChB) //enable LO to channel B
        {
            chip->SetActiveChannel(LMS7002M::ChA);
            chip->Modify_SPI_Reg_bits(LMS7param(EN_NEXTTX_TRF), enable?1:0);
        }
    }
    else
    {
        chip->SetActiveChannel(LMS7002M::ChSXR);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_DIR_SXRSXT), 1);
        //chip->Modify_SPI_Reg_bits(LMS7param(EN_G), (disabledChannels&0xC)==0xC?0:1);
        chip->Modify_SPI_Reg_bits(LMS7param(EN_G), 1);
        if (ch == LMS7002M::ChB) //enable LO to channel B
        {
            chip->SetActiveChannel(LMS7002M::ChA);
            chip->Modify_SPI_Reg_bits(LMS7param(EN_NEXTRX_RFE), enable?1:0);
        }
    }
    chip->SetActiveChannel(macBck);
}

// Setup default register values specifically for onboard LMS2 chip
static int InitLMS2(LMS7002M* lms, bool skipTune = false)
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
        {0x010A, 0xD54C}, {0x010B, 0x0001}, {0x010C, 0x8865}, {0x010D, 0x011A},
        {0x010E, 0x0000}, {0x010F, 0x3142}, {0x0110, 0x2B14}, {0x0111, 0x0000},
        {0x0112, 0x000C}, {0x0113, 0x03C2}, {0x0114, 0x01F0}, {0x0115, 0x000D},
        {0x0118, 0x418C}, {0x0119, 0xD292}, {0x011A, 0x3001}, {0x011C, 0x8941},
        {0x011D, 0x0000}, {0x011E, 0x0984}, {0x0120, 0xE6C0}, {0x0121, 0x3638},
        {0x0122, 0x0514}, {0x0123, 0x200F}, {0x0200, 0x00E1}, {0x0208, 0x017B},
        {0x020B, 0x4000}, {0x020C, 0x8000}, {0x0400, 0x8081}, {0x0404, 0x0006},
        {0x040B, 0x1020}, {0x040C, 0x00FB}
    };

    if (lms->ResetChip() != 0)
        return -1;

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 3);
    for (auto i : initVals)
        lms->SPI_write(i.adr, i.val, true);

    //lms->SPI_write(0x0082, 0x803E); // Power down AFE ADCs/DACs

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);

    // if(lms->CalibrateTxGain(0,nullptr) != 0)
    //     return -1;

    // EnableChannel(false, 2*i+1, false);
    // EnableChannel(true, 2*i+1, false);

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

// TODO: Setup default register values specifically for onboard LMS3 chip
static int InitLMS3(LMS7002M* lms, bool skipTune)
{
    return -1;
}

void LimeSDR_5GRadio::Configure(const SDRConfig cfg, uint8_t socIndex)
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

        // config validation complete, now do the actual configuration
        switch(socIndex)
        {
            case 0:
                // Turn off PAs before configuring chip to avoid unexpectedly strong signal input
                LMS1_PA_Enable(0, false);
                LMS1_PA_Enable(1, false);
                break;
            case 1:
                LMS2_PA_LNA_Enable(0, false, false);
                LMS2_PA_LNA_Enable(1, false, false);
                break;
        }

        LMS7002M* chip = mLMSChips.at(socIndex);
        if (!cfg.skipDefaults)
        {
            const bool skipTune = true;
            switch(socIndex)
            {
                case 0: InitLMS1(chip, skipTune); break;
                case 1: InitLMS2(chip, skipTune); break;
                case 2: InitLMS3(chip, skipTune); break;
            }
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
            if ( socIndex == 1 ) // LMS2 uses external ADC/DAC
            {
                EnableChannelLMS2(chip, Rx, i, ch.rxEnabled);
                EnableChannelLMS2(chip, Tx, i, ch.txEnabled);
            }
            else
            {
                chip->EnableChannel(Rx, i, ch.rxEnabled);
                chip->EnableChannel(Tx, i, ch.txEnabled);
            }
            if(socIndex == 0)
                LMS1SetPath(false, i, ch.rxPath);
            else if(socIndex == 1)
            {
                uint8_t path = ch.rxEnabled ? ch.rxPath : uint8_t(ePathLMS2_Rx::NONE);
                LMS2SetPath(false, i, path);
            }

            // if(ch.rxPath == 4)
            //     mLMSChips[0]->Modify_SPI_Reg_bits(LMS7_INPUT_CTL_PGA_RBB, 3); // baseband loopback

            if(socIndex == 0)
                LMS1SetPath(true, i, ch.txPath);
            else if(socIndex == 1)
            {
                uint8_t path = ch.txEnabled ? ch.txPath : uint8_t(ePathLMS2_Tx::NONE);
                LMS2SetPath(true, i, path);
            }

            if(socIndex == 0) {
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
            }
            // TODO: set gains, filters...
        }
        chip->SetActiveChannel(LMS7002M::ChA);

        double sampleRate;
        if (rxUsed)
        {
            sampleRate = cfg.channel[0].rxSampleRate;
        }
        else
        {
            sampleRate = cfg.channel[0].txSampleRate;
        }
        if(socIndex == 0) {
            LMS1_SetSampleRate(sampleRate, cfg.channel[0].rxOversample, cfg.channel[0].txOversample);
        }
        else if(socIndex == 1) {
            Equalizer::Config eqCfg;
            for(int i=0; i<2; ++i)
            {
                eqCfg.bypassRxEqualizer[i] = true;
                eqCfg.bypassTxEqualizer[i] = true;
                eqCfg.cfr[i].bypass = true;
                eqCfg.cfr[i].sleep = true;
                eqCfg.cfr[i].bypassGain = true;
                eqCfg.cfr[i].interpolation = cfg.channel[0].txOversample;
                eqCfg.fir[i].sleep = true;
                eqCfg.fir[i].bypass = true;
            }
            mEqualizer->Configure(eqCfg);
            LMS2_SetSampleRate(sampleRate, cfg.channel[0].txOversample);
        }

        for (int i = 0; i < 2; ++i) {
            chip->SetActiveChannel(i==0 ? LMS7002M::ChA : LMS7002M::ChB);
            const ChannelConfig &ch = cfg.channel[i];
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
        }
        chip->SetActiveChannel(LMS7002M::ChA);

        // Workaround: Toggle LimeLights transmit port to flush residual value from data interface
        uint16_t txMux = chip->Get_SPI_Reg_bits(LMS7param(TX_MUX));
        chip->Modify_SPI_Reg_bits(LMS7param(TX_MUX), 2);
        chip->Modify_SPI_Reg_bits(LMS7param(TX_MUX), txMux);

        // Turn on needed PAs
        for (int c = 0; c < 2; ++c) {
            const ChannelConfig &ch = cfg.channel[c];
            switch(socIndex)
            {
                case 0:
                    LMS1_PA_Enable(c, ch.txEnabled);
                    break;
                case 1:
                    LMS2_PA_LNA_Enable(c, ch.txEnabled, ch.rxEnabled);
                    break;
            }
        }
    } //try
    catch (std::logic_error &e) {
        printf("LimeSDR_5GRadio config: %s\n", e.what());
        throw;
    }
    catch (std::runtime_error &e) {
        throw;
    }
}

const SDRDevice::Descriptor &LimeSDR_5GRadio::GetDescriptor() const
{
    static SDRDevice::Descriptor d;
    d.spiSlaveIds = {
        {"LMS7002M_1", spi_LMS7002M_1},
        {"LMS7002M_2", spi_LMS7002M_2},
        {"LMS7002M_3", spi_LMS7002M_3},
        {"FPGA", spi_FPGA}
    };

    if (d.rfSOC.size() != 0) // fill only once
        return d;

    RFSOCDescripion soc;
    // LMS#1
    soc.channelCount = 2;
    soc.rxPathNames = {"None", "LNAH", "LNAL"};
    soc.txPathNames = {"None", "Band1", "Band2"};
    d.rfSOC.push_back(soc);

    // LMS#2
    soc.rxPathNames = {"None", "TDD", "FDD", "Calibration (LMS3)"};
    soc.txPathNames = {"None", "TDD", "FDD"};
    d.rfSOC.push_back(soc);

    // LMS#3
    soc.rxPathNames = {"None", "LNAH", "Calibration (LMS2)"};
    soc.txPathNames = {"None", "Band1"};
    d.rfSOC.push_back(soc);

    return d;
}

int LimeSDR_5GRadio::Init()
{
    struct regVal
    {
        uint16_t adr;
        uint16_t val;
    };

    const std::vector<regVal> mFPGAInitVals = {
        {0x00D1, 0x3357}, // RF Switches
        //{0x00D2, 0x003C} // PA controls
    };
    LMS1_PA_Enable(0, false);
    LMS1_PA_Enable(1, false);
    LMS2_PA_LNA_Enable(0, false, false);
    LMS2_PA_LNA_Enable(1, false, false);

    for (auto i : mFPGAInitVals)
        mFPGA->WriteRegister(i.adr, i.val);

    uint8_t paramId = 2;
    double dacVal = 65535;
    CustomParameterWrite(&paramId,&dacVal,1,"");
    paramId = 3;
    CustomParameterWrite(&paramId,&dacVal,1,"");

    // TODO:
    cdcm[0]->Reset(30.72e6, 25e6);
    const bool skipTune = true;
    InitLMS1(mLMSChips.at(0), skipTune);
    InitLMS2(mLMSChips.at(1), skipTune);
    InitLMS3(mLMSChips.at(2), skipTune);
    return 0;
}

SDRDevice::DeviceInfo LimeSDR_5GRadio::GetDeviceInfo()
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

void LimeSDR_5GRadio::Reset()
{
    // TODO:
    // for(auto iter : mLMSChips)
    //     iter->Reset();
}

/*
int LimeSDR_5GRadio::EnableChannel(SDRDevice::Dir dir, uint8_t channel, bool enabled)
{
    ValidateChannel(channel);
    LMS7002M* chip = mLMSChips[channel / 2];
    int ret = chip->EnableChannel(dir == SDRDevice::Dir::Tx ? LMS7002M::Tx : LMS7002M::Rx,
                                      channel & 1, enabled);
    // if (dir == SDRDevice::Dir::Tx) //always enable DAC1, otherwise sample rates <2.5MHz do not work
    //     chip->Modify_SPI_Reg_bits(LMS7_PD_TX_AFE1, 0);
    return ret;
}
*/

/*double LimeSDR_5GRadio::GetRate(Dir dir, uint8_t channel) const
{
    if(channel >= 2)
        throw std::logic_error("Invalid channel");
    LMS7002M* chip = mLMSChips[channel / 2];
    return chip->GetSampleRate(dir, (channel&1) ? LMS7002M::ChB : LMS7002M::ChA);
}

std::vector<std::string> LimeSDR_5GRadio::GetPathNames(SDRDevice::Dir dir, uint8_t channel) const
{
    if(channel == 0 || channel == 1) // LMS1
    {
        if (dir == SDRDevice::Dir::Tx)
            return {"NONE", "BAND1", "BAND2"};
        else
            return {"NONE", "LNAH", "LNAL", "LNAW_NC"};
    }

    if(channel == 2 || channel == 3) // LMS2
    {
        return {"NONE", "TDD_TX", "TDD_RX", "FDD", "Cal"};
    }

    if(channel == 4 || channel == 5) // LMS3
    {
        if (dir == SDRDevice::Dir::Tx)
            return {"NONE"};
        else
            return {"NONE", "LNAH", "Cal"};
    }
    return {};
}

uint8_t LimeSDR_5GRadio::GetPath(SDRDevice::Dir dir, uint8_t channel) const
{
    ValidateChannel(channel);
    LMS7002M* chip = mLMSChips[channel / 2];
    chip->SetActiveChannel((channel & 1) ? LMS7002M::ChB : LMS7002M::ChA);
    if (dir == SDRDevice::Dir::Tx)
        return chip->GetBandTRF();
    return chip->GetPathRFE();
}*/

double LimeSDR_5GRadio::GetClockFreq(uint8_t clk_id, uint8_t channel)
{
    ValidateChannel(channel);
    LMS7002M* chip = mLMSChips[channel / 2];
    return chip->GetClockFreq(static_cast<LMS7002M::ClockID>(clk_id), channel&1);
}

void LimeSDR_5GRadio::SetClockFreq(uint8_t clk_id, double freq, uint8_t channel)
{
    ValidateChannel(channel);
    LMS7002M* chip = mLMSChips[channel / 2];
    chip->SetClockFreq(static_cast<LMS7002M::ClockID>(clk_id), freq, channel&1);
}

void LimeSDR_5GRadio::Synchronize(bool toChip)
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

void LimeSDR_5GRadio::SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, uint32_t count)
{
    assert(mControlPort);
    assert(MOSI);
    LMS64CProtocol::LMS64CPacket pkt;
    pkt.status = STATUS_UNDEFINED;
    pkt.blockCount = 0;
    pkt.periphID = chipSelect;

    size_t srcIndex = 0;
    size_t destIndex = 0;
    const int maxBlocks = 14;
    int wrPrint = 4;
    int rdPrint = 4;
    while (srcIndex < count) {
        // fill packet with same direction operations
        const bool willDoWrite = MOSI[srcIndex] & (1 << 31);
        for (int i = 0; i < maxBlocks && srcIndex < count; ++i) {
            const bool isWrite = MOSI[srcIndex] & (1 << 31);
            if (isWrite != willDoWrite)
                break; // change between write/read, flush packet

            if (isWrite) {
                switch (chipSelect) {
                case spi_LMS7002M_1:
                case spi_LMS7002M_2:
                case spi_LMS7002M_3:
                    pkt.cmd = CMD_LMS7002_WR;
                    rdPrint = 0;
                    break;
                case spi_FPGA:
                    pkt.cmd = CMD_BRDSPI_WR;
                    rdPrint = 0;
                    break;
                default:
                    throw std::logic_error("LimeSDR_5GRadio SPI invalid SPI chip select");
                }
                int payloadOffset = pkt.blockCount * 4;
                pkt.payload[payloadOffset + 0] = MOSI[srcIndex] >> 24;
                pkt.payload[payloadOffset + 1] = MOSI[srcIndex] >> 16;
                pkt.payload[payloadOffset + 2] = MOSI[srcIndex] >> 8;
                pkt.payload[payloadOffset + 3] = MOSI[srcIndex];
            }
            else {
                switch (chipSelect) {
                case spi_LMS7002M_1:
                case spi_LMS7002M_2:
                case spi_LMS7002M_3:
                    pkt.cmd = CMD_LMS7002_RD;
                    wrPrint = 2;
                    rdPrint = 4;
                    break;
                case spi_FPGA:
                    pkt.cmd = CMD_BRDSPI_RD;
                    wrPrint = 2;
                    rdPrint = 4;
                    break;
                default:
                    throw std::logic_error("LimeSDR_5GRadio SPI invalid SPI chip select");
                }
                int payloadOffset = pkt.blockCount * 2;
                pkt.payload[payloadOffset + 0] = MOSI[srcIndex] >> 8;
                pkt.payload[payloadOffset + 1] = MOSI[srcIndex];
            }
            ++pkt.blockCount;
            ++srcIndex;
        }

        // flush packet
        //printPacket(pkt, wrPrint, "Wr:");
        int sent = 0;
        int recv = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        {
        std::lock_guard<std::mutex> lock(mCommsMutex);
        sent = mControlPort->WriteControl((uint8_t*)&pkt, sizeof(pkt), 100);
        if (mCallback_logData)
            mCallback_logData(true, (uint8_t*)&pkt, sizeof(pkt));
        if (sent != sizeof(pkt))
            throw std::runtime_error("SPI failed");
        t1 = std::chrono::high_resolution_clock::now();
        recv = mControlPort->ReadControl((uint8_t*)&pkt, sizeof(pkt), 1000);
        }
        if (mCallback_logData)
            mCallback_logData(false, (uint8_t*)&pkt, recv);

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
        auto t2 = std::chrono::high_resolution_clock::now();
        int duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if(duration > 100)
            printf("=======SPI read blocked for %ims\n", duration);
        //printPacket(pkt, rdPrint, "Rd:");
        pkt.blockCount = 0;
        pkt.status = STATUS_UNDEFINED;
    }
}

int LimeSDR_5GRadio::I2CWrite(int address, const uint8_t *data, uint32_t length)
{
    assert(mControlPort);
    LMS64CProtocol::LMS64CPacket pkt;
    int remainingBytes = length;
    const uint8_t* src = data;
    std::lock_guard<std::mutex> lock(mCommsMutex);
    while (remainingBytes > 0)
    {
        pkt.cmd = CMD_I2C_WR;
        pkt.status = STATUS_UNDEFINED;
        pkt.blockCount = remainingBytes > pkt.maxDataLength ? pkt.maxDataLength : remainingBytes;
        pkt.periphID = address;
        memcpy(pkt.payload, src, pkt.blockCount);
        src += pkt.blockCount;
        remainingBytes -= pkt.blockCount;
        int sent = mControlPort->WriteControl((uint8_t*)&pkt, sizeof(pkt), 100);
        if (sent != sizeof(pkt))
            throw std::runtime_error("I2C write failed");
        int recv = mControlPort->ReadControl((uint8_t*)&pkt, sizeof(pkt), 100);

        if (recv < pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
            throw std::runtime_error("I2C write failed");
    }
    return 0;
}

int LimeSDR_5GRadio::I2CRead(int address, uint8_t *data, uint32_t length)
{
    assert(mControlPort);
    LMS64CProtocol::LMS64CPacket pkt;
    int remainingBytes = length;
    uint8_t* dest = data;
    std::lock_guard<std::mutex> lock(mCommsMutex);
    while (remainingBytes > 0)
    {
        pkt.cmd = CMD_I2C_RD;
        pkt.status = STATUS_UNDEFINED;
        pkt.blockCount = remainingBytes > pkt.maxDataLength ? pkt.maxDataLength : remainingBytes;
        pkt.periphID = address;

        int sent = mControlPort->WriteControl((uint8_t*)&pkt, sizeof(pkt), 100);
        if (sent != sizeof(pkt))
            throw std::runtime_error("I2C read failed");
        int recv = mControlPort->ReadControl((uint8_t*)&pkt, sizeof(pkt), 100);

        memcpy(dest, pkt.payload, pkt.blockCount);
        dest += pkt.blockCount;
        remainingBytes -= pkt.blockCount;

        if (recv <= pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
            throw std::runtime_error("I2C read failed");
    }
    return 0;
}

int LimeSDR_5GRadio::StreamSetup(const StreamConfig &config, uint8_t moduleIndex)
{
    if (mStreamers.at(moduleIndex))
        return -1; // already running
    try {
        mStreamers.at(moduleIndex) = new TRXLooper_PCIE(
            mRXStreamPorts.at(moduleIndex),
            mTXStreamPorts.at(moduleIndex),
            mFPGA, mLMSChips.at(moduleIndex),
            moduleIndex
        );
        if (mCallback_logMessage)
            mStreamers[moduleIndex]->SetMessageLogCallback(mCallback_logMessage);
        LitePCIe* trxPort = mRXStreamPorts.at(moduleIndex);
        if(!trxPort->IsOpen())
            if (trxPort->Open(trxPort->GetPathName().c_str(), O_RDWR | O_NOCTTY | O_CLOEXEC | O_NONBLOCK) != 0)
            {
                char ctemp[128];
                sprintf(ctemp, "Failed to open device in stream start: %s", trxPort->GetPathName().c_str());
                throw std::runtime_error(ctemp);
            }
        mStreamers[moduleIndex]->Setup(config);
        mStreamConfig = config;
        return 0;
    }
    catch (std::logic_error &e) {
        printf("LimeSDR_5GRadio::StreamSetup logic_error %s\n", e.what());
        throw;
    }
    catch (std::runtime_error &e) {
        printf("LimeSDR_5GRadio::StreamSetup runtime_error %s\n", e.what());
        throw;
    }
}

void LimeSDR_5GRadio::StreamStart(uint8_t moduleIndex)
{
    mStreamers.at(moduleIndex)->Start();
}

void LimeSDR_5GRadio::StreamStop(uint8_t moduleIndex)
{
    SDRDevice::StreamStop(moduleIndex);
    LitePCIe* trxPort = mRXStreamPorts.at(moduleIndex);
    if (trxPort && trxPort->IsOpen())
        trxPort->Close();
}

void LimeSDR_5GRadio::StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats &status)
{
    TRXLooper *trx = mStreamers.at(moduleIndex);
    status.dataRate_Bps = trx->GetDataRate(false);
    status.txDataRate_Bps = trx->GetDataRate(true);
}

void LimeSDR_5GRadio::SetFPGAInterfaceFreq(uint8_t interp, uint8_t dec, double txPhase, double rxPhase)
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

void LimeSDR_5GRadio::LMS1_SetSampleRate(double f_Hz, uint8_t rxDecimation, uint8_t txInterpolation)
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

void LimeSDR_5GRadio::LMS1_PA_Enable(uint8_t chan, bool enabled)
{
    uint16_t pa_addr = 0x00D2;
    uint16_t pa_val = mFPGA->ReadRegister(pa_addr);

    const int bitMask = 1 << (5-chan);
    if (enabled)
        pa_val |= bitMask; // PA on
    else
        pa_val &= ~bitMask; // chan 0 = 5; chan 1 = 4
    mFPGA->WriteRegister(pa_addr, pa_val);
}

void LimeSDR_5GRadio::LMS1SetPath(bool tx, uint8_t chan, uint8_t pathId)
{
    uint16_t sw_addr = 0x00D1;
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

        if (path == LMS_PATH_TX1)
            sw_val |= 1 << (13-chan);   // chan 0 = 13; chan 1 = 12
        else if (path == LMS_PATH_TX2)
            sw_val &= ~(1 << (13-chan));

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

        if(path == LMS_PATH_LNAW)
            lime::warning("LNAW has no connection to RF ports");
        else if (path == LMS_PATH_LNAH)
            sw_val |= 1 << (11-chan);
        else if(path == LMS_PATH_LNAL)
            sw_val &= ~(1UL << (11-chan));

        mFPGA->WriteRegister(sw_addr, sw_val);
        lms->SetPathRFE(lime::LMS7002M::PathRFE(path));
    }
}

void LimeSDR_5GRadio::LMS2_PA_LNA_Enable(uint8_t chan, bool PAenabled, bool LNAenabled)
{
    uint16_t pa_addr = 0x00D2;
    struct RegPA
    {
        RegPA(uint16_t value) {
            lms1PA[0] = value & (1<<5);
            lms1PA[1] = value & (1<<4);
            lms2PA[0] = value & (1<<3);
            lms2PA[1] = value & (1<<2);
            lms2LNA[0] = !(value & (1<<1)); // 1=LNA is powered down
            lms2LNA[1] = !(value & (1<<0));
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

void LimeSDR_5GRadio::LMS2SetPath(bool tx, uint8_t chan, uint8_t path)
{
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

    int tx_path = LMS_PATH_TX1;
    int rx_path = LMS_PATH_LNAH;
    uint16_t shift = chan == 0 ? 0 : 2;
    if (path == 0)
    {
        tx_path = LMS_PATH_NONE;
        rx_path = LMS_PATH_NONE;
    }
    else if (tx && ePathLMS2_Tx(path) == ePathLMS2_Tx::TDD) // TDD_TX
    {
        if(chan == 0)
            sw_val &= ~(1 << 7);                // TRX1T to RSFW_TRX1
        else
            sw_val |= 1 << 9;                   // TRX2T to RSFW_TRX2
        sw_val |= 1 << (6+shift);               // TRX1 or TRX2 to J8 or J10
        sw_val &= ~(1 << (2+shift));            // RX1C or RX2C to RX1IN or RX2IN (LNA)
        sw_val |= 1 << (3+shift);               // RX1IN or RX2IN to RFSW_TRX1 or RFSW_TRX2
    }
    else if (!tx && ePathLMS2_Rx(path) == ePathLMS2_Rx::TDD) // TDD_RX
    {
        if(chan == 0)
            sw_val |= 1 << 7;                   // TRX1T to ground
        else
            sw_val &= ~(1 << 9);                // TRX2T to ground
        sw_val &= ~(1 << (6+shift));            // TRX1 or TRX2 to J8 or J10
        sw_val &= ~(1 << (2+shift));            // RX1C or RX2C to RX1IN or RX2IN (LNA)
        sw_val |= 1 << (3+shift);               // RX1IN or RX2IN to RFSW_TRX1 or RFSW_TRX1
    }
    else if (ePathLMS2_Rx(path) == ePathLMS2_Rx::FDD || ePathLMS2_Tx(path) == ePathLMS2_Tx::FDD) // FDD
    {
        if(chan == 0)
            sw_val &= ~(1 << 7);                // TRX1T to RSFW_TRX1
        else
            sw_val |= 1 << 9;                   // TRX2T to RSFW_TRX2
        sw_val |= 1 << (6+shift);               // TRX1 or TRX2 to J8 or J10
        sw_val &= ~(1 << (2+shift));            // RX1C or RX2C to RX1IN or RX2IN (LNA)
        sw_val &= ~(1 << (3+shift));            // RX1IN or RX2In to J9 or  J11
    }
    else if (!tx && ePathLMS2_Rx(path) == ePathLMS2_Rx::CALIBRATION) // Cal
    {
        if(chan == 0)
            sw_val |= 1 << 7;                   // TRX1T to ground
        else
            sw_val &= ~(1 << 9);                // TRX2T to ground
        sw_val |= 1 << (6+shift);               // TRX1 or TRX2 to J8 or J10
        sw_val |= 1 << (2+shift);               // RX1C or RX2C to LMS3 TX1_1 or TX2_1
        sw_val |= 1 << (3+shift);               // RX1IN or RX2IN to RFSW_TRX1 or RFSW_TRX1
    }

    mFPGA->WriteRegister(sw_addr, sw_val);
    lime::LMS7002M* lms = mLMSChips.at(1);
    lms->SetBandTRF(1); // LMS2 uses only BAND1
    lms->SetPathRFE(lime::LMS7002M::PathRFE(LMS7002M::PATH_RFE_LNAH)); // LMS2 only uses LNAH
}

void LimeSDR_5GRadio::LMS3SetPath(bool tx, uint8_t chan, uint8_t path)
{
    uint16_t sw_addr = 0x00D1;
    uint16_t sw_val = mFPGA->ReadRegister(sw_addr);
    lime::LMS7002M* lms = mLMSChips.at(0);

    if(tx)
    {
        lime::warning("TX has no connection to RF ports");
        lms->SetBandTRF(path);
    }
    else
    {
        if(path == LMS_PATH_NONE || path > 2)
        {
            lms->SetPathRFE(lime::LMS7002M::PathRFE(LMS_PATH_NONE));
            return;
        }
        else if(path == LMS_PATH_LNAH)
            sw_val &= ~(1 << (chan-4));
        else if (path == 2) // Calibration path
            sw_val |= 1 << (chan-4);

        mFPGA->WriteRegister(sw_addr, sw_val);
        lms->SetPathRFE(lime::LMS7002M::PathRFE(LMS_PATH_LNAH));
    }
}

void LimeSDR_5GRadio::LMS2_SetSampleRate(double f_Hz, uint8_t oversample)
{
    assert(cdcm[0]);
    double txClock = f_Hz;

    // Oversample is available only to Tx for LMS#2
    oversample = std::min(oversample, uint8_t(2));
    if(oversample == 2 || oversample == 0) // 0 is "auto", use max oversample
        txClock *= 2;

    mEqualizer->SetOversample(oversample);

    if(cdcm[0]->SetFrequency(CDCM_Y0Y1, txClock, false) != 0) // Tx Ch. A&B
        throw std::runtime_error("Failed to configure CDCM_Y0Y1");
    if(cdcm[0]->SetFrequency(CDCM_Y4, f_Hz, false) != 0) // Rx Ch. A
        throw std::runtime_error("Failed to configure CDCM_Y4");
    if(cdcm[0]->SetFrequency(CDCM_Y5, f_Hz, true) != 0) // Rx Ch. B
        throw std::runtime_error("Failed to configure CDCM_Y5");

    if (!cdcm[0]->IsLocked())
        throw std::runtime_error("CDCM is not locked");
}

int LimeSDR_5GRadio::CustomParameterWrite(const uint8_t *ids, const double *values, const size_t count, const std::string& units)
{
    assert(mControlPort);
    LMS64CProtocol::LMS64CPacket pkt;
    pkt.cmd = CMD_ANALOG_VAL_WR;
    pkt.status = STATUS_UNDEFINED;
    pkt.blockCount = count;
    pkt.periphID = 0;
    int byteIndex = 0;
    std::lock_guard<std::mutex> lock(mCommsMutex);
    for (size_t i = 0; i < count; ++i)
    {
        pkt.payload[byteIndex++] = ids[i];
        int powerOf10 = 0;
        if(values[i] > 65535.0 && (units != ""))
            powerOf10 = log10(values[i]/65.536)/3;
        if (values[i] < 65.536 && (units != ""))
            powerOf10 = log10(values[i]/65535.0) / 3;
        int unitsId = 0; // need to convert given units to their enum
        pkt.payload[byteIndex++] = unitsId << 4 | powerOf10;
        int value = values[i] / pow(10, 3*powerOf10);
        pkt.payload[byteIndex++] = (value >> 8);
        pkt.payload[byteIndex++] = (value & 0xFF);
    }
    int sent = mControlPort->WriteControl((uint8_t*)&pkt, sizeof(pkt), 100);
    if (sent != sizeof(pkt))
        throw std::runtime_error("CustomParameterWrite write failed");
    int recv = mControlPort->ReadControl((uint8_t*)&pkt, sizeof(pkt), 100);
    if (recv < pkt.headerSize || pkt.status != STATUS_COMPLETED_CMD)
        throw std::runtime_error("CustomParameterWrite write failed");
    return 0;
}

} //namespace lime
