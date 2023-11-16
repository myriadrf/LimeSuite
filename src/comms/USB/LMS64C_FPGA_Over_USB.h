#pragma once

#include "limesuite/IComms.h"
#include "USB_CSR_Pipe.h"
#include <memory>

namespace lime {

class LMS64C_FPGA_Over_USB : public IComms
{
  public:
    LMS64C_FPGA_Over_USB(std::shared_ptr<USB_CSR_Pipe> dataPort);

    void SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    void SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual int GPIODirRead(uint8_t* buffer, const size_t bufLength) override;
    virtual int GPIORead(uint8_t* buffer, const size_t bufLength) override;
    virtual int GPIODirWrite(const uint8_t* buffer, const size_t bufLength) override;
    virtual int GPIOWrite(const uint8_t* buffer, const size_t bufLength) override;

    virtual int CustomParameterWrite(
        const int32_t* ids, const double* values, const size_t count, const std::string& units) override;
    virtual int CustomParameterRead(const int32_t* ids, double* values, const size_t count, std::string* units) override;

    virtual int ProgramWrite(
        const char* data, size_t length, int prog_mode, int target, ProgressCallback callback = nullptr) override;

  private:
    std::shared_ptr<USB_CSR_Pipe> pipe;
};

} // namespace lime
