#include "CommsToDevice.h"

/**
  @brief Constructs the Serial Peripheral Interface communication layer.
  @param sdr The device to communicate with.
  @param spiDefaultSlave The default slave ID to use.
 */
SPIToSDR::SPIToSDR(lime::SDRDevice& sdr, uint32_t spiDefaultSlave)
    : device(sdr)
    , mSPIDefaultSlaveId(spiDefaultSlave)
{
}

int SPIToSDR::SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return device.SPI(mSPIDefaultSlaveId, MOSI, MISO, count);
}

int SPIToSDR::SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count)
{
    return device.SPI(spiBusAddress, MOSI, MISO, count);
}

/**
  @brief Constructs the Inter-Integrated Circuit communication layer.
  @param sdr The device to communicate with.
 */
I2CToSDR::I2CToSDR(lime::SDRDevice& sdr)
    : device(sdr)
{
}

int I2CToSDR::I2CWrite(int address, const uint8_t* data, uint32_t length)
{
    return device.I2CWrite(address, data, length);
}

int I2CToSDR::I2CRead(int address, uint8_t* dest, uint32_t length)
{
    return device.I2CRead(address, dest, length);
}