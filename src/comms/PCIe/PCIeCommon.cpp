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

int LMS64C_LMS7002M_Over_PCIe::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return SPI(0, MOSI, MISO, count);
}

int LMS64C_LMS7002M_Over_PCIe::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return LMS64CProtocol::LMS7002M_SPI(pipe, spiBusAddress, MOSI, MISO, count);
}

LMS64C_FPGA_Over_PCIe::LMS64C_FPGA_Over_PCIe(std::shared_ptr<LitePCIe> dataPort)
    : pipe(dataPort)
{
}

int LMS64C_FPGA_Over_PCIe::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return SPI(0, MOSI, MISO, count);
}

int LMS64C_FPGA_Over_PCIe::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return LMS64CProtocol::FPGA_SPI(pipe, MOSI, MISO, count);
}

int LMS64C_FPGA_Over_PCIe::CustomParameterWrite(const std::vector<CustomParameterIO>& parameters)
{
    return LMS64CProtocol::CustomParameterWrite(pipe, parameters);
}

int LMS64C_FPGA_Over_PCIe::CustomParameterRead(std::vector<CustomParameterIO>& parameters)
{
    return LMS64CProtocol::CustomParameterRead(pipe, parameters);
}

int LMS64C_FPGA_Over_PCIe::ProgramWrite(const char* data, size_t length, int prog_mode, int target, ProgressCallback callback)
{
    return LMS64CProtocol::ProgramWrite(
        pipe, data, length, prog_mode, static_cast<LMS64CProtocol::ProgramWriteTarget>(target), callback);
}

int LMS64C_FPGA_Over_PCIe::MemoryWrite(uint32_t address, const void* data, uint32_t dataLength)
{
    return LMS64CProtocol::MemoryWrite(pipe, address, data, dataLength);
}

int LMS64C_FPGA_Over_PCIe::MemoryRead(uint32_t address, void* data, uint32_t dataLength)
{
    return LMS64CProtocol::MemoryRead(pipe, address, data, dataLength);
}
