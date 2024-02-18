#include "common.h"
#include "gainTable.h"

#include <assert.h>
#include <chrono>
#include <iostream>
#include <math.h>
#include <mutex>
#include <sstream>

#include <limesuite/LMS7002M.h>
#include <limesuite/DeviceRegistry.h>

using namespace lime;
using namespace std;

static constexpr int LIME_MAX_UNIQUE_DEVICES = 16;
static constexpr int LIME_TRX_MAX_RF_PORT = 16;

typedef lime::SDRDevice::LogLevel LogLevel;

struct StreamStatus {
    lime::SDRDevice::StreamStats rx;
    lime::SDRDevice::StreamStats tx;
};

static std::mutex gainsMutex;
static std::array<StreamStatus, LIME_TRX_MAX_RF_PORT> portStreamStates;

static HostLogCallbackType hostCallback = nullptr;

RFNode::RFNode()
    : device(nullptr)
{
}

LimePluginContext::LimePluginContext()
    : rfdev(LIME_MAX_UNIQUE_DEVICES)
    , config(nullptr)
    , currentWorkingDirectory(nullptr)
{
    ports.reserve(LIME_TRX_MAX_RF_PORT);
}

enum CalibrateFlag { None = 0, DCIQ = 1, Filter = 2 };

template<class T>
static bool GetParam(LimePluginContext *context, T& pval, const char* prop_name_format, ...)
{
    char name[256];
    va_list args;
    va_start(args, prop_name_format);
    vsprintf(name, prop_name_format, args);
    va_end(args);

    double val = 0;
    if (!context->config->GetDouble(val, name))
        return false;
    pval = val;
    return true;
}

template<>
bool GetParam(LimePluginContext *context, std::string& pval, const char* prop_name_format, ...)
{
    char name[256];
    va_list args;
    va_start(args, prop_name_format);
    vsprintf(name, prop_name_format, args);
    va_end(args);

    return context->config->GetString(pval, name);
}

static lime::SDRDevice::LogLevel logVerbosity = lime::SDRDevice::LogLevel::DEBUG;

static void Log(SDRDevice::LogLevel lvl, const char* format, ...)
{
    if (lvl > logVerbosity)
        return;
    char msg[512];
    va_list args;
    va_start(args, format);
    vsprintf(msg, format, args);
    va_end(args);
    if (hostCallback)
        hostCallback(lvl, msg);
}

static void LogCallback(SDRDevice::LogLevel lvl, const char* msg)
{
    if (lvl > logVerbosity)
        return;
    if (hostCallback)
        hostCallback(lvl, msg);
}

static auto lastStreamUpdate = std::chrono::steady_clock::now();
bool OnStreamStatusChange(bool isTx, const SDRDevice::StreamStats* s, void* userData)
{
    StreamStatus& status = *static_cast<StreamStatus*>(userData);
    SDRDevice::StreamStats& dest = isTx ? status.tx : status.rx;

    dest.FIFO = s->FIFO;
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
        stringstream ss;
        ss << "Rx| Loss: " << status.rx.loss << " overrun: " << status.rx.overrun << " rate: " << status.rx.dataRate_Bps / 1e6
             << " MB/s"
             << "\nTx| Late: " << status.tx.late << " underrun: " << status.tx.underrun << " rate: " << status.tx.dataRate_Bps / 1e6
             << " MB/s";
        Log(LogLevel::WARNING, ss.str().c_str());
        lastStreamUpdate = now;
    }
    return false;
}

//min gain 0
//max gain ~70-76 (higher will probably degrade signal quality to much)
void lms7002m_set_tx_gain_func(LimePluginContext* context, double gain, int channel_num)
{
    int row = gain;
    if (row < 0 || row >= (int)(sizeof(txGainTable) / sizeof(TxGainRow)))
        return;

    std::lock_guard<std::mutex> lk(gainsMutex);

    SDRDevice* device = context->txChannels[channel_num].parent->device;
    LMS7002M* chip = static_cast<LMS7002M*>(device->GetInternalChip(context->txChannels[channel_num].parent->chipIndex));
    chip->Modify_SPI_Reg_bits(LMS7_MAC, context->txChannels[channel_num].chipChannel + 1);
    chip->Modify_SPI_Reg_bits(LMS7_LOSS_MAIN_TXPAD_TRF, txGainTable[row].main);
    chip->Modify_SPI_Reg_bits(LMS7_LOSS_LIN_TXPAD_TRF, txGainTable[row].lin);
    chip->Modify_SPI_Reg_bits(LMS7_MAC, 1);
    Log(LogLevel::DEBUG,
        "chip%i ch%i Tx gain set MAIN:%i, LIN:%i",
        context->txChannels[channel_num].parent->chipIndex,
        context->txChannels[channel_num].chipChannel,
        txGainTable[row].main,
        txGainTable[row].lin);
}

void lms7002m_set_rx_gain_func(LimePluginContext* context, double gain, int channel_num)
{
    int row = gain;
    if (row < 0 || row >= (int)(sizeof(rxGainTable) / sizeof(RxGainRow)))
        return;

    std::lock_guard<std::mutex> lk(gainsMutex);

    SDRDevice* device = context->rxChannels[channel_num].parent->device;
    LMS7002M* chip = static_cast<LMS7002M*>(device->GetInternalChip(context->rxChannels[channel_num].parent->chipIndex));
    chip->Modify_SPI_Reg_bits(LMS7_MAC, context->rxChannels[channel_num].chipChannel + 1);
    chip->Modify_SPI_Reg_bits(LMS7_G_LNA_RFE, rxGainTable[row].lna);
    chip->Modify_SPI_Reg_bits(LMS7_G_PGA_RBB, rxGainTable[row].pga);
    chip->Modify_SPI_Reg_bits(LMS7_MAC, 1);
    Log(LogLevel::DEBUG,
        "chip%i ch%i Rx gain set LNA:%i, PGA:%i",
        context->rxChannels[channel_num].parent->chipIndex,
        context->rxChannels[channel_num].chipChannel,
        rxGainTable[row].lna,
        rxGainTable[row].pga);
}

static void ConfigGains(LimePluginContext* context, const LimeParams* params, std::vector<ChannelData>& channelMapping, bool isTx)
{
    for (size_t ch = 0; ch < channelMapping.size(); ++ch)
    {
        if (isTx)
            lms7002m_set_tx_gain_func(context, params->tx.gain[ch], ch);
        else
            lms7002m_set_rx_gain_func(context, params->rx.gain[ch], ch);
    }
}

static int MapChannelsToPorts(int port,
    const std::deque<int>& deviceOrder,
    std::vector<RFNode>& rfdev,
    std::vector<ChannelData>& dirMapping,
    int port_channel_count)
{
    std::deque<int> assignedDevices = deviceOrder;
    int devIndex = assignedDevices.front();
    int remainingChannels = rfdev[devIndex].maxChannelsToUse;

    int chipRelativeChannelIndex = 0;
    for (int i=0; i<port_channel_count; ++i)
    {
        if (remainingChannels == 0)
        {
            assignedDevices.pop_front();
            if (assignedDevices.empty())
            {
                Log(LogLevel::ERROR, "port%i requires more channels than assigned devices have.", port);
                return -1;
            }
            devIndex = assignedDevices.front();
            remainingChannels = rfdev[devIndex].maxChannelsToUse;
            chipRelativeChannelIndex = 0;
        }
        ChannelData dep;
        dep.chipChannel = chipRelativeChannelIndex;
        ++chipRelativeChannelIndex;
        dep.parent = &rfdev[devIndex];
        dep.parent->assignedToPort = true;
        --remainingChannels;
        Log(LogLevel::DEBUG,
            "Channel%i : dev%i chipIndex:%i, chipChannel:%i",
            i,
            devIndex,
            rfdev[devIndex].chipIndex,
            dep.chipChannel);
        dirMapping.push_back(dep);
    }
    return 0;
}
/*
static void ParseAndWriteDeviceFPGARegisters(LimePluginContext* context, int devIndex)
{
    std::vector<uint32_t> writeRegisters;
    char varname[256];
    sprintf(varname, "dev%i_writeRegisters", devIndex);

    std::string value;
    if (context->config->GetString(value, varname))
    {
        char writeRegistersStr[512];
        strcpy(writeRegistersStr, value.c_str());
        char* token = strtok(writeRegistersStr, ";");
        while (token)
        {
            uint32_t spiVal = 0;
            sscanf(token, "%X", &spiVal);
            writeRegisters.push_back(spiVal | (1 << 31)); // adding spi write bit for convenience
            token = strtok(NULL, ";");
        }
    }

    SDRDevice* device = context->rfdev[devIndex].device;
    if (writeRegisters.size() > 0)
    {
        const auto slaves = device->GetDescriptor().spiSlaveIds;
        device->SPI(slaves.at("FPGA"), writeRegisters.data(), nullptr, writeRegisters.size());
    }
}
*/

int LimePlugin_Stop(LimePluginContext* context)
{
    for (auto& port : context->ports)
        port.composite->StreamStop();
    return 0;
}

int LimePlugin_Destroy(LimePluginContext* context)
{
    LimePlugin_Stop(context);
    for (auto& iter : context->uniqueDevices)
        DeviceRegistry::freeDevice(iter.second);
    return 0;
}

static int ConnectInitializeDevices(LimePluginContext* context)
{
    // Collect and connect specified unique nodes
    for (int i = 0; i < LIME_MAX_UNIQUE_DEVICES; ++i)
    {
        std::string deviceHandleStr;
        if (!GetParam(context, deviceHandleStr, "dev%i", i))
            continue;

        auto iter = context->uniqueDevices.find(deviceHandleStr);
        if (iter != context->uniqueDevices.end())
        {
            context->rfdev[i].device = iter->second;
            continue; // skip if the device already exists
        }

        std::vector<DeviceHandle> fullHandles = DeviceRegistry::enumerate(DeviceHandle(deviceHandleStr));
        if (fullHandles.size() > 1)
        {
            Log(LogLevel::ERROR, "%s : ambiguous handle, matches multiple devices.", deviceHandleStr.c_str());
            return -1;
        }
        if (fullHandles.empty())
        {
            Log(LogLevel::ERROR, "No device found to match handle: %s.", deviceHandleStr.c_str());
            return -1;
        }

        SDRDevice* device = DeviceRegistry::makeDevice(fullHandles.at(0));
        if (device != nullptr)
            Log(LogLevel::INFO, "Connected: %s", fullHandles.at(0).Serialize().c_str());
        else
        {
            Log(LogLevel::ERROR, "Failed to connect: %s", i, fullHandles.at(0).Serialize().c_str());
            return -1;
        }

        // Initialize device to default settings
        device->SetMessageLogCallback(LogCallback);
        device->EnableCache(false);
        device->Init();
        context->uniqueDevices[deviceHandleStr] = device;
        context->rfdev[i].device = device;
    }
    return 0;
}

static int LoadDevicesConfigurationFile(LimePluginContext* context)
{
    for (size_t i=0; i<context->rfdev.size(); ++i)
    {
        if (context->rfdev[i].device == nullptr)
            continue;

        size_t chipIndex = 0;
        GetParam(context, chipIndex, "dev%i_chipIndex", i);
        const auto& desc = context->rfdev[i].device->GetDescriptor();
        if (chipIndex >= desc.rfSOC.size())
        {
            Log(LogLevel::ERROR,
                "Invalid chipIndex (%i). dev%i has only %i chips.",
                chipIndex,
                i,
                desc.rfSOC.size());
            return -1;
        }
        context->rfdev[i].chipIndex = chipIndex;

        std::string filename;
        if (!GetParam(context, filename, "dev%i_ini", i))
            continue;

        LMS7002M* chip = static_cast<LMS7002M*>(context->rfdev[i].device->GetInternalChip(chipIndex));

        char configFilepath[512];
        if (filename[0] != '/') // is not global path
            sprintf(configFilepath, "%s/%s", context->currentWorkingDirectory, filename.c_str());
        else
            sprintf(configFilepath, "%s", filename.c_str());

        if (chip->LoadConfig(configFilepath, false) != OpStatus::SUCCESS)
        {
            Log(LogLevel::ERROR, "dev%s chip%i Error loading file: %s", i, chipIndex, configFilepath);
            return -1;
        }

        context->rfdev[i].config.skipDefaults = true;
        Log(LogLevel::INFO, "dev%i chip%i loaded with: %s", i, chipIndex, configFilepath);
    }
    return 0;
}

static int AssignDevicesToPorts(LimePluginContext* context)
{
    for (int p = 0; p < LIME_TRX_MAX_RF_PORT; ++p)
    {
        std::string deviceList;
        GetParam(context, deviceList, "port%i", p);
        if (deviceList.empty())
            continue;
        Log(LogLevel::DEBUG, "port%i: %s", p, deviceList.c_str());

        std::deque<int> assignedDevices;
        char tokens[512];
        sprintf(tokens, "%s", deviceList.c_str());
        char* token = strtok(tokens, ",");
        while (token)
        {
            if (strstr(token, "dev") != token)
            {
                // invalid device name
                Log(LogLevel::ERROR, "Port%i assigned invalid (%s) device.", p, token);
                return -1;
            }

            int devIndex = 0;
            sscanf(token + 3, "%i", &devIndex);
            assignedDevices.push_back(devIndex);
            token = strtok(NULL, ",");
        }
        context->portAssignements.push_back(assignedDevices);
        PortData port;

        while (!assignedDevices.empty())
        {
            RFNode* board = &context->rfdev[assignedDevices.front()];
            assert(board);
            board->portIndex = p;
            port.nodes.push_back(board);
            assignedDevices.pop_front();
        }
        context->ports.push_back(port);
    }
    if (context->ports.empty())
    {
        Log(LogLevel::ERROR, "No port has assigned devices.");
        return -1;
    }
    return 0;
}

static int AssignNodeParameters(LimePluginContext* context, RFNode& rf, bool Tx)
{
    int devIndex = rf.devIndex;
    int chipChannel = 0; //rf.chipChannel;
    int chipIndex = rf.chipIndex;
    SDRDevice::SDRConfig& config = rf.config;
    SDRDevice::ChannelConfig::Direction& channelCfg = Tx ? config.channel[chipChannel].tx : config.channel[chipChannel].rx;
    channelCfg.enabled = false;

    char prefix[32];
    sprintf(prefix, "dev%i_%s_", devIndex, Tx ? "tx" : "rx");

    rf.maxChannelsToUse = 2;
    GetParam(context, rf.maxChannelsToUse, "dev%i_maxChannelsToUse", devIndex);

    GetParam(context, channelCfg.oversample, "%soversample", prefix);
    GetParam(context, channelCfg.gfir.enabled, "%sgfir_enable", prefix);
    GetParam(context, channelCfg.gfir.bandwidth, "%sgfir_bandwidth", prefix);

    const auto& desc = rf.device->GetDescriptor().rfSOC[chipIndex];
    const auto& paths = desc.pathNames.at(Tx ? TRXDir::Tx : TRXDir::Rx);
    channelCfg.path = 0;

    std::string pathString;
    char varname[256];
    sprintf(varname, "%spath", prefix);
    if (GetParam(context, pathString, varname))
    {
        bool match = false;
        for (uint j = 0; j < paths.size(); ++j)
        {
            if (strcasecmp(paths[j].c_str(), pathString.c_str()) == 0)
            {
                channelCfg.path = j;
                match = true;
                break;
            }
        }
        if (!match)
        {
            Log(LogLevel::ERROR, "%s %s not found. Available: ", varname, pathString.c_str());
            for (const auto& iter : paths)
                Log(LogLevel::ERROR, "\"%s\" ", iter.c_str());
            return -1;
        }
    }

    char loFreqStr[1024];
    double freqOverride = 0;
    if (GetParam(context, freqOverride, "%s_lo_override", prefix) && freqOverride > 0)
    {
        Log(LogLevel::INFO, "dev%i chip%i ch%i %.2f", devIndex, rf.chipIndex, chipChannel, freqOverride);
        sprintf(loFreqStr,
            "expectedLO: %.3f MHz [override: %.3f (diff:%+.3f) MHz]",
            channelCfg.centerFrequency / 1.0e6,
            freqOverride / 1.0e6,
            (channelCfg.centerFrequency - freqOverride) / 1.0e6);
        channelCfg.centerFrequency = freqOverride;
    }
    else
        sprintf(loFreqStr, "LO: %.3f MHz", channelCfg.centerFrequency / 1.0e6);

    int flag = CalibrateFlag::Filter; // by default calibrate only filters
    std::string calibrationValue;
    if (GetParam(context, calibrationValue, "%scalibration", prefix))
    {
        if (!strcasecmp(calibrationValue.c_str(), "none"))
            flag = CalibrateFlag::None;
        else if ((!strcasecmp(calibrationValue.c_str(), "force")) || (!strcasecmp(calibrationValue.c_str(), "all")))
            flag = CalibrateFlag::Filter | CalibrateFlag::DCIQ;
        else if (!strcasecmp(calibrationValue.c_str(), "filter"))
            flag = CalibrateFlag::Filter;
        else if (!strcasecmp(calibrationValue.c_str(), "dciq"))
            flag = CalibrateFlag::DCIQ;
    }

    if (flag & DCIQ)
        channelCfg.calibrate = true;

    if (GetParam(context, rf.power_dBm, "%spower_dBm", prefix))
        rf.powerAvailable = true;

    for (int i = 1; i < 2; ++i)
    {
        if (Tx)
            config.channel[i].tx = channelCfg;
        else
            config.channel[i].rx = channelCfg;
    }
    return 0;
}

int LimePlugin_Init(LimePluginContext* context, HostLogCallbackType logFptr, LimeParamProvider* configProvider)
{
    context->hostLog = logFptr;
    hostCallback = logFptr;
    context->config = configProvider;

    int val = 0;
    if (GetParam(context, val, "logLevel"))
        logVerbosity = std::min(static_cast<LogLevel>(val), LogLevel::DEBUG);

    try
    {
        // Enumerate devices
        std::vector<DeviceHandle> devHandles = DeviceRegistry::enumerate();
        if (devHandles.empty())
        {
            Log(LogLevel::ERROR, "No LMS7002M nodes found!");
            return -1;
        }
        else
        {
            Log(LogLevel::DEBUG, "Available LMS7002M devices:");
            for (const auto& dev : devHandles)
                Log(LogLevel::DEBUG, "\t\"%s\"", dev.Serialize().c_str());
        }

        // Collect and initialize specified unique devices
        if (ConnectInitializeDevices(context) != 0)
            return -1;

        // Load configuration files for each chip if specified
        if (LoadDevicesConfigurationFile(context) != 0)
            return -1;
        // Aggregate devices into ports
        if (AssignDevicesToPorts(context) != 0)
            return -1;

        for (size_t i = 0; i < context->rfdev.size(); ++i)
        {
            auto& node = context->rfdev[i];
            if (!node.device)
                continue;
            node.devIndex = i;
            node.assignedToPort = false;
            if (AssignNodeParameters(context, node, false) != 0)
                return -1;
            if (AssignNodeParameters(context, node, true) != 0)
                return -1;
        }

        // load power settings
    /*    for (int p = 0; p < TRX_MAX_RF_PORT; ++p)
        {
            sprintf(varname, "port%i_linkFormat", p);
            char* linkFormat = trx_get_param_string(hostState, varname);
            if (linkFormat)
            {
                if (strcasecmp(linkFormat, "I16") == 0)
                    s->linkFormat[p] = lime::SDRDevice::StreamConfig::DataFormat::I16;
                else if (strcasecmp(linkFormat, "I12") == 0)
                    s->linkFormat[p] = lime::SDRDevice::StreamConfig::DataFormat::I12;
                else
                {
                    Log(LogLevel::WARNING, "Port[%i} Invalid link format (%s): falling back to I12", p, linkFormat);
                    s->linkFormat[p] = lime::SDRDevice::StreamConfig::DataFormat::I12;
                }
                free(linkFormat);
            }
            else
                s->linkFormat[p] = lime::SDRDevice::StreamConfig::DataFormat::I12;

            // absolute gain info
            for (int ch = 0; ch < TRX_MAX_CHANNELS; ++ch)
            {
                double dac = 0;
                sprintf(varname, "port%i_ch%i_pa_dac", p, ch);
                if (trx_get_param_double(hostState, &dac, varname) == 0)
                {
                    // TODO: this is board specific, need general API
                    int32_t paramId = 2 + ch;
                    std::string units = "";
                    s->device[p]->CustomParameterWrite({ { paramId, dac, units } });
                }
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

        s->samplesFormat = lime::SDRDevice::StreamConfig::F32;
*/
    } catch (std::logic_error& e)
    {
        fprintf(stderr, "Logic error: %s", e.what());
        return -1;
    } catch (std::runtime_error& e)
    {
        fprintf(stderr, "Runtime error: %s", e.what());
        return -1;
    }
    return 0;
}

static void ConfigAllChannels(LimePluginContext* context, const LimeParams* params, std::vector<ChannelData>& channelMapping, bool isTx)
{
    int ch = 0;
    for (auto& channelMap : channelMapping)
    {
        SDRDevice::ChannelConfig::Direction& channel = isTx ? channelMap.parent->config.channel[channelMap.chipChannel].tx
                                                            : channelMap.parent->config.channel[channelMap.chipChannel].rx;
        channel.enabled = true;
        int portIndex = channelMap.parent->portIndex;
        channel.sampleRate = params->rf_ports[portIndex].sample_rate;
        channel.centerFrequency = isTx ? params->tx.freq[ch] : params->rx.freq[ch];
        if (channel.gfir.bandwidth == 0)
            channel.gfir.bandwidth = isTx ? params->tx.bandwidth[ch] : params->rx.bandwidth[ch];
        if (channel.calibrate)
            channel.lpf = isTx ? params->tx.bandwidth[ch] : params->rx.bandwidth[ch];
        // if (isTx)
        //     trx_lms7002m_set_tx_gain_func(context, params->tx_gain[ch], ch);
        // else
        //     trx_lms7002m_set_rx_gain_func(context, params->rx_gain[ch], ch);

        const int chipIndex = channelMap.parent->chipIndex;
        const auto& desc = channelMap.parent->device->GetDescriptor().rfSOC[chipIndex];
        const auto& paths = desc.pathNames.at(isTx ? TRXDir::Tx : TRXDir::Rx);

        Log(LogLevel::VERBOSE,
        "dev%i %s chip%i ch%i , LO: %.3f MHz SR: %.3f MHz BW: %.3f MHz | path: %i('%s')",
        channelMap.parent->portIndex,
        isTx ? "Tx" : "Rx",
        chipIndex,
        channelMap.chipChannel,
        channel.centerFrequency / 1e6,
        channel.sampleRate / 1e6,
        channel.lpf / 1e6,
        channel.path,
        paths[channel.path].c_str());

        ++ch;
    }
}

int LimePlugin_Setup(LimePluginContext* context, const LimeParams* params)
{
    // map port channels to assigned devices
    for (size_t p = 0; p < params->rf_ports.size(); ++p)
    {
        std::deque<int> assignedDevices = context->portAssignements[p];
        if (MapChannelsToPorts(p, assignedDevices, context->rfdev, context->rxChannels, params->rf_ports[p].rx_channel_count) != 0)
            return -1;
        if (MapChannelsToPorts(p, assignedDevices, context->rfdev, context->txChannels, params->rf_ports[p].tx_channel_count) != 0)
            return -1;
    }

    // gather channel parameters and assign to device config
    ConfigAllChannels(context, params, context->rxChannels, false);
    ConfigAllChannels(context, params, context->txChannels, true);

    try
    {
        // configure all devices
        for (size_t i=0; i<context->rfdev.size(); ++i)
        {
            auto& dev = context->rfdev[i];
            if (dev.device == nullptr)
                continue;
            else if (dev.device != nullptr && !dev.assignedToPort)
            {
                Log(LogLevel::WARNING, "dev%i is not assigned to any port.", i);
                continue;
            }
            try
            {
                Log(LogLevel::DEBUG, "dev%i configure.", i);
                dev.device->Configure(dev.config, dev.chipIndex);
            } catch (...)
            {
                return -1;
            }
            //ParseAndWriteDeviceFPGARegisters(s1, i);
        }

        // override gains after device Configure
        ConfigGains(context, params, context->rxChannels, false);
        ConfigGains(context, params, context->txChannels, true);

        // setup data streaming
        int p=0;
        for (auto& port : context->ports)
        {
            SDRDevice::StreamConfig stream;
            stream.channels[TRXDir::Rx].resize(params->rf_ports[p].rx_channel_count);
            stream.channels[TRXDir::Tx].resize(params->rf_ports[p].tx_channel_count);
            stream.linkFormat = SDRDevice::StreamConfig::DataFormat::I16;
            stream.format = SDRDevice::StreamConfig::DataFormat::I16;

            // Initialize streams and map channels
            for (size_t ch = 0; ch < stream.channels[TRXDir::Rx].size(); ++ch)
                stream.channels[TRXDir::Rx][ch] = ch;
            for (size_t ch = 0; ch < stream.channels[TRXDir::Tx].size(); ++ch)
                stream.channels[TRXDir::Tx][ch] = ch;

            stream.statusCallback = OnStreamStatusChange;
            stream.userData = (void*)&portStreamStates[p];
            stream.hintSampleRate = params->rf_ports[p].sample_rate;

            std::vector<StreamAggregate> aggregates;
            for (auto& dev : port.nodes)
            {
                std::vector<int> channels;
                for (int i = 0; i < dev->maxChannelsToUse; ++i)
                    channels.push_back(i);
                aggregates.push_back({ dev->device, channels, dev->chipIndex });
            }

            Log(LogLevel::DEBUG,
            "Port[%i] Stream samples format: %s , link: %s",
            p,
            stream.format == SDRDevice::StreamConfig::DataFormat::F32 ? "F32" : "I16",
            stream.linkFormat == SDRDevice::StreamConfig::DataFormat::I12 ? "I12" : "I16");

            port.composite = new StreamComposite(aggregates);
            if (port.composite->StreamSetup(stream) != OpStatus::SUCCESS)
            {
                Log(LogLevel::ERROR, "Port%i stream setup failed.", p);
                return -1;
            }
            ++p;
        }
    } // try
    catch (std::logic_error& e)
    {
        Log(LogLevel::ERROR, "logic_error: %s", e.what());
        return -1;
    } catch (std::runtime_error& e)
    {
        Log(LogLevel::ERROR, "runtime_error: %s", e.what());
        return -1;
    }
    return 0;
}

int LimePlugin_Start(LimePluginContext* context)
{
    for (auto& port : context->ports)
    {
        port.composite->StreamStart();
    }
    return 0;
}

template<class T>
static int LimePlugin_Write(LimePluginContext* context, const T* const* samples, int count, int port, SDRDevice::StreamMeta& meta)
{
    if (!samples) // Nothing to transmit
        return 0;

    int samplesConsumed =
        context->ports[port].composite->StreamTx(samples, count, &meta);
    if (logVerbosity == LogLevel::DEBUG && samplesConsumed != count)
    {
        if (samplesConsumed < 0) // hardware timestamp is already ahead of meta.timestamp by (-samplesConsumed)
        {
            ++portStreamStates[port].tx.underrun;
            Log(LogLevel::DEBUG, "StreamTx: discarded samples write that is already late by (%i samples)", -samplesConsumed);
        }
        else
            Log(LogLevel::WARNING, "Tx not full consumed %i/%i", samplesConsumed, count);
    }
    return samplesConsumed;
}

int LimePlugin_Write_complex32f(LimePluginContext* context, const lime::complex32f_t* const* samples, int count, int port, SDRDevice::StreamMeta& meta)
{
    return LimePlugin_Write(context, samples, count, port, meta);
}

int LimePlugin_Write_complex16(LimePluginContext* context, const lime::complex16_t* const* samples, int count, int port, SDRDevice::StreamMeta& meta)
{
    return LimePlugin_Write(context, samples, count, port, meta);
}

template<class T>
static int LimePlugin_Read(LimePluginContext* context, T** samples, int count, int port, SDRDevice::StreamMeta& meta)
{
    meta.useTimestamp = false;
    meta.flush = false;
    int samplesGot = context->ports[port].composite->StreamRx(samples, count, &meta);

    if (samplesGot == 0)
    {
        // If the device is misconfigured it might not produce data for Rx
        // in that case the host will be stuck in loop attempting to read.
        // terminate the process to avoid freeze, since samples will never arrive.
        ++portStreamStates[port].tx.underrun;
        Log(LogLevel::ERROR, "Read timeout for Port[%i], device might be configured incorrectly.", port);
    }
    else if (samplesGot < 0)
    {
        Log(LogLevel::ERROR, "Error reading samples for Port[%i]: error code %i", port, samplesGot);
    }
    return samplesGot;
}

int LimePlugin_Read_complex32f(LimePluginContext* context, lime::complex32f_t** samples, int count, int port, SDRDevice::StreamMeta& meta)
{
    return LimePlugin_Read(context, samples, count, port, meta);
}

int LimePlugin_Read_complex16(LimePluginContext* context, lime::complex16_t** samples, int count, int port, SDRDevice::StreamMeta& meta)
{
    return LimePlugin_Read(context, samples, count, port, meta);
}
