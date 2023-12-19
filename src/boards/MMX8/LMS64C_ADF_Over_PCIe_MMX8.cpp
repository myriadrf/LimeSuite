#include "LMS64C_ADF_Over_PCIe_MMX8.h"

using namespace lime;

LMS64C_ADF_Over_PCIe_MMX8::LMS64C_ADF_Over_PCIe_MMX8(std::shared_ptr<LitePCIe> dataPort, uint32_t subdeviceIndex)
    : pipe(dataPort)
    , subdeviceIndex(subdeviceIndex)
{
}

int LMS64C_ADF_Over_PCIe_MMX8::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return LMS64CProtocol::ADF4002_SPI(pipe, MOSI, count, subdeviceIndex);
}

int LMS64C_ADF_Over_PCIe_MMX8::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return LMS64CProtocol::ADF4002_SPI(pipe, MOSI, count, subdeviceIndex);
}
