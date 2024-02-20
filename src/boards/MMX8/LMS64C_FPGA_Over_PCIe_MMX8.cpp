#include "LMS64C_FPGA_Over_PCIe_MMX8.h"

using namespace lime;

LMS64C_FPGA_Over_PCIe_MMX8::LMS64C_FPGA_Over_PCIe_MMX8(std::shared_ptr<LitePCIe> dataPort, uint32_t subdeviceIndex)
    : pipe(dataPort)
    , subdeviceIndex(subdeviceIndex)
{
}

OpStatus LMS64C_FPGA_Over_PCIe_MMX8::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return LMS64CProtocol::FPGA_SPI(pipe, MOSI, MISO, count, subdeviceIndex);
}

OpStatus LMS64C_FPGA_Over_PCIe_MMX8::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return LMS64CProtocol::FPGA_SPI(pipe, MOSI, MISO, count, subdeviceIndex);
}

OpStatus LMS64C_FPGA_Over_PCIe_MMX8::CustomParameterWrite(const std::vector<CustomParameterIO>& parameters)
{
    return LMS64CProtocol::CustomParameterWrite(pipe, parameters, subdeviceIndex);
}

OpStatus LMS64C_FPGA_Over_PCIe_MMX8::CustomParameterRead(std::vector<CustomParameterIO>& parameters)
{
    return LMS64CProtocol::CustomParameterRead(pipe, parameters, subdeviceIndex);
}

OpStatus LMS64C_FPGA_Over_PCIe_MMX8::ProgramWrite(
    const char* data, size_t length, int prog_mode, int target, ProgressCallback callback)
{
    return LMS64CProtocol::ProgramWrite(
        pipe, data, length, prog_mode, static_cast<LMS64CProtocol::ProgramWriteTarget>(target), callback, subdeviceIndex);
}

OpStatus LMS64C_FPGA_Over_PCIe_MMX8::MemoryWrite(uint32_t address, const void* data, uint32_t dataLength)
{
    return LMS64CProtocol::MemoryWrite(pipe, address, data, dataLength, subdeviceIndex);
}

OpStatus LMS64C_FPGA_Over_PCIe_MMX8::MemoryRead(uint32_t address, void* data, uint32_t dataLength)
{
    return LMS64CProtocol::MemoryRead(pipe, address, data, dataLength, subdeviceIndex);
}
