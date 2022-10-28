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
    TRXLooper_PCIE(LitePCIe *trxPort, FPGA *f, LMS7002M *chip, uint8_t moduleIndex);
    virtual ~TRXLooper_PCIE();
    virtual void Start();
  protected:
    virtual void ReceivePacketsLoop() override;
    virtual void TransmitPacketsLoop() override;
    LitePCIe *trxPort;
};

} // namespace lime

#endif // TRXLooper_PCIE_H
