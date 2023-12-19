#ifndef LIME_LMS64C_LMS7002M_OVER_PCIE_H
#define LIME_LMS64C_LMS7002M_OVER_PCIE_H

#include "limesuite/IComms.h"
#include "LitePCIe.h"
#include "PCIE_CSR_Pipe.h"

#include <cstdint>
#include <memory>

namespace lime {

/** @brief A class for communicating with a device's LMS7002M chip over a PCIe interface. */
class LMS64C_LMS7002M_Over_PCIe : public IComms
{
  public:
    LMS64C_LMS7002M_Over_PCIe(std::shared_ptr<LitePCIe> dataPort);
    virtual int SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual int SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

  private:
    PCIE_CSR_Pipe pipe;
};

} // namespace lime

#endif // LIME_LMS64C_LMS7002M_OVER_PCIE_H
