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

DevNode::DevNode()
    : device(nullptr)
{
}

LimePluginContext::LimePluginContext()
    : rfdev(LIME_MAX_UNIQUE_DEVICES)
    , config(nullptr)
    , samplesFormat(SDRDevice::StreamConfig::DataFormat::F32)
{
    ports.reserve(LIME_TRX_MAX_RF_PORT);
}

enum CalibrateFlag { None = 0, DCIQ = 1, Filter = 2 };

template<class T> static bool GetSetting(LimeSettingsProvider* settings, T* pval, const char* prop_name_format, ...)
{
    char name[256];
    va_list args;
    va_start(args, prop_name_format);
    vsprintf(name, prop_name_format, args);
    va_end(args);

    double val = 0;
    if (!settings->GetDouble(val, name))
        return false;
    *pval = val;
    return true;
}

template<> bool GetSetting(LimeSettingsProvider* settings, std::string* pval, const char* prop_name_format, ...)
{
    char name[256];
    va_list args;
    va_start(args, prop_name_format);
    vsprintf(name, prop_name_format, args);
    va_end(args);

    return settings->GetString(*pval, name);
}

template<class T> static bool GetParam(LimePluginContext* context, T& pval, const char* prop_name_format, ...)
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
void LimePlugin_SetTxGain(LimePluginContext* context, double gain, int channel_num)
{
    int row = gain;
    if (row < 0 || row >= static_cast<int>(txGainTable.size()))
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

void LimePlugin_SetRxGain(LimePluginContext* context, double gain, int channel_num)
{
    int row = gain;
    if (row < 0 || row >= static_cast<int>(rxGainTable.size()))
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

static void ConfigGains(
    LimePluginContext* context, const LimeRuntimeParameters* params, std::vector<ChannelData>& channelMapping, bool isTx)
{
    for (size_t ch = 0; ch < channelMapping.size(); ++ch)
    {
        if (isTx)
            LimePlugin_SetTxGain(context, params->tx.gain[ch], ch);
        else
            LimePlugin_SetRxGain(context, params->rx.gain[ch], ch);
    }
}

static OpStatus MapChannelsToDevices(
    std::vector<ChannelData>& channels, std::vector<PortData>& ports, const LimeRuntimeParameters& params, TRXDir dir)
{
    channels.clear();
    for (uint32_t p = 0; p < params.rf_ports.size(); ++p)
    {
        int port_channel_count = dir == TRXDir::Tx ? params.rf_ports[p].tx_channel_count : params.rf_ports[p].rx_channel_count;

        std::deque<DevNode*> assignedDevices;
        copy(ports[p].nodes.begin(), ports[p].nodes.end(), assignedDevices.begin());

        int remainingChannels = assignedDevices.front()->maxChannelsToUse;
        int chipRelativeChannelIndex = 0;

        for (int i = 0; i < port_channel_count; ++i)
        {
            if (remainingChannels == 0)
            {
                assignedDevices.pop_front();
                if (assignedDevices.empty())
                {
                    Log(LogLevel::ERROR, "port%i requires more channels than assigned devices have.", p);
                    return OpStatus::ERROR;
                }
                remainingChannels = assignedDevices.front()->maxChannelsToUse;
                chipRelativeChannelIndex = 0;
            }
            ChannelData lane;
            lane.chipChannel = chipRelativeChannelIndex;
            ++chipRelativeChannelIndex;
            lane.parent = assignedDevices.front();
            lane.parent->assignedToPort = true;
            --remainingChannels;
            // Log(LogLevel::DEBUG,
            //     "%s Channel%i : dev%i chipIndex:%i, chipChannel:%i",
            //     (dir == TRXDir::Tx ? "Tx" : "Rx"),
            //     static_cast<int>(channels.size()),
            //     lane.parent->devIndex,
            //     lane.parent->chipIndex,
            //     lane.chipChannel);
            channels.push_back(lane);
        }
    }
    return OpStatus::SUCCESS;
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
    {
        if (!port.composite)
            continue;
        port.composite->StreamStop();
    }
    return 0;
}

int LimePlugin_Destroy(LimePluginContext* context)
{
    LimePlugin_Stop(context);
    for (auto& iter : context->uniqueDevices)
        DeviceRegistry::freeDevice(iter.second);
    return 0;
}

static OpStatus ConnectInitializeDevices(LimePluginContext* context)
{
    // Collect and connect specified unique nodes
    for (int i = 0; i < LIME_MAX_UNIQUE_DEVICES; ++i)
    {
        DevNode& node = context->rfdev.at(i);
        if (node.handleString.empty())
            continue;

        std::vector<DeviceHandle> fullHandles = DeviceRegistry::enumerate(DeviceHandle(node.handleString));
        if (fullHandles.size() > 1)
        {
            Log(LogLevel::ERROR, "%s : ambiguous handle, matches multiple devices.", node.handleString.c_str());
            return OpStatus::INVALID_VALUE;
        }
        if (fullHandles.empty())
        {
            Log(LogLevel::ERROR, "No device found to match handle: %s.", node.handleString.c_str());
            return OpStatus::INVALID_VALUE;
        }
        auto iter = context->uniqueDevices.find(fullHandles.at(0).Serialize().c_str());
        if (iter != context->uniqueDevices.end())
        {
            node.device = iter->second;
            continue; // skip if the device already exists
        }

        SDRDevice* device = DeviceRegistry::makeDevice(fullHandles.at(0));
        if (device != nullptr)
            Log(LogLevel::INFO, "Connected: %s", fullHandles.at(0).Serialize().c_str());
        else
        {
            Log(LogLevel::ERROR, "Failed to connect: %s", fullHandles.at(0).Serialize().c_str());
            return OpStatus::ERROR;
        }

        // Initialize device to default settings
        device->SetMessageLogCallback(LogCallback);
        device->EnableCache(false);
        device->Init();
        context->uniqueDevices[fullHandles.at(0).Serialize().c_str()] = device;
        node.device = device;
    }
    return OpStatus::SUCCESS;
}

static OpStatus LoadDevicesConfigurationFile(LimePluginContext* context)
{
    for (size_t i = 0; i < context->rfdev.size(); ++i)
    {
        DevNode& node = context->rfdev.at(i);
        if (node.device == nullptr)
            continue;

        const auto& desc = node.device->GetDescriptor();
        if (node.chipIndex >= desc.rfSOC.size())
        {
            Log(LogLevel::ERROR, "Invalid chipIndex (%i). dev%i has only %i chips.", node.chipIndex, i, desc.rfSOC.size());
            return OpStatus::OUT_OF_RANGE;
        }

        if (node.iniFilename.empty())
            continue;

        LMS7002M* chip = static_cast<LMS7002M*>(node.device->GetInternalChip(node.chipIndex));

        char configFilepath[512];
        if (node.iniFilename[0] != '/') // is not global path
            sprintf(configFilepath, "%s/%s", context->currentWorkingDirectory.c_str(), node.iniFilename.c_str());
        else
            sprintf(configFilepath, "%s", node.iniFilename.c_str());

        if (chip->LoadConfig(configFilepath, false) != OpStatus::SUCCESS)
        {
            Log(LogLevel::ERROR, "dev%s chip%i Error loading file: %s", i, node.chipIndex, configFilepath);
            return OpStatus::ERROR;
        }

        node.config.skipDefaults = true;
        Log(LogLevel::INFO, "dev%i chip%i loaded with: %s", i, node.chipIndex, configFilepath);
    }
    return OpStatus::SUCCESS;
}

static OpStatus AssignDevicesToPorts(LimePluginContext* context)
{
    for (int p = 0; p < LIME_TRX_MAX_RF_PORT; ++p)
    {
        PortData& port = context->ports.at(p);
        char tokens[512];
        sprintf(tokens, "%s", port.deviceNames.c_str());
        char* token = strtok(tokens, ",");
        while (token)
        {
            if (strstr(token, "dev") != token)
            {
                // invalid device name
                Log(LogLevel::ERROR, "Port%i assigned invalid (%s) device.", p, token);
                return OpStatus::INVALID_VALUE;
            }

            int devIndex = 0;
            sscanf(token + 3, "%i", &devIndex);
            DevNode* assignedDeviceNode = &context->rfdev.at(devIndex);
            port.nodes.push_back(assignedDeviceNode);
            assignedDeviceNode->portIndex = p;
            assignedDeviceNode->assignedToPort = true;
            token = strtok(NULL, ",");
        }
    }
    return OpStatus::SUCCESS;
}

static void GatherEnvironmentSettings(LimePluginContext* context, LimeSettingsProvider* configProvider)
{
    int val = 0;
    if (GetParam(context, val, "logLevel"))
        logVerbosity = std::min(static_cast<LogLevel>(val), LogLevel::DEBUG);
}

static void GatherDeviceDirectionalSettings(
    LimeSettingsProvider* settings, DevNode::DirectionalSettings* dir, const char* varPrefix)
{
    GetSetting(settings, &dir->antenna, "%s_path", varPrefix);
    GetSetting(settings, &dir->lo_override, "%s_lo_override", varPrefix);
    GetSetting(settings, &dir->gfir_enable, "%s_gfir_enable", varPrefix);
    GetSetting(settings, &dir->gfir_bandwidth, "%s_gfir_bandwidth", varPrefix);
    dir->powerAvailable = GetSetting(settings, &dir->power_dBm, "%s_power_dBm", varPrefix);
    GetSetting(settings, &dir->calibration, "%s_calibration", varPrefix);
    GetSetting(settings, &dir->oversample, "%s_oversample", varPrefix);
}

static void GatherDeviceNodeSettings(LimePluginContext* context, LimeSettingsProvider* settings)
{
    context->rfdev.resize(LIME_MAX_UNIQUE_DEVICES);
    for (uint32_t i = 0; i < context->rfdev.size(); ++i)
    {
        DevNode& dev = context->rfdev.at(i);
        char devPrefix[16];
        sprintf(devPrefix, "dev%i", i);
        GetSetting(settings, &dev.handleString, devPrefix);
        GetSetting(settings, &dev.chipIndex, "%s_chip_index", devPrefix);
        GetSetting(settings, &dev.iniFilename, "%s_ini", devPrefix);
        GetSetting(settings, &dev.maxChannelsToUse, "%s_max_channels_to_use", devPrefix);
        GetSetting(settings, &dev.double_freq_conversion_to_lower_side, "%s_double_freq_conversion_to_lower_side", devPrefix);

        char dirPrefix[32];
        sprintf(dirPrefix, "%s_rx", devPrefix);
        GatherDeviceDirectionalSettings(settings, &dev.rxSettings, dirPrefix);
        sprintf(dirPrefix, "%s_tx", devPrefix);
        GatherDeviceDirectionalSettings(settings, &dev.txSettings, dirPrefix);
    }
}

static OpStatus GatherPortSettings(LimePluginContext* context, LimeSettingsProvider* settings)
{
    context->ports.resize(LIME_TRX_MAX_RF_PORT);
    int specifiedPortsCount = 0;
    for (uint32_t i = 0; i < context->ports.size(); ++i)
    {
        PortData& port = context->ports.at(i);
        char portPrefix[16];
        sprintf(portPrefix, "port%i", i);
        if (GetSetting(settings, &port.deviceNames, portPrefix))
            ++specifiedPortsCount;
    }

    if (specifiedPortsCount == 0)
    {
        Log(LogLevel::ERROR, "No ports have been specified.");
        return OpStatus::ERROR;
    }
    return OpStatus::SUCCESS;
}

static OpStatus TransferDeviceDirectionalSettings(
    DevNode& node, const DevNode::DirectionalSettings& settings, SDRDevice::ChannelConfig::Direction& trx, TRXDir dir)
{
    trx.enabled = false;
    trx.oversample = settings.oversample;
    trx.gfir.enabled = settings.gfir_enable;
    trx.gfir.bandwidth = settings.gfir_bandwidth;

    const auto& desc = node.device->GetDescriptor().rfSOC[node.chipIndex];
    const auto& paths = desc.pathNames.at(dir);
    trx.path = 0;

    if (!settings.antenna.empty())
    {
        bool match = false;
        for (size_t j = 0; j < paths.size(); ++j)
        {
            if (strcasecmp(paths[j].c_str(), settings.antenna.c_str()) == 0)
            {
                trx.path = j;
                match = true;
                break;
            }
        }
        if (!match)
        {
            Log(LogLevel::ERROR, "%s path not found. Available: ", settings.antenna.c_str());
            for (const auto& iter : paths)
                Log(LogLevel::ERROR, "\"%s\" ", iter.c_str());
            return OpStatus::INVALID_VALUE;
        }
    }

    char loFreqStr[1024];
    if (settings.lo_override > 0)
    {
        //Log(LogLevel::INFO, "dev%i chip%i ch%i %.2f", devIndex, rf.chipIndex, chipChannel, freqOverride);
        sprintf(loFreqStr,
            "expectedLO: %.3f MHz [override: %.3f (diff:%+.3f) MHz]",
            trx.centerFrequency / 1.0e6,
            settings.lo_override / 1.0e6,
            (trx.centerFrequency - settings.lo_override) / 1.0e6);
        trx.centerFrequency = settings.lo_override;
    }
    else
        sprintf(loFreqStr, "LO: %.3f MHz", trx.centerFrequency / 1.0e6);

    int flag = CalibrateFlag::Filter; // by default calibrate only filters
    if (!settings.calibration.empty())
    {
        const char* value = settings.calibration.c_str();
        if (!strcasecmp(value, "none"))
            flag = CalibrateFlag::None;
        else if ((!strcasecmp(value, "force")) || (!strcasecmp(value, "all")))
            flag = CalibrateFlag::Filter | CalibrateFlag::DCIQ;
        else if (!strcasecmp(value, "filter"))
            flag = CalibrateFlag::Filter;
        else if (!strcasecmp(value, "dciq"))
            flag = CalibrateFlag::DCIQ;
    }

    if (flag & DCIQ)
        trx.calibrate = true;

    // copy setting to all channels
    for (int i = 1; i < 2; ++i)
    {
        if (dir == TRXDir::Tx)
            node.config.channel[i].tx = trx;
        else
            node.config.channel[i].rx = trx;
    }
    return OpStatus::SUCCESS;
}

static OpStatus TransferSettingsToDevicesConfig(std::vector<DevNode>& nodes)
{
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        DevNode& node = nodes.at(i);
        if (!node.device)
            continue;
        node.devIndex = i;
        node.assignedToPort = false;

        for (int ch = 0; ch < node.maxChannelsToUse; ++ch)
        {
            OpStatus status = TransferDeviceDirectionalSettings(node, node.rxSettings, node.config.channel[ch].rx, TRXDir::Rx);
            if (status != OpStatus::SUCCESS)
                return status;
            status = TransferDeviceDirectionalSettings(node, node.txSettings, node.config.channel[ch].tx, TRXDir::Tx);
            if (status != OpStatus::SUCCESS)
                return status;
        }
    }
    return OpStatus::SUCCESS;
}

int LimePlugin_Init(LimePluginContext* context, HostLogCallbackType logFptr, LimeSettingsProvider* configProvider)
{
    context->hostLog = logFptr;
    hostCallback = logFptr;
    context->config = configProvider;

    GatherEnvironmentSettings(context, configProvider);
    GatherDeviceNodeSettings(context, configProvider);
    if (GatherPortSettings(context, configProvider) != OpStatus::SUCCESS)
        return -1;

    try
    {
        // Enumerate devices
        std::vector<DeviceHandle> devHandles = DeviceRegistry::enumerate();
        if (devHandles.empty())
        {
            Log(LogLevel::ERROR, "No connected devices discovered.");
            return -1;
        }
        else
        {
            Log(LogLevel::DEBUG, "Available devices:");
            for (const auto& dev : devHandles)
                Log(LogLevel::DEBUG, "\t\"%s\"", dev.Serialize().c_str());
        }

        // Collect and initialize specified unique devices
        if (ConnectInitializeDevices(context) != OpStatus::SUCCESS)
            return -1;

        // Load configuration files for each chip if specified
        if (LoadDevicesConfigurationFile(context) != OpStatus::SUCCESS)
            return -1;
        if (AssignDevicesToPorts(context) != OpStatus::SUCCESS)
            return -1;

        TransferSettingsToDevicesConfig(context->rfdev);

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

static void TransferRuntimeParametersToConfig(
    const LimeRuntimeParameters& runtimeParams, const std::vector<ChannelData>& channelMap, TRXDir dir)
{
    const bool isTx = dir == TRXDir::Tx;
    const LimeRuntimeParameters::ChannelParams& params = isTx ? runtimeParams.tx : runtimeParams.rx;
    for (size_t i = 0; i < params.freq.size(); ++i)
    {
        SDRDevice::ChannelConfig::Direction& trxConfig = dir == TRXDir::Tx
                                                             ? channelMap[i].parent->config.channel[channelMap[i].chipChannel].tx
                                                             : channelMap[i].parent->config.channel[channelMap[i].chipChannel].rx;
        trxConfig.enabled = true;
        const int portIndex = channelMap[i].parent->portIndex;
        trxConfig.sampleRate = runtimeParams.rf_ports[portIndex].sample_rate;
        trxConfig.centerFrequency = params.freq[i];
        if (trxConfig.gfir.bandwidth == 0) // update only if not set by settings file
            trxConfig.gfir.bandwidth = params.bandwidth[i];
        if (trxConfig.calibrate)
            trxConfig.lpf = params.bandwidth[i];

        const int chipIndex = channelMap[i].parent->chipIndex;
        const auto& desc = channelMap[i].parent->device->GetDescriptor().rfSOC[chipIndex];
        const auto& paths = desc.pathNames.at(isTx ? TRXDir::Tx : TRXDir::Rx);

        Log(LogLevel::VERBOSE,
            "%s channel%i: dev%i chip%i ch%i , LO: %.3f MHz SR: %.3f MHz BW: %.3f MHz | path: %i('%s')",
            isTx ? "Tx" : "Rx",
            i,
            channelMap[i].parent->devIndex,
            channelMap[i].parent->chipIndex,
            channelMap[i].chipChannel,
            trxConfig.centerFrequency / 1e6,
            trxConfig.sampleRate / 1e6,
            params.bandwidth[i] / 1e6,
            trxConfig.path,
            paths[trxConfig.path].c_str());
    }
}

OpStatus ConfigureStreaming(LimePluginContext* context, const LimeRuntimeParameters* params)
{
    for (size_t p = 0; p < context->ports.size(); ++p)
    {
        PortData& port = context->ports[p];
        if (port.nodes.empty())
            continue;

        SDRDevice::StreamConfig stream;
        stream.channels[TRXDir::Rx].resize(params->rf_ports[p].rx_channel_count);
        stream.channels[TRXDir::Tx].resize(params->rf_ports[p].tx_channel_count);
        stream.linkFormat = SDRDevice::StreamConfig::DataFormat::I16;
        stream.format = context->samplesFormat;
        stream.extraConfig.rx.samplesInPacket = 256;
        stream.extraConfig.rx.packetsInBatch = 4;
        stream.extraConfig.tx.packetsInBatch = 8;
        stream.extraConfig.tx.samplesInPacket = 256;

        // Initialize streams and map channels
        for (size_t ch = 0; ch < stream.channels[TRXDir::Rx].size(); ++ch)
            stream.channels[TRXDir::Rx][ch] = ch;
        for (size_t ch = 0; ch < stream.channels[TRXDir::Tx].size(); ++ch)
            stream.channels[TRXDir::Tx][ch] = ch;

        stream.statusCallback = OnStreamStatusChange;
        stream.userData = static_cast<void*>(&portStreamStates[p]);
        stream.hintSampleRate = params->rf_ports[p].sample_rate;

        std::vector<StreamAggregate> aggregates;
        for (auto& dev : port.nodes)
        {
            if (!dev->assignedToPort)
                continue;
            std::vector<int> channels;
            for (int i = 0; i < dev->maxChannelsToUse; ++i)
                channels.push_back(i);
            aggregates.push_back({ dev->device, channels, dev->chipIndex });
        }
        if (aggregates.empty())
            continue;

        Log(LogLevel::DEBUG,
            "Port[%i] Stream samples format: %s , link: %s",
            p,
            stream.format == SDRDevice::StreamConfig::DataFormat::F32 ? "F32" : "I16",
            stream.linkFormat == SDRDevice::StreamConfig::DataFormat::I12 ? "I12" : "I16");

        port.composite = new StreamComposite(aggregates);
        if (port.composite->StreamSetup(stream) != OpStatus::SUCCESS)
        {
            Log(LogLevel::ERROR, "Port%i stream setup failed.", p);
            return OpStatus::ERROR;
        }
    }
    return OpStatus::SUCCESS;
}

int LimePlugin_Setup(LimePluginContext* context, const LimeRuntimeParameters* params)
{
    OpStatus status = MapChannelsToDevices(context->rxChannels, context->ports, *params, TRXDir::Rx);
    if (status != OpStatus::SUCCESS)
        return -1;
    status = MapChannelsToDevices(context->txChannels, context->ports, *params, TRXDir::Tx);
    if (status != OpStatus::SUCCESS)
        return -1;

    TransferRuntimeParametersToConfig(*params, context->rxChannels, TRXDir::Rx);
    TransferRuntimeParametersToConfig(*params, context->txChannels, TRXDir::Tx);

    try
    {
        // configure all devices
        for (size_t i = 0; i < context->rfdev.size(); ++i)
        {
            DevNode& node = context->rfdev[i];
            if (node.device == nullptr)
                continue;
            else if (node.device != nullptr && !node.assignedToPort)
            {
                Log(LogLevel::WARNING, "dev%i is not assigned to any port.", i);
                continue;
            }
            try
            {
                Log(LogLevel::DEBUG, "dev%i configure.", i);
                OpStatus status = node.device->Configure(node.config, node.chipIndex);
                if (status != OpStatus::SUCCESS)
                    return -1;
            } catch (...)
            {
                return -1;
            }
            //ParseAndWriteDeviceFPGARegisters(s1, i);
        }

        // override gains after device Configure
        ConfigGains(context, params, context->rxChannels, false);
        ConfigGains(context, params, context->txChannels, true);
        if (ConfigureStreaming(context, params) != OpStatus::SUCCESS)
            return -1;
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
        if (!port.composite)
            continue;
        port.composite->StreamStart();
    }
    return 0;
}

template<class T>
static int LimePlugin_Write(LimePluginContext* context, const T* const* samples, int count, int port, SDRDevice::StreamMeta& meta)
{
    if (!samples) // Nothing to transmit
        return 0;

    int samplesConsumed = context->ports[port].composite->StreamTx(samples, count, &meta);
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

int LimePlugin_Write_complex32f(
    LimePluginContext* context, const lime::complex32f_t* const* samples, int count, int port, SDRDevice::StreamMeta& meta)
{
    return LimePlugin_Write(context, samples, count, port, meta);
}

int LimePlugin_Write_complex16(
    LimePluginContext* context, const lime::complex16_t* const* samples, int count, int port, SDRDevice::StreamMeta& meta)
{
    return LimePlugin_Write(context, samples, count, port, meta);
}

template<class T>
static int LimePlugin_Read(LimePluginContext* context, T** samples, int count, int port, SDRDevice::StreamMeta& meta)
{
    meta.waitForTimestamp = false;
    meta.flushPartialPacket = false;
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

int LimePlugin_Read_complex32f(
    LimePluginContext* context, lime::complex32f_t** samples, int count, int port, SDRDevice::StreamMeta& meta)
{
    return LimePlugin_Read(context, samples, count, port, meta);
}

int LimePlugin_Read_complex16(
    LimePluginContext* context, lime::complex16_t** samples, int count, int port, SDRDevice::StreamMeta& meta)
{
    return LimePlugin_Read(context, samples, count, port, meta);
}
