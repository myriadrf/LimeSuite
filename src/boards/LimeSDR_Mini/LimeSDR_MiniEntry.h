#ifndef LIME_LIMESDR_MINI_ENTRY_H
#define LIME_LIMESDR_MINI_ENTRY_H

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

#endif // LIME_LIMESDR_MINI_ENTRY_H