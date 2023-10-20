#ifndef PCIECOMMON_H
#define PCIECOMMON_H

#include "limesuite/IComms.h"
#include "LMS64CProtocol.h"
#include "LitePCIe.h"

using namespace lime;

class PCIE_CSR_Pipe : public ISerialPort
{
public:
    explicit PCIE_CSR_Pipe(LitePCIe& port);
    virtual int Write(const uint8_t* data, size_t length, int timeout_ms) override;
    virtual int Read(uint8_t* data, size_t length, int timeout_ms) override;
protected:
    LitePCIe& port;
};

class LMS64C_LMS7002M_Over_PCIe : public lime::IComms
{
public:
    LMS64C_LMS7002M_Over_PCIe(LitePCIe* dataPort);
    void SPI(const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override;
    void SPI(uint32_t spiBusAddress, const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override;
private:
    PCIE_CSR_Pipe pipe;
};

class LMS64C_FPGA_Over_PCIe : public lime::IComms
{
public:
    LMS64C_FPGA_Over_PCIe(LitePCIe* dataPort);

    void SPI(const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override;
    void SPI(uint32_t spiBusAddress, const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override;

    virtual int CustomParameterWrite(const int32_t *ids, const double *values, const size_t count, const std::string& units) override;
    virtual int CustomParameterRead(const int32_t *ids, double *values, const size_t count, std::string* units) override;

    virtual int ProgramWrite(const char* data, size_t length, int prog_mode, int target, ProgressCallback callback = nullptr) override;
private:
    PCIE_CSR_Pipe pipe;
};

#endif // PCIECOMMON_H