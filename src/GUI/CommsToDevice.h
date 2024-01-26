#pragma once

#include "limesuite/IComms.h"
#include "limesuite/SDRDevice.h"

/**
  @brief Helper class to initialize communications for UI modules that use I2C communication
    of a specific system-on-chip that is not aware of the whole SDRDevice.
 */
class SPIToSDR : public lime::ISPI
{
  public:
    SPIToSDR(lime::SDRDevice& sdr, uint32_t spiDefaultSlave);
    virtual int SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count);
    virtual int SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count);

  private:
    lime::SDRDevice& device;
    uint32_t mSPIDefaultSlaveId;
};

/**
  @brief Helper class to initialize communications for UI modules that use I2C communication
    of a specific system-on-chip that is not aware of the whole SDRDevice.
 */
class I2CToSDR : public lime::II2C
{
  public:
    I2CToSDR(lime::SDRDevice& sdr);
    virtual int I2CWrite(int address, const std::byte* data, uint32_t length);
    virtual int I2CRead(int address, std::byte* dest, uint32_t length);

  private:
    lime::SDRDevice& device;
};
