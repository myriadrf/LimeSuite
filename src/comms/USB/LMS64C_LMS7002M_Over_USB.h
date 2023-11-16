#pragma once

#include "limesuite/IComms.h"
#include "USB_CSR_Pipe.h"
#include <memory>

namespace lime {

class LMS64C_LMS7002M_Over_USB : public IComms
{
  public:
    LMS64C_LMS7002M_Over_USB(std::shared_ptr<USB_CSR_Pipe> dataPort);

    virtual void SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual void SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual int ResetDevice(int chipSelect) override;

  private:
    std::shared_ptr<USB_CSR_Pipe> pipe;
};

} // namespace lime
