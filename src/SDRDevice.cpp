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

// TODO: replace
///Enumeration of RF ports
enum {
    LMS_PATH_NONE = 0, ///<No active path (RX or TX)
    LMS_PATH_LNAH = 1, ///<RX LNA_H port
    LMS_PATH_LNAL = 2, ///<RX LNA_L port
    LMS_PATH_LNAW = 3, ///<RX LNA_W port
    LMS_PATH_TX1 = 1, ///<TX port 1
    LMS_PATH_TX2 = 2, ///<TX port 2
    LMS_PATH_AUTO = 255, ///<Automatically select port (if supported)
};

Range SDRDevice::GetAntennaRange(uint8_t moduleIndex, TRXDir direction, unsigned path) const
{
    if (direction == TRXDir::Tx)
    {
        switch (path)
        {
        case LMS_PATH_TX1:
            return { 30e6, 1.9e9, 0 };
        case LMS_PATH_TX2:
            return { 2e9, 2.6e9, 0 };
        default:
            return { 0, 0, 0 };
        }
    }
    else
    {
        switch (path)
        {
        case LMS_PATH_LNAH:
            return { 2e9, 2.6e9, 0 };
        case LMS_PATH_LNAL:
            return { 700e6, 900e6, 0 };
        case LMS_PATH_LNAW:
            return { 700e6, 2.6e9, 0 };
        default:
            return { 0, 0, 0 };
        }
    }

    return { 0, 0, 0 };
};
