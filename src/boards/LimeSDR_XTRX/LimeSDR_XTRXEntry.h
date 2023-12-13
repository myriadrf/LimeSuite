#ifndef LIME_LIMESDR_XTRXENTRY_H
#define LIME_LIMESDR_XTRXENTRY_H

#include "limesuite/DeviceRegistry.h"

namespace lime {

class LimeSDR_XTRXEntry : public DeviceRegistryEntry
{
  public:
    LimeSDR_XTRXEntry();
    virtual ~LimeSDR_XTRXEntry();
    std::vector<DeviceHandle> enumerate(const DeviceHandle& hint) override;
    SDRDevice* make(const DeviceHandle& handle) override;
};

} // namespace lime

#endif // LIME_LIMESDR_XTRXENTRY_H
