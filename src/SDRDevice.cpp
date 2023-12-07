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

std::vector<SDRDevice::MemoryDeviceListEntry> SDRDevice::Descriptor::ListMemoryDevices() const
{
    std::vector<MemoryDeviceListEntry> devices;

    for (const auto& memoryDevice : memoryDevices)
    {
        std::string deviceText = lime::MEMORY_DEVICES_TEXT.at(memoryDevice.first);

        for (std::size_t i = 0; i < memoryDevice.second.size(); i++)
        {
            std::string postfix = "";

            if (i > 0)
            {
                postfix = SEPARATOR_SYMBOL + std::to_string(i);
            }

            devices.push_back({ { memoryDevice.first, static_cast<uint32_t>(i) }, deviceText + postfix });
        }
    }

    return devices;
}
