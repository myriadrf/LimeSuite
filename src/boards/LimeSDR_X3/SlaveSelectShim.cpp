#include "SlaveSelectShim.h"
#include <memory>
#include "limesuite/IComms.h"

using namespace lime;

SlaveSelectShim::SlaveSelectShim(std::shared_ptr<IComms> comms, uint32_t slaveId)
    : port(comms)
    , slaveId(slaveId){};

OpStatus SlaveSelectShim::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return port->SPI(slaveId, MOSI, MISO, count);
}

OpStatus SlaveSelectShim::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return port->SPI(spiBusAddress, MOSI, MISO, count);
}

OpStatus SlaveSelectShim::ResetDevice()
{
    return port->ResetDevice(slaveId);
}
