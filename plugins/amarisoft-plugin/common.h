#pragma once

#include <limesuite/SDRDevice.h>
#include <limesuite/StreamComposite.h>

#include <map>
#include <vector>
#include <deque>

typedef void (*HostLogCallbackType)(lime::SDRDevice::LogLevel, const char*);

/// Interface for providing parameters from configuration file
class LimeSettingsProvider
{
  public:
    virtual ~LimeSettingsProvider(){};

    // return true if variable was found
    virtual bool GetString(std::string& dest, const char* varname) = 0;

    // return true if variable was found
    virtual bool GetDouble(double& dest, const char* varname) = 0;
};

// Individual RF SOC device configuration
struct DevNode {
  public:
    DevNode();

    struct DirectionalSettings {
        std::string antenna;
        std::string calibration;
        double lo_override;
        int oversample;
        int power_dBm;
        bool powerAvailable;
        bool gfir_enable;
        double gfir_bandwidth;
    };

    // settings from file
    std::string handleString;
    std::string iniFilename;
    int maxChannelsToUse; // how many channels can be used from this chip
    uint8_t chipIndex;
    bool double_freq_conversion_to_lower_side;
    DirectionalSettings rxSettings;
    DirectionalSettings txSettings;

    lime::SDRDevice* device; // chip owner
    lime::SDRDevice::SDRConfig config;
    int portIndex;
    int devIndex;
    bool assignedToPort;
};

struct ChannelData {
    DevNode* parent;
    int chipChannel;
};

// Ports/Cells that can have combined multiple RF devices to act as one
struct PortData {
    // settings from file
    std::string deviceNames;

    std::vector<DevNode*> nodes;
    lime::StreamComposite* composite;
};

struct LimePluginContext {
    LimePluginContext();
    std::vector<ChannelData> rxChannels;
    std::vector<ChannelData> txChannels;
    std::vector<PortData> ports;
    std::vector<DevNode> rfdev;
    std::map<std::string, lime::SDRDevice*> uniqueDevices;
    LimeSettingsProvider* config;
    lime::SDRDevice::StreamConfig::DataFormat samplesFormat;

    /* Path of the config file, not terminating by / */
    std::string currentWorkingDirectory;
    HostLogCallbackType hostLog;
};

struct LimeRuntimeParameters {
    struct ChannelParams {
        std::vector<int64_t> freq;
        std::vector<double> gain;
        std::vector<int> bandwidth;
    };

    ChannelParams rx;
    ChannelParams tx;

    struct PortParams {
        double sample_rate;
        int rx_channel_count;
        int tx_channel_count;
    };

    std::vector<PortParams> rf_ports;
};

void LimePlugin_SetTxGain(LimePluginContext* context, double gain, int channel_num);
void LimePlugin_SetRxGain(LimePluginContext* context, double gain, int channel_num);

// context should be allocated/freed by the host
int LimePlugin_Init(LimePluginContext* context, HostLogCallbackType logFptr, LimeSettingsProvider* configProvider);
int LimePlugin_Setup(LimePluginContext* context, const LimeRuntimeParameters* params);
int LimePlugin_Start(LimePluginContext* context);
int LimePlugin_Stop(LimePluginContext* context);
int LimePlugin_Destroy(LimePluginContext* context);

int LimePlugin_Write_complex32f(LimePluginContext* context,
    const lime::complex32f_t* const* samples,
    int nsamples,
    int port,
    lime::SDRDevice::StreamMeta& meta);
int LimePlugin_Write_complex16(
    LimePluginContext* context, const lime::complex16_t* const* samples, int nsamples, int port, lime::SDRDevice::StreamMeta& meta);
int LimePlugin_Read_complex32f(
    LimePluginContext* context, lime::complex32f_t** samples, int nsamples, int port, lime::SDRDevice::StreamMeta& meta);
int LimePlugin_Read_complex16(
    LimePluginContext* context, lime::complex16_t** samples, int nsamples, int port, lime::SDRDevice::StreamMeta& meta);

template<class T> void CopyCArrayToVector(std::vector<T>& vec, const T* arr, size_t count)
{
    vec.resize(count);
    memcpy(vec.data(), arr, count * sizeof(T));
}

template<class T, class S> void AssignCArrayToVector(std::vector<T>& vec, const S* arr, size_t count)
{
    vec.resize(count);
    for (size_t i = 0; i < count; ++i)
        vec[i] = arr[i];
}