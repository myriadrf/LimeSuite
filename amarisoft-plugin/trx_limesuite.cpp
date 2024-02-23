/*
 * LimeMicroSystem transceiver driver
 * Copyright (C) 2015-2018 Amarisoft/LimeMicroSystems
 */

#include "gainTable.h"
#include "common.h"

#include <stdarg.h>

typedef lime::SDRDevice::LogLevel LogLevel;

extern "C" {
#include "trx_driver.h"
}

using namespace std;
using namespace lime;

class AmarisoftParamProvider : public LimeParamProvider
{
public:
    AmarisoftParamProvider() : state(nullptr), blockAccess(false) {}

    void Init(TRXState* s)
    {
        state = s;
    }

    void Block()
    {
        blockAccess = true;
    }

    virtual bool GetString(std::string& dest, const char* varname) override
    {
        if (blockAccess)
        {
            return false;
        }
        char* ctemp = trx_get_param_string(state, varname);
        if (!ctemp)
            return false;

        dest = std::string(ctemp);
        free(ctemp);
        return true;
    }

    virtual bool GetDouble(double& dest, const char* varname) override
    {
        if (blockAccess)
        {
            return false;
        }
        return trx_get_param_double(state, &dest, varname) == 0 ? true : false;
    }
private:
    TRXState* state;
    bool blockAccess;
};

static lime::SDRDevice::LogLevel logVerbosity = lime::SDRDevice::LogLevel::DEBUG;
static void Log(SDRDevice::LogLevel lvl, const char* format, ...)
{
    if (lvl > logVerbosity)
        return;
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

static void LogCallback(SDRDevice::LogLevel lvl, const char* msg)
{
    if (lvl > logVerbosity)
        return;
    printf("%s\n", msg);
}

//static TRXStatistics trxstats;
static int trx_lms7002m_get_stats(TRXState* s, TRXStatistics* m)
{
    //LimePluginContext* lime = (LimePluginContext*)s->opaque;
    // TODO:
    // for (long p = 0; p < lime->deviceCount; ++p)
    // {
    //     StreamStatus& stats = portStreamStates[p];
    //     trxstats.rx_overflow_count = stats.rx.overrun + stats.rx.loss;
    //     trxstats.tx_underflow_count = stats.tx.late + stats.tx.underrun;
    // }
    // memcpy(m, &trxstats, sizeof(TRXStatistics));
    return 0;
}

/* Callback must allocate info buffer that will be displayed */
static void trx_lms7002m_dump_info(TRXState* s, trx_printf_cb cb, void* opaque)
{
    // LimePluginContext* lime = (LimePluginContext*)s->opaque;
    // std::stringstream ss;
    /*    for (long p = 0; p < lime->deviceCount; ++p)
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
           << " linkFormat: " << (lime->linkFormat[p] == SDRDevice::StreamConfig::DataFormat::I16 ? "I16" : "I12") << std::endl;
        // TODO: read FIFO usage
    }*/
    // const int len = ss.str().length();
    // char* buffer = (char*)malloc(len + 1);
    // cb(buffer, "%s\n", ss.str().c_str());
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
    LimePluginContext* lime = (LimePluginContext*)s->opaque;
    LimePlugin_Write_complex32f(lime, reinterpret_cast<const lime::complex32f_t* const*>(samples), count, port, meta);
}

// Read has to block until at least 1 sample is available and must return at most 'count' samples
// 'count' argument value depends on sample rate and is at most 4096 samples.
// timestamp is samples counter
// return number of samples produced
static int trx_lms7002m_read(TRXState* s, trx_timestamp_t* ptimestamp, void** samples, int count, int port, TRXReadMetadata* md)
{
    SDRDevice::StreamMeta meta;
    meta.useTimestamp = false;
    meta.flush = false;
    md->flags = 0;

    LimePluginContext* lime = (LimePluginContext*)s->opaque;
    int samplesGot = LimePlugin_Read_complex32f(lime, reinterpret_cast<lime::complex32f_t**>(samples), count, port, meta);
    *ptimestamp = meta.timestamp; // if timestamp is not updated, amarisoft will freeze
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
    LimePluginContext* s = static_cast<LimePluginContext*>(s1->opaque);
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
    RFNode* node = s->ports[p].nodes[0];
    if (!node || !node->device)
        return -1;
    double& rate = node->config.channel[0].rx.sampleRate;
    whichPort = (whichPort + 1); // mod, just in case.

    const float desiredSamplingRate = bandwidth * 1.536;
    //printf ("Required bandwidth:[%u], current sample_rate [%u]", bandwidth, s->sample_rate);
    if (rate <= 0) // sample rate not specified, align on 1.92Mhz
    {
        Log(LogLevel::VERBOSE,
            "Port[%i] Trying sample rates which are bandwidth(%u) * 1.536 = %f",
            p,
            bandwidth,
            desiredSamplingRate);
        for (uint32_t i = 0; i < sizeof(multipliers) / sizeof(uint8_t); ++i)
        {
            int n = multipliers[i];
            if (n * 1920000 > 122.88e6)
                break;
            Log(LogLevel::DEBUG, "\tPort[%i] Trying sample rate : bandwidth(%u) sample_rate(%u)", p, bandwidth, n * 1920000);
            if (desiredSamplingRate <= n * 1920000)
            {
                *psample_rate_num = n;
                psample_rate->num = n * 1920000;
                psample_rate->den = 1;
                rate = psample_rate->num;
                Log(LogLevel::INFO, "Port[%i] Automatic sample rate: %g MSps, n = [%u] * 1.92e6", p, rate / 1e6, n);
                return 0;
            }
        }
        Log(LogLevel::VERBOSE, "Port[%i] Trying sample rates which are close to bandwidth(%u)", p, bandwidth);
        for (uint32_t i = 0; i < sizeof(multipliers) / sizeof(uint8_t); ++i)
        {
            int n = multipliers[i];
            Log(LogLevel::DEBUG, "\tPort[%i] Trying sample rate : bandwidth(%u) sample_rate(%u)", p, bandwidth, n * 1920000);
            if (bandwidth <= n * 1920000)
            {
                *psample_rate_num = n;
                psample_rate->num = n * 1920000;
                psample_rate->den = 1;
                rate = psample_rate->num;
                Log(LogLevel::INFO, "Port[%i] Automatic sample rate: %g MSps, n = [%u] * 1.92e6", p, rate / 1e6, n);
                return 0;
            }
        }
        Log(LogLevel::ERROR, "Port[%i] Could not find suitable sampling rate for %i bandwidth", p, bandwidth);
    }
    else
    {
        if (rate < bandwidth)
        {
            Log(LogLevel::ERROR, "Port[%i] Manually specified sample rate %i is less than LTE bandwidth %i", p, rate, bandwidth);
            return -1;
        }
        Log(LogLevel::INFO, "Port[%i] Manually specified sample rate: %f MSps", p, rate / 1e6);
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
    // LimePluginContext* lime = static_cast<LimePluginContext*>(s1->opaque);
    int txExpectedSamples = 256;//lime->samplesInPacket[0];
    // if (lime->streamExtras[0] && lime->streamExtras[0]->txSamplesInPacket > 0)
    // {
    //     txExpectedSamples = lime->streamExtras[0]->txSamplesInPacket;
    // }
    // Log(LogLevel::DEBUG, "Hardware expected samples count in Tx packet : %i", txExpectedSamples);
    return txExpectedSamples;
}

static int trx_lms7002m_get_abs_rx_power_func(TRXState* s1, float* presult, int channel_num)
{
    LimePluginContext* lime = static_cast<LimePluginContext*>(s1->opaque);
    if (lime->rxChannels[channel_num].parent->powerAvailable)
    {
        *presult = lime->rxChannels[channel_num].parent->power_dBm;
        return 0;
    }
    else
        return -1;
}

static int trx_lms7002m_get_abs_tx_power_func(TRXState* s1, float* presult, int channel_num)
{
    LimePluginContext* lime = static_cast<LimePluginContext*>(s1->opaque);
    if (lime->txChannels[channel_num].parent->powerAvailable)
    {
        *presult = lime->txChannels[channel_num].parent->power_dBm;
        return 0;
    }
    else
        return -1;
}

//min gain 0
//max gain ~70-76 (higher will probably degrade signal quality to much)
static void trx_lms7002m_set_tx_gain_func(TRXState* s1, double gain, int channel_num)
{
    LimePluginContext* lime = static_cast<LimePluginContext*>(s1->opaque);
    lms7002m_set_tx_gain_func(lime, gain, channel_num);
}

static void trx_lms7002m_set_rx_gain_func(TRXState* s1, double gain, int channel_num)
{
    LimePluginContext* lime = static_cast<LimePluginContext*>(s1->opaque);
    lms7002m_set_rx_gain_func(lime, gain, channel_num);
}

static int trx_lms7002m_start(TRXState* s1, const TRXDriverParams* params)
{
    LimePluginContext* lime = static_cast<LimePluginContext*>(s1->opaque);

    LimeParams state;

    int rxCount = params->rx_channel_count;
    int txCount = params->tx_channel_count;

    CopyCArrayToVector(state.rx.freq, params->rx_freq, rxCount);
    CopyCArrayToVector(state.tx.freq, params->tx_freq, txCount);

    CopyCArrayToVector(state.rx.gain, params->rx_gain, rxCount);
    CopyCArrayToVector(state.tx.gain, params->tx_gain, txCount);

    CopyCArrayToVector(state.rx.bandwidth, params->rx_bandwidth, rxCount);
    CopyCArrayToVector(state.tx.bandwidth, params->tx_bandwidth, txCount);

    for (int i=0; i<params->rf_port_count; ++i)
    {
        double sample_rate = (double)params->sample_rate[i].num / params->sample_rate[i].den;
        state.rf_ports.push_back({sample_rate, params->rx_port_channel_count[i], params->tx_port_channel_count[i]});
    }

    int status = LimePlugin_Setup(lime, &state);
    if (status != 0)
        return status;
    return LimePlugin_Start(lime);
}

static void trx_lms7002m_end(TRXState* s1)
{
    LimePluginContext* lime = static_cast<LimePluginContext*>(s1->opaque);
    LimePlugin_Destroy(lime);
    delete lime;
}

AmarisoftParamProvider configProvider;

// Driver initialization called at eNB startup
int __attribute__((visibility("default"))) trx_driver_init(TRXState* hostState)
{
    if (hostState->trx_api_version != TRX_API_VERSION)
    {
        fprintf(stderr,
            "ABI compatibility mismatch between LTEENB and TRX driver (LTEENB ABI version=%d, TRX driver ABI version=%d)\n",
            hostState->trx_api_version,
            TRX_API_VERSION);
        return -1;
    }

    LimePluginContext* lime = new LimePluginContext();
    lime->currentWorkingDirectory = std::string(hostState->path);
    lime->samplesFormat = SDRDevice::StreamConfig::DataFormat::F32;
    configProvider.Init(hostState);

    if (LimePlugin_Init(lime, LogCallback, &configProvider) != 0)
        return -1;

    // Set callbacks
    hostState->opaque = lime;
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

    configProvider.Block(); // config parameters access is only allow within trx_driver_init
    return 0;
}
