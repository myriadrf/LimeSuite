#ifndef FPGA_MINI_H
#define FPGA_MINI_H

#include "FPGA_common.h"

namespace lime {

class ISPI;

class FPGA_Mini : public FPGA
{
  public:
    FPGA_Mini(std::shared_ptr<ISPI> fpgaSPI, std::shared_ptr<ISPI> lms7002mSPI);
    virtual ~FPGA_Mini(){};
    virtual int SetInterfaceFreq(double txRate_Hz, double rxRate_Hz, double txPhase, double rxPhase, int channel) override;
    virtual int SetInterfaceFreq(double txRate_Hz, double rxRate_Hz, int channel) override;
};

} // namespace lime
#endif // FPGA_MINI_H
