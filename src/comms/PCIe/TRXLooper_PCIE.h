#ifndef TRXLooper_PCIE_H
#define TRXLooper_PCIE_H

#include "dataTypes.h"
#include <vector>

#include "TRXLooper.h"

namespace lime {

class LitePCIe;

class TRXLooper_PCIE : public lime::TRXLooper
{
  public:
    TRXLooper_PCIE(LitePCIe *rxPort, LitePCIe *txPort, FPGA *f, LMS7002M *chip, uint8_t moduleIndex);
    virtual ~TRXLooper_PCIE();

  protected:
    virtual void ReceivePacketsLoop() override;
    virtual void TransmitPacketsLoop() override;
    LitePCIe *rxPort;
    LitePCIe *txPort;
};

} // namespace lime

#endif // TRXLooper_PCIE_H
