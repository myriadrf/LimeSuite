#include "CommonFunctions.h"
#include "lime/LimeSuite.h"
#include "limesuite/commonTypes.h"
#include "limesuite/DeviceHandle.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/GainTypes.h"
#include "limesuite/SDRDevice.h"
#include "Logger.h"
#include "MemoryPool.h"
#include "VersionInfo.h"

#include <algorithm>
#include <array>
#include <cassert>
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
    lime::SDRDevice::StreamConfig lastSavedStreamConfig;
    std::array<std::array<float_type, 2>, lime::SDRDevice::MAX_CHANNEL_COUNT> lastSavedLPFValue;
    StatsDeltas statsDeltas;

    uint8_t moduleIndex;

    std::vector<StreamBuffer> streamBuffers;
    lms_dev_info_t* deviceInfo;

    lime::eGainTypes rxGain;
    lime::eGainTypes txGain;

    LMS_APIDevice() = delete;
    LMS_APIDevice(lime::SDRDevice* device)
        : device(device)
        , lastSavedStreamConfig()
        , lastSavedLPFValue()
        , statsDeltas()
        , moduleIndex(0)
        , streamBuffers()
        , deviceInfo(nullptr)
        , rxGain(lime::eGainTypes::UNKNOWN)
        , txGain(lime::eGainTypes::UNKNOWN)
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

    const lime::SDRDevice::RFSOCDescriptor& GetRFSOCDescriptor() const
    {
        assert(device);
        return device->GetDescriptor().rfSOC.at(moduleIndex);
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

    if (chan >= apiDevice->GetRFSOCDescriptor().channelCount)
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

inline lms_range_t RangeToLMS_Range(const lime::Range& range)
{
    return { range.min, range.max, range.step };
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

    lime::TRXDir direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;
    try
    {
        apiDevice->device->EnableChannel(apiDevice->moduleIndex, direction, chan, enabled);
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

    try
    {
        apiDevice->device->SetSampleRate(apiDevice->moduleIndex, lime::TRXDir::Rx, 0, rate, oversample);
    } catch (...)
    {
        lime::error("Failed to set sampling rate.");

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

    lime::TRXDir direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    try
    {
        apiDevice->device->SetSampleRate(apiDevice->moduleIndex, direction, 0, rate, oversample);

    } catch (...)
    {
        lime::error("Failed to set %s sampling rate.", ToString(direction).c_str());

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

    lime::TRXDir direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;
    auto rate = apiDevice->device->GetSampleRate(apiDevice->moduleIndex, direction, 0);

    if (host_Hz)
        *host_Hz = rate;
    if (rf_Hz)
        *rf_Hz = rate;

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetSampleRateRange(lms_device_t* device, bool dir_tx, lms_range_t* range)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    if (range)
        *range = RangeToLMS_Range(apiDevice->GetRFSOCDescriptor().samplingRateRange);

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetNumChannels(lms_device_t* device, bool dir_tx)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    return apiDevice->GetRFSOCDescriptor().channelCount;
}

API_EXPORT int CALL_CONV LMS_SetLOFrequency(lms_device_t* device, bool dir_tx, size_t chan, float_type frequency)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::TRXDir direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    try
    {
        apiDevice->device->SetFrequency(apiDevice->moduleIndex, direction, chan, frequency);
    } catch (...)
    {
        lime::error("Failed to set %s LO frequency.", ToString(direction).c_str());

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

    if (frequency)
        *frequency = apiDevice->device->GetFrequency(apiDevice->moduleIndex, dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx, chan);

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetLOFrequencyRange(lms_device_t* device, bool dir_tx, lms_range_t* range)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    if (range)
        *range = RangeToLMS_Range(apiDevice->GetRFSOCDescriptor().frequencyRange);

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetAntennaList(lms_device_t* device, bool dir_tx, size_t chan, lms_name_t* list)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    const auto& rfSOC = apiDevice->GetRFSOCDescriptor();
    const auto& strings = rfSOC.pathNames.at(dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx);
    for (std::size_t i = 0; i < strings.size(); ++i)
    {
        CopyString(strings.at(i), list[i], sizeof(lms_name_t));
    }

    return strings.size();
}

API_EXPORT int CALL_CONV LMS_SetAntenna(lms_device_t* device, bool dir_tx, size_t chan, size_t path)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::TRXDir direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    try
    {
        apiDevice->device->SetAntenna(apiDevice->moduleIndex, direction, chan, path);
    } catch (...)
    {
        lime::error("Failed to set %s antenna.", ToString(direction).c_str());

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

    lime::TRXDir direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    return apiDevice->device->GetAntenna(apiDevice->moduleIndex, direction, chan);
}

API_EXPORT int CALL_CONV LMS_GetAntennaBW(lms_device_t* device, bool dir_tx, size_t chan, size_t path, lms_range_t* range)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::TRXDir direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;
    std::string pathName = apiDevice->GetRFSOCDescriptor().pathNames.at(direction).at(path);

    if (range)
    {
        *range = RangeToLMS_Range(apiDevice->GetRFSOCDescriptor().antennaRange.at(direction).at(pathName));
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetLPFBW(lms_device_t* device, bool dir_tx, size_t chan, float_type bandwidth)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    lime::TRXDir direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    try
    {
        apiDevice->device->SetLowPassFilter(apiDevice->moduleIndex, direction, chan, bandwidth);
        apiDevice->lastSavedLPFValue[chan][dir_tx] = bandwidth;
    } catch (...)
    {
        lime::error("Failed to set %s LPF bandwidth.", ToString(direction).c_str());

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

    lime::TRXDir direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    if (range)
        *range = RangeToLMS_Range(apiDevice->GetRFSOCDescriptor().lowPassFilterRange.at(direction));

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetNormalizedGain(lms_device_t* device, bool dir_tx, size_t chan, float_type gain)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    gain = std::clamp(gain, 0.0, 1.0);

    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;
    auto gainToUse = dir_tx ? apiDevice->txGain : apiDevice->rxGain;

    auto range = apiDevice->GetRFSOCDescriptor().gainRange.at(direction).at(gainToUse);

    gain = range.min + gain * (range.max - range.min);

    try
    {
        apiDevice->device->SetGain(apiDevice->moduleIndex, direction, chan, gainToUse, gain);
    } catch (...)
    {
        lime::error("Failed to set %s normalized gain.", ToString(direction).c_str());

        return -1;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetGaindB(lms_device_t* device, bool dir_tx, size_t chan, unsigned gain)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;
    auto gainToUse = dir_tx ? apiDevice->txGain : apiDevice->rxGain;

    try
    {
        apiDevice->device->SetGain(apiDevice->moduleIndex, direction, chan, gainToUse, gain);
    } catch (...)
    {
        lime::error("Failed to set %s gain.", ToString(direction).c_str());

        return -1;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetNormalizedGain(lms_device_t* device, bool dir_tx, size_t chan, float_type* gain)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;
    auto gainToUse = dir_tx ? apiDevice->txGain : apiDevice->rxGain;

    auto range = apiDevice->GetRFSOCDescriptor().gainRange.at(direction).at(gainToUse);

    double deviceGain = 0.0;
    int returnValue = apiDevice->device->GetGain(apiDevice->moduleIndex, direction, chan, gainToUse, deviceGain);

    if (gain)
        *gain = (deviceGain - range.min) / (range.max - range.min);

    return returnValue;
}

API_EXPORT int CALL_CONV LMS_GetGaindB(lms_device_t* device, bool dir_tx, size_t chan, unsigned* gain)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;
    auto gainToUse = dir_tx ? apiDevice->txGain : apiDevice->rxGain;
    auto deviceGain = 0.0;

    int returnValue = apiDevice->device->GetGain(apiDevice->moduleIndex, direction, chan, gainToUse, deviceGain);

    if (gain)
        *gain = std::lround(deviceGain) + 12;

    return returnValue;
}

API_EXPORT int CALL_CONV LMS_Calibrate(lms_device_t* device, bool dir_tx, size_t chan, double bw, unsigned flags)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    try
    {
        apiDevice->device->Calibrate(apiDevice->moduleIndex, direction, chan, bw);
    } catch (...)
    {
        lime::error("Failed to calibrate %s channel %i.", ToString(direction).c_str(), chan);

        return -1;
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

    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    auto enumToTestStruct = [](lms_testsig_t signal) -> lime::SDRDevice::ChannelConfig::Direction::TestSignal {
        switch (signal)
        {
        case LMS_TESTSIG_NONE:
            return { false };
        case LMS_TESTSIG_DC:
            return { true, true };
        case LMS_TESTSIG_NCODIV8:
            return { true,
                false,
                lime::SDRDevice::ChannelConfig::Direction::TestSignal::Divide::Div8,
                lime::SDRDevice::ChannelConfig::Direction::TestSignal::Scale::Half };
        case LMS_TESTSIG_NCODIV4:
            return { true,
                false,
                lime::SDRDevice::ChannelConfig::Direction::TestSignal::Divide::Div4,
                lime::SDRDevice::ChannelConfig::Direction::TestSignal::Scale::Half };
        case LMS_TESTSIG_NCODIV8F:
            return { true,
                false,
                lime::SDRDevice::ChannelConfig::Direction::TestSignal::Divide::Div8,
                lime::SDRDevice::ChannelConfig::Direction::TestSignal::Scale::Full };
        case LMS_TESTSIG_NCODIV4F:
            return { true,
                false,
                lime::SDRDevice::ChannelConfig::Direction::TestSignal::Divide::Div4,
                lime::SDRDevice::ChannelConfig::Direction::TestSignal::Scale::Full };
        default:
            throw std::logic_error("Unexpected enumerator lms_testsig_t value");
        }
    };

    try {
        apiDevice->device->SetTestSignal(apiDevice->moduleIndex, direction, chan, enumToTestStruct(sig), dc_i, dc_q);
    }
    catch (...)
    {
        lime::error("Failed to set %s channel %i test signal.", ToString(direction).c_str(), chan);
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

    lime::SDRDevice::StreamConfig config = apiDevice->lastSavedStreamConfig;
    config.bufferSize = stream->fifoSize;

    auto channel = stream->channel & ~LMS_ALIGN_CH_PHASE; // Clear the align phase bit

    config.channels.at(stream->isTx ? lime::TRXDir::Tx : lime::TRXDir::Rx).push_back(channel);

    config.alignPhase = stream->channel & LMS_ALIGN_CH_PHASE;

    switch (stream->dataFmt)
    {
    case lms_stream_t::LMS_FMT_F32:
        config.format = lime::SDRDevice::StreamConfig::DataFormat::F32;
        break;
    case lms_stream_t::LMS_FMT_I16:
        config.format = lime::SDRDevice::StreamConfig::DataFormat::I16;
        break;
    case lms_stream_t::LMS_FMT_I12:
        config.format = lime::SDRDevice::StreamConfig::DataFormat::I12;
        break;
    default:
        return lime::error("Setup stream failed: invalid data format.");
    }

    switch (stream->linkFmt)
    {
    case lms_stream_t::LMS_LINK_FMT_I16:
        config.linkFormat = lime::SDRDevice::StreamConfig::DataFormat::I16;
        break;
    case lms_stream_t::LMS_LINK_FMT_I12:
    case lms_stream_t::LMS_LINK_FMT_DEFAULT:
        config.linkFormat = lime::SDRDevice::StreamConfig::DataFormat::I12;
        break;
    default:
        return lime::error("Setup stream failed: invalid link data format.");
    }

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
    const std::size_t rxChannelCount = handle->parent->lastSavedStreamConfig.channels.at(lime::TRXDir::Rx).size();
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
        if (handle->parent->lastSavedStreamConfig.channels.at(lime::TRXDir::Rx).at(i) == streamChannel)
        {
            sampleBuffer[i] = reinterpret_cast<T*>(samples);
        }
        else
        {
            sampleBuffer[i] = reinterpret_cast<T*>(handle->memoryPool.Allocate(sample_count * sampleSize));

            handle->parent->streamBuffers.push_back({ sampleBuffer[i],
                &handle->memoryPool,
                direction,
                handle->parent->lastSavedStreamConfig.channels.at(lime::TRXDir::Rx).at(i),
                0 });
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
    const std::size_t txChannelCount = handle->parent->lastSavedStreamConfig.channels.at(lime::TRXDir::Tx).size();
    const std::size_t sampleSize = sizeof(T);

    std::vector<const T*> sampleBuffer(txChannelCount, nullptr);

    for (auto& buffer : handle->parent->streamBuffers)
    {
        if (buffer.direction == direction)
        {
            for (std::size_t i = 0; i < txChannelCount; ++i)
            {
                if (handle->parent->lastSavedStreamConfig.channels.at(lime::TRXDir::Tx).at(i) == buffer.channel)
                {
                    sampleBuffer[i] = reinterpret_cast<const T*>(buffer.buffer);
                    break;
                }
            }
        }
    }

    for (uint8_t i = 0; i < txChannelCount; ++i)
    {
        if (handle->parent->lastSavedStreamConfig.channels.at(lime::TRXDir::Tx).at(i) == streamChannel)
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

    if (val)
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

    const auto& descriptor = apiDevice->device->GetDescriptor();

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

    if (freq)
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
        lime::error("Failed to set clock%li.", clk_id);

        return -1;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetChipTemperature(lms_device_t* dev, size_t ind, float_type* temp)
{
    LMS_APIDevice* apiDevice = CheckDevice(dev);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    if (temp)
        *temp = apiDevice->device->GetTemperature(apiDevice->moduleIndex);
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

    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    if (bandwidth)
        *bandwidth = apiDevice->device->GetLowPassFilter(apiDevice->moduleIndex, direction, chan);

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetLPF(lms_device_t* device, bool dir_tx, size_t chan, bool enabled)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    try
    {
        apiDevice->device->SetLowPassFilter(
            apiDevice->moduleIndex, direction, chan, apiDevice->lastSavedLPFValue[chan][dir_tx]); // TODO: fix
    } catch (...)
    {
        lime::error("Failed to set %s channel %li LPF.", ToString(direction).c_str(), chan);

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

    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    auto testSignal = apiDevice->device->GetTestSignal(apiDevice->moduleIndex, direction, chan);

    if (!testSignal.enabled)
    {
        *sig = LMS_TESTSIG_NONE;
        return 0;
    }

    if (testSignal.dcMode)
    {
        *sig = LMS_TESTSIG_DC;
        return 0;
    }

    if (testSignal.divide == lime::SDRDevice::ChannelConfig::Direction::TestSignal::Divide::Div4 &&
        testSignal.scale == lime::SDRDevice::ChannelConfig::Direction::TestSignal::Scale::Half)
    {
        *sig = LMS_TESTSIG_NCODIV4;
        return 0;
    }

    if (testSignal.divide == lime::SDRDevice::ChannelConfig::Direction::TestSignal::Divide::Div8 &&
        testSignal.scale == lime::SDRDevice::ChannelConfig::Direction::TestSignal::Scale::Half)
    {
        *sig = LMS_TESTSIG_NCODIV8;
        return 0;
    }

    if (testSignal.divide == lime::SDRDevice::ChannelConfig::Direction::TestSignal::Divide::Div4 &&
        testSignal.scale == lime::SDRDevice::ChannelConfig::Direction::TestSignal::Scale::Full)
    {
        *sig = LMS_TESTSIG_NCODIV4F;
        return 0;
    }

    if (testSignal.divide == lime::SDRDevice::ChannelConfig::Direction::TestSignal::Divide::Div8 &&
        testSignal.scale == lime::SDRDevice::ChannelConfig::Direction::TestSignal::Scale::Full)
    {
        *sig = LMS_TESTSIG_NCODIV8F;
        return 0;
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

    // TODO: check status
    apiDevice->device->LoadConfig(apiDevice->moduleIndex, filename);

    return 0;
}

API_EXPORT int CALL_CONV LMS_SaveConfig(lms_device_t* device, const char* filename)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    // TODO: check status
    apiDevice->device->SaveConfig(apiDevice->moduleIndex, filename);

    return 0;
}

API_EXPORT void LMS_RegisterLogHandler(LMS_LogHandler handler)
{
    if (handler != nullptr)
    {
        lime::registerLogHandler(APIMsgHandler);
        api_msg_handler = handler;
    }
    else
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
    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    // TODO: check status
    apiDevice->device->ConfigureGFIR(apiDevice->moduleIndex, direction, chan & 1, { enabled, bandwidth });

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetGFIRCoeff(
    lms_device_t* device, bool dir_tx, size_t chan, lms_gfir_t filt, const float_type* coef, size_t count)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    std::vector<double> coefficients(coef, coef + count);

    // TODO: check status
    apiDevice->device->SetGFIRCoefficients(
        apiDevice->moduleIndex, dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx, chan, static_cast<uint8_t>(filt), coefficients);

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetGFIRCoeff(lms_device_t* device, bool dir_tx, size_t chan, lms_gfir_t filt, float_type* coef)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    const uint8_t count = filt == LMS_GFIR3 ? 120 : 40;

    // TODO: check status
    auto coefficients = apiDevice->device->GetGFIRCoefficients(
        apiDevice->moduleIndex, dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx, chan, static_cast<uint8_t>(filt));

    for (std::size_t i = 0; i < count; ++i)
    {
        coef[i] = coefficients.at(i);
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetGFIR(lms_device_t* device, bool dir_tx, size_t chan, lms_gfir_t filt, bool enabled)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }
    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    // TODO: check status
    apiDevice->device->SetGFIR(apiDevice->moduleIndex, direction, chan, filt, enabled);
    return 0;
}

API_EXPORT int CALL_CONV LMS_ReadParam(lms_device_t* device, struct LMS7Parameter param, uint16_t* val)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    if (val)
        *val = apiDevice->device->GetParameter(apiDevice->moduleIndex, 0, param.address, param.msb, param.lsb);

    return 0;
}

API_EXPORT int CALL_CONV LMS_WriteParam(lms_device_t* device, struct LMS7Parameter param, uint16_t val)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    // TODO: check status
    apiDevice->device->SetParameter(apiDevice->moduleIndex, 0, param.address, param.msb, param.lsb, val);

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetNCOFrequency(lms_device_t* device, bool dir_tx, size_t ch, const float_type* freq, float_type pho)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, ch);
    if (apiDevice == nullptr)
    {
        return -1;
    }
    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    for (int i = 0; i < LMS_NCO_VAL_COUNT; ++i)
    {
        // TODO: check status
        apiDevice->device->SetNCOFrequency(apiDevice->moduleIndex, direction, ch, i, freq[i], pho);
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_GetNCOFrequency(lms_device_t* device, bool dir_tx, size_t chan, float_type* freq, float_type* pho)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;
    for (int i = 0; i < LMS_NCO_VAL_COUNT; ++i)
    {
        // TODO: check status
        freq[i] = apiDevice->device->GetNCOFrequency(apiDevice->moduleIndex, direction, chan, i);
    }

    if (pho != nullptr)
    {
        uint16_t value = apiDevice->device->ReadRegister(apiDevice->moduleIndex, dir_tx ? 0x0241 : 0x0441);
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

    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;
    // TODO: check status
    apiDevice->device->SetNCOFrequency(apiDevice->moduleIndex, direction, ch, 0, fcw);

    if (phase != nullptr)
    {
        for (unsigned i = 0; i < LMS_NCO_VAL_COUNT; i++)
        {
            uint16_t addr = dir_tx ? 0x0244 : 0x0444;
            uint16_t pho = static_cast<uint16_t>(65536 * (phase[i] / 360));
            apiDevice->device->WriteRegister(apiDevice->moduleIndex, addr + i, pho);
        }

        auto& selectionParameter = dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX;
        apiDevice->device->SetParameter(
            apiDevice->moduleIndex, ch, selectionParameter.address, selectionParameter.msb, selectionParameter.lsb, 0);
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

    auto direction = dir_tx ? lime::TRXDir::Tx : lime::TRXDir::Rx;

    if (phase != nullptr)
    {
        apiDevice->device->SetParameter(apiDevice->moduleIndex, ch, "MAC", ch);

        for (std::size_t i = 0; i < LMS_NCO_VAL_COUNT; ++i)
        {
            uint16_t addr = dir_tx ? 0x0244 : 0x0444;
            uint16_t pho = apiDevice->device->ReadRegister(apiDevice->moduleIndex, addr + i);

            phase[i] = 360 * pho / 65536.0;
        }
    }

    if (fcw != nullptr)
    {
        *fcw = apiDevice->device->GetNCOFrequency(apiDevice->moduleIndex, direction, ch, 0);
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

    auto& cmixBypassParameter = dir_tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP;
    auto& cmixGainParameter = dir_tx ? LMS7_CMIX_GAIN_TXTSP : LMS7_CMIX_GAIN_RXTSP;
    auto& selectionParameter = dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX;
    auto& cmixSelectionParameter = dir_tx ? LMS7_CMIX_SC_TXTSP : LMS7_CMIX_SC_RXTSP;

    apiDevice->device->SetParameter(apiDevice->moduleIndex,
        chan,
        cmixBypassParameter.address,
        cmixBypassParameter.msb,
        cmixBypassParameter.lsb,
        ind < 0 ? 1 : 0);
    apiDevice->device->SetParameter(
        apiDevice->moduleIndex, chan, cmixGainParameter.address, cmixGainParameter.msb, cmixGainParameter.lsb, ind < 0 ? 0 : 1);

    if (ind < LMS_NCO_VAL_COUNT)
    {
        apiDevice->device->SetParameter(
            apiDevice->moduleIndex, chan, selectionParameter.address, selectionParameter.msb, selectionParameter.lsb, ind);

        apiDevice->device->SetParameter(apiDevice->moduleIndex,
            chan,
            cmixSelectionParameter.address,
            cmixSelectionParameter.msb,
            cmixSelectionParameter.lsb,
            down);
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

    auto& cmixParameter = dir_tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP;
    auto& selParameter = dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX;

    if (apiDevice->device->GetParameter(
            apiDevice->moduleIndex, chan, cmixParameter.address, cmixParameter.msb, cmixParameter.lsb) != 0)
    {
        lime::error("NCO is disabled.");
        return -1;
    }

    return apiDevice->device->GetParameter(apiDevice->moduleIndex, chan, selParameter.address, selParameter.msb, selParameter.lsb);
}

API_EXPORT int CALL_CONV LMS_WriteLMSReg(lms_device_t* device, uint32_t address, uint16_t val)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    try {
        apiDevice->device->WriteRegister(apiDevice->moduleIndex, address, val);
    }
    catch(...)
    {
        return lime::error("Failed to write register at %04X.", address);
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_ReadLMSReg(lms_device_t* device, uint32_t address, uint16_t* val)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    try {
        if (val)
            *val = apiDevice->device->ReadRegister(apiDevice->moduleIndex, address);
    }
    catch (...)
    {
        return lime::error("Failed to read register at %04X.", address);
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_WriteFPGAReg(lms_device_t* device, uint32_t address, uint16_t val)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    try {
        apiDevice->device->WriteRegister(apiDevice->moduleIndex, address, val, true);
    }
    catch (...)
    {
        return lime::error("Failed to write register at %04X.", address);
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_ReadFPGAReg(lms_device_t* device, uint32_t address, uint16_t* val)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    try {
        if (val)
            *val = apiDevice->device->ReadRegister(apiDevice->moduleIndex, address, true);
    }
    catch (...)
    {
        return lime::error("Failed to read register at %04X.", address);
    }

    return 0;
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
        lime::error("Invalid programming mode.");

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
            const auto& region = dataStorage->regions.at(lime::eMemoryRegion::VCTCXO_DAC);

            return apiDevice->device->MemoryWrite(dataStorage, region, &val);
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

    if (val)
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
            const auto& region = dataStorage->regions.at(lime::eMemoryRegion::VCTCXO_DAC);

            return apiDevice->device->MemoryRead(dataStorage, region, val);
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
