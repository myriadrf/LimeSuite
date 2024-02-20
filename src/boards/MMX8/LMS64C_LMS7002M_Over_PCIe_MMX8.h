#ifndef LIME_LMS64C_LMS7002M_OVER_PCIE_MMX8_H
#define LIME_LMS64C_LMS7002M_OVER_PCIE_MMX8_H

#include "limesuite/IComms.h"
#include "LitePCIe.h"
#include "PCIE_CSR_Pipe.h"

#include <cstdint>
#include <memory>

namespace lime {

/** @brief A class for communicating with MMX8's subdevice's LMS7002M chips. */
class LMS64C_LMS7002M_Over_PCIe_MMX8 : public IComms
{
  public:
    /**
      @brief Construct a new LMS64C_LMS7002M_Over_PCIe_MMX8 object
      @param dataPort The PCIe data bus to use.
      @param subdeviceIndex The subdevice index for which this class is created.
     */
    LMS64C_LMS7002M_Over_PCIe_MMX8(std::shared_ptr<LitePCIe> dataPort, uint32_t subdeviceIndex);
    virtual OpStatus SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual OpStatus SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual OpStatus ResetDevice(int chipSelect) override;

  private:
    PCIE_CSR_Pipe pipe;
    uint32_t subdeviceIndex;
};

} // namespace lime

#endif // LIME_LMS64C_LMS7002M_OVER_PCIE_MMX8_H
