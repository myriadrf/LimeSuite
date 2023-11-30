#include "CommonFunctions.h"
#include "lime/LimeSuite.h"
#include "limesuite/DeviceHandle.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/LMS7002M.h"
#include "limesuite/LMS7002M_parameters.h"
#include "limesuite/SDRDevice.h"
#include "LMS7002M_SDRDevice.h"
#include "Logger.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace {

struct LMS_APIDevice;

struct StreamHandle {
    LMS_APIDevice* parent;
    bool isStreamStartedFromAPI;
    bool isStreamActuallyStarted;

    StreamHandle() = delete;
    StreamHandle(LMS_APIDevice* parent)
        : parent(parent)
        , isStreamStartedFromAPI(false)
        , isStreamActuallyStarted(false)
    {
    }
};

static std::vector<StreamHandle*> streamHandles;

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
    lime::TRXDir direction;
    uint8_t channel;
    int samplesProduced;

    StreamBuffer() = delete;
};

struct LMS_APIDevice {
    lime::SDRDevice* device;
    lime::SDRDevice::SDRConfig lastSavedSDRConfig;
    lime::SDRDevice::StreamConfig lastSavedStreamConfig;
    StatsDeltas statsDeltas;

    std::vector<StreamBuffer> streamBuffers;

    LMS_APIDevice() = delete;
    LMS_APIDevice(lime::SDRDevice* device)
        : device(device)
        , lastSavedSDRConfig()
        , lastSavedStreamConfig()
        , statsDeltas()
    {
    }
};

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
    if (apiDevice == nullptr)
    {
        return nullptr;
    }

    const lime::SDRDevice::Descriptor& descriptor = apiDevice->device->GetDescriptor();

    if (chan >= descriptor.rfSOC[0].channelCount)
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
            return i;
        }
    }

    streamHandles.push_back(new StreamHandle{ parent });
    return streamHandles.size() - 1;
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
            std::strncpy(dev_list[i], str.c_str(), sizeof(lms_info_str_t) - 1);
            dev_list[i][sizeof(lms_info_str_t) - 1] = 0;
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

    lime::DeviceRegistry::freeDevice(apiDevice->device);

    delete apiDevice;
    return LMS_SUCCESS;
}

// API_EXPORT int CALL_CONV LMS_Reset(lms_device_t* device)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     return lms ? lms->Reset() : -1;
// }

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
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, 0);
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
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, 0);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    return 0;
}

// API_EXPORT int CALL_CONV LMS_SetSampleRateDir(lms_device_t* device, bool dir_tx, float_type rate, size_t oversample)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     return lms ? lms->SetRate(dir_tx, rate, oversample) : -1;
// }

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

// API_EXPORT int CALL_CONV LMS_GetSampleRateRange(lms_device_t* device, bool dir_tx, lms_range_t* range)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     if (!lms)
//         return -1;
//     auto retRange = lms->GetRateRange(dir_tx);
//     range->min = retRange.min;
//     range->max = retRange.max;
//     return LMS_SUCCESS;
// }

API_EXPORT int CALL_CONV LMS_Init(lms_device_t* device)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    return apiDevice->device->Init();
}

// API_EXPORT int CALL_CONV LMS_ReadCustomBoardParam(lms_device_t* device, uint8_t param_id, float_type* val, lms_name_t units)
// {
//     auto conn = CheckConnection(device);
//     if (conn == nullptr)
//         return -1;
//     std::string str;
//     int ret = conn->CustomParameterRead(&param_id, val, 1, &str);
//     if (units)
//         strncpy(units, str.c_str(), sizeof(lms_name_t) - 1);
//     return ret;
// }

// API_EXPORT int CALL_CONV LMS_WriteCustomBoardParam(lms_device_t* device, uint8_t param_id, float_type val, const lms_name_t units)
// {
//     auto conn = CheckConnection(device);
//     if (conn == nullptr)
//         return -1;

//     std::string str = units == nullptr ? "" : units;
//     return conn->CustomParameterWrite(&param_id, &val, 1, str);
// }

// API_EXPORT int CALL_CONV LMS_VCTCXOWrite(lms_device_t* device, uint16_t val)
// {
//     if (LMS_WriteCustomBoardParam(device, BOARD_PARAM_DAC, val, "") < 0)
//         return -1;

//     auto conn = CheckConnection(device);
//     auto port = dynamic_cast<lime::LMS64CProtocol*>(conn);
//     if (port) //can use LMS64C protocol to write eeprom value
//     {
//         lime::DeviceInfo dinfo = port->GetDeviceInfo();
//         if (dinfo.deviceName == lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI_V2)) //LimeSDR-Mini v2.x
//         {
//             unsigned char packet[64] = {
//                 0x8C, 0, 56, 0, 0, 0, 0, 0, 0x02, 0, 0, 0, 0, 2, 0, 0xFF, 0, 0, 0, 1
//             }; //packet: Flash write 2 btes, addr 16
//             packet[32] = val & 0xFF; //values start at offset=32
//             packet[33] = val >> 8;

//             if (port->Write(packet, 64) != 64 || port->Read(packet, 64, 2000) != 64 || packet[1] != 1)
//                 return -1;
//         }
//         else
//         {
//             unsigned char packet[64] = {
//                 0x8C, 0, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 16, 0, 3
//             }; //packet: eeprom write 2 btes, addr 16
//             packet[32] = val & 0xFF; //values start at offset=32
//             packet[33] = val >> 8;

//             if (port->Write(packet, 64) != 64 || port->Read(packet, 64, 2000) != 64 || packet[1] != 1)
//                 return -1;
//         }
//     }
//     return LMS_SUCCESS;
// }

// API_EXPORT int CALL_CONV LMS_VCTCXORead(lms_device_t* device, uint16_t* val)
// {
//     auto conn = CheckConnection(device);
//     if (!conn)
//         return -1;
//     auto port = dynamic_cast<lime::LMS64CProtocol*>(conn);
//     if (port) //can use LMS64C protocol to read eeprom value
//     {
//         lime::DeviceInfo dinfo = port->GetDeviceInfo();
//         if (dinfo.deviceName == lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI_V2)) //LimeSDR-Mini v2.x
//         {
//             unsigned char packet[64] = {
//                 0x8D, 0, 56, 0, 0, 0, 0, 0, 0x02, 0, 0, 0, 0, 2, 0, 0xFF, 0, 0, 0, 1
//             }; //packet: eeprom read 2 bytes, addr 16
//             if (port->Write(packet, 64) != 64 || port->Read(packet, 64, 2000) != 64 || packet[1] != 1)
//                 return -1;
//             *val = packet[32] | (packet[33] << 8); //values start at offset=32
//         }
//         else
//         {
//             unsigned char packet[64] = {
//                 0x8D, 0, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 16, 0, 3
//             }; //packet: eeprom read 2 bytes, addr 16
//             if (port->Write(packet, 64) != 64 || port->Read(packet, 64, 2000) != 64 || packet[1] != 1)
//                 return -1;
//             *val = packet[32] | (packet[33] << 8); //values start at offset=32
//         }
//     }
//     else //fall back to reading runtime value
//     {
//         uint8_t id = BOARD_PARAM_DAC;
//         double dval;
//         if (conn->CustomParameterRead(&id, &dval, 1, nullptr) != LMS_SUCCESS)
//             return -1;
//         *val = dval;
//     }
//     return LMS_SUCCESS;
// }

// API_EXPORT int CALL_CONV LMS_GetClockFreq(lms_device_t* device, size_t clk_id, float_type* freq)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     if (!lms)
//         return -1;
//     *freq = lms->GetClockFreq(clk_id);
//     return *freq > 0 ? 0 : -1;
// }

// API_EXPORT int CALL_CONV LMS_SetClockFreq(lms_device_t* device, size_t clk_id, float_type freq)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     return lms ? lms->SetClockFreq(clk_id, freq) : -1;
// }

// API_EXPORT int CALL_CONV LMS_Synchronize(lms_device_t* dev, bool toChip)
// {
//     lime::LMS7_Device* lms = CheckDevice(dev);
//     return lms ? lms->Synchronize(toChip) : -1;
// }

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

// API_EXPORT int CALL_CONV LMS_EnableCache(lms_device_t* dev, bool enable)
// {
//     lime::LMS7_Device* lms = CheckDevice(dev);
//     return lms ? lms->EnableCache(enable) : -1;
// }

// API_EXPORT int CALL_CONV LMS_GetChipTemperature(lms_device_t* dev, size_t ind, float_type* temp)
// {
//     *temp = 0;
//     lime::LMS7_Device* lms = CheckDevice(dev);
//     if (!lms)
//         return -1;
//     if (lms->ReadLMSReg(0x2F) == 0x3840)
//     {
//         lime::error("Feature is not available on this chip revision.");
//         return -1;
//     }
//     *temp = lms->GetChipTemperature(ind);
//     return 0;
// }

API_EXPORT int CALL_CONV LMS_GetNumChannels(lms_device_t* device, bool dir_tx)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    return apiDevice->device->GetDescriptor().rfSOC[0].channelCount;
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
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, 0);
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

// API_EXPORT int CALL_CONV LMS_GetLOFrequencyRange(lms_device_t* device, bool dir_tx, lms_range_t* range)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     if (!lms)
//         return -1;
//     auto retRange = lms->GetFrequencyRange(dir_tx);
//     range->min = retRange.min;
//     range->max = retRange.max;
//     range->step = 0;
//     return LMS_SUCCESS;
// }

namespace {

static const uint8_t lms_name_t_MAX_LENGTH = 16;

inline void CopyStringVectorIntoList(std::vector<std::string> strings, lms_name_t* list)
{
    for (std::size_t i = 0; i < strings.size(); ++i)
    {
        std::strncpy(list[i], strings.at(i).c_str(), lms_name_t_MAX_LENGTH - 1);
        list[i][lms_name_t_MAX_LENGTH - 1] = 0;
    }
}

} // namespace

API_EXPORT int CALL_CONV LMS_GetAntennaList(lms_device_t* device, bool dir_tx, size_t chan, lms_name_t* list)
{
    LMS_APIDevice* apiDevice = CheckDevice(device, chan);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    auto rfSOC = apiDevice->device->GetDescriptor().rfSOC[0];

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
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, 0);
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

// API_EXPORT int CALL_CONV LMS_GetAntennaBW(lms_device_t* device, bool dir_tx, size_t chan, size_t path, lms_range_t* range)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, chan);
//     if (!lms)
//         return -1;

//     lime::LMS7_Device::Range ret;
//     if (dir_tx)
//         ret = lms->GetTxPathBand(path, chan);
//     else
//         ret = lms->GetRxPathBand(path, chan);

//     range->max = ret.max;
//     range->min = ret.min;
//     range->step = 0;

//     return LMS_SUCCESS;
// }

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

    try
    {
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, 0);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    return 0;
}

// API_EXPORT int CALL_CONV LMS_GetLPFBW(lms_device_t* device, bool dir_tx, size_t chan, float_type* bandwidth)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, chan);
//     if (!lms)
//         return -1;
//     *bandwidth = lms->GetLPFBW(dir_tx, chan);
//     return LMS_SUCCESS;
// }

// API_EXPORT int CALL_CONV LMS_SetLPF(lms_device_t* device, bool dir_tx, size_t chan, bool enabled)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, chan);
//     return lms ? lms->SetLPF(dir_tx, chan, enabled, -1) : -1;
// }

// API_EXPORT int CALL_CONV LMS_SetGFIRLPF(lms_device_t* device, bool dir_tx, size_t chan, bool enabled, float_type bandwidth)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, chan);
//     return lms ? lms->ConfigureGFIR(dir_tx, chan, enabled, bandwidth) : -1;
// }

API_EXPORT int CALL_CONV LMS_GetLPFBWRange(lms_device_t* device, bool dir_tx, lms_range_t* range)
{
    LMS_APIDevice* apiDevice = CheckDevice(device);
    if (apiDevice == nullptr)
    {
        return -1;
    }

    if (dir_tx)
    {
        range->min = 5e6;
        range->max = 130e6;
        range->step = 0;
    }
    else
    {
        range->min = 1.4001e6;
        range->max = 130e6;
        range->step = 0;
    }

    return 0;
}

API_EXPORT int CALL_CONV LMS_SetNormalizedGain(lms_device_t* device, bool dir_tx, size_t chan, float_type gain)
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
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, 0);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    return 0;
}

// API_EXPORT int CALL_CONV LMS_SetGaindB(lms_device_t* device, bool dir_tx, size_t chan, unsigned gain)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, chan);
// #ifdef NEW_GAIN_BEHAVIOUR
//     return lms ? lms->SetGain(dir_tx, chan, dir_tx ? gain : double(gain - 12.0f)) : -1;
// #else
//     return lms ? lms->SetGain(dir_tx, chan, int(gain - 12)) : -1;
// #endif
// }

namespace {

inline double GetGain(LMS_APIDevice* apiDevice, bool dir_tx, size_t chan)
{
    lime::SDRDevice::SDRConfig& config = apiDevice->lastSavedSDRConfig;

    if (dir_tx)
    {
        return config.channel[chan].tx.gain;
    }

    return config.channel[chan].rx.gain;
}

} // namespace

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
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, 0);
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

// API_EXPORT int CALL_CONV LMS_LoadConfig(lms_device_t* device, const char* filename)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     return lms ? lms->LoadConfig(filename) : -1;
// }

// API_EXPORT int CALL_CONV LMS_SaveConfig(lms_device_t* device, const char* filename)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);

//     return lms ? lms->SaveConfig(filename) : -1;
// }

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

// API_EXPORT int CALL_CONV LMS_GetTestSignal(lms_device_t* device, bool dir_tx, size_t chan, lms_testsig_t* sig)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, chan);
//     if (!lms)
//         return -1;

//     int tmp = lms->GetTestSignal(dir_tx, chan);
//     if (tmp < 0)
//         return -1;

//     *sig = (lms_testsig_t)tmp;
//     return LMS_SUCCESS;
// }

// API_EXPORT int CALL_CONV LMS_SetNCOFrequency(lms_device_t* device, bool dir_tx, size_t ch, const float_type* freq, float_type pho)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, ch);
//     if (!lms)
//         return -1;

//     if (freq != nullptr)
//     {
//         for (unsigned i = 0; i < LMS_NCO_VAL_COUNT; i++)
//             if (lms->SetNCOFreq(dir_tx, ch, i, freq[i]) != 0)
//                 return -1;
//         lms->WriteParam(dir_tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP, 0);
//         lms->WriteParam(dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX, 0, ch);
//     }
//     return lms->GetLMS()->SetNCOPhaseOffsetForMode0(dir_tx, pho);
// }

// API_EXPORT int CALL_CONV LMS_GetNCOFrequency(lms_device_t* device, bool dir_tx, size_t chan, float_type* freq, float_type* pho)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, chan);
//     if (!lms)
//         return -1;

//     if (freq != nullptr)
//         for (unsigned i = 0; i < LMS_NCO_VAL_COUNT; i++)
//             freq[i] = std::fabs(lms->GetNCOFreq(dir_tx, chan, i));

//     if (pho != nullptr)
//     {
//         uint16_t value = lms->ReadLMSReg(dir_tx ? 0x0241 : 0x0441, chan / 2);
//         *pho = 360.0 * value / 65536.0;
//     }
//     return 0;
// }

// API_EXPORT int CALL_CONV LMS_SetNCOPhase(lms_device_t* device, bool dir_tx, size_t ch, const float_type* phase, float_type fcw)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, ch);
//     if (!lms)
//         return -1;

//     if (lms->SetNCOFreq(dir_tx, ch, 0, fcw) != 0)
//         return -1;

//     if (phase != nullptr)
//     {
//         for (unsigned i = 0; i < LMS_NCO_VAL_COUNT; i++)
//             if (lms->SetNCOPhase(dir_tx, ch, i, phase[i]) != 0)
//                 return -1;
//         if ((lms->WriteParam(dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX, 0, ch) != 0))
//             return -1;
//     }
//     return 0;
// }

// API_EXPORT int CALL_CONV LMS_GetNCOPhase(lms_device_t* device, bool dir_tx, size_t ch, float_type* phase, float_type* fcw)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, ch);
//     if (!lms)
//         return -1;

//     if (phase != nullptr)
//         for (unsigned i = 0; i < LMS_NCO_VAL_COUNT; i++)
//             phase[i] = lms->GetNCOPhase(dir_tx, ch, i);

//     if (fcw != nullptr)
//         *fcw = lms->GetNCOFreq(dir_tx, ch, 0);

//     return 0;
// }

// API_EXPORT int CALL_CONV LMS_SetNCOIndex(lms_device_t* device, bool dir_tx, size_t chan, int ind, bool down)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, chan);
//     if (!lms)
//         return -1;

//     if ((lms->WriteParam(dir_tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP, ind < 0 ? 1 : 0, chan) != 0) ||
//         (lms->WriteParam(dir_tx ? LMS7_CMIX_GAIN_TXTSP : LMS7_CMIX_GAIN_RXTSP, ind < 0 ? 0 : 1, chan) != 0))
//         return -1;

//     if (ind < LMS_NCO_VAL_COUNT)
//     {
//         if ((lms->WriteParam(dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX, ind) != 0) ||
//             (lms->WriteParam(dir_tx ? LMS7_CMIX_SC_TXTSP : LMS7_CMIX_SC_RXTSP, down) != 0))
//             return -1;
//     }
//     else
//     {
//         lime::error("Invalid NCO index value.");
//         return -1;
//     }
//     return 0;
// }

// API_EXPORT int CALL_CONV LMS_GetNCOIndex(lms_device_t* device, bool dir_tx, size_t chan)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, chan);
//     if (!lms)
//         return -1;

//     if (lms->ReadParam(dir_tx ? LMS7_CMIX_BYP_TXTSP : LMS7_CMIX_BYP_RXTSP, chan) != 0)
//     {
//         lime::error("NCO is disabled.");
//         return -1;
//     }
//     return lms->ReadParam(dir_tx ? LMS7_SEL_TX : LMS7_SEL_RX, chan);
// }

// API_EXPORT int CALL_CONV LMS_ReadLMSReg(lms_device_t* device, uint32_t address, uint16_t* val)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     if (!lms)
//         return -1;
//     *val = lms->ReadLMSReg(address);
//     return LMS_SUCCESS;
// }

// API_EXPORT int CALL_CONV LMS_WriteLMSReg(lms_device_t* device, uint32_t address, uint16_t val)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     return lms ? lms->WriteLMSReg(address, val) : -1;
// }

// API_EXPORT int CALL_CONV LMS_ReadFPGAReg(lms_device_t* device, uint32_t address, uint16_t* val)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     if (!lms)
//         return -1;
//     int value = lms->ReadFPGAReg(address);
//     if (value < 0)
//         return value; // operation failed return error code
//     else if (val)
//         *val = value;
//     return LMS_SUCCESS;
// }

// API_EXPORT int CALL_CONV LMS_WriteFPGAReg(lms_device_t* device, uint32_t address, uint16_t val)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     return lms ? lms->WriteFPGAReg(address, val) : -1;
// }

// API_EXPORT int CALL_CONV LMS_ReadParam(lms_device_t* device, struct LMS7Parameter param, uint16_t* val)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     if (!lms)
//         return -1;
//     *val = lms->ReadParam(param);
//     return LMS_SUCCESS;
//     ;
// }

// API_EXPORT int CALL_CONV LMS_WriteParam(lms_device_t* device, struct LMS7Parameter param, uint16_t val)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     if (!lms)
//         return -1;
//     return lms->WriteParam(param, val);
// }

// API_EXPORT int CALL_CONV LMS_SetGFIRCoeff(
//     lms_device_t* device, bool dir_tx, size_t chan, lms_gfir_t filt, const float_type* coef, size_t count)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, chan);
//     return lms ? lms->SetGFIRCoef(dir_tx, chan, filt, coef, count) : -1;
// }

// API_EXPORT int CALL_CONV LMS_GetGFIRCoeff(lms_device_t* device, bool dir_tx, size_t chan, lms_gfir_t filt, float_type* coef)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, chan);
//     return lms ? lms->GetGFIRCoef(dir_tx, chan, filt, coef) : -1;
// }

// API_EXPORT int CALL_CONV LMS_SetGFIR(lms_device_t* device, bool dir_tx, size_t chan, lms_gfir_t filt, bool enabled)
// {
//     lime::LMS7_Device* lms = CheckDevice(device, chan);
//     return lms ? lms->SetGFIR(dir_tx, chan, filt, enabled) : -1;
// }

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
        apiDevice->device->Configure(apiDevice->lastSavedSDRConfig, 0);
    } catch (...)
    {
        lime::error("Device configuration failed.");

        return -1;
    }

    lime::SDRDevice::StreamConfig config = apiDevice->lastSavedStreamConfig;
    config.bufferSize = stream->fifoSize;

    auto channel = stream->channel & (0xffffffff - LMS_ALIGN_CH_PHASE); // Clear the align phase bit
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

    // config.performanceLatency = stream->throughputVsLatency;

    auto returnValue = apiDevice->device->StreamSetup(config, 0);

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
        handle->parent->device->StreamStart(0);

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
        handle->parent->device->StreamStop(0);

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

    const uint32_t streamChannel = stream->channel & (0xffffffff - LMS_ALIGN_CH_PHASE);
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

                delete[] reinterpret_cast<T*>(buffer.buffer);
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
            sampleBuffer[i] = new T[sample_count];

            handle->parent->streamBuffers.push_back(
                { sampleBuffer[i], direction, handle->parent->lastSavedStreamConfig.rxChannels[i], 0 });
        }
    }

    lime::SDRDevice::StreamMeta metadata{ 0, false, false };
    int samplesProduced = handle->parent->device->StreamRx(0, sampleBuffer.data(), sample_count, &metadata);

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

    const uint32_t streamChannel = stream->channel & (0xffffffff - LMS_ALIGN_CH_PHASE);
    const uint8_t txChannelCount = handle->parent->lastSavedStreamConfig.txCount;
    const std::size_t sampleSize = sizeof(T);

    std::vector<bool> found(txChannelCount, false);

    for (auto& buffer : handle->parent->streamBuffers)
    {
        if (buffer.direction == direction)
        {
            for (uint8_t i = 0; i < txChannelCount; ++i)
            {
                if (handle->parent->lastSavedStreamConfig.txChannels[i] == buffer.channel)
                {
                    found[i] = true;
                    break;
                }
            }
        }
    }

    for (uint8_t i = 0; i < txChannelCount; ++i)
    {
        if (handle->parent->lastSavedStreamConfig.txChannels[i] == streamChannel)
        {
            found[i] = true;
            break;
        }
    }

    if (std::all_of(found.begin(), found.end(), [](bool item) { return item; }))
    {
        std::vector<const T*> sampleBuffer(txChannelCount);

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

        lime::SDRDevice::StreamMeta metadata{ 0, false, false };

        if (meta != nullptr)
        {
            metadata.flush = meta->flushPartialPacket;
            metadata.useTimestamp = meta->waitForTimestamp;
            metadata.timestamp = meta->timestamp;
        }

        int samplesSent = handle->parent->device->StreamTx(0, sampleBuffer.data(), sample_count, &metadata);

        for (auto it = handle->parent->streamBuffers.begin(); it != handle->parent->streamBuffers.end(); it++)
        {
            auto& buffer = *it;
            if (buffer.direction == direction && buffer.channel != streamChannel)
            {
                delete[] reinterpret_cast<T*>(buffer.buffer);
                handle->parent->streamBuffers.erase(it--);
            }
        }

        return samplesSent;
    }

    T* buffer = new T[sample_count];
    std::memcpy(buffer, samples, sample_count * sampleSize);
    handle->parent->streamBuffers.push_back({ reinterpret_cast<void*>(buffer), direction, static_cast<uint8_t>(streamChannel), 0 });

    // Can't really know what to return here just yet, so just returning that all of them have passed through.
    return sample_count;
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

// API_EXPORT int CALL_CONV LMS_UploadWFM(lms_device_t* device, const void** samples, uint8_t chCount, size_t sample_count, int format)
// {
//     lime::LMS7_Device* lms = (lime::LMS7_Device*)device;
//     lime::StreamConfig::StreamDataFormat fmt;
//     switch (format)
//     {
//     case 0:
//         fmt = lime::StreamConfig::StreamDataFormat::FMT_INT12;
//         break;
//     case 1:
//         fmt = lime::StreamConfig::StreamDataFormat::FMT_INT16;
//         break;
//     case 2:
//         fmt = lime::StreamConfig::StreamDataFormat::FMT_FLOAT32;
//         break;
//     default:
//         fmt = lime::StreamConfig::StreamDataFormat::FMT_INT12;
//         break;
//     }
//     return lms->UploadWFM(samples, chCount, sample_count, fmt);
// }

// API_EXPORT int CALL_CONV LMS_EnableTxWFM(lms_device_t* device, unsigned ch, bool active)
// {
//     uint16_t regAddr = 0x000D;
//     uint16_t regValue = 0;
//     int status = 0;
//     status = LMS_WriteFPGAReg(device, 0xFFFF, 1 << (ch / 2));
//     if (status != 0)
//         return status;
//     status = LMS_ReadFPGAReg(device, regAddr, &regValue);
//     if (status != 0)
//         return status;
//     regValue = regValue & ~0x6; //clear WFM_LOAD, WFM_PLAY
//     regValue |= active << 1;
//     status = LMS_WriteFPGAReg(device, regAddr, regValue);
//     return status;
// }

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
        handle->parent->device->StreamStatus(0, &stats, nullptr);
        break;
    case lime::TRXDir::Tx:
        handle->parent->device->StreamStatus(0, nullptr, &stats);
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

    // status->sampleRate;
    status->linkRate = stats.dataRate_Bps;
    status->timestamp = stats.timestamp;

    return 0;
}

// API_EXPORT const lms_dev_info_t* CALL_CONV LMS_GetDeviceInfo(lms_device_t* device)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     return lms ? lms->GetInfo() : nullptr;
// }

// API_EXPORT int CALL_CONV LMS_GetProgramModes(lms_device_t* device, lms_name_t* list)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     if (!lms)
//         return -1;

//     auto names = lms->GetProgramModes();
//     if (list != nullptr)
//         for (size_t i = 0; i < names.size(); i++)
//         {
//             strncpy(list[i], names[i].c_str(), sizeof(lms_name_t) - 1);
//             list[i][sizeof(lms_name_t) - 1] = 0;
//         }
//     return names.size();
// }

// API_EXPORT int CALL_CONV LMS_Program(
//     lms_device_t* device, const char* data, size_t size, const lms_name_t mode, lms_prog_callback_t callback)
// {
//     lime::LMS7_Device* lms = CheckDevice(device);
//     if (!lms)
//         return -1;

//     std::string prog_mode(mode);
//     return lms->Program(prog_mode, data, size, callback);
// }

// API_EXPORT const char* CALL_CONV LMS_GetLastErrorMessage(void)
// {
//     return lime::GetLastErrorMessage();
// }

// API_EXPORT const char* LMS_GetLibraryVersion()
// {
//     static char libraryVersion[32];
//     sprintf(libraryVersion, "%.31s", lime::GetLibraryVersion().c_str());
//     return libraryVersion;
// }

// static LMS_LogHandler api_msg_handler;
// static void APIMsgHandler(const lime::LogLevel level, const char* message)
// {
//     api_msg_handler(level, message);
// }

// API_EXPORT void LMS_RegisterLogHandler(LMS_LogHandler handler)
// {
//     if (handler)
//     {
//         lime::registerLogHandler(APIMsgHandler);
//         api_msg_handler = handler;
//     }
//     else
//         lime::registerLogHandler(nullptr);
// }

// extern "C" API_EXPORT int CALL_CONV LMS_TransferLMS64C(lms_device_t* dev, int cmd, uint8_t* data, size_t* len)
// {
//     auto conn = CheckConnection(dev);
//     if (!conn)
//         return -1;

//     lime::LMS64CProtocol::GenericPacket pkt;

//     pkt.cmd = lime::eCMD_LMS(cmd);
//     for (size_t i = 0; i < *len; ++i)
//         pkt.outBuffer.push_back(data[i]);

//     lime::LMS64CProtocol* port = dynamic_cast<lime::LMS64CProtocol*>(conn);
//     if (port->TransferPacket(pkt) != 0)
//         return -1;

//     for (size_t i = 0; i < pkt.inBuffer.size(); ++i)
//         data[i] = pkt.inBuffer[i];
//     *len = pkt.inBuffer.size();

//     if (pkt.status != lime::STATUS_COMPLETED_CMD)
//     {
//         lime::error("%s", lime::status2string(pkt.status));
//         return -1;
//     }

//     return LMS_SUCCESS;
// }
