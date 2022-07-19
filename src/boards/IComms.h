#pragma once

namespace lime {
class IComms
{
  public:
    IComms(){};
    virtual void SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, size_t count) = 0;

    /*!
     * Write to an available I2C slave.
     * @param address I2C slave address
     * @param data output buffer
     * @param length output data length
     * @return 0-success
     */
    virtual int I2CWrite(int address, const uint8_t *data, size_t length) = 0;

    /*!
     * Read from an available I2C slave.
     * The data parameter can be used to pass optional write data.
     * Some implementations can combine a write + read transaction.
     * If the device contains multiple I2C masters,
     * the address bits can encode which master.
     * \param addr the address of the slave
     * \param [inout] dest buffer to store read data from the slave
     * \param length Number of bytes to read
     * @return 0-success
     */
    virtual int I2CRead(int addr, uint8_t *dest, size_t length) = 0;
};

} // namespace lime
