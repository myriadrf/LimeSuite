#pragma once

#include "USBGeneric.h"

namespace lime {

class LimeSDREntry : public USBEntry
{
  public:
    LimeSDREntry();

#ifndef __unix__
    virtual std::vector<DeviceHandle> enumerate(const DeviceHandle& hint) override;
#endif
    virtual SDRDevice* make(const DeviceHandle& handle) override;
};

} // namespace lime