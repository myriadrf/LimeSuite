#ifndef LIME_USB_CSR_PIPE_MINI_H
#define LIME_USB_CSR_PIPE_MINI_H

#include "comms/USB/FT601/FT601.h"
#include "comms/USB/USB_CSR_Pipe.h"

namespace lime {

/** @brief Class for interfacing with Control/Status registers (CSR) of LimeSDR-Mini. */
class USB_CSR_Pipe_Mini : public USB_CSR_Pipe
{
  public:
    /**
      @brief Constructs a new USB_CSR_Pipe_Mini object
      @param port The FT601 communications port to use.
     */
    explicit USB_CSR_Pipe_Mini(FT601& port);

    virtual int Write(const uint8_t* data, std::size_t length, int timeout_ms) override;
    virtual int Read(uint8_t* data, std::size_t length, int timeout_ms) override;

  protected:
    FT601& port;
};

} // namespace lime

#endif // LIME_USB_CSR_PIPE_MINI_H
