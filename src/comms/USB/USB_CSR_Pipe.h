#ifndef LIME_USB_CSR_PIPE_H
#define LIME_USB_CSR_PIPE_H

#include "ISerialPort.h"

namespace lime {

class USB_CSR_Pipe : public ISerialPort
{
  public:
    explicit USB_CSR_Pipe(){};

    virtual int Write(const uint8_t* data, size_t length, int timeout_ms) override = 0;
    virtual int Read(uint8_t* data, size_t length, int timeout_ms) override = 0;
};

} // namespace lime

#endif // LIME_USB_CSR_PIPE_H
