#ifndef TRXLooper_USB_H
#define TRXLooper_USB_H

#include "dataTypes.h"
#include <vector>

#include "TRXLooper.h"

namespace lime {

class USBGeneric;

const int samples12InPkt = 1360;
const int samples16InPkt = 1020;

class TRXLooper_USB : public lime::TRXLooper
{
  public:
    TRXLooper_USB(USBGeneric *comms, FPGA *f, LMS7002M *chip, uint8_t rxEndPt, uint8_t txEndPt);
    virtual ~TRXLooper_USB();

  protected:
    virtual void ReceivePacketsLoop() override;
    virtual void TransmitPacketsLoop() override;
    USBGeneric *comms;
    const uint8_t rxEndPt;
    const uint8_t txEndPt;

    std::atomic<bool> terminateRx;
    std::atomic<bool> terminateTx;

    std::atomic<uint32_t> rxDataRate_Bps;
    std::atomic<uint32_t> txDataRate_Bps;

    std::atomic<uint64_t> rxLastTimestamp;
    std::atomic<uint64_t> txLastTimestamp;
};

} // namespace lime

#endif /* TRXLooper_USB_H */
