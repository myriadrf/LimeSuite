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
    LMS64C_FPGA_Over_USB(std::shared_ptr<USB_CSR_Pipe> dataPort);

    virtual int SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual int SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual int GPIODirRead(std::byte* buffer, const size_t bufLength) override;
    virtual int GPIORead(std::byte* buffer, const size_t bufLength) override;
    virtual int GPIODirWrite(const std::byte* buffer, const size_t bufLength) override;
    virtual int GPIOWrite(const std::byte* buffer, const size_t bufLength) override;

    virtual int CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) override;
    virtual int CustomParameterRead(std::vector<CustomParameterIO>& parameters) override;

    virtual int ProgramWrite(
        const std::byte* data, size_t length, int prog_mode, int target, ProgressCallback callback = nullptr) override;

  private:
    std::shared_ptr<USB_CSR_Pipe> pipe;
};

} // namespace lime

#endif // LIME_LMS64C_FPGA_OVER_USB_H
