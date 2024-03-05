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
    OpStatus SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, double txPhase, double rxPhase, int chipIndex = 0) override;
    OpStatus SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, int chipIndex = 0) override;

  protected:
    virtual OpStatus SetPllFrequency(const uint8_t pllIndex, const double inputFreq, std::vector<FPGA_PLL_clock>& clocks) override;
};

} // namespace lime
#endif // FPGA_Q_H
