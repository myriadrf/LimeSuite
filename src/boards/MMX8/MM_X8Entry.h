#ifndef LIME_MM_X8ENTRY_H
#define LIME_MM_X8ENTRY_H

#include "limesuite/DeviceRegistry.h"

namespace lime {

/** @brief A class for LimeSDR MMX8 device registry entry. */
class LimeSDR_MMX8Entry : public DeviceRegistryEntry
{
  public:
    LimeSDR_MMX8Entry();
    virtual ~LimeSDR_MMX8Entry();
    std::vector<DeviceHandle> enumerate(const DeviceHandle& hint) override;
    SDRDevice* make(const DeviceHandle& handle) override;
};

} // namespace lime

#endif // LIME_MM_X8ENTRY_H
