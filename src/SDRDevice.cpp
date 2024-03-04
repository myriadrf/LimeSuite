#include "limesuite/SDRDevice.h"

using namespace lime;

SDRDevice::StreamConfig::Extras::Extras()
{
    memset(this, 0, sizeof(Extras));
    usePoll = true;
};

SDRDevice::StreamConfig::StreamConfig()
{
    memset(this, 0, sizeof(StreamConfig));
}

SDRDevice::StreamConfig::~StreamConfig()
{
    if (extraConfig)
        delete extraConfig;
}

SDRDevice::StreamConfig& SDRDevice::StreamConfig::operator=(const SDRDevice::StreamConfig& src)
{
    if (this == &src)
        return *this;

    if (extraConfig)
    {
        delete extraConfig;
        extraConfig = nullptr;
    }
    memcpy(this, &src, sizeof(SDRDevice::StreamConfig));
    if (src.extraConfig)
    {
        this->extraConfig = new Extras();
        *this->extraConfig = *src.extraConfig;
    }
    return *this;
}

void SDRDevice::StreamStart(const std::vector<uint8_t> moduleIndexes)
{
    for (uint8_t i : moduleIndexes)
        StreamStart(i);
};

void SDRDevice::StreamStop(const std::vector<uint8_t> moduleIndexes)
{
    for (uint8_t i : moduleIndexes)
        StreamStop(i);
};