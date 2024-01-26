#ifndef PCIECOMMON_H
#define PCIECOMMON_H

#include "limesuite/IComms.h"
#include "LMS64CProtocol.h"
#include "LitePCIe.h"

using namespace lime;

/** @brief An abstract class for interfacing with Control/Status registers (CSR) of a PCIe device. */
class PCIE_CSR_Pipe : public ISerialPort
{
  public:
    explicit PCIE_CSR_Pipe(std::shared_ptr<LitePCIe> port);
    virtual int Write(const std::byte* data, size_t length, int timeout_ms) override;
    virtual int Read(std::byte* data, size_t length, int timeout_ms) override;

  protected:
    std::shared_ptr<LitePCIe> port;
};

/** @brief A class for communicating with a device's LMS7002M chip over a PCIe interface. */
class LMS64C_LMS7002M_Over_PCIe : public lime::IComms
{
  public:
    LMS64C_LMS7002M_Over_PCIe(std::shared_ptr<LitePCIe> dataPort);
    virtual int SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual int SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

  private:
    PCIE_CSR_Pipe pipe;
};

/** @brief A class for communicating with a device's FPGA over a PCIe interface. */
class LMS64C_FPGA_Over_PCIe : public lime::IComms
{
  public:
    LMS64C_FPGA_Over_PCIe(std::shared_ptr<LitePCIe> dataPort);

    virtual int SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;
    virtual int SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) override;

    virtual int CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) override;
    virtual int CustomParameterRead(std::vector<CustomParameterIO>& parameters) override;

    virtual int ProgramWrite(
        const std::byte* data, size_t length, int prog_mode, int target, ProgressCallback callback = nullptr) override;

    virtual int MemoryWrite(uint32_t address, const std::byte* data, uint32_t dataLength) override;
    virtual int MemoryRead(uint32_t address, std::byte* data, uint32_t dataLength) override;

  private:
    PCIE_CSR_Pipe pipe;
};

#endif // PCIECOMMON_H