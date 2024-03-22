#ifndef FPGA_MINI_H
#define FPGA_MINI_H

#include "FPGA_common.h"

namespace lime {

class ISPI;

/** @brief Class for communicating with the Altera MAX 10 FPGA in the LimeSDR Mini. */
class FPGA_Mini : public FPGA
{
  public:
    FPGA_Mini(std::shared_ptr<ISPI> fpgaSPI, std::shared_ptr<ISPI> lms7002mSPI);
    virtual ~FPGA_Mini(){};
    virtual OpStatus SetInterfaceFreq(
        double txRate_Hz, double rxRate_Hz, double txPhase, double rxPhase, int chipIndex = 0) override;
    virtual OpStatus SetInterfaceFreq(double txRate_Hz, double rxRate_Hz, int chipIndex = 0) override;
};

} // namespace lime
#endif // FPGA_MINI_H
