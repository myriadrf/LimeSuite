#ifndef LIME_USB_CSR_PIPE_MINI_H
#define LIME_USB_CSR_PIPE_MINI_H

#include "FT601/FT601.h"
#include "USB_CSR_Pipe.h"

namespace lime {

class USB_CSR_Pipe_Mini : public USB_CSR_Pipe
{
  public:
    explicit USB_CSR_Pipe_Mini(FT601& port);

    virtual int Write(const uint8_t* data, size_t length, int timeout_ms) override;
    virtual int Read(uint8_t* data, size_t length, int timeout_ms) override;

  protected:
    FT601& port;
};

} // namespace lime

#endif // LIME_USB_CSR_PIPE_MINI_H