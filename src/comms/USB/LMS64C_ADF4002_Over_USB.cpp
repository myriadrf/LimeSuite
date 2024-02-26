#include "LMS64C_ADF4002_Over_USB.h"
#include "LMS64CProtocol.h"

using namespace lime;

LMS64C_ADF4002_Over_USB::LMS64C_ADF4002_Over_USB(std::shared_ptr<USB_CSR_Pipe> dataPort)
    : pipe(dataPort)
{
}

OpStatus LMS64C_ADF4002_Over_USB::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return LMS64CProtocol::ADF4002_SPI(*pipe, MOSI, count);
}

OpStatus LMS64C_ADF4002_Over_USB::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return LMS64CProtocol::ADF4002_SPI(*pipe, MOSI, count);
}
