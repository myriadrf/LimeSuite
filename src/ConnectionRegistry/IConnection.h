/**
    @file IConnection.h
    @author Lime Microsystems
    @brief Interface class for connection types
*/

#ifndef ICONNECTION_H
#define ICONNECTION_H

#include <LimeSuiteConfig.h>
#include <ConnectionHandle.h>
#include <string>
#include <vector>
#include <cstring> //memset
#include <functional>
#include <stdint.h>

namespace lime{

/*!
 * Information about the set of available hardware on a device.
 * This includes available ICs, streamers, and version info.
 *
 * This structure provides SPI slave addresses for one or more RFICs
 * and slave addresses or I2C addresses for commonly supported ICs.
 * A -1 for an address number indicates that it is not available.
 */
struct LIME_API DeviceInfo
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

    //! Gateware version as a string
    std::string gatewareVersion;
    //! Gateware revision as a string
    std::string gatewareRevision;
    //! Which board should use this gateware
    std::string gatewareTargetBoard;

    //! The hardware version as a string
    std::string hardwareVersion;

    //! The protocol version as a string
    std::string protocolVersion;

    //! A unique board serial number
    uint64_t boardSerialNumber;

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
struct LIME_API StreamMetadata
{
    StreamMetadata(void);

    /*!
     * The timestamp in clock units
     * Set to 0 when the timestamp is not applicable.
     * See GetHardwareTimestampRate() for tick rate.
     */
    uint64_t timestamp;

    //! True to indicate that the timestamp is valid
    bool hasTimestamp;

    /*!
     * True to indicate the end of a stream buffer.
     * When false, subsequent calls continue the stream.
     */
    bool endOfBurst;

    /*!
     * True to indicate that the timestamp was late.
     * Used in stream status reporting.
     */
    bool lateTimestamp;

    /*!
     * True to indicate that a packet was dropped
     * perhaps in a receiver overflow event.
     */
    bool packetDropped;
};

/*!
 * The stream config structure is used with the SetupStream() API.
 */
struct LIME_API StreamConfig
{
    StreamConfig(void);

    //! True for transmit stream, false for receive
    bool isTx;

    /*!
     * A list of channels to use.
     *  - Example ChA on RFIC0: [0]
     *  - Example MIMO on RFIC0: [0, 1]
     *  - Example MIMO on RFIC1: [2, 3]
     */
    uint8_t channelID;

    float performanceLatency;

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
class LIME_API IConnection
{
public:

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
    virtual int TransactSPI(const int addr, const uint32_t *writeData, uint32_t *readData, const size_t size);

    virtual int WriteLMS7002MSPI(const uint32_t *writeData, size_t size,unsigned periphID = 0)=0;
    virtual int ReadLMS7002MSPI(const uint32_t *writeData, uint32_t *readData, size_t size, unsigned periphID = 0)=0;

    /*!
     * Write to an available I2C slave.
     * @param addr the address of the slave
     * @param data an array of bytes write out
     * @return the transaction success state
     */
    virtual int WriteI2C(const int addr, const std::string &data);

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
    virtual int ReadI2C(const int addr, const size_t numBytes, std::string &data);

    /***********************************************************************
     * LMS7002M Driver callbacks
     **********************************************************************/

    /*!
     * Perform reset sequence on the device.
     * Typically this will reset the RFIC using a GPIO,
     * and possibly other ICs located on the device.
     */
    virtual int DeviceReset(int ind=0);

    /*!
     * Called by the LMS7002M driver after potential band-selection changes.
     * Implementations may have additional external bands to switch via GPIO.
     * @param channel the channel index number (Ex: 0 and 1 for RFIC0)
     * @param trfBand the transmit frontend band selection: 1 or 2
     * @param rfePath the receive frontend path selection: LMS7002M::PathRFE
     */
    virtual void UpdateExternalBandSelect(const size_t channel, const int trfBand, const int rfePath);

    /*!
     * Called by the LMS7002M driver after TSP/DIQ rate changes.
     * Implementations may use these rate updates to configure
     * internal parameters or perhaps PLL circuitry in a FPGA.
     * @param channel the channel index number (Ex: 0 and 1 for RFIC0)
     * @param txRate the baseband transmit data rate in Hz (BBIC to RFIC)
     * @param rxRate the baseband receive data rate in Hz (RFIC to BBIC)
     */
    virtual int UpdateExternalDataRate(const size_t channel, const double txRate, const double rxRate);
    virtual int UpdateExternalDataRate(const size_t channel, const double txRate, const double rxRate, const double txPhase, const double rxPhase);

    /*!
     * Called by the LMS7002M driver before the board begins self-calibration.
     * Implementations should perform the necessary steps to power down
     * external amplifiers and disable any BBIC interfaces which
     * may be affected by the change in BBIC interface clock rate.
     * @param channel the channel index number (Ex: 0 and 1 for RFIC0)
     */
    virtual void EnterSelfCalibration(const size_t channel);

    /*!
     * Called by the LMS7002M driver after the board completes self-calibration.
     * Implementations should restore the board to the pre-calibration state.
     * @param channel the channel index number (Ex: 0 and 1 for RFIC0)
     */
    virtual void ExitSelfCalibration(const size_t channel);

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
    virtual int SetReferenceClockRate(const double rate);

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
    virtual int SetTxReferenceClockRate(const double rate);

    /***********************************************************************
     * Timestamp API
     **********************************************************************/

    /*!
     * Get the current timestamp in clock units.
     */
    virtual uint64_t GetHardwareTimestamp(void);

    /*!
     * Set the current timestamp in clock units.
     */
    virtual void SetHardwareTimestamp(const uint64_t now);

    /*!
     * Get the rate of the current timestamp in ticks per second.
     * This call may be used often and should return a cached value.
     */
    virtual double GetHardwareTimestampRate(void);

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
     * @return 0-success, other failure
     */
    virtual int SetupStream(size_t &streamID, const StreamConfig &config);

    /*!
     * Close an open stream give the stream ID.
     * This invalidates the stream ID
     * @param streamID the configured stream identifier
     * @return 0-success, other failure
     */
    virtual int CloseStream(const size_t streamID);

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
     * ControlStream() is resonsible for dis/enabling the stream
     *
     * - Use enable to activate/deactivate the stream.
     *
     * @param streamID the stream index number
     * @param enable true to enable streaming, false to halt streaming
     * @return true for success, otherwise false
     */
    virtual int ControlStream(const size_t streamID, const bool enable);

    /*!
     * Read blocking data from the stream into the specified buffer.
     *
     * @param streamID the RX stream index number
     * @param buffer an array of buffers pointers
     * @param length the number of samples per buffer
     * @param timeout_ms the timeout in milliseconds
     * @param metadata [out] optional stream metadata
     * @return the number of samples read or error code
     */
    virtual int ReadStream(const size_t streamID, void* buffer, const size_t length, const long timeout_ms, StreamMetadata &metadata);

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
    virtual int WriteStream(const size_t streamID, const void *buffs, const size_t length, const long timeout_ms, const StreamMetadata &metadata);

    /*!
     * Read reported stream status events such as
     * overflow, underflow, late transmit, end of burst.
     *
     * @param streamID the RX stream index number
     * @param timeout_ms the timeout in milliseconds
     * @param [out] metadata stream status metadata
     * @return 0 on success, -1 for timeout no data
     */
    virtual int ReadStreamStatus(const size_t streamID, const long timeout_ms, StreamMetadata &metadata);

    /**	@brief Uploads waveform to on board memory for later use
    @param samples multiple channel samples data
    @param chCount number of waveform channels
    @param sample_count number of samples in each channel
    @param format waveform data format
    @param epIndex endpoint identifier
    */
    virtual int UploadWFM(const void* const* samples, uint8_t chCount, size_t sample_count, StreamConfig::StreamDataFormat format, int epIndex);

    /**	@brief Read raw stream data from device streaming port
    @param buffer       read buffer pointer
    @param length       number of bytes to read
    @param epIndex      endpoint identifier?
    @param timeout_ms   timeout in milliseconds
    */
    virtual int ReadRawStreamData(char* buffer, unsigned length, int epIndex, int timeout_ms = 100);

    /***********************************************************************
     * Programming API
     **********************************************************************/

    /*!
     * Callback from programming processes
     * @param bsent number of bytes transferred
     * @param btotal total number of bytes to send
     * @param progressMsg string describing current progress state
     * @return 0-continue programming, 1-abort operation
     */
    typedef std::function<bool(int bsent, int btotal, const char* progressMsg)> ProgrammingCallback;

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
    virtual int ProgramWrite(const char *buffer, const size_t length, const int programmingMode, const int index, ProgrammingCallback callback = 0);

    /**	@brief Reads current program from selected device
        @param buffer destination for binary program data
        @param length buffer length to read
        @param index target device number
        @param callback callback for progress reporting or early termination
        @return the operation success state
    */
    virtual int ProgramRead(char *buffer, const size_t length, const int index, ProgrammingCallback callback = 0);

    enum MCU_PROG_MODE
    {
        RESET = 0,
        EEPROM_AND_SRAM,
        SRAM,
        BOOT_SRAM_FROM_EEPROM
    };
    /** @brief Uploads program to MCU
        @param buffer binary program data
        @param length buffer length
        @param mode MCU programing mode RESET, EEPROM_AND_SRAM, SRAM, BOOT_SRAM_FROM_EEPROM
        @param callback callback for progress reporting or early termination
        @return 0-success

        This could be a quite long operation, use callback to get progress info or to terminate early
    */
    virtual int ProgramMCU(const uint8_t *buffer, const size_t length, const MCU_PROG_MODE mode, ProgrammingCallback callback = 0)=0;

    /*!
     * Download up-to-date images files and flash the board when applicable.
     *
     * Program update is an implementation specific function that may handle
     * several different use cases depending upon what options are applicable:
     *
     * - If a board relies on certain firmware and gateware images,
     *   these images will be downloaded if not present on the system.
     *
     * - If the board has a programmable flash for firmware and gateware,
     *   then up-to-date images will be written to the flash on the board.
     *
     * @param download true to enable downloading missing images
     * @param callback callback for progress reporting or early termination
     * @return 0-success
     */
    virtual int ProgramUpdate(const bool download = true, ProgrammingCallback callback = 0);

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
    virtual int WriteRegisters(const uint32_t *addrs, const uint32_t *data, const size_t size);

    //! Write a single device register
    int WriteRegister(const uint32_t addr, const uint32_t data);

    /**	@brief Bulk read device registers.
     * ReadRegisters() writes multiple registers and supports 32-bit addresses and data.
     * ReadRegisters() can support multiple devices by dispatching based on the address.
     * @param addrs an array of 32-bit register addresses
     * @param [out] data an array of 32-bit register data
     * @param size the number of entries in addrs and data
     * @return the operation success state
     */
    virtual int ReadRegisters(const uint32_t *addrs, uint32_t *data, const size_t size);

    //! Read a single device register
    template <typename ReadType>
    int ReadRegister(const uint32_t addr, ReadType &data);

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
    virtual int CustomParameterWrite(const uint8_t *ids, const double *values, const size_t count, const std::string* units);

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
    bool mSystemBigEndian;

private:
    friend class ConnectionRegistry;
    ConnectionHandle _handle;
};

template <typename ReadType>
int IConnection::ReadRegister(const uint32_t addr, ReadType &data)
{
    uint32_t data32 = 0;
    int st = this->ReadRegisters(&addr, &data32, 1);
    data = ReadType(data32);
    return st;
}

class LIME_API IStreamChannel
{
public:
    struct Info
    {
        float sampleRate;
        int fifoSize;
        int fifoItemsCount;
        int overrun;
        int underrun;
        bool active;
        float linkRate;
        int droppedPackets;
        uint64_t timestamp;
    };
    IStreamChannel(){};
    IStreamChannel(IConnection* port, StreamConfig conf){};
    virtual int Start() = 0;
    virtual int Stop() = 0;
    virtual ~IStreamChannel(){};

    struct Metadata
    {
        enum
        {
            SYNC_TIMESTAMP = 1,
            END_BURST = 2,
            OVERWRITE_OLD = 4,
        };
        uint64_t timestamp;
        uint32_t flags;
    };

    /** @brief Returns samples from receiver FIFO
        @param samples destination array of data type used in SetupStream()
        @param count number of samples to read
        @param metadata [in,out?] information about the transfer
	@param timeout_ms return error if operation does not complete in timeout_ms (milliseconds)
        @return number of samples received
    */
    virtual int Read(void* samples, const uint32_t count, Metadata* metadata, const int32_t timeout_ms = 100) = 0;

    /** @brief Writes samples to transmitter FIFO
        @param samples source array of data type used in SetupStream()
        @param count number of samples to write
	@param metadata [in,out?] information about the transfer
	@param timeout_ms return error if operation does not complete in timeout_ms (milliseconds)
        @return number of samples transmitted
    */
    virtual int Write(const void* samples, const uint32_t count, const Metadata* metadata, const int32_t timeout_ms = 100) = 0;

    virtual Info GetInfo() = 0;
};

}
#endif

