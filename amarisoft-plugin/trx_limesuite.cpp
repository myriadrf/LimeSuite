/*
 * LimeMicroSystem transceiver driver
 * Copyright (C) 2015-2018 Amarisoft/LimeMicroSystems
 */
#include <limesuite/SDRDevice.h>
#include <limesuite/DeviceRegistry.h>
#include <limesuite/LMS7002M.h>

#include <sstream>
#include <stdarg.h>
#include <map>
#include <math.h>
#include <iostream>
#include <atomic>
#include <vector>
#include <chrono>
#include <mutex>
#include <array>

extern "C" {
#include "trx_driver.h"
}

using namespace std;
using namespace lime;

typedef lime::SDRDevice::LogLevel LogLevel;

std::mutex gainsMutex;

static lime::SDRDevice::LogLevel logVerbosity = lime::SDRDevice::ERROR;
static std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

struct RxGainRow {
    int lna;
    int pga;
};

static RxGainRow rxGainTable[] = { { 1, 12 },
    { 1, 13 },
    { 2, 12 },
    { 2, 13 },
    { 2, 14 },
    { 3, 12 },
    { 3, 13 },
    { 3, 14 },
    { 4, 12 },
    { 4, 13 },
    { 4, 14 },
    { 5, 12 },
    { 5, 13 },
    { 5, 14 },
    { 6, 12 },
    { 6, 13 },
    { 6, 14 },
    { 7, 12 },
    { 7, 13 },
    { 7, 14 },
    { 8, 12 },
    { 8, 13 },
    { 8, 14 },
    { 9, 12 },
    { 9, 13 },
    { 9, 14 },
    { 10, 12 },
    { 11, 12 },
    { 12, 12 },
    { 13, 12 },
    { 14, 12 },
    { 15, 12 },
    { 15, 13 },
    { 15, 14 },
    { 15, 15 },
    { 15, 16 },
    { 15, 17 },
    { 15, 18 },
    { 15, 19 },
    { 15, 20 },
    { 15, 21 },
    { 15, 22 },
    { 15, 23 },
    { 15, 24 },
    { 15, 25 },
    { 15, 26 },
    { 15, 27 },
    { 15, 28 },
    { 15, 29 },
    { 15, 30 },
    { 15, 31 } };
static_assert(sizeof(rxGainTable) / sizeof(RxGainRow) == 51, "missing rx gains rows");

struct TxGainRow {
    int main;
    int lin;
};

static TxGainRow txGainTable[] = { { 30, 30 },
    { 30, 30 },
    { 29, 29 },
    { 29, 29 },
    { 28, 28 },
    { 28, 28 },
    { 27, 27 },
    { 27, 27 },
    { 26, 26 },
    { 26, 26 },
    { 25, 25 },
    { 25, 25 },
    { 24, 24 },
    { 24, 24 },
    { 23, 23 },
    { 23, 23 },
    { 22, 22 },
    { 22, 22 },
    { 21, 21 },
    { 21, 21 },
    { 20, 20 },
    { 20, 20 },
    { 19, 19 },
    { 19, 19 },
    { 18, 18 },
    { 18, 18 },
    { 17, 17 },
    { 17, 17 },
    { 16, 16 },
    { 16, 16 },
    { 15, 15 },
    { 15, 15 },
    { 14, 14 },
    { 14, 14 },
    { 13, 13 },
    { 13, 13 },
    { 12, 12 },
    { 12, 12 },
    { 11, 11 },
    { 11, 11 },
    { 10, 10 },
    { 9, 9 },
    { 8, 8 },
    { 7, 7 },
    { 6, 6 },
    { 5, 5 },
    { 4, 4 },
    { 3, 3 },
    { 2, 2 },
    { 1, 1 },
    { 0, 0 } };
static_assert(sizeof(txGainTable) / sizeof(TxGainRow) == 51, "missing tx gains rows");

static inline int64_t ts_to_time(int64_t fs, int64_t ts)
{
    int n, r;
    n = (ts / fs);
    r = (ts % fs);
    return (int64_t)n * 1000000 + (((int64_t)r * 1000000) / fs);
}

struct PortChPair {
    uint8_t port;
    uint8_t ch;
};
// mappings from just channel number to actual port's channel
static PortChPair gMapTxChannelToPortCh[TRX_MAX_CHANNELS] = {};
static PortChPair gMapRxChannelToPortCh[TRX_MAX_CHANNELS] = {};

static inline const std::string strFormat(const char* format, ...)
{
    char ctemp[512];

    va_list args;
    va_start(args, format);
    vsnprintf(ctemp, 256, format, args);
    va_end(args);

    return std::string(ctemp);
}

static void Log(LogLevel lvl, const char* format, ...)
{
    if (lvl > logVerbosity)
        return;
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

static void LogCallback(SDRDevice::LogLevel lvl, const char* msg)
{
    if (lvl > logVerbosity)
        return;
    printf("%s\n", msg);
}

enum CalibrateFlag { None = 0, DCIQ = 1, Filter = 2 };

static const int NO_ERROR = 0;

struct LimeState {
    SDRDevice* device[TRX_MAX_RF_PORT];
    int rxPath[TRX_MAX_RF_PORT];
    int txPath[TRX_MAX_RF_PORT];
    bool skipConfig[TRX_MAX_RF_PORT];
    SDRDevice::StreamConfig streamCfg[TRX_MAX_RF_PORT];
    int deviceCount;
    uint8_t chipIndex[TRX_MAX_RF_PORT];
    int tx_channel_count[TRX_MAX_RF_PORT];
    int rx_channel_count[TRX_MAX_RF_PORT];
    int tcxo_calc; // values from 0 to 255
    bool tx_gfir_enable[TRX_MAX_RF_PORT];
    bool rx_gfir_enable[TRX_MAX_RF_PORT];
    int tx_gfir_bandwidth[TRX_MAX_RF_PORT];
    int rx_gfir_bandwidth[TRX_MAX_RF_PORT];
    int sample_rate[TRX_MAX_RF_PORT];
    int calibrate[TRX_MAX_RF_PORT];
    float rxGainNorm[TRX_MAX_RF_PORT];
    float txGainNorm[TRX_MAX_RF_PORT];
    int rxPower_dBm[TRX_MAX_RF_PORT][TRX_MAX_CHANNELS];
    int txPower_dBm[TRX_MAX_RF_PORT][TRX_MAX_CHANNELS];
    bool rxPowerAvailable[TRX_MAX_RF_PORT][TRX_MAX_CHANNELS];
    bool txPowerAvailable[TRX_MAX_RF_PORT][TRX_MAX_CHANNELS];
    uint16_t samplesInPacket[TRX_MAX_RF_PORT];
    lime::SDRDevice::StreamConfig::DataFormat samplesFormat;
    lime::SDRDevice::StreamConfig::DataFormat linkFormat[TRX_MAX_RF_PORT];

    // gain overrides, temporary while noramlized gain is unavailable
    int glna[TRX_MAX_RF_PORT];
    int gpga[TRX_MAX_RF_PORT];
    int iamp[TRX_MAX_RF_PORT];
    int lossmain[TRX_MAX_RF_PORT];
    int rxOversample[TRX_MAX_RF_PORT];
    int txOversample[TRX_MAX_RF_PORT];
    double rx_LO_override[TRX_MAX_RF_PORT];
    double tx_LO_override[TRX_MAX_RF_PORT];
    std::vector<uint32_t> writeRegisters[TRX_MAX_RF_PORT];
    SDRDevice::StreamConfig::Extras* streamExtras[TRX_MAX_RF_PORT];

    LimeState()
    {
#define memnull(x) memset(x, 0, sizeof(x));
        memnull(device);
        memnull(rxPath);
        memnull(txPath);
        memnull(skipConfig);
        memnull(chipIndex);
        memnull(tx_channel_count);
        memnull(rx_channel_count);
        //memnull(tcxo_calc);
        memnull(tx_gfir_enable);
        memnull(rx_gfir_enable);
        memnull(sample_rate);
        memnull(calibrate);
        memnull(samplesInPacket);
        memnull(glna);
        memnull(gpga);
        memnull(rxOversample);
        memnull(txOversample);
        memnull(streamExtras);
        memnull(rx_LO_override);
        memnull(tx_LO_override);
#undef memnull
    }
};

struct StreamStatus {
    SDRDevice::StreamStats rx;
    SDRDevice::StreamStats tx;
};
static std::array<StreamStatus, TRX_MAX_RF_PORT> portStreamStates;

static auto lastStreamUpdate = std::chrono::steady_clock::now();
static bool OnStreamStatusChange(bool isTx, const SDRDevice::StreamStats* s, void* userData)
{
    StreamStatus& status = *static_cast<StreamStatus*>(userData);
    SDRDevice::StreamStats& dest = isTx ? status.tx : status.rx;

    dest.FIFO_filled = s->FIFO_filled;
    dest.dataRate_Bps = s->dataRate_Bps;
    dest.overrun = s->overrun;
    dest.underrun = s->underrun;
    dest.loss = s->loss;
    if (!isTx) // Tx dropped packets are reported from Rx received packet flags
        status.tx.late = s->late;

    // reporting every dropped packet can be quite spammy, so print info only periodically
    auto now = chrono::steady_clock::now();
    if (chrono::duration_cast<chrono::milliseconds>(now - lastStreamUpdate) > chrono::milliseconds(500) &&
        logVerbosity >= LogLevel::WARNING)
    {
        cout << "Rx| Loss: " << status.rx.loss << " overrun: " << status.rx.overrun << " rate: " << status.rx.dataRate_Bps / 1e6
             << " MB/s"
             << "\nTx| Late: " << status.tx.late << " underrun: " << status.tx.underrun << " rate: " << status.tx.dataRate_Bps / 1e6
             << " MB/s" << std::endl;
        lastStreamUpdate = now;
    }
    return false;
}

static TRXStatistics trxstats;
static int trx_lms7002m_get_stats(TRXState* s, TRXStatistics* m)
{
    LimeState* lime = (LimeState*)s->opaque;
    for (long p = 0; p < lime->deviceCount; ++p)
    {
        StreamStatus& stats = portStreamStates[p];
        trxstats.rx_overflow_count = stats.rx.overrun + stats.rx.loss;
        trxstats.tx_underflow_count = stats.tx.late + stats.tx.underrun;
    }
    memcpy(m, &trxstats, sizeof(TRXStatistics));
    return 0;
}

/* Callback must allocate info buffer that will be displayed */
static void trx_lms7002m_dump_info(TRXState* s, trx_printf_cb cb, void* opaque)
{
    LimeState* lime = (LimeState*)s->opaque;
    std::stringstream ss;
    for (long p = 0; p < lime->deviceCount; ++p)
    {
        SDRDevice* dev = lime->device[p];
        StreamStatus& stats = portStreamStates[p];

        SDRDevice::StreamStats rx;
        SDRDevice::StreamStats tx;
        dev->StreamStatus(lime->chipIndex[p], &rx, &tx);
        stats.rx.dataRate_Bps = rx.dataRate_Bps;
        stats.tx.dataRate_Bps = tx.dataRate_Bps;

        ss << "\nPort" << p << " [Rx| Loss: " << stats.rx.loss << " overrun: " << stats.rx.overrun
           << " rate: " << stats.rx.dataRate_Bps / 1e6 << " MB/s]"
           << "[Tx| Late: " << stats.tx.late << " underrun: " << stats.tx.underrun << " rate: " << stats.tx.dataRate_Bps / 1e6
           << " MB/s]"
           << " linkFormat: " << (lime->linkFormat[p] == SDRDevice::StreamConfig::I16 ? "I16" : "I12") << std::endl;
        // TODO: read FIFO usage
    }
    const int len = ss.str().length();
    char* buffer = (char*)malloc(len + 1);
    cb(buffer, "%s\n", ss.str().c_str());
}

// count - can be configured by 'trx_get_tx_samples_per_packet_func' callback
// if samples==NULL, Tx has to be disabled
// samples are ranged [-1.0 : +1.0]
static void trx_lms7002m_write(
    TRXState* s, trx_timestamp_t timestamp, const void** samples, int count, int port, TRXWriteMetadata* md)
{
    if (!samples) // Nothing to transmit
        return;

    SDRDevice::StreamMeta meta;
    meta.timestamp = timestamp;
    meta.useTimestamp = true;
    meta.flush = (md->flags & TRX_WRITE_MD_FLAG_END_OF_BURST);

    // samples format conversion is done internally
    LimeState* lime = (LimeState*)s->opaque;
    int samplesConsumed =
        lime->device[port]->StreamTx(lime->chipIndex[port], reinterpret_cast<const lime::complex32f_t**>(samples), count, &meta);
    if (logVerbosity == LogLevel::DEBUG && samplesConsumed != count)
    {
        if (samplesConsumed < 0) // hardware timestamp is already ahead of meta.timestamp by (-samplesConsumed)
        {
            ++portStreamStates[port].tx.underrun;
            Log(LogLevel::DEBUG, "StreamTx: discarded samples write that is already late by (%i samples)\n", -samplesConsumed);
        }
        // else
        //     Log(LogLevel::DEBUG, "Tx not full consumed %i/%i\n", samplesConsumed, count);
    }
}

// Read has to block until at least 1 sample is available and must return at most 'count' samples
// 'count' argument value depends on sample rate and is at most 4096 samples.
// timestamp is samples counter
// return number of samples produced
static int trx_lms7002m_read(TRXState* s, trx_timestamp_t* ptimestamp, void** samples, int count, int port, TRXReadMetadata* md)
{
    LimeState* lime = (LimeState*)s->opaque;

    SDRDevice::StreamMeta meta;
    meta.useTimestamp = false;
    meta.flush = false;

    md->flags = 0;
    int samplesGot =
        lime->device[port]->StreamRx(lime->chipIndex[port], reinterpret_cast<lime::complex32f_t**>(samples), count, &meta);

    if (samplesGot == 0)
    {
        // If the device is misconfigured it might not produce data for Rx
        // in that case the host will be stuck in loop attempting to read.
        // terminate the process to avoid freeze, since samples will never arrive.
        Log(LogLevel::ERROR, "Read timeout for Port[%i], device might be configured incorrectly\n", port);
        exit(-EIO);
    }
    else if (samplesGot < 0)
    {
        Log(LogLevel::ERROR, "Error reading samples for Port[%i]: error code %i\n", port, samplesGot);
        exit(-EIO);
    }
    *ptimestamp = meta.timestamp;
    return samplesGot;
}

// Return in *psample_rate the sample rate supported by the device
// corresponding to a LTE bandwidth of 'bandwidth' Hz. Also return
// in n=*psample_rate_num the wanted sample rate before the
// interpolator as 'n * 1.92' MHz. 'n' must currently be of the
// form 2^n1*3^n2*5^n3.
// Return 0 if OK, -1 if none.
static int trx_lms7002m_get_sample_rate(TRXState* s1, TRXFraction* psample_rate, int* psample_rate_num, int bandwidth)
{
    LimeState* s = static_cast<LimeState*>(s1->opaque);
    // multipliers that can be made using 2^n1*3^n2*5^n3, n1 >= 1
    const uint8_t multipliers[] = { 2,
        4,
        6,
        8,
        10,
        12,
        16,
        18,
        20,
        24,
        //30, 32, 36, 40, 48, 50, 54, 60, 64
        // 8, 10, 12, 16, 24,
        32,
        40,
        48,
        50,
        64 };

    // trx_lms7002m_get_sample_rate seems to be called for each Port, but the API does not provide index.
    // workaround here assume that they are being configured in order 0,1,2...
    static int whichPort = 0;
    int p = whichPort;
    int& rate = s->sample_rate[p];

    whichPort = (whichPort + 1) % s->deviceCount; // mod, just in case.

    const float desiredSamplingRate = bandwidth * 1.536;
    //printf ("Required bandwidth:[%u], current sample_rate [%u]\n", bandwidth, s->sample_rate);
    if (rate <= 0) // sample rate not specified, align on 1.92Mhz
    {
        Log(LogLevel::VERBOSE,
            "Port[%i] Trying sample rates which are bandwidth(%u) * 1.536 = %f\n",
            p,
            bandwidth,
            desiredSamplingRate);
        for (uint32_t i = 0; i < sizeof(multipliers) / sizeof(uint8_t); ++i)
        {
            int n = multipliers[i];
            if (n * 1920000 > 122.88e6)
                break;
            Log(LogLevel::DEBUG, "\tPort[%i] Trying sample rate : bandwidth(%u) sample_rate(%u)\n", p, bandwidth, n * 1920000);
            if (desiredSamplingRate <= n * 1920000)
            {
                *psample_rate_num = n;
                psample_rate->num = n * 1920000;
                psample_rate->den = 1;
                rate = psample_rate->num;
                Log(LogLevel::INFO, "Port[%i] Automatic sample rate: %g MSps, n = [%u] * 1.92e6\n", p, rate / 1e6, n);
                return 0;
            }
        }
        Log(LogLevel::VERBOSE, "Port[%i] Trying sample rates which are close to bandwidth(%u)\n", p, bandwidth);
        for (uint32_t i = 0; i < sizeof(multipliers) / sizeof(uint8_t); ++i)
        {
            int n = multipliers[i];
            Log(LogLevel::DEBUG, "\tPort[%i] Trying sample rate : bandwidth(%u) sample_rate(%u)\n", p, bandwidth, n * 1920000);
            if (bandwidth <= n * 1920000)
            {
                *psample_rate_num = n;
                psample_rate->num = n * 1920000;
                psample_rate->den = 1;
                rate = psample_rate->num;
                Log(LogLevel::INFO, "Port[%i] Automatic sample rate: %g MSps, n = [%u] * 1.92e6\n", p, rate / 1e6, n);
                return 0;
            }
        }
        Log(LogLevel::ERROR, "Port[%i] Could not find suitable sampling rate for %i bandwidth\n", p, bandwidth);
    }
    else
    {
        if (rate < bandwidth)
        {
            Log(LogLevel::ERROR, "Port[%i] Manually specified sample rate %i is less than LTE bandwidth %i\n", p, rate, bandwidth);
            return -1;
        }
        Log(LogLevel::INFO, "Port[%i] Manually specified sample rate: %f MSps\n", p, rate / 1e6);
        psample_rate->num = rate;
        psample_rate->den = 1;
        *psample_rate_num = rate / 1920000;
        return 0;
    }
    return -1;
}

// return expected number of samples in Tx packet
// !!! sometimes trx_lms7002m_write gets calls with samples count more than returned here
static int trx_lms7002m_get_tx_samples_per_packet_func(TRXState* s1)
{
    LimeState* lime = (LimeState*)s1->opaque;
    int txExpectedSamples = lime->samplesInPacket[0];
    if (lime->streamExtras[0] && lime->streamExtras[0]->txSamplesInPacket > 0)
    {
        txExpectedSamples = lime->streamExtras[0]->txSamplesInPacket;
    }
    Log(LogLevel::DEBUG, "Hardware expected samples count in Tx packet : %i\n", txExpectedSamples);
    return txExpectedSamples;
}

static int trx_lms7002m_get_abs_rx_power_func(TRXState* s1, float* presult, int channel_num)
{
    LimeState* s = (LimeState*)s1->opaque;
    const PortChPair& pair = gMapTxChannelToPortCh[channel_num];
    if (s->rxPowerAvailable[pair.port][pair.ch])
    {
        *presult = s->rxPower_dBm[pair.port][pair.ch];
        return 0;
    }
    else
        return -1;
}

static int trx_lms7002m_get_abs_tx_power_func(TRXState* s1, float* presult, int channel_num)
{
    LimeState* s = (LimeState*)s1->opaque;
    const PortChPair& pair = gMapTxChannelToPortCh[channel_num];
    if (s->txPowerAvailable[pair.port][pair.ch])
    {
        *presult = s->txPower_dBm[pair.port][pair.ch];
        return 0;
    }
    else
        return -1;
}

//min gain 0
//max gain ~70-76 (higher will probably degrade signal quality to much)
static void trx_lms7002m_set_tx_gain_func(TRXState* s1, double gain, int channel_num)
{
    LimeState* lime = (LimeState*)s1->opaque;
    int row = gain;
    if (row < 0 || row >= (int)(sizeof(txGainTable) / sizeof(TxGainRow)))
        return;

    std::lock_guard<std::mutex> lk(gainsMutex);

    const PortChPair& pair = gMapTxChannelToPortCh[channel_num];
    LMS7002M* chip = static_cast<LMS7002M*>(lime->device[pair.port]->GetInternalChip(lime->chipIndex[pair.port]));
    chip->Modify_SPI_Reg_bits(LMS7_MAC, pair.ch + 1);
    chip->Modify_SPI_Reg_bits(LMS7_LOSS_MAIN_TXPAD_TRF, txGainTable[row].main);
    chip->Modify_SPI_Reg_bits(LMS7_LOSS_LIN_TXPAD_TRF, txGainTable[row].lin);
    chip->Modify_SPI_Reg_bits(LMS7_MAC, 1);
    Log(LogLevel::DEBUG,
        "Port[%i] ch[%i] Tx gain set MAIN:%i, LIN:%i\n",
        pair.port,
        pair.ch,
        txGainTable[row].main,
        txGainTable[row].lin);
}

static void trx_lms7002m_set_rx_gain_func(TRXState* s1, double gain, int channel_num)
{
    LimeState* lime = (LimeState*)s1->opaque;
    int row = gain;
    if (row < 0 || row >= (int)(sizeof(rxGainTable) / sizeof(RxGainRow)))
        return;

    std::lock_guard<std::mutex> lk(gainsMutex);

    const PortChPair& pair = gMapRxChannelToPortCh[channel_num];
    LMS7002M* chip = static_cast<LMS7002M*>(lime->device[pair.port]->GetInternalChip(lime->chipIndex[pair.port]));
    chip->Modify_SPI_Reg_bits(LMS7_MAC, pair.ch + 1);
    chip->Modify_SPI_Reg_bits(LMS7_G_LNA_RFE, rxGainTable[row].lna);
    chip->Modify_SPI_Reg_bits(LMS7_G_PGA_RBB, rxGainTable[row].pga);
    chip->Modify_SPI_Reg_bits(LMS7_MAC, 1);
    Log(LogLevel::DEBUG,
        "Port[%i] ch[%i] Rx gain set LNA:%i, PGA:%i\n",
        pair.port,
        pair.ch,
        rxGainTable[row].lna,
        rxGainTable[row].pga);
}

static int trx_lms7002m_start(TRXState* s1, const TRXDriverParams* hostState)
{
    LimeState* lime = (LimeState*)s1->opaque;

    try
    {

        int rxChannelOffset = 0;
        int txChannelOffset = 0;
        for (int p = 0; p < hostState->rf_port_count; ++p)
        {
            SDRDevice* portDevice = lime->device[p];
            SDRDevice::SDRConfig config;

            lime->tx_channel_count[p] = hostState->tx_port_channel_count[p];
            lime->rx_channel_count[p] = hostState->rx_port_channel_count[p];

            const double samplingRate = (float)hostState->sample_rate[p].num / hostState->sample_rate[p].den;
            config.channel[0].rx.sampleRate = samplingRate;
            config.channel[0].tx.sampleRate = samplingRate;
            config.channel[0].rx.oversample = lime->rxOversample[p];
            config.channel[0].tx.oversample = lime->txOversample[p];
            config.channel[1].rx.sampleRate = samplingRate;
            config.channel[1].tx.sampleRate = samplingRate;
            config.channel[1].rx.oversample = lime->rxOversample[p];
            config.channel[1].tx.oversample = lime->txOversample[p];

            Log(LogLevel::DEBUG,
                "Port[%i] sampleRate: %.3f MHz, rxOversample x%i, txOversample x%i (0 is 'auto' max available)\n",
                p,
                samplingRate / 1e6,
                lime->rxOversample[p],
                lime->txOversample[p]);
            Log(LogLevel::DEBUG,
                "Port[%i] required channel count, Rx[%i], Tx[%i]\n",
                p,
                hostState->rx_port_channel_count[p],
                hostState->tx_port_channel_count[p]);

            // int availableRxChannels = LMS_GetNumChannels(portDevice, Dir::Rx);
            // if (availableRxChannels < hostState->rx_port_channel_count[p])
            // {
            //     printf("Error: device assigned to Port[%i] supports only %i Rx channels, required %i\n", p, availableRxChannels, hostState->rx_port_channel_count[p]);
            //     return -2;
            // }
            // int availableTxChannels = LMS_GetNumChannels(portDevice, Dir::Tx);
            // if (availableTxChannels < hostState->tx_port_channel_count[p])
            // {
            //     printf("Error: device assigned to Port[%i] supports only %i Tx channels, required %i\n", p, availableTxChannels, hostState->tx_port_channel_count[p]);
            //     return -3;
            // }

            for (int ch = 0; ch < lime->rx_channel_count[p]; ++ch)
            {
                auto paths = portDevice->GetDescriptor().rfSOC[lime->chipIndex[p]].rxPathNames;
                double freq = hostState->rx_freq[rxChannelOffset + ch];
                char loFreqStr[1024];
                if (lime->rx_LO_override[p] > 0)
                {
                    freq = lime->rx_LO_override[p];
                    sprintf(loFreqStr,
                        "expectedLO: %.3f MHz [override: %.3f (diff:%+.3f) MHz]",
                        hostState->rx_freq[rxChannelOffset + ch] / 1.0e6,
                        lime->rx_LO_override[p] / 1.0e6,
                        (hostState->rx_freq[rxChannelOffset + ch] - lime->rx_LO_override[p]) / 1.0e6);
                }
                else
                    sprintf(loFreqStr, "LO: %.3f MHz", freq / 1.0e6);
                gMapRxChannelToPortCh[rxChannelOffset + ch] = { (uint8_t)p, (uint8_t)ch };
                Log(LogLevel::INFO,
                    "Port[%i] Rx CH[%i] %s, SR: %.3f MHz BW: %.3f MHz | chipIndex: %i, path: %i('%s')\n",
                    p,
                    ch,
                    loFreqStr,
                    samplingRate / 1e6,
                    hostState->rx_bandwidth[rxChannelOffset + ch] / 1e6,
                    lime->chipIndex[p],
                    lime->rxPath[p],
                    paths[lime->rxPath[p]].c_str());
                config.channel[ch].rx.enabled = true;
                config.channel[ch].rx.centerFrequency = freq;
                config.channel[ch].rx.path = lime->rxPath[p];
                // if (lime->rxGainNorm[p] >= 0 )
                //     config.channel[ch].rxGainNormalized = lime->rxGainNorm[p];
                config.channel[ch].rx.gfir.enabled = lime->rx_gfir_enable[p];
                if (lime->rx_gfir_bandwidth[p] > 0)
                    config.channel[ch].rx.gfir.bandwidth = lime->rx_gfir_bandwidth[p];
                else
                    config.channel[ch].rx.gfir.bandwidth = hostState->rx_bandwidth[rxChannelOffset + ch];

                if (lime->calibrate[p] & DCIQ)
                    config.channel[ch].rx.calibrate = true;
                if (lime->calibrate[p] & Filter)
                {
                    config.channel[ch].rx.lpf = hostState->rx_bandwidth[rxChannelOffset + ch];
                    Log(LogLevel::DEBUG, "Port[%i][%i] rxLPF:%.3f MHz\n", p, ch, config.channel[ch].rx.lpf / 1e6);
                }
            }

            for (int ch = 0; ch < lime->tx_channel_count[p]; ++ch)
            {
                auto paths = portDevice->GetDescriptor().rfSOC[lime->chipIndex[p]].txPathNames;
                double freq = hostState->tx_freq[txChannelOffset + ch];
                char loFreqStr[1024];
                if (lime->tx_LO_override[p] > 0)
                {
                    freq = lime->tx_LO_override[p];
                    sprintf(loFreqStr,
                        "expectedLO: %.3f MHz [override: %.3f (diff:%+.3f) MHz]",
                        hostState->tx_freq[txChannelOffset + ch] / 1.0e6,
                        lime->tx_LO_override[p] / 1.0e6,
                        (hostState->tx_freq[txChannelOffset + ch] - lime->tx_LO_override[p]) / 1.0e6);
                }
                else
                    sprintf(loFreqStr, "LO: %.3f MHz", freq / 1.0e6);
                gMapTxChannelToPortCh[txChannelOffset + ch] = { (uint8_t)p, (uint8_t)ch };
                Log(LogLevel::INFO,
                    "Port[%i] Tx CH[%i] %s, SR: %.3f MHz BW: %.3f MHz | chipIndex: %i, path: %i('%s')\n",
                    p,
                    ch,
                    loFreqStr,
                    samplingRate / 1.0e6,
                    hostState->tx_bandwidth[txChannelOffset + ch] / 1.0e6,
                    lime->chipIndex[p],
                    lime->txPath[p],
                    paths[lime->txPath[p]].c_str());
                config.channel[ch].tx.enabled = true;
                config.channel[ch].tx.centerFrequency = freq;
                config.channel[ch].tx.path = lime->txPath[p];

                config.channel[ch].tx.gfir.enabled = lime->tx_gfir_enable[p];
                if (lime->tx_gfir_bandwidth[p] > 0)
                    config.channel[ch].tx.gfir.bandwidth = lime->tx_gfir_bandwidth[p];
                else
                    config.channel[ch].tx.gfir.bandwidth = hostState->tx_bandwidth[txChannelOffset + ch];
                // if (lime->txGainNorm[p] >= 0 )
                //     config.channel[ch].txGainNormalized = lime->txGainNorm[p];

                if (lime->calibrate[p] & DCIQ)
                    config.channel[ch].tx.calibrate = true;
                if (lime->calibrate[p] & Filter)
                {
                    config.channel[ch].tx.lpf = hostState->tx_bandwidth[txChannelOffset + ch];
                    Log(LogLevel::DEBUG, "Port[%i][%i] txLPF:%.3f MHz\n", p, ch, config.channel[ch].tx.lpf / 1e6);
                }
            }

            try
            {
                config.skipDefaults = lime->skipConfig[p];
                portDevice->Configure(config, lime->chipIndex[p]);

                for (int c = 0; c < hostState->rx_channel_count; ++c)
                    trx_lms7002m_set_rx_gain_func(s1, hostState->rx_gain[c], c);

                for (int c = 0; c < hostState->tx_channel_count; ++c)
                    trx_lms7002m_set_tx_gain_func(s1, hostState->tx_gain[c], c);

                // override gains
                LMS7002M* chip = static_cast<LMS7002M*>(portDevice->GetInternalChip(lime->chipIndex[p]));
                for (int mac = 1; mac <= 2; ++mac)
                {
                    chip->Modify_SPI_Reg_bits(LMS7_MAC, mac);
                    if (lime->glna[p] >= 0)
                        chip->Modify_SPI_Reg_bits(LMS7_G_LNA_RFE, lime->glna[p]);
                    if (lime->gpga[p] >= 0)
                        chip->Modify_SPI_Reg_bits(LMS7_G_PGA_RBB, lime->gpga[p]);
                    if (lime->iamp[p] >= 0)
                        chip->Modify_SPI_Reg_bits(LMS7_CG_IAMP_TBB, lime->iamp[p]);
                    if (lime->lossmain[p] >= 0)
                        chip->Modify_SPI_Reg_bits(LMS7_LOSS_MAIN_TXPAD_TRF, lime->lossmain[p]);
                }
                chip->Modify_SPI_Reg_bits(LMS7_MAC, 1);

                if (lime->writeRegisters[p].size() > 0)
                {
                    const auto slaves = lime->device[p]->GetDescriptor().spiSlaveIds;
                    lime->device[p]->SPI(
                        slaves.at("FPGA"), lime->writeRegisters[p].data(), nullptr, lime->writeRegisters[p].size());
                }
            } catch (std::logic_error& e)
            {
                Log(LogLevel::ERROR, "Port%i configure failed: %s\n", p, e.what());
                return -1;
            } catch (std::runtime_error& e)
            {
                Log(LogLevel::ERROR, "Port%i configure failed: %s\n", p, e.what());
                return -1;
            }

            SDRDevice::StreamConfig& stream = lime->streamCfg[p];
            stream.rxCount = lime->rx_channel_count[p];
            stream.txCount = lime->tx_channel_count[p];
            stream.linkFormat = lime->linkFormat[p];
            stream.format = lime->samplesFormat;

            // Initialize streams and map channels
            for (int ch = 0; ch < stream.rxCount; ++ch)
                stream.rxChannels[ch] = ch;
            for (int ch = 0; ch < stream.txCount; ++ch)
                stream.txChannels[ch] = ch;

            stream.statusCallback = OnStreamStatusChange;
            stream.userData = (void*)&portStreamStates[p];
            stream.hintSampleRate = samplingRate;

            stream.extraConfig = lime->streamExtras[p] ? lime->streamExtras[p] : nullptr;

            lime->samplesInPacket[p] = 256;
            Log(LogLevel::DEBUG,
                "Port[%i] Stream samples format: %s , link: %s\n",
                p,
                stream.format == SDRDevice::StreamConfig::DataFormat::F32 ? "F32" : "I16",
                stream.linkFormat == SDRDevice::StreamConfig::DataFormat::I12 ? "I12" : "I16");
            if (portDevice->StreamSetup(stream, lime->chipIndex[p]) != 0)
            {
                Log(LogLevel::ERROR, "Port%i stream setup failed.\n", p);
                return -1;
            }

            rxChannelOffset += hostState->rx_port_channel_count[p];
            txChannelOffset += hostState->tx_port_channel_count[p];
        }

        for (int p = 0; p < hostState->rf_port_count; ++p)
            lime->device[p]->StreamStart(lime->chipIndex[p]);
        startTime = std::chrono::high_resolution_clock::now();

    } // try
    catch (std::logic_error& e)
    {
        Log(LogLevel::ERROR, "logic_error: %s\n", e.what());
        return -1;
    } catch (std::runtime_error& e)
    {
        Log(LogLevel::ERROR, "runtime_error: %s\n", e.what());
        return -1;
    }
    return 0;
}

static void trx_lms7002m_end(TRXState* s1)
{
    LimeState* lime = (LimeState*)s1->opaque;
    for (int p = 0; p < lime->deviceCount; ++p)
    {
        SDRDevice* portDevice = lime->device[p];
        portDevice->StreamStop(lime->chipIndex[p]);
        DeviceRegistry::freeDevice(portDevice);
    }
    free(lime);
}

// Driver initialization called at eNB startup
int __attribute__((visibility("default"))) trx_driver_init(TRXState* hostState)
{
    double val;
    if (hostState->trx_api_version != TRX_API_VERSION)
    {
        fprintf(stderr,
            "ABI compatibility mismatch between LTEENB and TRX driver (LTEENB ABI version=%d, TRX driver ABI version=%d)\n",
            hostState->trx_api_version,
            TRX_API_VERSION);
        return -1;
    }

    LimeState* s = new LimeState();
    if (trx_get_param_double(hostState, &val, "logLevel") >= 0)
        logVerbosity = (LogLevel)min((int)val, (int)LogLevel::DEBUG);

    try
    {
        // Enumerate devices
        std::vector<DeviceHandle> devHandles = DeviceRegistry::enumerate();
        if (devHandles.size() == 0)
        {
            fprintf(stderr, "No LMS7002M boards found!\n");
            return -1;
        }
        else
        {
            Log(LogLevel::DEBUG, "Available LMS7002M devices:\n");
            for (const auto& dev : devHandles)
                Log(LogLevel::DEBUG, "\t\"%s\"\n", dev.Serialize().c_str());
        }

        // Connect and initialize
        for (int i = 0; i < TRX_MAX_RF_PORT; ++i)
        {
            char varname[128];
            sprintf(varname, "port%i", i);
            char* deviceHandleStr = trx_get_param_string(hostState, varname);
            if (deviceHandleStr == nullptr)
                continue;

            Log(LogLevel::DEBUG, "Port[%i] specified handle: %s\n", i, deviceHandleStr);
            std::vector<DeviceHandle> fullHandles = DeviceRegistry::enumerate(DeviceHandle(deviceHandleStr));
            SDRDevice* device = DeviceRegistry::makeDevice(fullHandles.at(0));
            free(deviceHandleStr);

            if (device != nullptr)
                Log(LogLevel::INFO, "Port[%i] connected: %s\n", i, fullHandles.at(0).Serialize().c_str());
            else
            {
                Log(LogLevel::ERROR, "Port[%i] failed to connect: %s\n", i, fullHandles.at(0).Serialize().c_str());
                return -1;
            }
            s->device[i] = device;
            s->deviceCount++;
        }

        // if arguments did not have device assignments to ports
        // use first available device for port0
        if (s->deviceCount == 0)
        {
            Log(LogLevel::DEBUG, "Port[0] use: %s\n", devHandles.at(0).Serialize().c_str());
            SDRDevice* device = DeviceRegistry::makeDevice(devHandles.at(0));
            if (device == nullptr)
            {
                Log(LogLevel::ERROR, "Port[0] failed to connect: %s\n", devHandles.at(0).Serialize().c_str());
                return -1;
            }
            s->device[0] = device;
            s->deviceCount++;
        }

        // Initialize devices to default settings
        for (int p = 0; p < s->deviceCount; ++p)
        {
            s->device[p]->SetMessageLogCallback(LogCallback);
            s->device[p]->Init();
        }

        // Load configuration files for each chip if specified
        // PORTS can have assigned the same SDR device, so all devices reset/initialization
        // has to be done before setting up parameters, as they might get lost during initialization

        // Parse device channels mapping to stack's PORTS
        for (int p = 0; p < TRX_MAX_RF_PORT && p < s->deviceCount; ++p)
        {
            char varname[128];
            double index = 0;

            sprintf(varname, "port%i_chipIndex", p);
            if (trx_get_param_double(hostState, &index, varname) == 0)
                s->chipIndex[p] = index;
            else
                s->chipIndex[p] = 0;

            auto desc = s->device[p]->GetDescriptor();
            if (s->chipIndex[p] >= desc.rfSOC.size())
            {
                Log(LogLevel::ERROR,
                    "Invalid chipIndex(%i). Port%i device has only %i chips\n",
                    s->chipIndex[p],
                    p,
                    desc.rfSOC.size());
                return -1;
            }

            sprintf(varname, "port%i_ini", p);
            char* filename = trx_get_param_string(hostState, varname);
            if (filename)
            {
                LMS7002M* chip = static_cast<LMS7002M*>(s->device[p]->GetInternalChip(s->chipIndex[p]));

                char configFilepath[512];
                if (filename[0] != '/') // is not global path
                    sprintf(configFilepath, "%s/%s", hostState->path, filename);
                else
                    sprintf(configFilepath, "%s", filename);

                if (chip->LoadConfig(configFilepath, false) != 0)
                {
                    Log(LogLevel::ERROR, "Error loading file: %s\n", filename);
                    return -1;
                }
                s->skipConfig[p] = true;
                Log(LogLevel::INFO, "Port[%i] : LMS7002M[%i] loaded with: %s\n", p, s->chipIndex[p], filename);
                free(filename);
            }

            sprintf(varname, "port%i_rxPath", p);
            char* rxPathString = trx_get_param_string(hostState, varname);
            s->rxPath[p] = 0;
            if (rxPathString)
            {
                bool match = false;
                auto paths = desc.rfSOC[s->chipIndex[p]].rxPathNames;
                for (uint j = 0; j < paths.size(); ++j)
                {
                    if (strcasecmp(paths[j].c_str(), rxPathString) == 0)
                    {
                        s->rxPath[p] = j;
                        match = true;
                        break;
                    }
                }
                if (!match)
                {
                    Log(LogLevel::ERROR, "%s %s not found. Available: ", varname, rxPathString);
                    for (auto iter : paths)
                        Log(LogLevel::ERROR, "\"%s\" ", iter.c_str());
                    Log(LogLevel::ERROR, "\n");
                    return -1;
                }
                free(rxPathString);
            }

            sprintf(varname, "port%i_txPath", p);
            char* txPathString = trx_get_param_string(hostState, varname);
            s->txPath[p] = 0;
            if (txPathString)
            {
                bool match = false;
                auto paths = desc.rfSOC[s->chipIndex[p]].txPathNames;
                for (uint j = 0; j < paths.size(); ++j)
                {
                    if (strcasecmp(paths[j].c_str(), txPathString) == 0)
                    {
                        s->txPath[p] = j;
                        match = true;
                        break;
                    }
                }
                if (!match)
                {
                    Log(LogLevel::ERROR, "%s %s not found. Available: ", varname, txPathString);
                    for (auto iter : paths)
                        Log(LogLevel::ERROR, "\"%s\" ", iter.c_str());
                    Log(LogLevel::ERROR, "\n");
                    return -1;
                }
                free(txPathString);
            }
        }

        // load power settings
        for (int p = 0; p < TRX_MAX_RF_PORT; ++p)
        {
            char varname[128];
            sprintf(varname, "port%i_rx_lo_override", p);
            s->rx_LO_override[p] = 0;
            if (trx_get_param_double(hostState, &s->rx_LO_override[p], varname) == 0)
                Log(LogLevel::INFO, "%s %.2f\n", varname, s->rx_LO_override[p]);

            sprintf(varname, "port%i_tx_lo_override", p);
            s->tx_LO_override[p] = 0;
            if (trx_get_param_double(hostState, &s->tx_LO_override[p], varname) == 0)
                Log(LogLevel::INFO, "%s %.2f\n", varname, s->tx_LO_override[p]);

            sprintf(varname, "port%i_rx_gainNormalized", p); // provided value should be [0:1]
            s->rxGainNorm[p] = -1; //nanf("");
            if (trx_get_param_double(hostState, &val, varname) == 0)
            {
                if (val < 0 || val > 1)
                {
                    val = std::min(std::max(0.0, val), 1.0);
                    Log(LogLevel::WARNING, "%s out of range, clamping to %g", varname, val);
                }
                s->rxGainNorm[p] = val;
                Log(LogLevel::INFO, "%s %.2f\n", varname, s->rxGainNorm[p]);
            }
            sprintf(varname, "port%i_tx_gainNormalized", p);
            s->txGainNorm[p] = -1; //nanf("");
            if (trx_get_param_double(hostState, &val, varname) == 0)
            {
                if (val < 0 || val > 1)
                {
                    val = std::min(std::max(0.0, val), 1.0);
                    Log(LogLevel::WARNING, "%s out of range, clamping to %g", varname, val);
                }
                s->txGainNorm[p] = val;
                Log(LogLevel::INFO, "%s %.2f\n", varname, s->txGainNorm[p]);
            }

            s->glna[p] = -1;
            s->gpga[p] = -1;
            s->iamp[p] = -1;
            s->lossmain[p] = -1;
            sprintf(varname, "port%i_G_LNA_RFE", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
                s->glna[p] = val;
            sprintf(varname, "port%i_G_PGA_RBB", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
                s->gpga[p] = val;
            sprintf(varname, "port%i_CG_IAMP_TBB", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
                s->iamp[p] = val;
            sprintf(varname, "port%i_LOSS_MAIN_TXPAD_TRF", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
                s->lossmain[p] = val;

            sprintf(varname, "port%i_rx_oversample", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
                s->rxOversample[p] = val;

            sprintf(varname, "port%i_tx_oversample", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
                s->txOversample[p] = val;

            // calibrations setup
            for (int p = 0; p < s->deviceCount; ++p)
                s->device[p]->EnableCache(false);
            s->calibrate[p] = CalibrateFlag::Filter; // by default calibrate only filters
            sprintf(varname, "port%i_calibration", p);
            char* calibration = trx_get_param_string(hostState, varname);
            if (calibration)
            {
                if (!strcasecmp(calibration, "none"))
                    s->calibrate[p] = 0;
                else if ((!strcasecmp(calibration, "force")) || (!strcasecmp(calibration, "all")))
                    s->calibrate[p] = CalibrateFlag::Filter | CalibrateFlag::DCIQ;
                else if (!strcasecmp(calibration, "filter"))
                    s->calibrate[p] = CalibrateFlag::Filter;
                else if (!strcasecmp(calibration, "dciq"))
                    s->calibrate[p] = CalibrateFlag::DCIQ;
                free(calibration);
            }

            sprintf(varname, "port%i_linkFormat", p);
            char* linkFormat = trx_get_param_string(hostState, varname);
            if (linkFormat)
            {
                if (strcasecmp(linkFormat, "I16") == 0)
                    s->linkFormat[p] = lime::SDRDevice::StreamConfig::I16;
                else if (strcasecmp(linkFormat, "I12") == 0)
                    s->linkFormat[p] = lime::SDRDevice::StreamConfig::I12;
                else
                {
                    Log(LogLevel::WARNING, "Port[%i} Invalid link format (%s): falling back to I12\n", p, linkFormat);
                    s->linkFormat[p] = lime::SDRDevice::StreamConfig::I12;
                }
                free(linkFormat);
            }
            else
                s->linkFormat[p] = lime::SDRDevice::StreamConfig::I12;

            // absolute gain info
            for (int ch = 0; ch < TRX_MAX_CHANNELS; ++ch)
            {
                sprintf(varname, "port%i_ch%i_rxPower_dBm", p, ch);
                double power_dBm;
                if (trx_get_param_double(hostState, &power_dBm, varname) == 0)
                {
                    s->rxPowerAvailable[p][ch] = true;
                    s->rxPower_dBm[p][ch] = power_dBm;
                }
                sprintf(varname, "port%i_ch%i_txPower_dBm", p, ch);
                if (trx_get_param_double(hostState, &power_dBm, varname) == 0)
                {
                    s->txPowerAvailable[p][ch] = true;
                    s->txPower_dBm[p][ch] = power_dBm;
                }

                double dac = 0;
                sprintf(varname, "port%i_ch%i_pa_dac", p, ch);
                if (trx_get_param_double(hostState, &dac, varname) == 0)
                {
                    // TODO: this is board specific, need general API
                    int32_t paramId = 2 + ch;
                    std::string units = "";
                    s->device[p]->CustomParameterWrite(paramId, dac, units);
                }
            }

            sprintf(varname, "port%i_rx_gfir_enable", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
                s->rx_gfir_enable[p] = (val != 0);

            sprintf(varname, "port%i_rx_gfir_bandwidth", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
                s->rx_gfir_bandwidth[p] = val;

            sprintf(varname, "port%i_tx_gfir_enable", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
                s->tx_gfir_enable[p] = (val != 0);

            sprintf(varname, "port%i_tx_gfir_bandwidth", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
                s->tx_gfir_bandwidth[p] = val;

            sprintf(varname, "port%i_writeRegisters", p);
            char* writeRegisters = trx_get_param_string(hostState, varname);
            if (writeRegisters)
            {
                char* token = strtok(writeRegisters, ";");
                while (token)
                {
                    uint32_t spiVal = 0;
                    sscanf(token, "%X", &spiVal);
                    s->writeRegisters[p].push_back(spiVal | (1 << 31)); // adding spi write bit for convenience
                    token = strtok(NULL, ";");
                }
                free(writeRegisters);
            }

            SDRDevice::StreamConfig::Extras* extra = new SDRDevice::StreamConfig::Extras();

            sprintf(varname, "port%i_syncPPS", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
            {
                extra->waitPPS = val != 0;
                s->streamExtras[p] = extra;
            }
            sprintf(varname, "port%i_usePoll", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
            {
                extra->usePoll = val != 0;
                s->streamExtras[p] = extra;
            }
            sprintf(varname, "port%i_rxSamplesInPacket", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
            {
                extra->rxSamplesInPacket = val;
                s->streamExtras[p] = extra;
            }
            sprintf(varname, "port%i_rxPacketsInBatch", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
            {
                extra->rxPacketsInBatch = val;
                s->streamExtras[p] = extra;
            }
            sprintf(varname, "port%i_txMaxPacketsInBatch", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
            {
                extra->txMaxPacketsInBatch = val;
                s->streamExtras[p] = extra;
            }
            sprintf(varname, "port%i_txSamplesInPacket", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
            {
                extra->txSamplesInPacket = val;
                s->streamExtras[p] = extra;
            }
            sprintf(varname, "port%i_double_freq_conversion_to_lower_side", p);
            if (trx_get_param_double(hostState, &val, varname) == 0)
            {
                extra->negateQ = val;
                s->streamExtras[p] = extra;
            }
        }

        // TODO: right now no need to specify samples format, as only floating point is supported by Amarisoft
        // char* samplesFormat = trx_get_param_string(hostState, "samplesFormat");
        // if (samplesFormat)
        // {
        //     printf("Config format: %s\n", samplesFormat);
        //     if (strcasecmp(samplesFormat, "I16")  == 0)
        //         s->samplesFormat = lime::SDRDevice::StreamConfig::I16;
        //     else if (strcasecmp(samplesFormat, "F32")  == 0)
        //         s->samplesFormat = lime::SDRDevice::StreamConfig::F32;
        //     else
        //     {
        //         printf("Invalid samples format (%s): falling back to F32\n", samplesFormat);
        //         s->samplesFormat = lime::SDRDevice::StreamConfig::F32;
        //     }
        //     free(samplesFormat);
        // }
        // else
        s->samplesFormat = lime::SDRDevice::StreamConfig::F32;

    } catch (std::logic_error& e)
    {
        fprintf(stderr, "Logic error: %s\n", e.what());
        return -1;
    } catch (std::runtime_error& e)
    {
        fprintf(stderr, "Runtime error: %s\n", e.what());
        return -1;
    }

    // Set callbacks
    hostState->opaque = s;
    hostState->trx_end_func = trx_lms7002m_end;
    // hostState->trx_write_func = // Deprecated
    // hostState->trx_read_func = // Deprecated
    hostState->trx_write_func2 = trx_lms7002m_write;
    hostState->trx_read_func2 = trx_lms7002m_read;
    hostState->trx_start_func = trx_lms7002m_start;
    hostState->trx_get_sample_rate_func = trx_lms7002m_get_sample_rate;
    hostState->trx_get_tx_samples_per_packet_func = trx_lms7002m_get_tx_samples_per_packet_func;
    hostState->trx_get_abs_rx_power_func = trx_lms7002m_get_abs_rx_power_func;
    hostState->trx_get_abs_tx_power_func = trx_lms7002m_get_abs_tx_power_func;
    hostState->trx_set_tx_gain_func = trx_lms7002m_set_tx_gain_func;
    hostState->trx_set_rx_gain_func = trx_lms7002m_set_rx_gain_func;
    hostState->trx_get_stats = trx_lms7002m_get_stats;
    hostState->trx_dump_info = trx_lms7002m_dump_info;
    return 0;
}
