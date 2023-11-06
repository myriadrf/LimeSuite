#include "USBCommon.h"

namespace lime {

LMS64C_LMS7002M_Over_USB::LMS64C_LMS7002M_Over_USB(std::shared_ptr<USB_CSR_Pipe> dataPort)
    : pipe(dataPort)
{
}

void LMS64C_LMS7002M_Over_USB::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    LMS64CProtocol::LMS7002M_SPI(*pipe, 0, MOSI, MISO, count);
}

void LMS64C_LMS7002M_Over_USB::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    LMS64CProtocol::LMS7002M_SPI(*pipe, spiBusAddress, MOSI, MISO, count);
}

int LMS64C_LMS7002M_Over_USB::ResetDevice(int chipSelect)
{
    return LMS64CProtocol::DeviceReset(*pipe, chipSelect);
};

LMS64C_FPGA_Over_USB::LMS64C_FPGA_Over_USB(std::shared_ptr<USB_CSR_Pipe> dataPort)
    : pipe(dataPort)
{
}

void LMS64C_FPGA_Over_USB::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    SPI(0, MOSI, MISO, count);
}

void LMS64C_FPGA_Over_USB::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    LMS64CProtocol::FPGA_SPI(*pipe, MOSI, MISO, count);
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

int LMS64C_FPGA_Over_USB::CustomParameterWrite(
    const int32_t* ids, const double* values, const size_t count, const std::string& units)
{
    return LMS64CProtocol::CustomParameterWrite(*pipe, ids, values, count, units);
}

int LMS64C_FPGA_Over_USB::CustomParameterRead(const int32_t* ids, double* values, const size_t count, std::string* units)
{
    return LMS64CProtocol::CustomParameterRead(*pipe, ids, values, count, units);
}

int LMS64C_FPGA_Over_USB::ProgramWrite(const char* data, size_t length, int prog_mode, int target, ProgressCallback callback)
{
    return LMS64CProtocol::ProgramWrite(*pipe, data, length, prog_mode, (LMS64CProtocol::ProgramWriteTarget)target, callback);
}

} // namespace lime