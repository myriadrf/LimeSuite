#ifndef LIME_USB_CSR_Pipe_SDR_H
#define LIME_USB_CSR_Pipe_SDR_H

#include "USB_CSR_Pipe.h"
#include "FX3/FX3.h"

#include <cstdint>

namespace lime {

/** @brief Class for interfacing with Control/Status registers (CSR) of LimeSDR-USB. */
class USB_CSR_Pipe_SDR : public USB_CSR_Pipe
{
  public:
    explicit USB_CSR_Pipe_SDR(FX3& port);

    virtual int Write(const uint8_t* data, std::size_t length, int timeout_ms) override;
    virtual int Read(uint8_t* data, std::size_t length, int timeout_ms) override;

  protected:
    FX3& port;
};

} // namespace lime

#endif // LIME_USB_CSR_Pipe_SDR_H