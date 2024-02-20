#ifndef LIME_LMS64C_FPGA_OVER_USB_H
#define LIME_LMS64C_FPGA_OVER_USB_H

#include "limesuite/IComms.h"
#include "USB_CSR_Pipe.h"
#include <memory>

namespace lime {

/** @brief A class for communicating with a device's FPGA over a USB interface. */
class LMS64C_FPGA_Over_USB : public IComms
{
  public:
    /**
      @brief Constructs a new LMS64C_FPGA_Over_USB object.
      @param dataPort The USB communications pipe to use.
     */
    LMS64C_FPGA_Over_USB(std::shared_ptr<USB_CSR_Pipe> dataPort);

    virtual OpStatus SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual OpStatus SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual OpStatus GPIODirRead(uint8_t* buffer, const size_t bufLength) override;
    virtual OpStatus GPIORead(uint8_t* buffer, const size_t bufLength) override;
    virtual OpStatus GPIODirWrite(const uint8_t* buffer, const size_t bufLength) override;
    virtual OpStatus GPIOWrite(const uint8_t* buffer, const size_t bufLength) override;

    virtual OpStatus CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) override;
    virtual OpStatus CustomParameterRead(std::vector<CustomParameterIO>& parameters) override;

    virtual OpStatus ProgramWrite(
        const char* data, size_t length, int prog_mode, int target, ProgressCallback callback = nullptr) override;

  private:
    std::shared_ptr<USB_CSR_Pipe> pipe;
};

} // namespace lime

#endif // LIME_LMS64C_FPGA_OVER_USB_H
