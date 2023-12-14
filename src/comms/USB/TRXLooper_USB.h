#ifndef TRXLooper_USB_H
#define TRXLooper_USB_H

#include "limesuite/SDRDevice.h"
#include "TRXLooper.h"

namespace lime {

class USBGeneric;

/** @brief Class responsible for receiving and transmitting continuous sample data from a USB device */
class TRXLooper_USB : public TRXLooper
{
  public:
    TRXLooper_USB(std::shared_ptr<USBGeneric> comms, FPGA* f, LMS7002M* chip, uint8_t rxEndPt, uint8_t txEndPt);
    virtual ~TRXLooper_USB();

    virtual void Setup(const SDRDevice::StreamConfig& config) override;

  protected:
    virtual int RxSetup() override;
    virtual void ReceivePacketsLoop() override;

    virtual int TxSetup() override;
    virtual void TransmitPacketsLoop() override;

    std::shared_ptr<USBGeneric> comms;
    const uint8_t rxEndPt;
    const uint8_t txEndPt;

  private:
    bool GetSamplesPacket(SamplesPacketType** srcPkt);
    void NegateQ(SamplesPacketType* packet, TRXDir direction);
};

} // namespace lime

#endif /* TRXLooper_USB_H */
