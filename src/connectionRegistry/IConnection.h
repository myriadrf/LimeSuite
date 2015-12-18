/**
    @file IConnection.h
    @author Lime Microsystems
    @brief Interface class for connection types
*/

#ifndef ICONNECTION_H
#define ICONNECTION_H

#include <string>
#include <vector>
#include <mutex>
#include <cstring> //memset
#include <functional>

//! REMOVE ME -- part of the old API
#include "../lms7002m/lms7002_defines.h"

//! REMOVE ME -- part of the old API
struct LMSinfo
{
    eLMS_DEV device;
    eEXP_BOARD expansion;
    int firmware;
    int hardware;
    int protocol;
};

enum OperationStatus
{
    SUCCESS = 0,
    FAILED,
    UNSUPPORTED,
};

using namespace std;

/*!
 * Information about a particular RFIC on an IConnection.
 * RFICInfo associates streaming channels and SPI slaves.
 */
struct RFICInfo
{

    RFICInfo(void);

    /*!
     * The SPI index number used to access the lime RFIC.
     * This index will be used in the spi access functions.
     */
    int spiIndexRFIC;

    /*!
     * The SPI index number used to access the Si5351
     * found on some development boards. -1 when not present.
     */
    int spiIndexSi5351;

    /*!
     * The channel number used in the read stream API.
     * Set to -1 when RX streaming not available.
     */
    int rxChannel;

    /*!
     * The channel number used in the write stream API.
     * Set to -1 when TX streaming not available.
     */
    int txChannel;
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
     * Set to -1 when the timestamp is not applicable.
     */
    long long timestamp;

    /*!
     * True to indicate the end of a stream buffer.
     * When false, subsequent calls continue the stream.
     */
    bool endOfBurst;
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

    //! IConnection constructor
    IConnection(void);

    //! IConnection destructor
    virtual ~IConnection(void);

    /*!
     * RFIC enumeration API.
     * @return a list of RFICInfos
     */
    virtual std::vector<RFICInfo> listRFICs(void);

   /*!
    * @brief Bulk SPI write/read transaction.
    *
    * The transactSPI function is capable of bulk writes and bulk reads
    * of SPI registers in an arbitrary IC (up to 32-bits per transaction).
    *
    * The readData parameter may be NULL to indicate a write-only operation,
    * the underlying implementation may be able to optimize out the readback.
    *
    * @param index the SPI device number
    * @param writeData SPI bits to write out
    * @param [out] readData stores readback data
    * @param size the number of SPI transactions
    * @return the transaction success state
    */
    virtual OperationStatus transactSPI(const int index, const uint32_t *writeData, uint32_t *readData, const size_t size);

    /*!
     * The RX stream control call configures a channel to
     * stream at a particular time, requests burst,
     * or to start or stop continuous streaming.
     *
     * - Use the metadata's optional timestamp to control stream time
     * - Use the metadata's end of burst to request stream bursts
     * - Without end of burst, the burstSize affects continuous streaming
     *
     * @param channel the RX channel index number
     * @param burstSize the burst size when metadata has end of burst
     * @param metadata time and burst options
     * @return true for success, otherwise false
     */
    virtual bool rxStreamControl(const int channel, const size_t burstSize, const StreamMetadata &metadata);

    /*!
     * Read blocking data from the stream into the specified buffer.
     *
     * @param channel the RX channel index number
     * @param buffer the buffer to be filled
     * @param length the number of bytes in the buffer
     * @param timeout_ms the timeout in milliseconds
     * @param [out] metadata optional stream metadata
     * @return the number of bytes read or error code
     */
    virtual int readStream(const int channel, char *buffer, const size_t length, const long timeout_ms, StreamMetadata &metadata);

    /*!
     * Write blocking data into the stream from the specified buffer.
     *
     * - The metadata timestamp corresponds to the start of the buffer.
     * - The end of burst only applies when all bytes have been written.
     *
     * @param channel the TX channel index number
     * @param buffer the buffer to be read from
     * @param length the number of bytes in the buffer
     * @param timeout_ms the timeout in milliseconds
     * @param metadata optional stream metadata
     * @return the number of bytes written or error code
     */
    virtual int writeStream(const int channel, const char *buffer, const size_t length, const long timeout_ms, const StreamMetadata &metadata);

/***********************************************************************
 * !!! Below is the old IConnection and LMScomms API
 * It remains here to enable compiling until its replaced
 **********************************************************************/

    /// Supported connection types.
    enum eConnectionType
    {
        CONNECTION_UNDEFINED = -1,
        COM_PORT = 0,
        USB_PORT = 1,
        SPI_PORT = 2,
        //insert new types here
        CONNECTION_TYPES_COUNT //used only for memory allocation
    };

    enum eLMS_PROTOCOL
    {
        LMS_PROTOCOL_UNDEFINED = 0,
        LMS_PROTOCOL_DIGIC,
        LMS_PROTOCOL_LMS64C,
        LMS_PROTOCOL_NOVENA,
    };

    enum DeviceStatus
    {
        SUCCESS,
        FAILURE,
        END_POINTS_NOT_FOUND,
        CANNOT_CLAIM_INTERFACE
    };

    enum TransferStatus
    {
        TRANSFER_SUCCESS,
        TRANSFER_FAILED,
        NOT_CONNECTED
    };

    struct GenericPacket
    {   
        GenericPacket()
        {
            cmd = CMD_GET_INFO;
            status = STATUS_UNDEFINED;
        }

        eCMD_LMS cmd;
        eCMD_STATUS status;
        vector<unsigned char> outBuffer;
        vector<unsigned char> inBuffer;
    };

    struct ProtocolDIGIC
    {
        static const int pktLength = 64;
        static const int maxDataLength = 60;
        ProtocolDIGIC() : cmd(0), i2cAddr(0), blockCount(0) {};
        unsigned char cmd;
        unsigned char i2cAddr;
        unsigned char blockCount;
        unsigned char reserved;
        unsigned char data[maxDataLength];
    };

    struct ProtocolLMS64C
    {
        static const int pktLength = 64;
        static const int maxDataLength = 56;
        ProtocolLMS64C() :cmd(0),status(STATUS_UNDEFINED),blockCount(0)
        {
            memset(reserved, 0, 5);
        };
        unsigned char cmd;
        unsigned char status;
        unsigned char blockCount;
        unsigned char reserved[5];
        unsigned char data[maxDataLength];
    };

    struct ProtocolNovena
    {
        static const int pktLength = 128;
        static const int maxDataLength = 128;
        ProtocolNovena() :cmd(0),status(0) {};
        unsigned char cmd;
        unsigned char status;
        unsigned char blockCount;
        unsigned char data[maxDataLength];
    };

    virtual TransferStatus TransferPacket(GenericPacket &pkt);

    LMSinfo GetInfo();

    int ReadStream(char *buffer, int length, unsigned int timeout_ms)
    {
        /*int handle = activeControlPort->BeginDataReading(buffer, length);
        activeControlPort->WaitForReading(handle, timeout_ms);
            long received = length;
            activeControlPort->FinishDataReading(buffer, received, handle);
            return received;
        */
        long len = length;
        int status = this->ReadDataBlocking(buffer, len, 0);
        return len;
    }

    /***********************************************************************
     * !!! Below is the old IConnection Enumeration API
     * It remains here to enable compiling until its replaced
     **********************************************************************/

	virtual int RefreshDeviceList() = 0;
	virtual DeviceStatus Open(unsigned i) = 0;
	virtual void Close() = 0;
	virtual bool IsOpen() = 0;
	virtual int GetOpenedIndex() = 0;

	virtual int Write(const unsigned char *buffer, int length, int timeout_ms = 0) = 0;
	virtual int Read(unsigned char *buffer, int length, int timeout_ms = 0) = 0;

	virtual std::vector<std::string> GetDeviceNames() = 0;

	virtual eConnectionType GetType() { return m_connectionType; };
	virtual bool SetParam(const char *name, const char* value) {return false;};

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
    unsigned char* PreparePacket(const GenericPacket &pkt, int &length, const eLMS_PROTOCOL protocol);
    int ParsePacket(GenericPacket &pkt, const unsigned char* buffer, const int length, const eLMS_PROTOCOL protocol);
    eConnectionType m_connectionType;
    std::function<void(bool, const unsigned char*, const unsigned int)> callback_logData;
    bool mSystemBigEndian;
    std::mutex mControlPortLock;
};

#endif

