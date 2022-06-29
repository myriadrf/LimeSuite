#include "SDRDevice.h"

#include "DeviceExceptions.h"
#include "LMSBoards.h"

namespace lime {

SDRDevice::SDRDevice()
{
}

SDRDevice::~SDRDevice()
{
}

DeviceInfo SDRDevice::GetDeviceInfo(void)
{
    DeviceInfo info;
    //initialize to UNKNOWN board type
    //causes lms7_device::CreateDevice() to use LMS7_Generic
    info.deviceName = GetDeviceName(LMS_DEV_UNKNOWN);
    info.expansionName = GetExpansionBoardName(EXP_BOARD_UNKNOWN);
    return info;
}

void SDRDevice::SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, size_t count)
{
    throw(OperationNotSupported("TransactSPI not implemented"));
}

int SDRDevice::I2CWrite(int address, const uint8_t *data, size_t length)
{
    throw(OperationNotSupported("WriteI2C not implemented"));
}

int SDRDevice::I2CRead(int addr, uint8_t *dest, size_t length)
{
    throw(OperationNotSupported("ReadI2C not implemented"));
}

int SDRDevice::GPIOWrite(const uint8_t *buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIOWrite not implemented"));
}

int SDRDevice::GPIORead(uint8_t *buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIORead not implemented"));
}

int SDRDevice::GPIODirWrite(const uint8_t *buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIODirWrite not implemented"));
}

int SDRDevice::GPIODirRead(uint8_t *buffer, const size_t bufLength)
{
    throw(OperationNotSupported("GPIODirRead not implemented"));
}

int SDRDevice::CustomParameterWrite(const uint8_t *ids, const double *values, const size_t count, const std::string& units)
{
    throw(OperationNotSupported("CustomParameterWrite not implemented"));
}
int SDRDevice::CustomParameterRead(const uint8_t *ids, double *values, const size_t count, std::string* units)
{
    throw(OperationNotSupported("CustomParameterRead not implemented"));
}
void SDRDevice::SetDataLogCallback(std::function<void(bool, const unsigned char*, const unsigned int)> callback)
{
    throw(OperationNotSupported("SetDataLogCallback not implemented"));
}

}
