#pragma once

#include "USBEntry.h"
#include "limesuite/DeviceHandle.h"

namespace lime {

class LimeSDR_MiniEntry : public USBEntry
{
  public:
    LimeSDR_MiniEntry();
#ifndef __unix__
    virtual std::vector<DeviceHandle> enumerate(const DeviceHandle& hint) override;
#endif
    virtual SDRDevice* make(const DeviceHandle& handle) override;
};

} // namespace lime
