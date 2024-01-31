#include "LMS64C_FPGA_Over_USB.h"
#include "protocols/LMS64CProtocol.h"

using namespace lime;

LMS64C_FPGA_Over_USB::LMS64C_FPGA_Over_USB(std::shared_ptr<USB_CSR_Pipe> dataPort)
    : pipe(dataPort)
{
}

int LMS64C_FPGA_Over_USB::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return SPI(0, MOSI, MISO, count);
}

int LMS64C_FPGA_Over_USB::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return LMS64CProtocol::FPGA_SPI(*pipe, MOSI, MISO, count);
}

int LMS64C_FPGA_Over_USB::GPIODirRead(uint8_t* buffer, const size_t bufLength)
{
    return LMS64CProtocol::GPIODirRead(*pipe, buffer, bufLength);
}

int LMS64C_FPGA_Over_USB::GPIORead(uint8_t* buffer, const size_t bufLength)
{
    return LMS64CProtocol::GPIORead(*pipe, buffer, bufLength);
}

int LMS64C_FPGA_Over_USB::GPIODirWrite(const uint8_t* buffer, const size_t bufLength)
{
    return LMS64CProtocol::GPIODirWrite(*pipe, buffer, bufLength);
}

int LMS64C_FPGA_Over_USB::GPIOWrite(const uint8_t* buffer, const size_t bufLength)
{
    return LMS64CProtocol::GPIOWrite(*pipe, buffer, bufLength);
}

int LMS64C_FPGA_Over_USB::CustomParameterWrite(const std::vector<CustomParameterIO>& parameters)
{
    return LMS64CProtocol::CustomParameterWrite(*pipe, parameters);
}

int LMS64C_FPGA_Over_USB::CustomParameterRead(std::vector<CustomParameterIO>& parameters)
{
    return LMS64CProtocol::CustomParameterRead(*pipe, parameters);
}

int LMS64C_FPGA_Over_USB::ProgramWrite(const char* data, size_t length, int prog_mode, int target, ProgressCallback callback)
{
    return LMS64CProtocol::ProgramWrite(
        *pipe, data, length, prog_mode, static_cast<LMS64CProtocol::ProgramWriteTarget>(target), callback);
}
