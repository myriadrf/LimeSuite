#ifndef LIME_SLAVESELECTSHIM_H
#define LIME_SLAVESELECTSHIM_H

#include "limesuite/config.h"
#include "limesuite/IComms.h"
#include <memory>

namespace lime {

/** @brief Communications helper to divert data to specific device. */
class LIME_API SlaveSelectShim : public ISPI
{
  public:
    SlaveSelectShim(std::shared_ptr<IComms> comms, uint32_t slaveId);
    virtual OpStatus SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual OpStatus SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual OpStatus ResetDevice();

  private:
    std::shared_ptr<IComms> port;
    uint32_t slaveId;
};

} // namespace lime

#endif // LIME_SLAVESELECTSHIM_H