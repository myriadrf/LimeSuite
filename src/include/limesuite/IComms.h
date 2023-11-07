#pragma once

#include <stdint.h>
#include <string>
#include "limesuite/config.h"

namespace lime {
class LIME_API ISPI
{
  public:
    // Default path for writing/reading registers
    virtual void SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) = 0;

    // Writing/reading registers for specific slave
    virtual void SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) = 0;
};

class LIME_API II2C
{
  public:
    /*!
     * Write to an available I2C slave.
     * @param address I2C slave address
     * @param data output buffer
     * @param length output data length
     * @return 0-success
     */
    virtual int I2CWrite(int address, const uint8_t* data, uint32_t length) = 0;

    /*!
     * Read from an available I2C slave.
     * The data parameter can be used to pass optional write data.
     * Some implementations can combine a write + read transaction.
     * If the device contains multiple I2C masters,
     * the address bits can encode which master.
     * \param address the address of the slave
     * \param [inout] dest buffer to store read data from the slave
     * \param length Number of bytes to read
     * @return 0-success
     */
    virtual int I2CRead(int address, uint8_t* dest, uint32_t length) = 0;
};

class IComms : public ISPI
{
  public:
    virtual ~IComms(){};

    bool IsOpen();

    virtual int GPIODirRead(uint8_t* buffer, const size_t bufLength) { return -1; };
    virtual int GPIORead(uint8_t* buffer, const size_t bufLength) { return -1; };
    virtual int GPIODirWrite(const uint8_t* buffer, const size_t bufLength) { return -1; };
    virtual int GPIOWrite(const uint8_t* buffer, const size_t bufLength) { return -1; };

    virtual int CustomParameterWrite(const int32_t* ids, const double* values, const size_t count, const std::string& units)
    {
        return -1;
    };
    virtual int CustomParameterRead(const int32_t* ids, double* values, const size_t count, std::string* units) { return -1; };

    typedef bool (*ProgressCallback)(size_t bytesSent, size_t bytesTotal, const char* progressMsg); // return true to stop progress
    virtual int ProgramWrite(const char* data, size_t length, int prog_mode, int target, ProgressCallback callback = nullptr)
    {
        return -1;
    }
    virtual int ResetDevice(int chipSelect) { return -1; };
};

} // namespace lime
