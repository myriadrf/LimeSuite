#ifndef LIME_LIMESDRENTRY_H
#define LIME_LIMESDRENTRY_H

#include "USBEntry.h"

namespace lime {

/** @brief A class for a LimeSDR-USB device entry. */
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

#endif // LIME_LIMESDRENTRY_H
