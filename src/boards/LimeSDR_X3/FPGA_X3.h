#ifndef FPGA_X3_H
#define FPGA_X3_H

#include "FPGA_common.h"

namespace lime {

class ISPI;

class FPGA_X3 : public FPGA
{
  public:
    FPGA_X3(lime::ISPI* fpgaSPI, lime::ISPI* lms7002mSPI);
    virtual ~FPGA_X3(){};
    int SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, double txPhase, double rxPhase, int ch = 0) override;
    int SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, int ch = 0) override;
    int SetPllFrequency(const uint8_t pllIndex, const double inputFreq, FPGA_PLL_clock* clocks, const uint8_t clockCount);
};

} // namespace lime
#endif // FPGA_Q_H
