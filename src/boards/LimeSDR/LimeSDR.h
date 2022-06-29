#ifndef LIME_LIMESDR_H
#define LIME_LIMESDR_H

#include "SDRDevice.h"
#include "DeviceRegistry.h"
#include "DeviceHandle.h"
#include <vector>

namespace lime
{

class USBGeneric;
class LMS7002M;

class LimeSDR : public SDRDevice
{
public:
    LimeSDR(lime::USBGeneric* conn);
    virtual ~LimeSDR();

    virtual DeviceInfo GetDeviceInfo() override;

    virtual void SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, size_t count) override;

protected:
    LMS7002M* mLMSChip;
    USBGeneric* comms;
};

class LimeSDREntry : public DeviceRegistryEntry
{
public:
    LimeSDREntry();
    virtual ~LimeSDREntry();
    std::vector<DeviceHandle> enumerate(const DeviceHandle& hint) override;
    SDRDevice* make(const DeviceHandle& handle) override;
};

}

#endif	/* LIME_LIMESDR_H */
