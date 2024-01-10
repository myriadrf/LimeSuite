#include "limesuite/SDRDevice.h"

using namespace lime;

SDRDevice::StreamConfig::Extras::Extras()
{
    memset(this, 0, sizeof(Extras));
    usePoll = true;
};

SDRDevice::StreamConfig::StreamConfig()
    : format{ DataFormat::I16 }
    , linkFormat{ DataFormat::I16 }
    , bufferSize{ 0 }
    , hintSampleRate{ 0 }
    , alignPhase{ false }
    , statusCallback{ nullptr }
    , userData{ nullptr }
    , extraConfig{ nullptr }
{
    channels[TRXDir::Rx] = {};
    channels[TRXDir::Tx] = {};
}

SDRDevice::StreamConfig::~StreamConfig()
{
    if (extraConfig)
    {
        delete extraConfig;
    }
}

SDRDevice::StreamConfig& SDRDevice::StreamConfig::operator=(const SDRDevice::StreamConfig& src)
{
    if (this == &src)
    {
        return *this;
    }

    if (extraConfig)
    {
        delete extraConfig;
        extraConfig = nullptr;
    }

    channels = src.channels;
    format = src.format;
    linkFormat = src.linkFormat;
    bufferSize = src.bufferSize;
    hintSampleRate = src.hintSampleRate;
    alignPhase = src.alignPhase;
    statusCallback = src.statusCallback;
    userData = src.userData;

    if (src.extraConfig)
    {
        extraConfig = new Extras();
        *extraConfig = *src.extraConfig;
    }
    return *this;
}

const char SDRDevice::Descriptor::DEVICE_NUMBER_SEPARATOR_SYMBOL = '@';
const char SDRDevice::Descriptor::PATH_SEPARATOR_SYMBOL = '/';
