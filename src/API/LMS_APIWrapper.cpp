#include "lime/LimeSuite.h"
#include "limesuite/commonTypes.h"
#include "limesuite/DeviceHandle.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/LMS7002M.h"
#include "limesuite/LMS7002M_parameters.h"
#include "limesuite/SDRDevice.h"
#include "LMS7002M_SDRDevice.h"
#include "Logger.h"
#include "MemoryPool.h"
#include "VersionInfo.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace {

struct StatsDeltas {
    lime::DeltaVariable<uint32_t> underrun;
    lime::DeltaVariable<uint32_t> overrun;
    lime::DeltaVariable<uint32_t> droppedPackets;

    StatsDeltas()
        : underrun(0)
        , overrun(0)
        , droppedPackets(0)
    {
    }
};

struct StreamBuffer {
    void* buffer;
    lime::MemoryPool* ownerMemoryPool;
    lime::TRXDir direction;
    uint8_t channel;
    int samplesProduced;

    StreamBuffer() = delete;
};

struct LMS_APIDevice {
    lime::SDRDevice* device;
    lime::SDRDevice::SDRConfig lastSavedSDRConfig;
    lime::SDRDevice::StreamConfig lastSavedStreamConfig;
    std::array<std::array<float_type, 2>, lime::SDRDevice::MAX_CHANNEL_COUNT> lastSavedLPFValue;
    StatsDeltas statsDeltas;

    uint8_t moduleIndex;

    std::vector<StreamBuffer> streamBuffers;
    lms_dev_info_t* deviceInfo;

    LMS_APIDevice() = delete;
    LMS_APIDevice(lime::SDRDevice* device)
        : device(device)
        , lastSavedSDRConfig()
        , lastSavedStreamConfig()
        , lastSavedLPFValue()
        , statsDeltas()
        , moduleIndex(0)
        , streamBuffers()
        , deviceInfo(nullptr)
    {
    }

    ~LMS_APIDevice()
    {
        lime::DeviceRegistry::freeDevice(device);

        if (deviceInfo != nullptr)
        {
            delete deviceInfo;
        }
    }
};

struct StreamHandle {
    static constexpr std::size_t MAX_ELEMENTS_IN_BUFFER = 4096;

    LMS_APIDevice* parent;
    bool isStreamStartedFromAPI;
    bool isStreamActuallyStarted;
    lime::MemoryPool memoryPool;

    StreamHandle() = delete;
    StreamHandle(LMS_APIDevice* parent)
        : parent(parent)
        , isStreamStartedFromAPI(false)
        , isStreamActuallyStarted(false)
        , memoryPool(1, sizeof(lime::complex32f_t) * MAX_ELEMENTS_IN_BUFFER, 4096, "StreamHandleMemoryPool")
    {
    }
};

static std::vector<StreamHandle*> streamHandles;

inline LMS_APIDevice* CheckDevice(lms_device_t* device)
{
    if (device == nullptr)
    {
        lime::error("Device cannot be NULL.");
        return nullptr;
    }

    return static_cast<LMS_APIDevice*>(device);
}

inline LMS_APIDevice* CheckDevice(lms_device_t* device, unsigned chan)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr || apiDevice->device == nullptr)
    {
        return nullptr;
    }

    const lime::SDRDevice::Descriptor& descriptor = apiDevice->device->GetDescriptor();
    if (chan >= descriptor.rfSOC[apiDevice->moduleIndex].channelCount)
    {
        lime::error("Invalid channel number.");
        return nullptr;
    }

    return apiDevice;
}

inline std::size_t GetStreamHandle(LMS_APIDevice* parent)
{
    for (std::size_t i = 0; i < streamHandles.size(); i++)
    {
        if (streamHandles.at(i) == nullptr)
        {
            streamHandles.at(i) = new StreamHandle{ parent };
            return i;
        }
    }

    streamHandles.push_back(new StreamHandle{ parent });
    return streamHandles.size() - 1;
}

inline void CopyString(const std::string& source, char* destination, std::size_t destinationLength)
{
    std::strncpy(destination, source.c_str(), destinationLength - 1);
    destination[destinationLength - 1] = 0;
}

inline void CopyStringVectorIntoList(std::vector<std::string> strings, lms_name_t* list)
{
    for (std::size_t i = 0; i < strings.size(); ++i)
    {
        CopyString(strings.at(i), list[i], sizeof(lms_name_t));
    }
}

inline lms_range_t RangeToLMS_Range(const lime::Range& range)
{
    return { range.min, range.max, range.step };
}

inline double GetGain(LMS_APIDevice* apiDevice, bool dir_tx, size_t chan)
{
    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;

    if (dir_tx)
    {
        return config.channel[chan].tx.gain;
    }

    return config.channel[chan].rx.gain;
}

inline lime::SDRDevice::SDRConfig GetCurrentConfiguration(LMS_APIDevice* apiDevice)
{
    lime::SDRDevice::SDRConfig configuration;

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(apiDevice->moduleIndex));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        throw std::logic_error("Device is not an LMS device.");
    }

    configuration.referenceClockFreq = lms->GetReferenceClk_SX(lime::TRXDir::Rx);

    auto rxFrequency = lms->GetFrequencySX(lime::TRXDir::Rx);
    auto txFrequency = lms->GetFrequencySX(lime::TRXDir::Tx);

    auto channelCount = apiDevice->device->GetDescriptor().rfSOC.at(apiDevice->moduleIndex).channelCount;

    for (int i = 0; i < channelCount; ++i)
    {
        configuration.channel[i].rx.centerFrequency = rxFrequency;
        configuration.channel[i].tx.centerFrequency = txFrequency;

        configuration.channel[i].rx.sampleRate =
            lms->GetSampleRate(lime::TRXDir::Rx, i == 0 ? lime::LMS7002M::Channel::ChA : lime::LMS7002M::Channel::ChB);
        configuration.channel[i].tx.sampleRate =
            lms->GetSampleRate(lime::TRXDir::Tx, i == 0 ? lime::LMS7002M::Channel::ChA : lime::LMS7002M::Channel::ChB);

        // TODO: find ways to get current values for all other fields.
    }

    return configuration;
}

static LMS_LogHandler api_msg_handler;
static void APIMsgHandler(const lime::LogLevel level, const char* message)
{
    api_msg_handler(static_cast<int>(level), message);
}

static lms_prog_callback_t programmingCallback;
static bool ProgrammingCallback(size_t bsent, size_t btotal, const char* statusMessage)
{
    return programmingCallback(static_cast<int>(bsent), static_cast<int>(btotal), statusMessage);
}

} //unnamed namespace

API_EXPORT int CALL_CONV LMS_GetDeviceList(lms_info_str_t* dev_list)
{
    std::vector<lime::DeviceHandle> handles = lime::DeviceRegistry::enumerate();

    if (dev_list != nullptr)
    {
        for (std::size_t i = 0; i < handles.size(); ++i)
        {
            std::string str = handles[i].Serialize();
            CopyString(str, dev_list[i], sizeof(lms_info_str_t));
        }
    }

    return handles.size();
}

API_EXPORT int CALL_CONV LMS_Open(lms_device_t** device, const lms_info_str_t info, void* args)
{
    if (device == nullptr)
    {
        lime::error("Device pointer cannot be NULL.");
        return -1;
    }

    std::vector<lime::DeviceHandle> handles = lime::DeviceRegistry::enumerate();
    for (auto& handle : handles)
    {
        if (info == nullptr || std::strcmp(handle.Serialize().c_str(), info) == 0)
        {
            auto dev = lime::DeviceRegistry::makeDevice(handle);

            if (dev == nullptr)
            {
                lime::error("Unable to open device.");
                return -1;
            }

            auto apiDevice = new LMS_APIDevice{ dev };

            *device = apiDevice;
            return LMS_SUCCESS;
        }
    }

    lime::error("Specified device could not be found.");
    return -1;
}

API_EXPORT int CALL_CONV LMS_Close(lms_device_t* device)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    delete apiDevice;
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_Init(lms_device_t* device)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    try
    {
        int returnCode = apiDevice->device->Init();

        apiDevice->lastSavedSDRConfig = GetCurrentConfiguration(apiDevice);

        return returnCode;
    } catch (...)
    {
        return -1;
    }
}

API_EXPORT int CALL_CONV LMS_Reset(lms_device_t* device)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    try
    {
        apiDevice->device->Reset();
    } catch (...)
    {
        return -1;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_EnableChannel(lms_device_t* device, bool dir_tx, size_t chan, bool enabled)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;
    const double defaultFrequency = 1e8;

    if (dir_tx)
    {
        config.channel[chan].tx.enabled = enabled;

        if (config.channel[chan].tx.centerFrequency == 0)
        {
            config.channel[chan].tx.centerFrequency = defaultFrequency;
        }
    }
    else
    {
        config.channel[chan].rx.enabled = enabled;

        if (config.channel[chan].rx.centerFrequency == 0)
        {
            config.channel[chan].rx.centerFrequency = defaultFrequency;
        }
    }

    try
    {
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, apiDevice->moduleIndex);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetSampleRate(lms_device_t* device, float_type rate, size_t oversample)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;

    for (std::size_t i = 0; i < lime::SDRDevice::MAX_CHANNEL_COUNT; ++i)
    {
        config.channel[i].rx.sampleRate = rate;
        config.channel[i].rx.oversample = oversample;

        config.channel[i].tx.sampleRate = rate;
        config.channel[i].tx.oversample = oversample;
    }

    try
    {
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, apiDevice->moduleIndex);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetSampleRateDir(lms_device_t* device, bool dir_tx, float_type rate, size_t oversample)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;

    for (std::size_t i = 0; i < lime::SDRDevice::MAX_CHANNEL_COUNT; ++i)
    {
        if (dir_tx)
        {
            config.channel[i].tx.sampleRate = rate;
            config.channel[i].tx.oversample = oversample;
        }
        else
        {
            config.channel[i].rx.sampleRate = rate;
            config.channel[i].rx.oversample = oversample;
        }
    }

    try
    {
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, apiDevice->moduleIndex);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetSampleRate(lms_device_t* device, bool dir_tx, size_t chan, float_type* host_Hz, float_type* rf_Hz)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;

    if (dir_tx)
    {
        *host_Hz = config.channel[chan].tx.sampleRate;
        *rf_Hz = config.channel[chan].tx.sampleRate * config.channel[chan].tx.oversample;
    }
    else
    {
        *host_Hz = config.channel[chan].rx.sampleRate;
        *rf_Hz = config.channel[chan].rx.sampleRate * config.channel[chan].rx.oversample;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetSampleRateRange(lms_device_t* device, bool dir_tx, lms_range_t* range)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    *range = RangeToLMS_Range(apiDevice->device->GetDescriptor().rfSOC[apiDevice->moduleIndex].samplingRateRange);

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetNumChannels(lms_device_t* device, bool dir_tx)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    return apiDevice->device->GetDescriptor().rfSOC[apiDevice->moduleIndex].channelCount;
}

API_EXPORT int CALL_CONV LMS_SetLOFrequency(lms_device_t* device, bool dir_tx, size_t chan, float_type frequency)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;

    if (dir_tx)
    {
        config.channel[chan].tx.centerFrequency = frequency;

        const bool txMIMO = config.channel[0].tx.enabled && config.channel[1].tx.enabled;
        if (txMIMO && config.channel[0].tx.centerFrequency != config.channel[1].tx.centerFrequency)
        {
            // Don't configure just yet, wait for both frequencies to be set.
            return 0;
        }
    }
    else
    {
        config.channel[chan].rx.centerFrequency = frequency;

        const bool rxMIMO = config.channel[0].rx.enabled && config.channel[1].rx.enabled;
        if (rxMIMO && config.channel[0].rx.centerFrequency != config.channel[1].rx.centerFrequency)
        {
            // Don't configure just yet, wait for both frequencies to be set.
            return 0;
        }
    }

    try
    {
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, apiDevice->moduleIndex);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetLOFrequency(lms_device_t* device, bool dir_tx, size_t chan, float_type* frequency)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    if (dir_tx)
    {
        *frequency = apiDevice->lastSavedSDRConfig.channel[chan].tx.centerFrequency;
    }
    else
    {
        *frequency = apiDevice->lastSavedSDRConfig.channel[chan].rx.centerFrequency;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetLOFrequencyRange(lms_device_t* device, bool dir_tx, lms_range_t* range)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    *range = RangeToLMS_Range(apiDevice->device->GetDescriptor().rfSOC[apiDevice->moduleIndex].frequencyRange);

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetAntennaList(lms_device_t* device, bool dir_tx, size_t chan, lms_name_t* list)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    auto rfSOC = apiDevice->device->GetDescriptor().rfSOC[apiDevice->moduleIndex];

    if (dir_tx)
    {
        CopyStringVectorIntoList(rfSOC.txPathNames, list);
    }
    else
    {
        CopyStringVectorIntoList(rfSOC.rxPathNames, list);
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetAntenna(lms_device_t* device, bool dir_tx, size_t chan, size_t path)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;

    if (dir_tx)
    {
        config.channel[chan].tx.path = path;
    }
    else
    {
        config.channel[chan].rx.path = path;
    }

    try
    {
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, apiDevice->moduleIndex);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetAntenna(lms_device_t* device, bool dir_tx, size_t chan)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;

    if (dir_tx)
    {
        return config.channel[chan].tx.path;
    }

    return config.channel[chan].rx.path;
}

API_EXPORT int CALL_CONV LMS_GetAntennaBW(lms_device_t* device, bool dir_tx, size_t chan, size_t path, lms_range_t* range)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::TRXDir direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;
    std::string pathName = dir_tx ? apiDevice->device->GetDescriptor().rfSOC.at(apiDevice->moduleIndex).txPathNames.at(path)
                                  : apiDevice->device->GetDescriptor().rfSOC.at(apiDevice->moduleIndex).rxPathNames.at(path);

    *range = RangeToLMS_Range(
        apiDevice->device->GetDescriptor().rfSOC.at(apiDevice->moduleIndex).antennaRange.at(direction).at(pathName));

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetLPFBW(lms_device_t* device, bool dir_tx, size_t chan, float_type bandwidth)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;

    if (dir_tx)
    {
        config.channel[chan].tx.lpf = bandwidth;
    }
    else
    {
        config.channel[chan].rx.lpf = bandwidth;
    }

    apiDevice->lastSavedLPFValue[chan][dir_tx] = bandwidth;

    try
    {
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, apiDevice->moduleIndex);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetLPFBWRange(lms_device_t* device, bool dir_tx, lms_range_t* range)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    if (dir_tx)
    {
        *range = RangeToLMS_Range({ 5e6, 130e6, 0 });
    }
    else
    {
        *range = RangeToLMS_Range({ 1.4001e6, 130e6, 0 });
    }

    return 0;
}

// TODO: Implement properly once the Gain API is completed
API_EXPORT int CALL_CONV LMS_SetNormalizedGain(lms_device_t* device, bool dir_tx, size_t chan, float_type gain)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    if (gain > 1.0)
    {
        gain = 1.0;
    }
    else if (gain < 0)
    {
        gain = 0;
    }

    const lms_range_t range{ -12, dir_tx ? 64.0 : 61.0, 0 };
    gain = range.min + gain * (range.max - range.min);

    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;

    if (dir_tx)
    {
        config.channel[chan].tx.gain = gain;
    }
    else
    {
        config.channel[chan].rx.gain = gain;
    }

    try
    {
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, apiDevice->moduleIndex);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    return 0;
}

// TODO: Implement properly once the Gain API is completed
API_EXPORT int CALL_CONV LMS_SetGaindB(lms_device_t* device, bool dir_tx, size_t chan, unsigned gain)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;

    if (dir_tx)
    {
        config.channel[chan].tx.gain = gain;
    }
    else
    {
        config.channel[chan].rx.gain = gain;
    }

    try
    {
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, apiDevice->moduleIndex);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    return 0;
}

// TODO: Implement properly once the Gain API is completed
API_EXPORT int CALL_CONV LMS_GetNormalizedGain(lms_device_t* device, bool dir_tx, size_t chan, float_type* gain)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    const lms_range_t range{ -12, dir_tx ? 64.0 : 61.0, 0 };
    *gain = (GetGain(apiDevice, dir_tx, chan) - range.min) / (range.max - range.min);

    return LMS_SUCCESS;
}

// TODO: Implement properly once the Gain API is completed
API_EXPORT int CALL_CONV LMS_GetGaindB(lms_device_t* device, bool dir_tx, size_t chan, unsigned* gain)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    *gain = static_cast<unsigned>(GetGain(apiDevice, dir_tx, chan) + 12 + 0.5);
    return 0;
}

API_EXPORT int CALL_CONV LMS_Calibrate(lms_device_t* device, bool dir_tx, size_t chan, double bw, unsigned flags)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;

    if (dir_tx)
    {
        config.channel[chan].tx.calibrate = true;
    }
    else
    {
        config.channel[chan].rx.calibrate = true;
    }

    try
    {
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, apiDevice->moduleIndex);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    if (dir_tx)
    {
        config.channel[chan].tx.calibrate = false;
    }
    else
    {
        config.channel[chan].rx.calibrate = false;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetTestSignal(
    lms_device_t* device, bool dir_tx, size_t chan, lms_testsig_t sig, int16_t dc_i, int16_t dc_q)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    if (sig > LMS_TESTSIG_DC)
    {
        lime::error("Invalid signal.");
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(chan / 2));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan % 2) + 1);

    if (dir_tx == false)
    {
        if (lms->Modify_SPI_Reg_bits(LMS7param(INSEL_RXTSP), sig != LMS_TESTSIG_NONE, true) != 0)
        {
            return -1;
        }

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV8F)
        {
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1);
        }
        else if (sig == LMS_TESTSIG_NCODIV4 || sig == LMS_TESTSIG_NCODIV4F)
        {
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 2);
        }

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV4)
        {
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 0);
        }
        else if (sig == LMS_TESTSIG_NCODIV8F || sig == LMS_TESTSIG_NCODIV4F)
        {
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 1);
        }

        lms->Modify_SPI_Reg_bits(LMS7param(TSGMODE_RXTSP), sig == LMS_TESTSIG_DC);
    }
    else
    {
        if (lms->Modify_SPI_Reg_bits(LMS7param(INSEL_TXTSP), sig != LMS_TESTSIG_NONE) != 0)
        {
            return -1;
        }

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV8F)
        {
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), 1);
        }
        else if (sig == LMS_TESTSIG_NCODIV4 || sig == LMS_TESTSIG_NCODIV4F)
        {
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP), 2);
        }

        if (sig == LMS_TESTSIG_NCODIV8 || sig == LMS_TESTSIG_NCODIV4)
        {
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_TXTSP), 0);
        }
        else if (sig == LMS_TESTSIG_NCODIV8F || sig == LMS_TESTSIG_NCODIV4F)
        {
            lms->Modify_SPI_Reg_bits(LMS7param(TSGFC_TXTSP), 1);
        }

        lms->Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), sig == LMS_TESTSIG_DC);
    }

    if (sig == LMS_TESTSIG_DC)
    {
        return lms->LoadDC_REG_IQ(dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx, dc_i, dc_q);
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetupStream(lms_device_t* device, lms_stream_t* stream)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    if (stream == nullptr)
    {
        lime::error("Stream cannot be NULL.");
        return -1;
    }

    // Configure again in case some skips were made in validation before hand.
    try
    {
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, apiDevice->moduleIndex);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    lime::SDRDevice::StreamConfig config = apiDevice->lastSavedStreamConfig;
    config.bufferSize = stream->fifoSize;

    auto channel = stream->channel & ~LMS_ALIGN_CH_PHASE; // Clear the align phase bit
    if (stream->isTx)
    {
        config.txChannels[config.txCount++] = channel;
    }
    else
    {
        config.rxChannels[config.rxCount++] = channel;
    }

    config.alignPhase = stream->channel & LMS_ALIGN_CH_PHASE;

    switch (stream->dataFmt)
    {
    case lms_stream_t::LMS_FMT_F32:
        config.format = lime::SDRDevice::StreamConfig::DataFormat::F32;
        config.linkFormat = lime::SDRDevice::StreamConfig::DataFormat::I16;
        break;
    case lms_stream_t::LMS_FMT_I16:
        config.format = lime::SDRDevice::StreamConfig::DataFormat::I16;
        config.linkFormat = lime::SDRDevice::StreamConfig::DataFormat::I16;
        break;
    case lms_stream_t::LMS_FMT_I12:
        config.format = lime::SDRDevice::StreamConfig::DataFormat::I12;
        config.linkFormat = lime::SDRDevice::StreamConfig::DataFormat::I12;
        break;
    default:
        config.format = lime::SDRDevice::StreamConfig::DataFormat::F32;
        config.linkFormat = lime::SDRDevice::StreamConfig::DataFormat::I16;
    }

    // TODO: check functionality
    // switch (stream->linkFmt)
    // {
    // case lms_stream_t::LMS_LINK_FMT_I16:
    //     config.linkFormat = lime::SDRDevice::StreamConfig::DataFormat::I16;
    //     break;
    // case lms_stream_t::LMS_LINK_FMT_I12:
    //     config.linkFormat = lime::SDRDevice::StreamConfig::DataFormat::I12;
    // case lms_stream_t::LMS_LINK_FMT_DEFAULT: // do nothing
    //     break;
    // }

    // TODO: check functionality
    // config.performanceLatency = stream->throughputVsLatency;

    auto returnValue = apiDevice->device->StreamSetup(config, apiDevice->moduleIndex);

    if (returnValue == 0)
    {
        apiDevice->lastSavedStreamConfig = config;
    }

    stream->handle = GetStreamHandle(apiDevice);

    return returnValue;
}

API_EXPORT int CALL_CONV LMS_DestroyStream(lms_device_t* device, lms_stream_t* stream)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    if (stream == nullptr)
    {
        lime::error("Stream cannot be NULL.");
        return -1;
    }

    auto& streamHandle = streamHandles.at(stream->handle);
    if (streamHandle != nullptr)
    {
        delete streamHandle;
        streamHandle = nullptr;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_StartStream(lms_stream_t* stream)
{
    if (stream == nullptr || stream->handle < 0)
    {
        return -1;
    }

    auto& handle = streamHandles.at(stream->handle);
    if (handle == nullptr || handle->parent == nullptr)
    {
        return -1;
    }

    handle->isStreamStartedFromAPI = true;

    if (!handle->isStreamActuallyStarted)
    {
        handle->parent->device->StreamStart(handle->parent->moduleIndex);

        for (auto& streamHandle : streamHandles)
        {
            if (streamHandle->parent != handle->parent)
            {
                continue;
            }

            streamHandle->isStreamActuallyStarted = true;
        }
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_StopStream(lms_stream_t* stream)
{
    if (stream == nullptr || stream->handle < 0)
    {
        return -1;
    }

    auto& handle = streamHandles.at(stream->handle);
    if (handle == nullptr || handle->parent == nullptr)
    {
        return -1;
    }

    handle->isStreamStartedFromAPI = false;

    if (handle->isStreamActuallyStarted)
    {
        handle->parent->device->StreamStop(handle->parent->moduleIndex);

        for (auto& streamHandle : streamHandles)
        {
            if (streamHandle->parent != handle->parent)
            {
                continue;
            }

            streamHandle->isStreamActuallyStarted = false;
        }
    }

    return 0;
}

namespace {

template<class T>
int ReceiveStream(lms_stream_t* stream, void* samples, size_t sample_count, lms_stream_meta_t* meta, unsigned timeout_ms)
{
    auto& handle = streamHandles.at(stream->handle);
    if (handle == nullptr || handle->parent == nullptr)
    {
        return -1;
    }

    const auto direction = stream->isTx ? lime::TRXDir::Tx : lime::TRXDir::Rx;
    if (direction == lime::TRXDir::Tx)
    {
        lime::error("Invalid direction.");
        return -1;
    }

    const uint32_t streamChannel = stream->channel & ~LMS_ALIGN_CH_PHASE;
    const uint8_t rxChannelCount = handle->parent->lastSavedStreamConfig.rxCount;
    const std::size_t sampleSize = sizeof(T);

    if (rxChannelCount > 1)
    {
        for (auto it = handle->parent->streamBuffers.begin(); it != handle->parent->streamBuffers.end(); it++)
        {
            auto& buffer = *it;
            if (buffer.direction == direction && buffer.channel == streamChannel)
            {
                std::memcpy(samples, buffer.buffer, sample_count * sampleSize);
                int samplesProduced = buffer.samplesProduced;

                buffer.ownerMemoryPool->Free(buffer.buffer);
                handle->parent->streamBuffers.erase(it);

                return samplesProduced;
            }
        }
    }

    // Related cache not found, need to make one up.
    std::vector<T*> sampleBuffer(rxChannelCount);
    for (uint8_t i = 0; i < rxChannelCount; ++i)
    {
        if (handle->parent->lastSavedStreamConfig.rxChannels[i] == streamChannel)
        {
            sampleBuffer[i] = reinterpret_cast<T*>(samples);
        }
        else
        {
            sampleBuffer[i] = reinterpret_cast<T*>(handle->memoryPool.Allocate(sample_count * sampleSize));

            handle->parent->streamBuffers.push_back(
                { sampleBuffer[i], &handle->memoryPool, direction, handle->parent->lastSavedStreamConfig.rxChannels[i], 0 });
        }
    }

    lime::SDRDevice::StreamMeta metadata{ 0, false, false };
    int samplesProduced =
        handle->parent->device->StreamRx(handle->parent->moduleIndex, sampleBuffer.data(), sample_count, &metadata);

    for (auto& buffer : handle->parent->streamBuffers)
    {
        if (buffer.direction == direction)
        {
            buffer.samplesProduced = samplesProduced;
        }
    }

    if (meta != nullptr)
    {
        meta->timestamp = metadata.timestamp;
    }

    return samplesProduced;
}

} // namespace

API_EXPORT int CALL_CONV LMS_RecvStream(
    lms_stream_t* stream, void* samples, size_t sample_count, lms_stream_meta_t* meta, unsigned timeout_ms)
{
    if (stream == nullptr || stream->handle < 0)
    {
        return -1;
    }

    std::size_t samplesProduced = 0;
    switch (stream->dataFmt)
    {
    case lms_stream_t::LMS_FMT_F32:
        samplesProduced = ReceiveStream<lime::complex32f_t>(stream, samples, sample_count, meta, timeout_ms);
        break;
    case lms_stream_t::LMS_FMT_I16:
    case lms_stream_t::LMS_FMT_I12:
        samplesProduced = ReceiveStream<lime::complex16_t>(stream, samples, sample_count, meta, timeout_ms);
    default:
        break;
    }

    return samplesProduced;
}

namespace {

template<class T>
int SendStream(lms_stream_t* stream, const void* samples, size_t sample_count, const lms_stream_meta_t* meta, unsigned timeout_ms)
{
    auto& handle = streamHandles.at(stream->handle);
    if (handle == nullptr || handle->parent == nullptr)
    {
        return -1;
    }

    const auto direction = stream->isTx ? lime::TRXDir::Tx : lime::TRXDir::Rx;
    if (direction == lime::TRXDir::Rx)
    {
        lime::error("Invalid direction.");
        return -1;
    }

    const uint32_t streamChannel = stream->channel & ~LMS_ALIGN_CH_PHASE;
    const uint8_t txChannelCount = handle->parent->lastSavedStreamConfig.txCount;
    const std::size_t sampleSize = sizeof(T);

    std::vector<const T*> sampleBuffer(txChannelCount, nullptr);

    for (auto& buffer : handle->parent->streamBuffers)
    {
        if (buffer.direction == direction)
        {
            for (uint8_t i = 0; i < txChannelCount; ++i)
            {
                if (handle->parent->lastSavedStreamConfig.txChannels[i] == buffer.channel)
                {
                    sampleBuffer[i] = reinterpret_cast<const T*>(buffer.buffer);
                    break;
                }
            }
        }
    }

    for (uint8_t i = 0; i < txChannelCount; ++i)
    {
        if (handle->parent->lastSavedStreamConfig.txChannels[i] == streamChannel)
        {
            sampleBuffer[i] = reinterpret_cast<const T*>(samples);
            break;
        }
    }

    if (std::any_of(sampleBuffer.begin(), sampleBuffer.end(), [](const T* item) { return item == nullptr; }))
    {
        auto buffer = reinterpret_cast<T*>(handle->memoryPool.Allocate(sample_count * sampleSize));
        std::memcpy(buffer, samples, sample_count * sampleSize);
        handle->parent->streamBuffers.push_back(
            { reinterpret_cast<void*>(buffer), &handle->memoryPool, direction, static_cast<uint8_t>(streamChannel), 0 });

        // Can't really know what to return here just yet, so just returning that all of them have passed through.
        return sample_count;
    }

    lime::SDRDevice::StreamMeta metadata{ 0, false, false };

    if (meta != nullptr)
    {
        metadata.flush = meta->flushPartialPacket;
        metadata.useTimestamp = meta->waitForTimestamp;
        metadata.timestamp = meta->timestamp;
    }

    int samplesSent = handle->parent->device->StreamTx(handle->parent->moduleIndex, sampleBuffer.data(), sample_count, &metadata);

    for (auto it = handle->parent->streamBuffers.begin(); it != handle->parent->streamBuffers.end(); it++)
    {
        auto& buffer = *it;
        if (buffer.direction == direction && buffer.channel != streamChannel)
        {
            buffer.ownerMemoryPool->Free(buffer.buffer);

            handle->parent->streamBuffers.erase(it--);
        }
    }

    return samplesSent;
}

} // namespace

API_EXPORT int CALL_CONV LMS_SendStream(
    lms_stream_t* stream, const void* samples, size_t sample_count, const lms_stream_meta_t* meta, unsigned timeout_ms)
{
    if (stream == nullptr || stream->handle < 0)
    {
        return -1;
    }

    int samplesSent = 0;

    switch (stream->dataFmt)
    {
    case lms_stream_t::LMS_FMT_F32:
        samplesSent = SendStream<lime::complex32f_t>(stream, samples, sample_count, meta, timeout_ms);
        break;
    case lms_stream_t::LMS_FMT_I16:
    case lms_stream_t::LMS_FMT_I12:
        samplesSent = SendStream<lime::complex16_t>(stream, samples, sample_count, meta, timeout_ms);
    default:
        break;
    }

    return samplesSent;
}

API_EXPORT int CALL_CONV LMS_GetStreamStatus(lms_stream_t* stream, lms_stream_status_t* status)
{
    if (stream == nullptr || stream->handle < 0)
    {
        return -1;
    }

    auto& handle = streamHandles.at(stream->handle);
    if (handle == nullptr || handle->parent == nullptr)
    {
        return -1;
    }

    if (status == nullptr)
    {
        return -1;
    }

    lime::SDRDevice::StreamStats stats;
    lime::TRXDir direction = stream->isTx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    switch (direction)
    {
    case lime::TRXDir::Rx:
        handle->parent->device->StreamStatus(handle->parent->moduleIndex, &stats, nullptr);
        break;
    case lime::TRXDir::Tx:
        handle->parent->device->StreamStatus(handle->parent->moduleIndex, nullptr, &stats);
        break;
    default:
        break;
    }

    status->active = handle->isStreamStartedFromAPI;
    status->fifoFilledCount = stats.FIFO.usedCount;
    status->fifoSize = stats.FIFO.totalCount;

    handle->parent->statsDeltas.underrun.set(stats.underrun);
    status->underrun = handle->parent->statsDeltas.underrun.delta();
    handle->parent->statsDeltas.underrun.checkpoint();

    handle->parent->statsDeltas.overrun.set(stats.overrun);
    status->overrun = handle->parent->statsDeltas.overrun.delta();
    handle->parent->statsDeltas.overrun.checkpoint();

    handle->parent->statsDeltas.droppedPackets.set(stats.loss);
    status->droppedPackets = handle->parent->statsDeltas.underrun.delta();
    handle->parent->statsDeltas.droppedPackets.checkpoint();

    // status->sampleRate; // Is noted as unused
    status->linkRate = stats.dataRate_Bps;
    status->timestamp = stats.timestamp;

    return 0;
}

API_EXPORT int CALL_CONV LMS_GPIORead(lms_device_t* dev, uint8_t* buffer, size_t len)
{
    LMS_APIDevice* apiDevice = CheckDevice(dev);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    return apiDevice->device->GPIORead(buffer, len);
}

API_EXPORT int CALL_CONV LMS_GPIOWrite(lms_device_t* dev, const uint8_t* buffer, size_t len)
{
    LMS_APIDevice* apiDevice = CheckDevice(dev);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    return apiDevice->device->GPIOWrite(buffer, len);
}

API_EXPORT int CALL_CONV LMS_GPIODirRead(lms_device_t* dev, uint8_t* buffer, size_t len)
{
    LMS_APIDevice* apiDevice = CheckDevice(dev);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    return apiDevice->device->GPIODirRead(buffer, len);
}

API_EXPORT int CALL_CONV LMS_GPIODirWrite(lms_device_t* dev, const uint8_t* buffer, size_t len)
{
    LMS_APIDevice* apiDevice = CheckDevice(dev);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    return apiDevice->device->GPIODirWrite(buffer, len);
}

API_EXPORT int CALL_CONV LMS_ReadCustomBoardParam(lms_device_t* device, uint8_t param_id, float_type* val, lms_name_t units)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    std::vector<lime::CustomParameterIO> parameter{ { param_id, *val, units } };
    int returnValue = apiDevice->device->CustomParameterRead(parameter);

    if (returnValue < 0)
    {
        return -1;
    }

    *val = parameter[0].value;
    if (units != nullptr)
    {
        CopyString(parameter[0].units, units, sizeof(lms_name_t));
    }

    return returnValue;
}

API_EXPORT int CALL_CONV LMS_WriteCustomBoardParam(lms_device_t* device, uint8_t param_id, float_type val, const lms_name_t units)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    std::vector<lime::CustomParameterIO> parameter{ { param_id, val, units } };

    return apiDevice->device->CustomParameterWrite(parameter);
}

API_EXPORT const lms_dev_info_t* CALL_CONV LMS_GetDeviceInfo(lms_device_t* device)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return nullptr;
    }

    auto descriptor = apiDevice->device->GetDescriptor();

    if (apiDevice->deviceInfo == nullptr)
    {
        apiDevice->deviceInfo = new lms_dev_info_t;
    }

    CopyString(descriptor.name, apiDevice->deviceInfo->deviceName, sizeof(apiDevice->deviceInfo->deviceName));
    CopyString(descriptor.expansionName, apiDevice->deviceInfo->expansionName, sizeof(apiDevice->deviceInfo->expansionName));
    CopyString(descriptor.firmwareVersion, apiDevice->deviceInfo->firmwareVersion, sizeof(apiDevice->deviceInfo->firmwareVersion));
    CopyString(descriptor.hardwareVersion, apiDevice->deviceInfo->hardwareVersion, sizeof(apiDevice->deviceInfo->hardwareVersion));
    CopyString(descriptor.protocolVersion, apiDevice->deviceInfo->protocolVersion, sizeof(apiDevice->deviceInfo->protocolVersion));
    apiDevice->deviceInfo->boardSerialNumber = descriptor.serialNumber;
    CopyString(descriptor.gatewareVersion, apiDevice->deviceInfo->gatewareVersion, sizeof(apiDevice->deviceInfo->gatewareVersion));
    CopyString(descriptor.gatewareTargetBoard,
        apiDevice->deviceInfo->gatewareTargetBoard,
        sizeof(apiDevice->deviceInfo->gatewareTargetBoard));

    return apiDevice->deviceInfo;
}

API_EXPORT const char* LMS_GetLibraryVersion()
{
    static constexpr std::size_t LIBRARY_VERSION_SIZE = 32;
    static char libraryVersion[LIBRARY_VERSION_SIZE];

    CopyString(lime::GetLibraryVersion(), libraryVersion, sizeof(libraryVersion));
    return libraryVersion;
}

API_EXPORT int CALL_CONV LMS_GetClockFreq(lms_device_t* device, size_t clk_id, float_type* freq)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    *freq = apiDevice->device->GetClockFreq(clk_id, apiDevice->moduleIndex * 2);
    return *freq > 0 ? 0 : -1;
}

API_EXPORT int CALL_CONV LMS_SetClockFreq(lms_device_t* device, size_t clk_id, float_type freq)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    try
    {
        apiDevice->device->SetClockFreq(clk_id, freq, apiDevice->moduleIndex * 2);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetChipTemperature(lms_device_t* dev, size_t ind, float_type* temp)
{
    *temp = 0;

    LMS_APIDevice* apiDevice = CheckDevice(dev);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(ind));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    if (lms->SPI_read(0x2F) == 0x3840)
    {
        lime::error("Feature is not available on this chip revision.");
        return -1;
    }

    *temp = lms->GetTemperature();
    return 0;
}

API_EXPORT int CALL_CONV LMS_Synchronize(lms_device_t* dev, bool toChip)
{
    LMS_APIDevice* apiDevice = CheckDevice(dev);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    try
    {
        apiDevice->device->Synchronize(toChip);
    } catch (...)
    {
        return -1;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_EnableCache(lms_device_t* dev, bool enable)
{
    LMS_APIDevice* apiDevice = CheckDevice(dev);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    try
    {
        apiDevice->device->EnableCache(enable);
    } catch (...)
    {
        return -1;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetLPFBW(lms_device_t* device, bool dir_tx, size_t chan, float_type* bandwidth)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;

    if (dir_tx)
    {
        *bandwidth = config.channel[chan].tx.lpf;
    }
    else
    {
        *bandwidth = config.channel[chan].rx.lpf;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetLPF(lms_device_t* device, bool dir_tx, size_t chan, bool enabled)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;

    if (enabled)
    {
        if (dir_tx)
        {
            config.channel[chan].tx.lpf = apiDevice->lastSavedLPFValue[chan][dir_tx];
        }
        else
        {
            config.channel[chan].rx.lpf = apiDevice->lastSavedLPFValue[chan][dir_tx];
        }
    }
    else
    {
        if (dir_tx)
        {
            config.channel[chan].tx.lpf = 130e6;
        }
        else
        {
            config.channel[chan].rx.lpf = 130e6;
        }
    }

    try
    {
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, apiDevice->moduleIndex);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetTestSignal(lms_device_t* device, bool dir_tx, size_t chan, lms_testsig_t* sig)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(chan / 2));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    if (dir_tx)
    {
        if (lms->Get_SPI_Reg_bits(LMS7param(INSEL_TXTSP)) == 0)
        {
            *sig = static_cast<lms_testsig_t>(LMS_TESTSIG_NONE);
            return 0;
        }
        else if (lms->Get_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP)) != 0)
        {
            *sig = static_cast<lms_testsig_t>(LMS_TESTSIG_DC);
            return 0;
        }
        else
        {
            *sig = static_cast<lms_testsig_t>(
                lms->Get_SPI_Reg_bits(LMS7param(TSGFCW_TXTSP)) + 2 * lms->Get_SPI_Reg_bits(LMS7param(TSGFC_TXTSP), true));
            return 0;
        }
    }
    else
    {
        if (lms->Get_SPI_Reg_bits(LMS7param(INSEL_RXTSP)) == 0)
        {
            *sig = static_cast<lms_testsig_t>(LMS_TESTSIG_NONE);
            return 0;
        }
        else if (lms->Get_SPI_Reg_bits(LMS7param(TSGMODE_RXTSP)) != 0)
        {
            *sig = static_cast<lms_testsig_t>(LMS_TESTSIG_DC);
            return 0;
        }
        else
        {
            *sig = static_cast<lms_testsig_t>(
                lms->Get_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP)) + 2 * lms->Get_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), true));
            return 0;
        }
    }

    return -1;
}

API_EXPORT int CALL_CONV LMS_LoadConfig(lms_device_t* device, const char* filename)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(apiDevice->moduleIndex));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    return lms ? lms->LoadConfig(filename) : -1;
}

API_EXPORT int CALL_CONV LMS_SaveConfig(lms_device_t* device, const char* filename)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(apiDevice->moduleIndex));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    return lms ? lms->SaveConfig(filename) : -1;
}

API_EXPORT void LMS_RegisterLogHandler(LMS_LogHandler handler)
{
    if (handler != nullptr)
    {
        lime::registerLogHandler(APIMsgHandler);
        api_msg_handler = handler;
    }

    lime::registerLogHandler(nullptr);
}

API_EXPORT const char* CALL_CONV LMS_GetLastErrorMessage(void)
{
    return lime::GetLastErrorMessage();
}

API_EXPORT int CALL_CONV LMS_SetGFIRLPF(lms_device_t* device, bool dir_tx, size_t chan, bool enabled, float_type bandwidth)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(chan / 2));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    return lms->SetGFIRFilter(dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx, chan & 1, enabled, bandwidth);
}

API_EXPORT int CALL_CONV LMS_SetGFIRCoeff(
    lms_device_t* device, bool dir_tx, size_t chan, lms_gfir_t filt, const float_type* coef, size_t count)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(chan / 2));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    std::vector<int16_t> convertedCoefficients(count);

    for (std::size_t i = 0; i < count; ++i)
    {
        convertedCoefficients[i] = static_cast<int16_t>(coef[i]);
    }

    return lms->SetGFIRCoefficients(
        dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx, static_cast<uint8_t>(filt), convertedCoefficients.data(), count);
}

API_EXPORT int CALL_CONV LMS_GetGFIRCoeff(lms_device_t* device, bool dir_tx, size_t chan, lms_gfir_t filt, float_type* coef)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(chan / 2));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    const uint8_t count = filt == LMS_GFIR3 ? 120 : 40;
    std::vector<int16_t> coefficientBuffer(count);

    auto returnValue = lms->GetGFIRCoefficients(
        dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx, static_cast<uint8_t>(filt), coefficientBuffer.data(), count);

    for (std::size_t i = 0; i < count; ++i)
    {
        coef[i] = static_cast<float_type>(coefficientBuffer[i]);
    }

    return returnValue;
}

API_EXPORT int CALL_CONV LMS_SetGFIR(lms_device_t* device, bool dir_tx, size_t chan, lms_gfir_t filt, bool enabled)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(chan / 2));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), (chan % 2) + 1);

    if (dir_tx)
    {
        if (filt == LMS_GFIR1)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_TXTSP), enabled == false) != 0)
                return -1;
        }
        else if (filt == LMS_GFIR2)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_TXTSP), enabled == false) != 0)
                return -1;
        }
        else if (filt == LMS_GFIR3)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_TXTSP), enabled == false) != 0)
                return -1;
        }
    }
    else
    {
        if (filt == LMS_GFIR1)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), enabled == false) != 0)
                return -1;
        }
        else if (filt == LMS_GFIR2)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), enabled == false) != 0)
                return -1;
        }
        else if (filt == LMS_GFIR3)
        {
            if (lms->Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), enabled == false) != 0)
                return -1;
        }
        bool sisoDDR = lms->Get_SPI_Reg_bits(LMS7_LML1_SISODDR);
        if (chan % 2)
        {
            lms->Modify_SPI_Reg_bits(LMS7param(CDSN_RXBLML), !(enabled | sisoDDR));
            lms->Modify_SPI_Reg_bits(LMS7param(CDS_RXBLML), enabled ? 3 : 0);
        }
        else
        {
            lms->Modify_SPI_Reg_bits(LMS7param(CDSN_RXALML), !(enabled | sisoDDR));
            lms->Modify_SPI_Reg_bits(LMS7param(CDS_RXALML), enabled ? 3 : 0);
        }
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_ReadParam(lms_device_t* device, struct LMS7Parameter param, uint16_t* val)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(apiDevice->moduleIndex));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    *val = lms->Get_SPI_Reg_bits(param);
    return 0;
}

API_EXPORT int CALL_CONV LMS_WriteParam(lms_device_t* device, struct LMS7Parameter param, uint16_t val)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(apiDevice->moduleIndex));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    return lms->Modify_SPI_Reg_bits(param, val);
}

API_EXPORT int CALL_CONV LMS_SetNCOFrequency(lms_device_t* device, bool dir_tx, size_t ch, const float_type* freq, float_type pho)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, ch);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(ch / 2));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    if (freq != nullptr)
    {
        if (lms->SetNCOFrequencies(dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx, freq, LMS_NCO_VAL_COUNT, pho) != 0)
        {
            return -1;
        }

        lms->Modify_SPI_Reg_bits(dir_tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP, 0);
        lms->Modify_SPI_Reg_bits(dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX, 0, ch);
    }

    return lms->SetNCOPhaseOffsetForMode0(dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx, pho);
}

API_EXPORT int CALL_CONV LMS_GetNCOFrequency(lms_device_t* device, bool dir_tx, size_t chan, float_type* freq, float_type* pho)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(chan / 2));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    if (freq != nullptr)
    {
        for (unsigned i = 0; i < LMS_NCO_VAL_COUNT; i++)
        {
            freq[i] = std::fabs(lms->GetNCOFrequency(dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx, i));
        }
    }

    if (pho != nullptr)
    {
        uint16_t value = lms->SPI_read(dir_tx ? 0x0241 : 0x0441);
        *pho = 360.0 * value / 65536.0;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetNCOPhase(lms_device_t* device, bool dir_tx, size_t ch, const float_type* phase, float_type fcw)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, ch);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(ch / 2));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    if (lms->SetNCOFrequency(dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx, 0, fcw) != 0)
        return -1;

    if (phase != nullptr)
    {
        if (lms->SetNCOPhases(dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx, phase, LMS_NCO_VAL_COUNT, fcw) != 0)
        {
            return -1;
        }

        if (lms->Modify_SPI_Reg_bits(dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX, 0, ch) != 0)
        {
            return -1;
        }
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetNCOPhase(lms_device_t* device, bool dir_tx, size_t ch, float_type* phase, float_type* fcw)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, ch);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(ch / 2));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    if (phase != nullptr)
    {
        auto phases = lms->GetNCOPhases(dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx, fcw);

        for (std::size_t i = 0; i < phases.size(); ++i)
        {
            phase[i] = phases[i];
        }
    }

    if (fcw != nullptr)
    {
        *fcw = lms->GetNCOFrequency(dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx, 0);
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetNCOIndex(lms_device_t* device, bool dir_tx, size_t chan, int ind, bool down)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(chan / 2));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    if ((lms->Modify_SPI_Reg_bits(dir_tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP, ind < 0 ? 1 : 0, chan) != 0) ||
        (lms->Modify_SPI_Reg_bits(dir_tx ? LMS7_CMIX_GAIN_TXTSP : LMS7_CMIX_GAIN_RXTSP, ind < 0 ? 0 : 1, chan) != 0))
    {
        return -1;
    }

    if (ind < LMS_NCO_VAL_COUNT)
    {
        if ((lms->Modify_SPI_Reg_bits(dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX, ind) != 0) ||
            (lms->Modify_SPI_Reg_bits(dir_tx ? LMS7_CMIX_SC_TXTSP : LMS7_CMIX_SC_RXTSP, down) != 0))
        {
            return -1;
        }
    }
    else
    {
        lime::error("Invalid NCO index value.");
        return -1;
    }
    return 0;
}

API_EXPORT int CALL_CONV LMS_GetNCOIndex(lms_device_t* device, bool dir_tx, size_t chan)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(chan / 2));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    if (lms->Get_SPI_Reg_bits(dir_tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP, chan) != 0)
    {
        lime::error("NCO is disabled.");
        return -1;
    }

    return lms->Get_SPI_Reg_bits(dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX, chan);
}

API_EXPORT int CALL_CONV LMS_WriteLMSReg(lms_device_t* device, uint32_t address, uint16_t val)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(apiDevice->moduleIndex));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    return lms->SPI_write(address, val);
}

API_EXPORT int CALL_CONV LMS_ReadLMSReg(lms_device_t* device, uint32_t address, uint16_t* val)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M* lms = static_cast<lime::LMS7002M*>(apiDevice->device->GetInternalChip(apiDevice->moduleIndex));
    if (lms == nullptr)
    {
        lime::error("Device is not an LMS device.");
        return -1;
    }

    *val = lms->SPI_read(address);
    return 0;
}

API_EXPORT int CALL_CONV LMS_WriteFPGAReg(lms_device_t* device, uint32_t address, uint16_t val)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M_SDRDevice* sdrDevice = dynamic_cast<lime::LMS7002M_SDRDevice*>(apiDevice->device);
    if (sdrDevice == nullptr)
    {
        lime::error("Device is not an LMS SDR device.");
        return -1;
    }

    return sdrDevice->WriteFPGARegister(address, val);
}

API_EXPORT int CALL_CONV LMS_ReadFPGAReg(lms_device_t* device, uint32_t address, uint16_t* val)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::LMS7002M_SDRDevice* sdrDevice = dynamic_cast<lime::LMS7002M_SDRDevice*>(apiDevice->device);
    if (sdrDevice == nullptr)
    {
        lime::error("Device is not an LMS SDR device.");
        return -1;
    }

    int value = sdrDevice->ReadFPGARegister(address);

    if (value < 0)
    {
        return value; // operation failed return error code
    }

    if (val != nullptr)
    {
        *val = value;
    }

    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_UploadWFM(lms_device_t* device, const void** samples, uint8_t chCount, size_t sample_count, int format)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    auto config = apiDevice->lastSavedStreamConfig;

    lime::SDRDevice::StreamConfig::DataFormat dataFormat;
    switch (format)
    {
    case 0:
        dataFormat = lime::SDRDevice::StreamConfig::DataFormat::I12;
        break;
    case 1:
        dataFormat = lime::SDRDevice::StreamConfig::DataFormat::I16;
        break;
    case 2:
        dataFormat = lime::SDRDevice::StreamConfig::DataFormat::F32;
        break;
    default:
        dataFormat = lime::SDRDevice::StreamConfig::DataFormat::I12;
        break;
    }

    config.txCount = chCount;
    config.format = dataFormat;

    return apiDevice->device->UploadTxWaveform(config, apiDevice->moduleIndex, samples, sample_count);
}

API_EXPORT int CALL_CONV LMS_EnableTxWFM(lms_device_t* device, unsigned ch, bool active)
{
    uint16_t regAddr = 0x000D;
    uint16_t regValue = 0;

    int status = LMS_WriteFPGAReg(device, 0xFFFF, 1 << (ch / 2));
    if (status != 0)
    {
        return status;
    }

    status = LMS_ReadFPGAReg(device, regAddr, &regValue);
    if (status != 0)
    {
        return status;
    }

    regValue = regValue & ~0x6; //clear WFM_LOAD, WFM_PLAY
    regValue |= active << 1;
    status = LMS_WriteFPGAReg(device, regAddr, regValue);
    return status;
}

API_EXPORT int CALL_CONV LMS_GetProgramModes(lms_device_t* device, lms_name_t* list)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    const auto& memoryDevices = apiDevice->device->GetDescriptor().memoryDevices;
    if (list != nullptr)
    {
        std::size_t index = 0;

        for (const auto& memoryDevice : memoryDevices)
        {
            CopyString(memoryDevice.first, list[index++], sizeof(lms_name_t));
        }
    }

    return memoryDevices.size();
}

API_EXPORT int CALL_CONV LMS_Program(
    lms_device_t* device, const char* data, size_t size, const lms_name_t mode, lms_prog_callback_t callback)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    std::string prog_mode{ mode };

    try
    {
        programmingCallback = callback;

        const auto& memoryDevice = apiDevice->device->GetDescriptor().memoryDevices.at(prog_mode);

        return memoryDevice->ownerDevice->UploadMemory(memoryDevice->memoryDeviceType, 0, data, size, ProgrammingCallback);
    } catch (std::out_of_range& e)
    {
        lime::error("Mode not found.");

        return -1;
    }
}

API_EXPORT int CALL_CONV LMS_VCTCXOWrite(lms_device_t* device, uint16_t val)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    if (LMS_WriteCustomBoardParam(device, BOARD_PARAM_DAC, val, "") < 0)
    {
        return -1;
    }

    auto memoryDevice = lime::eMemoryDevice::EEPROM;
    try
    {
        const auto& dataStorage = apiDevice->device->GetDescriptor().memoryDevices.at(lime::MEMORY_DEVICES_TEXT.at(memoryDevice));
        try
        {
            return apiDevice->device->MemoryWrite(dataStorage, lime::eMemoryRegion::VCTCXO_DAC, &val);
        } catch (std::out_of_range& e)
        {
            lime::error("VCTCXO address not found.");

            return -1;
        }
    } catch (std::out_of_range& e)
    {
        lime::error("EEPROM not found.");

        return -1;
    } catch (...)
    {
        return -1;
    }
}

namespace {

static int VCTCXOReadFallbackPath(LMS_APIDevice* apiDevice, uint16_t* val)
{
    std::vector<lime::CustomParameterIO> parameters{ { BOARD_PARAM_DAC, 0, "" } };

    if (apiDevice->device->CustomParameterRead(parameters) != 0)
    {
        return -1;
    }

    *val = parameters.at(0).value;
    return 0;
}

} // namespace

API_EXPORT int CALL_CONV LMS_VCTCXORead(lms_device_t* device, uint16_t* val)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    auto memoryDevice = lime::eMemoryDevice::EEPROM;
    try
    {
        const auto& dataStorage = apiDevice->device->GetDescriptor().memoryDevices.at(lime::MEMORY_DEVICES_TEXT.at(memoryDevice));
        try
        {
            return apiDevice->device->MemoryRead(dataStorage, lime::eMemoryRegion::VCTCXO_DAC, val);
        } catch (std::out_of_range& e)
        {
            lime::error("VCTCXO address not found.");
            return VCTCXOReadFallbackPath(apiDevice, val);
        }
    } catch (std::out_of_range& e)
    {
        lime::warning("EEPROM not found.");
        return VCTCXOReadFallbackPath(apiDevice, val);
    } catch (...)
    {
        return -1;
    }
}
