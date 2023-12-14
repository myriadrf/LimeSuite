#ifndef FPGA_X3_H
#define FPGA_X3_H

#include "FPGA_common.h"

namespace lime {

class ISPI;

/** @brief Class for communicating with the Xilinx Artix-7 XC7A200T FPGA in the LimeSDR X3. */
class FPGA_X3 : public FPGA
{
  public:
    FPGA_X3(std::shared_ptr<ISPI> fpgaSPI, std::shared_ptr<ISPI> lms7002mSPI);
    virtual ~FPGA_X3(){};
    int SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, double txPhase, double rxPhase, int ch = 0) override;
    int SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, int ch = 0) override;
    int SetPllFrequency(const uint8_t pllIndex, const double inputFreq, FPGA_PLL_clock* clocks, const uint8_t clockCount);
};

} // namespace lime
#endif // FPGA_Q_H
