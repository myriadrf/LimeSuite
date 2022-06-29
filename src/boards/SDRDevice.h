#ifndef LIME_SDRDevice_H
#define LIME_SDRDevice_H

#include "DeviceHandle.h"
#include "IConnection.h"

namespace lime {

class SDRDevice
{
public:
    SDRDevice();
    virtual ~SDRDevice();

    //! Get the connection handle that was used to create this connection
    const DeviceHandle &GetHandle(void) const;

    /*!
     * Get information about a device
     * for displaying helpful information
     * or for making device-specific decisions.
     */
    virtual DeviceInfo GetDeviceInfo(void);

    /*!
     * @brief Bulk SPI write/read transaction.
     *
     * The transactSPI function is capable of bulk writes and bulk reads
     * of SPI registers in an arbitrary IC (up to 32-bits per transaction).
     *
     * MISO (Master input Slave output) may be NULL to indicate a write-only operation,
     * the underlying implementation may be able to optimize out the readback.
     *
     * @param chipSelect SPI target chip address
     * @param MOSI Master output Slave input, data to write
     * @param [out] MISO Master input Slave output, buffer to fill with read data
     * @param count Number of SPI transactions
     * @return 0-success
     */
    virtual void SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, size_t count);

    /*!
     * Write to an available I2C slave.
     * @param address I2C slave address
     * @param data output buffer
     * @param length output data length
     * @return 0-success
     */
    virtual int I2CWrite(int address, const uint8_t *data, size_t length);

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
    virtual int I2CRead(int addr, uint8_t *dest, size_t length);

    /***********************************************************************
     * GPIO API
     **********************************************************************/

    /**	@brief Writes GPIO values to device
    @param buffer for source of GPIO values LSB first, each bit sets GPIO state
    @param bufLength buffer length
    @return the operation success state
    */
    virtual int GPIOWrite(const uint8_t *buffer, const size_t bufLength);

    /**	@brief Reads GPIO values from device
    @param buffer destination for GPIO values LSB first, each bit represent GPIO state
    @param bufLength buffer length to read
    @return the operation success state
    */
    virtual int GPIORead(uint8_t *buffer, const size_t bufLength);

    /**	@brief Write GPIO direction control values to device.
    @param buffer with GPIO direction configuration (0 input, 1 output)
    @param bufLength buffer length
    @return the operation success state
    */
    virtual int GPIODirWrite(const uint8_t *buffer, const size_t bufLength);

    /**	@brief Read GPIO direction configuration from device
    @param buffer to put GPIO direction configuration (0 input, 1 output)
    @param bufLength buffer length to read
    @return the operation success state
    */
    virtual int GPIODirRead(uint8_t *buffer, const size_t bufLength);

    /***********************************************************************
     * Aribtrary settings API
     **********************************************************************/

    /** @brief Sets custom on board control to given value units
    @param ids indexes of selected controls
    @param values new control values
    @param count number of values to write
    @param units (optional) when not null specifies value units (e.g V, A, Ohm, C... )
    @return the operation success state
    */
    virtual int CustomParameterWrite(const uint8_t *ids, const double *values, const size_t count, const std::string& units);

    /** @brief Returns value of custom on board control
	@param ids indexes of controls to read
	@param values retrieved control values
	@param count number of values to read
	@param units (optional) when not null returns value units (e.g V, A, Ohm, C... )
	@return the operation success state
    */
    virtual int CustomParameterRead(const uint8_t *ids, double *values, const size_t count, std::string* units);

    /** @brief Sets callback function which gets called each time data is sent or received
    */
    void SetDataLogCallback(std::function<void(bool, const unsigned char*, const unsigned int)> callback);
protected:
    std::function<void(bool, const unsigned char*, const unsigned int)> callback_logData;

private:
    friend class ConnectionRegistry;
    ConnectionHandle _handle;
};

}
#endif

