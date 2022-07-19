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

SDRDevice::DeviceInfo SDRDevice::GetDeviceInfo(void)
{
    SDRDevice::DeviceInfo info;
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

int SDRDevice::ReadLMSReg(uint16_t address, int ind)
{
    uint32_t mosi = address;
    uint32_t miso = 0;
    const auto slaves = GetDescriptor().spiSlaveIds;
    SPI(slaves.at("LMS7002M"), &mosi, &miso, 1);
    return miso & 0xFFFF;
}
void SDRDevice::WriteLMSReg(uint16_t address, uint16_t val, int ind)
{
    const uint32_t mosi = ((address | 0x8000) << 16) | val;
    const auto slaves = GetDescriptor().spiSlaveIds;
    SPI(slaves.at("LMS7002M"), &mosi, nullptr, 1);
}
int SDRDevice::ReadFPGAReg(uint16_t address)
{
    uint32_t mosi = address;
    uint32_t miso = 0;
    const auto slaves = GetDescriptor().spiSlaveIds;
    SPI(slaves.at("FPGA"), &mosi, &miso, 1);
    return miso & 0xFFFF;
}
void SDRDevice::WriteFPGAReg(uint16_t address, uint16_t val)
{
    const uint32_t mosi = ((address | 0x8000) << 16) | val;
    const auto slaves = GetDescriptor().spiSlaveIds;
    SPI(slaves.at("FPGA"), &mosi, nullptr, 1);
}

void *SDRDevice::GetInternalChip(uint32_t index)
{
    throw std::logic_error("SDR device does not have internal chips");
}

int SDRDevice::StreamStart(const StreamConfig &config)
{
    throw(OperationNotSupported("StreamStart not implemented"));
}

void SDRDevice::StreamStop() {}

int SDRDevice::StreamRx(uint8_t channel, void **samples, uint32_t count, StreamMeta *meta)
{
    throw(OperationNotSupported("StreamRx not implemented"));
}

int SDRDevice::StreamTx(uint8_t channel, const void **samples, uint32_t count,
                        const StreamMeta *meta)
{
    throw(OperationNotSupported("StreamTx not implemented"));
}

void SDRDevice::StreamStatus(uint8_t channel, StreamStats &status)
{
    throw(OperationNotSupported("StreamStatus not implemented"));
}

} // namespace lime
