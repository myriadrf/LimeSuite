#ifndef LIME_LMS64C_LMS7002M_OVER_USB_H
#define LIME_LMS64C_LMS7002M_OVER_USB_H

#include "limesuite/IComms.h"
#include "USB_CSR_Pipe.h"
#include <memory>

namespace lime {

/** @brief A class for communicating with a device's LMS7002M chip over a USB interface. */
class LMS64C_LMS7002M_Over_USB : public IComms
{
  public:
    /**
      @brief Constructs a new LMS64C_LMS7002M_Over_USB object
      @param dataPort The USB communications pipe to use.
     */
    LMS64C_LMS7002M_Over_USB(std::shared_ptr<USB_CSR_Pipe> dataPort);

    virtual int SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual int SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual int ResetDevice(int chipSelect) override;

  private:
    std::shared_ptr<USB_CSR_Pipe> pipe;
};

} // namespace lime

#endif // LIME_LMS64C_LMS7002M_OVER_USB_H
