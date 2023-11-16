#pragma once
#include "FT601/FT601.h"
#include "USB_CSR_Pipe.h"

static constexpr int streamBulkWriteAddr = 0x03;
static constexpr int streamBulkReadAddr = 0x83;

static constexpr int ctrlBulkWriteAddr = 0x02;
static constexpr int ctrlBulkReadAddr = 0x82;

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
