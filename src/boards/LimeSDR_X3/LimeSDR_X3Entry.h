#ifndef LIME_LIMESDR_X3ENTRY_H
#define LIME_LIMESDR_X3ENTRY_H

#include "limesuite/DeviceRegistry.h"

namespace lime {

class LimeSDR_X3Entry : public DeviceRegistryEntry
{
  public:
    LimeSDR_X3Entry();
    virtual ~LimeSDR_X3Entry();
    std::vector<DeviceHandle> enumerate(const DeviceHandle& hint) override;
    SDRDevice* make(const DeviceHandle& handle) override;
};

} // namespace lime

#endif // LIME_LIMESDR_X3ENTRY_H
