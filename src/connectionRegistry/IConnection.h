/**
    @file IConnection.h
    @author Lime Microsystems
    @brief Interface class for connection types
*/

#ifndef ICONNECTION_H
#define ICONNECTION_H

#include <ConnectionHandle.h>
#include <string>
#include <vector>
#include <cstring> //memset
#include <functional>

enum OperationStatus
{
    SUCCESS = 0,
    FAILED,
    UNSUPPORTED,
    DISCONNECTED,
    USER_ABORTED,
};

/*!
 * Information about the set of available hardware on a device.
 * This includes available ICs, streamers, and version info.
 *
 * This structure provides SPI slave addresses for one or more RFICs
 * and slave addresses or I2C addresses for commonly supported ICs.
 * A -1 for an address number indicates that it is not available.
 */
struct DeviceInfo
{
    DeviceInfo(void);

    //! The displayable name for the device
    std::string deviceName;

    /*! The displayable name for the expansion card
     * Ex: if the RFIC is on a daughter-card
     */
    std::string expansionName;

    //! The firmware version as a string
    std::string firmwareVersion;

    //! The hardware version as a string
    std::string hardwareVersion;

    //! The protocol version as a string
    std::string protocolVersion;

    /*!
     * The SPI address numbers used to access each LMS7002M.
     * This index will be used in the spi access functions.
     */
    std::vector<int> addrsLMS7002M;

    /*!
     * The I2C address number used to access the Si5351
     * found on some development boards. -1 when not present.
     */
    int addrSi5351;

    /*!
     * The SPI address number used to access the ADF4002
     * found on some development boards. -1 when not present.
     */
    int addrADF4002;
};

/*!
 * The Stream metadata structure is used with the streaming API to exchange
 * extra data associated with the stream such as timestamps and burst info.
 */
struct StreamMetadata
{
    StreamMetadata(void);

    /*!
     * The timestamp in clock units
     * Set to 0 when the timestamp is not applicable.
     */
    uint64_t timestamp;

    /*!
     * True to indicate the end of a stream buffer.
     * When false, subsequent calls continue the stream.
     */
    bool endOfBurst;
};

/*!
 * The stream config structure is used with the SetupStream() API.
 */
struct StreamConfig
{
    StreamConfig(void);

    //! True for transmit stream, false for receive
    bool isTx;

    /*!
     * The array of channels used by the stream.
     * Configurations depend on hardware support.
     * Example [0] for channel 0 on RFIC0
     * Example [0, 1] for MIMO on RFIC0
     * Example [2, 3] for MIMO on RFIC1
     */
    std::vector<size_t> channels;

    //! Possible stream data formats
    enum StreamDataFormat
    {
        STREAM_12_BIT_IN_16,
        STREAM_12_BIT_COMPRESSED,
        STREAM_COMPLEX_FLOAT32,
    };

    /*!
     * The buffer length is a size in samples
     * that used for allocating internal buffers.
     * Default: 0, meaning automatic selection
     */
    size_t bufferLength;

    //! The format of the samples in Read/WriteStream().
    StreamDataFormat format;

    /*!
     * The format of samples over the wire.
     * This is not the format presented to the API caller.
     * Choosing a compressed format can decrease link use
     * at the expense of additional processing on the PC
     * Default: STREAM_12_BIT_IN_16
     */
    StreamDataFormat linkFormat;
};

/*!
 * IConnection is the interface class for a device with 1 or more Lime RFICs.
 * The LMS7002M driver class calls into IConnection to interface with the hardware
 * to implement high level functions on top of low-level SPI and GPIO.
 * Device developers will implement a custom IConnection for their hardware
 * as an abstraction for streaming and low-level SPI and configuration access.
 */
class IConnection
{
public:
    /*!
     * Callback from programming processes
     * @param bsent number of bytes transferred
     * @param btotal total number of bytes to send
     * @param progressMsg string describing current progress state
     * @return 0-continue programming, 1-abort operation
     */
    typedef std::function<bool(int bsent, int btotal, const char* progressMsg)> ProgrammingCallback;

    //! IConnection constructor
    IConnection(void);

    //! IConnection destructor
    virtual ~IConnection(void);

    //! Get the connection handle that was used to create this connection
    const ConnectionHandle &GetHandle(void) const;

    /*!
     * Is this connection open?
     * The constructor should attempt to connect but may fail,
     * or the connection may go down at a later time.
     * @return true when the connection is available
     */
    virtual bool IsOpen(void);

    /*!
     * Get information about a device
     * for displaying helpful information
     * or for making device-specific decisions.
     */
    virtual DeviceInfo GetDeviceInfo(void);

    /*!
     * Perform reset sequence on the device.
     * Typically this will reset the RFIC using a GPIO,
     * and possibly other ICs located on the device.
     */
    virtual OperationStatus DeviceReset(void);

    /***********************************************************************
     * Serial API
     **********************************************************************/

    /*!
     * @brief Bulk SPI write/read transaction.
     *
     * The transactSPI function is capable of bulk writes and bulk reads
     * of SPI registers in an arbitrary IC (up to 32-bits per transaction).
     *
     * The readData parameter may be NULL to indicate a write-only operation,
     * the underlying implementation may be able to optimize out the readback.
     *
     * @param addr the SPI device address
     * @param writeData SPI bits to write out
     * @param [out] readData stores readback data
     * @param size the number of SPI transactions
     * @return the transaction success state
     */
    virtual OperationStatus TransactSPI(const int addr, const uint32_t *writeData, uint32_t *readData, const size_t size);

    /*!
     * Write to an available I2C slave.
     * @param addr the address of the slave
     * @param data an array of bytes write out
     * @return the transaction success state
     */
    virtual OperationStatus WriteI2C(const int addr, const std::string &data);

    /*!
     * Read from an available I2C slave.
     * The data parameter can be used to pass optional write data.
     * Some implementations can combine a write + read transaction.
     * If the device contains multiple I2C masters,
     * the address bits can encode which master.
     * \param addr the address of the slave
     * \param numBytes the number of bytes to read
     * \param [inout] data an array of bytes read from the slave
     * @return the transaction success state
     */
    virtual OperationStatus ReadI2C(const int addr, const size_t numBytes, std::string &data);

    /***********************************************************************
     * Antenna API
     **********************************************************************/

    /*!
     * Called by the LMS7002M driver after potential band-selection changes.
     * Implementations may have additional external bands to switch via GPIO.
     * @param trfBand the SEL_BAND2_TRF config bits
     * @param rfeBand the SEL_PATH_RFE config bits
     */
    virtual void UpdateExternalBandSelect(const int trfBand, const int rfeBand);

    /***********************************************************************
     * Reference clocks API
     **********************************************************************/

    /*!
     * Query the frequency of the reference clock.
     * Some implementations have a fixed reference,
     * some have a programmable synthesizer like Si5351C.
     * @return the reference clock rate in Hz
     */
    virtual double GetReferenceClockRate(void);

    /*!
     * Set the programmable reference clock rate.
     * Some implementations use the programmable Si5351C.
     * @param rate the clock rate in Hz
     */
    virtual void SetReferenceClockRate(const double rate);

    /*!
     * Query the TX frequency of the reference clock.
     * Some boards will use a separate tx reference, otherwise
     * this call simply forwards to GetReferenceClockRate()
     * @return the reference clock rate in Hz
     */
    virtual double GetTxReferenceClockRate(void);

    /*!
     * Set the programmable TX reference clock rate.
     * Some boards will use a separate tx reference, otherwise
     * this call simply forwards to SetReferenceClockRate()
     * @param rate the clock rate in Hz
     */
    virtual void SetTxReferenceClockRate(const double rate);

    /***********************************************************************
     * Stream API
     **********************************************************************/

    /*!
     * Setup a stream with a request configuration.
     * SetupStream() either sets a valid stream ID
     * to be used with the other stream API calls,
     * or a helpful error message when setup fails.
     *
     * SetupStream() may fail for a variety of reasons
     * such as invalid channel, format, or buffer configurations,
     * the stream is already open, or streaming not supported.
     *
     * @param [out] streamID the configured stream identifier
     * @param config the requested stream configuration
     * @return an error message or empty string on success
     */
    virtual std::string SetupStream(size_t &streamID, const StreamConfig &config);

    /*!
     * Close an open stream give the stream ID.
     * This invalidates the stream ID
     * @param streamID the configured stream identifier
     */
    virtual void CloseStream(const size_t streamID);

    /*!
     * Get the transfer size per buffer in samples.
     * Use the stream size buffers when possible
     * with the ReadStream()/WriteStream() API
     * to match up with the link transfer size.
     * Consider this an optimization.
     * @param streamID the configured stream identifier
     * @return the transfer size per buffer in samples
     */
    virtual size_t GetStreamSize(const size_t streamID);

    /*!
     * Control streaming activation, bursts, and timing.
     * While SetupStream() sets up and allocates resources,
     * ControlStream() is resonsible for dis/enabling the
     * stream and providing advanced burst and timing controls.
     *
     * - Use enable to activate/deactivate the stream.
     * - Use the metadata's optional timestamp to control stream time
     * - Use the metadata's end of burst to request stream bursts
     * - Without end of burst, the burstSize affects continuous streaming
     *
     * @param streamID the stream index number
     * @param enable true to enable streaming, false to halt streaming
     * @param burstSize the burst size when metadata has end of burst
     * @param metadata time and burst options
     * @return true for success, otherwise false
     */
    virtual bool ControlStream(const size_t streamID, const bool enable, const size_t burstSize = 0, const StreamMetadata &metadata = StreamMetadata());

    /*!
     * Read blocking data from the stream into the specified buffer.
     *
     * @param streamID the RX stream index number
     * @param buffs an array of buffers pointers
     * @param length the number of samples per buffer
     * @param timeout_ms the timeout in milliseconds
     * @param [out] metadata optional stream metadata
     * @return the number of samples read or error code
     */
    virtual int ReadStream(const size_t streamID, void * const *buffs, const size_t length, const long timeout_ms, StreamMetadata &metadata);

    /*!
     * Write blocking data into the stream from the specified buffer.
     *
     * - The metadata timestamp corresponds to the start of the buffer.
     * - The end of burst only applies when all bytes have been written.
     *
     * @param streamID the TX stream stream number
     * @param buffs an array of buffers pointers
     * @param length the number of samples per buffer
     * @param timeout_ms the timeout in milliseconds
     * @param metadata optional stream metadata
     * @return the number of samples written or error code
     */
    virtual int WriteStream(const size_t streamID, const void * const *buffs, const size_t length, const long timeout_ms, const StreamMetadata &metadata);

    /***********************************************************************
     * Programming API
     **********************************************************************/

    /** @brief Uploads program to selected device
        @param buffer binary program data
        @param length buffer length
        @param programmingMode to RAM, to FLASH, to EEPROM, etc..
        @param index target device number
        @param callback callback for progress reporting or early termination
        @return the operation success state

        Can be used to program MCU, FPGA, write external on board memory.
        This could be a quite long operation, use callback to get progress info or to terminate early
    */
    virtual OperationStatus ProgramWrite(const char *buffer, const size_t length, const int programmingMode, const int index, ProgrammingCallback callback = 0);

    /**	@brief Reads current program from selected device
        @param destination buffer for binary program data
        @param length buffer length to read
        @param index target device number
        @param callback callback for progress reporting or early termination
        @return the operation success state
    */
    virtual OperationStatus ProgramRead(char *buffer, const size_t length, const int index, ProgrammingCallback callback = 0);

    /***********************************************************************
     * GPIO API
     **********************************************************************/

    /**	@brief Writes GPIO values to device
    @param source buffer for GPIO values LSB first, each bit sets GPIO state
    @param bufLength buffer length
    @return the operation success state
    */
    virtual OperationStatus GPIOWrite(const uint8_t *buffer, const size_t bufLength);

    /**	@brief Reads GPIO values from device
    @param destination buffer for GPIO values LSB first, each bit represent GPIO state
    @param bufLength buffer length to read
    @return the operation success state
    */
    virtual OperationStatus GPIORead(uint8_t *buffer, const size_t bufLength);

    /***********************************************************************
     * Register API
     **********************************************************************/

    /**	@brief Bulk write device registers.
     * WriteRegisters() writes multiple registers and supports 32-bit addresses and data.
     * WriteRegisters() can support multiple devices by dispatching based on the address.
     * @param addrs an array of 32-bit register addresses
     * @param data an array of 32-bit register data
     * @param size the number of entries in addrs and data
     * @return the operation success state
     */
    virtual OperationStatus WriteRegisters(const uint32_t *addrs, const uint32_t *data, const size_t size);

    //! Write a single device register
    OperationStatus WriteRegister(const uint32_t addr, const uint32_t data);

    /**	@brief Bulk read device registers.
     * ReadRegisters() writes multiple registers and supports 32-bit addresses and data.
     * ReadRegisters() can support multiple devices by dispatching based on the address.
     * @param addrs an array of 32-bit register addresses
     * @param [out] data an array of 32-bit register data
     * @param size the number of entries in addrs and data
     * @return the operation success state
     */
    virtual OperationStatus ReadRegisters(const uint32_t *addrs, uint32_t *data, const size_t size);

    //! Read a single device register
    template <typename ReadType>
    OperationStatus ReadRegister(const uint32_t addr, ReadType &data);

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
    virtual OperationStatus CustomParameterWrite(const uint8_t *ids, const double *values, const int count, const std::string* units);

    /** @brief Returns value of custom on board control
	@param ids indexes of controls to read
	@param values retrieved control values
	@param count number of values to read
	@param units (optional) when not null returns value units (e.g V, A, Ohm, C... )
	@return the operation success state
    */
    virtual OperationStatus CustomParameterRead(const uint8_t *ids, double *values, const int count, std::string* units);

    /***********************************************************************
     * !!! Below is the old IConnection Streaming API
     * It remains here to enable compiling until its replaced
     **********************************************************************/

	virtual int BeginDataReading(char *buffer, long length){ return -1; };
	virtual int WaitForReading(int contextHandle, unsigned int timeout_ms){ return 0;};
	virtual int FinishDataReading(char *buffer, long &length, int contextHandle){ return 0;}
	virtual void AbortReading(){};
    virtual int ReadDataBlocking(char *buffer, long &length, int timeout_ms){ return 0; }

	virtual int BeginDataSending(const char *buffer, long length){ return -1; };
	virtual int WaitForSending(int contextHandle, unsigned int timeout_ms){ return 0;};
	virtual int FinishDataSending(const char *buffer, long &length, int contextHandle){ return 0;}
	virtual void AbortSending(){};

    /** @brief Sets callback function which gets called each time data is sent or received
    */
    void SetDataLogCallback(std::function<void(bool, const unsigned char*, const unsigned int)> callback);

protected:
    std::function<void(bool, const unsigned char*, const unsigned int)> callback_logData;
    bool mSystemBigEndian;

private:
    friend class ConnectionRegistry;
    ConnectionHandle _handle;
};

template <typename ReadType>
OperationStatus IConnection::ReadRegister(const uint32_t addr, ReadType &data)
{
    uint32_t data32 = 0;
    OperationStatus st = this->ReadRegisters(&addr, &data32, 1);
    data = ReadType(data32);
    return st;
}

#endif

