#pragma once

#include "USBCommon.h"
#include "FX3/FX3.h"

#include <cstdint>

using namespace lime;

class USB_CSR_Pipe_SDR : public USB_CSR_Pipe
{
  public:
    explicit USB_CSR_Pipe_SDR(FX3& port);

    virtual int Write(const uint8_t* data, size_t length, int timeout_ms) override;
    virtual int Read(uint8_t* data, size_t length, int timeout_ms) override;

  protected:
    FX3& port;
};
