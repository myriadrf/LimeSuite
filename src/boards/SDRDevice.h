#ifndef LIME_SDRDevice_H
#define LIME_SDRDevice_H

#include "DeviceHandle.h"
#include "IComms.h"
#include "LMS7002M_parameters.h"
#include "PacketsFIFO.h"
#include "dataTypes.h"

#include <vector>
#include <unordered_map>
#include <functional>
#include <string.h>
#include "commonTypes.h"

namespace lime {

class LMS7002M;
class TRXLooper;
class FPGA;

/// SDRDevice can have multiple modules (RF chips), that can operate independently

class LIME_API SDRDevice : public IComms
{
  public:
    static constexpr uint8_t MAX_CHANNEL_COUNT = 16;
    static constexpr uint8_t MAX_RFSOC_COUNT = 16;

    typedef void(*DataCallbackType)(bool, const uint8_t*, const uint32_t);

    enum ClockID
    {
        CLK_REFERENCE = 0,
        CLK_SXR = 1, ///RX LO clock
        CLK_SXT = 2, ///TX LO clock
        CLK_CGEN = 3,
        ///RXTSP reference clock (read-only)
        CLK_RXTSP = 4,
        ///TXTSP reference clock (read-only)
        CLK_TXTSP = 5
    };

    typedef std::unordered_map<std::string, uint32_t> SlaveNameIds_t;

    struct RFSOCDescripion
    {
        uint8_t channelCount;
        std::vector<std::string> rxPathNames;
        std::vector<std::string> txPathNames;
    };

    // General information about device internals, static capabilities
    struct Descriptor
    {
        std::string name; /// The displayable name for the device
        /*! The displayable name for the expansion card
        * Ex: if the RFIC is on a daughter-card
        */
        std::string expansionName;
        std::string firmwareVersion; /// The firmware version as a string
        std::string gatewareVersion; /// Gateware version as a string
        std::string gatewareRevision; /// Gateware revision as a string
        std::string gatewareTargetBoard; /// Which board should use this gateware
        std::string hardwareVersion; /// The hardware version as a string
        std::string protocolVersion; /// The protocol version as a string
        uint64_t serialNumber; /// A unique board serial number

        SlaveNameIds_t spiSlaveIds; // names and SPI bus numbers of internal chips
        //uint8_t rfSOC_count; // how many independent RF chips are on board
        std::vector<RFSOCDescripion> rfSOC;
    };

    struct StreamStats
    {
        StreamStats() {
            memset(this, 0, sizeof(StreamStats));
        }
        uint64_t timestamp;
        float FIFO_filled;
        float dataRate_Bps;
        float txDataRate_Bps;
        uint32_t overrun;
        uint32_t underrun;
        uint32_t loss;
        uint32_t late;
        bool isTx;
    };

    // channels order and data transmission formats setup
    struct StreamConfig
    {
        typedef bool (*StatusCallbackFunc)(const StreamStats*, void*);
        enum DataFormat
        {
            I16,
            I12,
            F32,
        };

        StreamConfig(){
            memset(this, 0, sizeof(StreamConfig));
        }

        uint8_t rxCount;
        uint8_t rxChannels[MAX_CHANNEL_COUNT];
        uint8_t txCount;
        uint8_t txChannels[MAX_CHANNEL_COUNT];

        DataFormat format;     // samples format used for Read/Write functions
        DataFormat linkFormat; // samples format used in transport layer Host<->FPGA

        /// memory size to allocate for each channel buffering
        /// Default: 0 - allow to decide internally
        uint32_t bufferSize;
        bool alignPhase; // attempt to do phases alignment between paired channels

        StatusCallbackFunc statusCallback;
        void* userData; // will be supplied to statusCallback
        // TODO: callback for drops and errors
    };

    struct StreamMeta
    {
        int64_t timestamp;
        bool useTimestamp;
        bool flushPartialPacket;
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
        DeviceInfo(void) : boardSerialNumber(0){};

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
    };

    struct ChannelConfig
    {
        ChannelConfig()
        {
            memset(this, 0, sizeof(ChannelConfig));
        }
        double rxCenterFrequency;
        double txCenterFrequency;
        double rxNCOoffset;
        double txNCOoffset;
        double rxSampleRate;
        double txSampleRate;
        double rxGain;
        double txGain;
        uint8_t rxPath;
        uint8_t txPath;
        double rxLPF;
        double txLPF;
        uint8_t rxOversample;
        uint8_t txOversample;
        bool rxEnabled;
        bool txEnabled;
        bool rxCalibrate;
        bool txCalibrate;
        bool rxTestSignal;
        bool txTestSignal;

        // TODO:
        // GFIR
        // TestSignal
        // NCOs
    };

    struct SDRConfig
    {
        SDRConfig() : referenceClockFreq(0), skipDefaults(false) {};
        double referenceClockFreq;
        ChannelConfig channel[MAX_CHANNEL_COUNT];
        // Loopback setup?
        bool skipDefaults; // skip default values initialization and write on top of current config
    };

public:
    SDRDevice();
    virtual ~SDRDevice();

    virtual void Configure(const SDRConfig config, uint8_t moduleIndex) = 0;

    /// Returns SPI slave names and chip select IDs for use with SDRDevice::SPI()
    virtual const Descriptor &GetDescriptor() const = 0;

    //! Get the connection handle that was used to create this connection
    const DeviceHandle &GetHandle(void) const;

    virtual int Init() = 0;
    virtual void Reset();

    //virtual double GetRate(Dir dir, uint8_t channel) const = 0;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) = 0;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) = 0;

    virtual void Synchronize(bool toChip);
    virtual void EnableCache(bool enable);

    virtual int StreamSetup(const StreamConfig &config, uint8_t moduleIndex) = 0;
    virtual void StreamStart(uint8_t moduleIndex);
    virtual void StreamStop(uint8_t moduleIndex);

    virtual int StreamRx(uint8_t channel, void **samples, uint32_t count, StreamMeta *meta);
    virtual int StreamTx(uint8_t channel, const void **samples, uint32_t count,
                         const StreamMeta *meta);
    virtual void StreamStatus(uint8_t channel, SDRDevice::StreamStats &status) = 0;

    /*!
     * @brief Bulk SPI write/read transaction.
     *
     * The transactSPI function is capable of bulk writes and bulk reads
     * of SPI registers in an arbitrary IC (up to 32-bits per transaction).
     *
     * MISO (Master input Slave output) may be NULL to indicate a write-only operation,
     * the underlying implementation may be able to optimize out the readback.
     *
     * @param spiBusAddress SPI target chip address
     * @param MOSI Master output Slave input, data to write
     * @param [out] MISO Master input Slave output, buffer to fill with read data
     * @param count Number of SPI transactions
     * @return 0-success
     */
    virtual void SPI(uint32_t spiBusAddress, const uint32_t *MOSI, uint32_t *MISO,
                     size_t count) override;

    /*!
     * Write to an available I2C slave.
     * @param address I2C slave address
     * @param data output buffer
     * @param length output data length
     * @return 0-success
     */
    virtual int I2CWrite(int address, const uint8_t *data, size_t length) override;

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
    virtual int I2CRead(int addr, uint8_t *dest, size_t length) override;

    /***********************************************************************
     * GPIO API
     **********************************************************************/

    /**    @brief Writes GPIO values to device
    @param buffer for source of GPIO values LSB first, each bit sets GPIO state
    @param bufLength buffer length
    @return the operation success state
    */
    virtual int GPIOWrite(const uint8_t *buffer, const size_t bufLength);

    /**    @brief Reads GPIO values from device
    @param buffer destination for GPIO values LSB first, each bit represent GPIO state
    @param bufLength buffer length to read
    @return the operation success state
    */
    virtual int GPIORead(uint8_t *buffer, const size_t bufLength);

    /**    @brief Write GPIO direction control values to device.
    @param buffer with GPIO direction configuration (0 input, 1 output)
    @param bufLength buffer length
    @return the operation success state
    */
    virtual int GPIODirWrite(const uint8_t *buffer, const size_t bufLength);

    /**    @brief Read GPIO direction configuration from device
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

    /// @brief Sets callback function which gets called each time data is sent or received
    void SetDataLogCallback(DataCallbackType callback);

    virtual void *GetInternalChip(uint32_t index);
    virtual void SetFPGAInterfaceFreq(uint8_t interp, uint8_t dec, double txPhase,
                                      double rxPhase) = 0;

  protected:
    struct PartialPacket
    {
        PartialPacket() : timestamp(0), start(0), end(0){};
        lime::complex16_t chA[4096];
        lime::complex16_t chB[4096];
        uint64_t timestamp;
        uint16_t start;
        uint16_t end;
    };
    PartialPacket rxCrumbs[3]; // TODO: make members
    PartialPacket txCrumbs[3]; // TODO: make members

    DataCallbackType mCallback_logData;
    std::vector<LMS7002M*> mLMSChips;
    std::vector<TRXLooper*> mStreamers;

    std::vector< PacketsFIFO<FPGA_DataPacket> *> rxFIFOs;
    std::vector< PacketsFIFO<FPGA_DataPacket> *> txFIFOs;
    StreamConfig mStreamConfig;
    FPGA *mFPGA;

  private:
    friend class DeviceRegistry;
    DeviceHandle _handle;
};

}
#endif

