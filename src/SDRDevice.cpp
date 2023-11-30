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

Range SDRDevice::GetAntennaRange(uint8_t moduleIndex, TRXDir direction, unsigned path) const
{
    if (direction == TRXDir::Tx)
    {
        switch (path)
        {
        case 1:
            return { 30e6, 1.9e9, 0 };
        case 2:
            return { 2e9, 2.6e9, 0 };
        default:
            return { 0, 0, 0 };
        }
    }
    else
    {
        switch (path)
        {
        case 1:
            return { 2e9, 2.6e9, 0 };
        case 2:
            return { 700e6, 900e6, 0 };
        case 3:
            return { 700e6, 2.6e9, 0 };
        default:
            return { 0, 0, 0 };
        }
    }

    return { 0, 0, 0 };
};
