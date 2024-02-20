#ifndef FPGA_5G_H
#define FPGA_5G_H

#include "FPGA_common.h"

namespace lime {

class ISPI;

/** @brief Class for communicating with the Xilinx XC7A50T-2CPG236I FPGA in the LimeSDR XTRX. */
class FPGA_XTRX : public FPGA
{
  public:
    FPGA_XTRX(std::shared_ptr<ISPI> fpgaSPI, std::shared_ptr<ISPI> lms7002mSPI);
    virtual ~FPGA_XTRX(){};
    OpStatus SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, double txPhase, double rxPhase) override;
    OpStatus SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, int channel) override;
    OpStatus SetPllFrequency(const uint8_t pllIndex, const double inputFreq, FPGA_PLL_clock* clocks, const uint8_t clockCount);
};

} // namespace lime
#endif // FPGA_Q_H
