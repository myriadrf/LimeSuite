#ifndef FPGA_5G_H
#define FPGA_5G_H

#include "FPGA_common.h"

namespace lime
{

class ISPI;

class FPGA_XTRX : public FPGA
{
public:
    FPGA_XTRX(lime::ISPI* fpgaSPI, lime::ISPI* lms7002mSPI);
    virtual ~FPGA_XTRX(){};
    int SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, double txPhase, double rxPhase, int ch = 0) override;
    int SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, int ch = 0) override;
    int SetPllFrequency(const uint8_t pllIndex, const double inputFreq, FPGA_PLL_clock* clocks, const uint8_t clockCount);
};

}
#endif // FPGA_Q_H
