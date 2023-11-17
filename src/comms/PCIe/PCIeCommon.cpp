#include "PCIeCommon.h"

using namespace lime;

PCIE_CSR_Pipe::PCIE_CSR_Pipe(std::shared_ptr<LitePCIe> port)
    : port(port)
{
}

int PCIE_CSR_Pipe::Write(const uint8_t* data, size_t length, int timeout_ms)
{
    return port->WriteControl(data, length, timeout_ms);
}
int PCIE_CSR_Pipe::Read(uint8_t* data, size_t length, int timeout_ms)
{
    return port->ReadControl(data, length, timeout_ms);
}

LMS64C_LMS7002M_Over_PCIe::LMS64C_LMS7002M_Over_PCIe(std::shared_ptr<LitePCIe> dataPort)
    : pipe(dataPort)
{
}

void LMS64C_LMS7002M_Over_PCIe::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    SPI(0, MOSI, MISO, count);
    return;
}
void LMS64C_LMS7002M_Over_PCIe::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    LMS64CProtocol::LMS7002M_SPI(pipe, spiBusAddress, MOSI, MISO, count);
    return;
}

LMS64C_FPGA_Over_PCIe::LMS64C_FPGA_Over_PCIe(std::shared_ptr<LitePCIe> dataPort)
    : pipe(dataPort)
{
}

void LMS64C_FPGA_Over_PCIe::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    SPI(0, MOSI, MISO, count);
}

void LMS64C_FPGA_Over_PCIe::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    LMS64CProtocol::FPGA_SPI(pipe, MOSI, MISO, count);
}

int LMS64C_FPGA_Over_PCIe::CustomParameterWrite(
    const int32_t* ids, const double* values, const size_t count, const std::string& units)
{
    return LMS64CProtocol::CustomParameterWrite(pipe, ids, values, count, units);
}

int LMS64C_FPGA_Over_PCIe::CustomParameterRead(
    const int32_t* ids, double* values, const size_t count, std::vector<std::reference_wrapper<std::string>>& units)
{
    return LMS64CProtocol::CustomParameterRead(pipe, ids, values, count, units);
}

int LMS64C_FPGA_Over_PCIe::ProgramWrite(const char* data, size_t length, int prog_mode, int target, ProgressCallback callback)
{
    return LMS64CProtocol::ProgramWrite(pipe, data, length, prog_mode, (LMS64CProtocol::ProgramWriteTarget)target, callback);
}
