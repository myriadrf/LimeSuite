#include "LMS64C_LMS7002M_Over_PCIe_MMX8.h"

using namespace lime;

LMS64C_LMS7002M_Over_PCIe_MMX8::LMS64C_LMS7002M_Over_PCIe_MMX8(std::shared_ptr<LitePCIe> dataPort, uint32_t subdeviceIndex)
    : pipe(dataPort)
    , subdeviceIndex(subdeviceIndex)
{
}

OpStatus LMS64C_LMS7002M_Over_PCIe_MMX8::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return SPI(0, MOSI, MISO, count);
}

OpStatus LMS64C_LMS7002M_Over_PCIe_MMX8::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return LMS64CProtocol::LMS7002M_SPI(pipe, spiBusAddress, MOSI, MISO, count, subdeviceIndex);
}

OpStatus LMS64C_LMS7002M_Over_PCIe_MMX8::ResetDevice(int chipSelect)
{
    return LMS64CProtocol::DeviceReset(pipe, chipSelect, subdeviceIndex);
}
