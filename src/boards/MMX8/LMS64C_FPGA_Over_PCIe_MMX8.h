#ifndef LIME_LMS64C_FPGA_OVER_PCIE_MMX8_H
#define LIME_LMS64C_FPGA_OVER_PCIE_MMX8_H

#include "limesuite/IComms.h"
#include "LitePCIe.h"
#include "PCIE_CSR_Pipe.h"

#include <cstdint>
#include <memory>

namespace lime {

/** @brief A class for communicating with MMX8's subdevice's FPGA chips. */
class LMS64C_FPGA_Over_PCIe_MMX8 : public IComms
{
  public:
    LMS64C_FPGA_Over_PCIe_MMX8(std::shared_ptr<LitePCIe> dataPort, uint32_t subdeviceIndex);

    virtual int SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual int SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual int CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) override;
    virtual int CustomParameterRead(std::vector<CustomParameterIO>& parameters) override;

    virtual int ProgramWrite(
        const char* data, size_t length, int prog_mode, int target, ProgressCallback callback = nullptr) override;

    virtual int MemoryWrite(uint32_t address, const void* data, uint32_t dataLength) override;
    virtual int MemoryRead(uint32_t address, void* data, uint32_t dataLength) override;

  private:
    PCIE_CSR_Pipe pipe;
    uint32_t subdeviceIndex;
};

} // namespace lime

#endif // LIME_LMS64C_FPGA_OVER_PCIE_MMX8_H