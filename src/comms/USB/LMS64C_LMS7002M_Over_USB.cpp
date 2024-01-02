#include "LMS64C_LMS7002M_Over_USB.h"
#include "LMS64CProtocol.h"

using namespace lime;

LMS64C_LMS7002M_Over_USB::LMS64C_LMS7002M_Over_USB(std::shared_ptr<USB_CSR_Pipe> dataPort)
    : pipe(dataPort)
{
}

int LMS64C_LMS7002M_Over_USB::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return LMS64CProtocol::LMS7002M_SPI(*pipe, 0, MOSI, MISO, count);
}

int LMS64C_LMS7002M_Over_USB::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return LMS64CProtocol::LMS7002M_SPI(*pipe, spiBusAddress, MOSI, MISO, count);
}

int LMS64C_LMS7002M_Over_USB::ResetDevice(int chipSelect)
{
    return LMS64CProtocol::DeviceReset(*pipe, chipSelect);
}
