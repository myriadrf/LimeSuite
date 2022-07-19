#ifndef LIME_SDRDevice_H
#define LIME_SDRDevice_H

#include "DeviceHandle.h"
#include "IComms.h"
#include "LMS7002M_parameters.h"

#include <vector>
#include <unordered_map>
#include <functional>
#include <string.h>

namespace lime {

class LIME_API SDRDevice : public IComms
{
  public:
    static constexpr uint8_t MAX_CHANNEL_COUNT = 16;

    enum Dir
    {
        Rx = false,
        Tx = true
    };

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

    struct Range
    {
        Range(double a = 0, double b = 0) : min(a), max(b){};
        double min;
        double max;
    };

    typedef std::unordered_map<std::string, uint32_t> SlaveNameIds_t;

    struct Descriptor
    {
        SlaveNameIds_t spiSlaveIds; // names and SPI bus numbers of internal chips
    };

    struct StreamConfig
    {
        enum DataFormat
        {
            FMT_INT16,
            FMT_INT12,
            FMT_FLOAT32,
        };

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

        // TODO: callback for drops and errors
    };

    struct StreamMeta
    {
        uint64_t timestamp;
        bool useTimestamp;
        bool flushPartialPacket;
    };

    struct StreamStats
    {
        uint64_t timestamp;
        float rxFIFO_filled;
        float txFIFO_filled;
        float rxDataRate_Bps;
        float txDataRate_Bps;
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
        double rxSampleRate;
        double txSampleRate;
        double rxGain;
        double txGain;
        uint8_t rxPath;
        uint8_t txPath;
        double rxLPF;
        double txLPF;
        bool rxEnabled;
        bool txEnabled;
        bool rxCalibrate;
        bool txCalibrate;
        // GFIR
        // TestSignal
        // NCOs
    };

    struct SDRConfig
    {
        SDRConfig() : referenceClockFreq(0){};
        double referenceClockFreq;
        ChannelConfig channel[MAX_CHANNEL_COUNT];
        // Loopback setup?
    };

public:
    SDRDevice();
    virtual ~SDRDevice();

    virtual void Configure(const SDRConfig config) = 0;

    /// Returns SPI slave names and chip select IDs for use with SDRDevice::SPI()
    virtual const Descriptor &GetDescriptor() const = 0;

    //! Get the connection handle that was used to create this connection
    const DeviceHandle &GetHandle(void) const;

    virtual int Init() = 0;
    virtual uint8_t GetNumChannels() const = 0;
    virtual void Reset() = 0;
    virtual int EnableChannel(SDRDevice::Dir dir, uint8_t channel, bool enabled) = 0;
    // virtual int SetRate(double f_MHz, int oversample);
    // virtual int SetRate(bool tx, double f_MHz, unsigned oversample = 0);
    // virtual int SetRate(unsigned ch, double rxRate, double txRate, unsigned oversample = 0);

    virtual double GetRate(SDRDevice::Dir dir, uint8_t channel) const = 0;
    virtual SDRDevice::Range GetRateRange(SDRDevice::Dir dir, uint8_t channel) const = 0;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) = 0;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) = 0;

    virtual SDRDevice::Range GetFrequencyRange(SDRDevice::Dir dir) const = 0;
    virtual std::vector<std::string> GetPathNames(SDRDevice::Dir dir, uint8_t channel) const = 0;
    virtual uint8_t GetPath(SDRDevice::Dir dir, uint8_t channel) const = 0;

    virtual void Synchronize(bool toChip) = 0;
    virtual void EnableCache(bool enable) = 0;

    virtual uint16_t ReadParam(const LMS7Parameter &param, uint8_t channel = 0,
                               bool forceReadFromChip = false) const = 0;
    virtual int WriteParam(const LMS7Parameter &param, uint16_t val, uint8_t channel = 0) = 0;

    virtual int ReadLMSReg(uint16_t address, int ind = 0);
    virtual void WriteLMSReg(uint16_t address, uint16_t val, int ind = 0);
    virtual int ReadFPGAReg(uint16_t address);
    virtual void WriteFPGAReg(uint16_t address, uint16_t val);

    virtual double GetTemperature(uint8_t id) = 0;

    /// Get information about a device for displaying helpful information or for making device-specific decisions.
    virtual SDRDevice::DeviceInfo GetDeviceInfo(void);

    virtual int StreamStart(const StreamConfig &config);
    virtual void StreamStop();

    virtual int StreamRx(uint8_t channel, void **samples, uint32_t count, StreamMeta *meta);
    virtual int StreamTx(uint8_t channel, const void **samples, uint32_t count,
                         const StreamMeta *meta);
    virtual void StreamStatus(uint8_t channel, SDRDevice::StreamStats &status);

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
    virtual void SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO,
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

    /** @brief Sets callback function which gets called each time data is sent or received
    */
    void SetDataLogCallback(std::function<void(bool, const unsigned char*, const unsigned int)> callback);

    virtual void *GetInternalChip(uint32_t index);
    virtual void SetFPGAInterfaceFreq(uint8_t interp, uint8_t dec, double txPhase,
                                      double rxPhase) = 0;

  protected:
    std::function<void(bool, const unsigned char *, const unsigned int)> callback_logData;

  private:
    friend class DeviceRegistry;
    DeviceHandle _handle;
};

}
#endif

