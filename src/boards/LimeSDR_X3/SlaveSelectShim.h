#pragma once
#include "limesuite/IComms.h"
#include <memory>

namespace lime {

// Communications helper to divert data to specific device
class SlaveSelectShim : public ISPI
{
  public:
    SlaveSelectShim(std::shared_ptr<IComms> comms, uint32_t slaveId);
    virtual void SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual void SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual int ResetDevice();

  private:
    std::shared_ptr<IComms> port;
    uint32_t slaveId;
};

} // namespace lime