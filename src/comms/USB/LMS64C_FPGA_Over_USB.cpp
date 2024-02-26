#include "LMS64C_FPGA_Over_USB.h"
#include "protocols/LMS64CProtocol.h"

using namespace lime;

LMS64C_FPGA_Over_USB::LMS64C_FPGA_Over_USB(std::shared_ptr<USB_CSR_Pipe> dataPort)
    : pipe(dataPort)
{
}

OpStatus LMS64C_FPGA_Over_USB::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return SPI(0, MOSI, MISO, count);
}

OpStatus LMS64C_FPGA_Over_USB::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return LMS64CProtocol::FPGA_SPI(*pipe, MOSI, MISO, count);
}

OpStatus LMS64C_FPGA_Over_USB::GPIODirRead(uint8_t* buffer, const size_t bufLength)
{
    return LMS64CProtocol::GPIODirRead(*pipe, buffer, bufLength);
}

OpStatus LMS64C_FPGA_Over_USB::GPIORead(uint8_t* buffer, const size_t bufLength)
{
    return LMS64CProtocol::GPIORead(*pipe, buffer, bufLength);
}

OpStatus LMS64C_FPGA_Over_USB::GPIODirWrite(const uint8_t* buffer, const size_t bufLength)
{
    return LMS64CProtocol::GPIODirWrite(*pipe, buffer, bufLength);
}

OpStatus LMS64C_FPGA_Over_USB::GPIOWrite(const uint8_t* buffer, const size_t bufLength)
{
    return LMS64CProtocol::GPIOWrite(*pipe, buffer, bufLength);
}

OpStatus LMS64C_FPGA_Over_USB::CustomParameterWrite(const std::vector<CustomParameterIO>& parameters)
{
    return LMS64CProtocol::CustomParameterWrite(*pipe, parameters);
}

OpStatus LMS64C_FPGA_Over_USB::CustomParameterRead(std::vector<CustomParameterIO>& parameters)
{
    return LMS64CProtocol::CustomParameterRead(*pipe, parameters);
}

OpStatus LMS64C_FPGA_Over_USB::ProgramWrite(const char* data, size_t length, int prog_mode, int target, ProgressCallback callback)
{
    return LMS64CProtocol::ProgramWrite(
        *pipe, data, length, prog_mode, static_cast<LMS64CProtocol::ProgramWriteTarget>(target), callback);
}

OpStatus LMS64C_FPGA_Over_USB::MemoryWrite(uint32_t address, const void* data, uint32_t dataLength)
{
    return LMS64CProtocol::MemoryWrite(*pipe, address, data, dataLength);
}

OpStatus LMS64C_FPGA_Over_USB::MemoryRead(uint32_t address, void* data, uint32_t dataLength)
{
    return LMS64CProtocol::MemoryRead(*pipe, address, data, dataLength);
}
