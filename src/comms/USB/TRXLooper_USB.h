#ifndef TRXLooper_USB_H
#define TRXLooper_USB_H

#include "dataTypes.h"
#include <vector>

#include "TRXLooper.h"

namespace lime {

class USBGeneric;

class TRXLooper_USB : public lime::TRXLooper
{
  public:
    TRXLooper_USB(USBGeneric *comms, FPGA *f, LMS7002M *chip, uint8_t rxEndPt, uint8_t txEndPt);
    virtual ~TRXLooper_USB();

    virtual void Setup(const lime::SDRDevice::StreamConfig &config) override;

  protected:
    virtual int RxSetup() override;
    virtual void ReceivePacketsLoop() override;

    virtual int TxSetup() override;
    virtual void TransmitPacketsLoop() override;

    USBGeneric *comms;
    const uint8_t rxEndPt;
    const uint8_t txEndPt;
  private:
    bool GetSamplesPacket(SamplesPacketType** srcPkt);
    void NegateQ(SamplesPacketType* packet, TRXDir direction);
};

} // namespace lime

#endif /* TRXLooper_USB_H */
