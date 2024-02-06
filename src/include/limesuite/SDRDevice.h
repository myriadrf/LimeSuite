#ifndef LIME_SDRDevice_H
#define LIME_SDRDevice_H

#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "limesuite/config.h"
#include "limesuite/commonTypes.h"
#include "limesuite/complex.h"
#include "limesuite/GainTypes.h"
#include "limesuite/IComms.h"
#include "limesuite/MemoryDevices.h"
#include "limesuite/MemoryRegions.h"

namespace lime {

struct DeviceNode;

/// @brief Class for holding information about an SDR (Software Defined Radio) device.
/// SDRDevice can have multiple modules (RF chips), that can operate independently.
class LIME_API SDRDevice
{
  public:
    static constexpr uint8_t MAX_CHANNEL_COUNT = 16; ///< Maximum amount of channels an SDR Device can hold
    static constexpr uint8_t MAX_RFSOC_COUNT = 16; ///< Maximum amount of Radio-Frequency System-on-Chips

    /// @brief Enumerator to define the log level of a log message.
    enum class LogLevel : uint8_t { CRITICAL, ERROR, WARNING, INFO, VERBOSE, DEBUG };

    struct GainValue {
        uint16_t hardwareRegisterValue;
        float actualGainValue;
    };

    /// @brief General information about the Radio-Frequency System-on-Chip (RFSoC).
    struct RFSOCDescriptor {
        std::string name;
        uint8_t channelCount;
        std::unordered_map<TRXDir, std::vector<std::string>> pathNames;

        Range frequencyRange; ///< Deliverable frequency capabilities of the device
        Range samplingRateRange; ///< Sampling rate capabilities of the device

        std::unordered_map<TRXDir, std::unordered_map<std::string, Range>> antennaRange; ///< Antenna recommended bandwidths
        std::unordered_map<TRXDir, Range> lowPassFilterRange;

        std::unordered_map<TRXDir, std::set<eGainTypes>> gains;
        std::unordered_map<TRXDir, std::unordered_map<eGainTypes, Range>> gainRange;
        std::unordered_map<TRXDir, std::unordered_map<eGainTypes, std::vector<GainValue>>> gainValues;
    };

    /// @brief Structure for the information of a custom parameter.
    struct CustomParameter {
        std::string name; ///< The name of the custom parameter
        int32_t id; ///< The identifier of the custom parameter
        int32_t minValue; ///< The minimum possible value of the custom parameter
        int32_t maxValue; ///< The maximum possible value of the custom parameter
        bool readOnly; ///< Denotes whether this value is read only or not
    };

    /// @brief Structure for storing the information of a memory region.
    struct Region {
        int32_t address; ///< Starting address of the memory region
        int32_t size; ///< The size of the memory region
    };

    /// @brief Describes a data storage of a certain type a device holds.
    struct DataStorage {
        SDRDevice* ownerDevice; ///< Pointer to the device that actually owns the data storage
        eMemoryDevice memoryDeviceType; ///< The type of memory being described
        std::unordered_map<eMemoryRegion, Region> regions; ///< The documented memory regions of the data storage

        /// @brief Constructs a new Data Storage object
        /// @param device The device this storage belongs to.
        /// @param type The type of memory being described in this object.
        /// @param regions The memory regions this memory contains.
        DataStorage(SDRDevice* device = nullptr,
            eMemoryDevice type = eMemoryDevice::COUNT,
            std::unordered_map<eMemoryRegion, Region> regions = {})
            : ownerDevice(device)
            , memoryDeviceType(type)
            , regions(regions)
        {
        }
    };

    /// @brief General information about device internals, static capabilities.
    struct Descriptor {
        std::string name; ///< The displayable name for the device
        /*! The displayable name for the expansion card
        * Ex: if the RFIC is on a daughter-card.
        */
        std::string expansionName;
        std::string firmwareVersion; ///< The firmware version as a string
        std::string gatewareVersion; ///< Gateware version as a string
        std::string gatewareRevision; ///< Gateware revision as a string
        std::string gatewareTargetBoard; ///< Which board should use this gateware
        std::string hardwareVersion; ///< The hardware version as a string
        std::string protocolVersion; ///< The protocol version as a string
        uint64_t serialNumber{ 0 }; ///< A unique board serial number

        std::map<std::string, uint32_t> spiSlaveIds; ///< Names and SPI bus numbers of internal chips
        std::vector<RFSOCDescriptor> rfSOC; ///< Descriptors of all RFSoC devices within this device
        std::vector<CustomParameter> customParameters; ///< Descriptions of all custom parameters of this device
        /** Descriptions of all memory storage devices on this device */
        std::map<std::string, std::shared_ptr<DataStorage>> memoryDevices;
        std::shared_ptr<DeviceNode> socTree; ///< The device's subdevices tree view representation

        static const char DEVICE_NUMBER_SEPARATOR_SYMBOL; ///< The symbol used to separate the device's name from its number
        static const char PATH_SEPARATOR_SYMBOL; ///< The symbol that separates the device's name from its parent's name
    };

    /// @brief Structure for holding the statistics of a stream
    struct StreamStats {
        /// @brief Structure for storing the first in first out queue statistics
        struct FIFOStats {
            std::size_t totalCount; ///< The total amount of samples that can be in the FIFO queue.
            std::size_t usedCount; ///< The amount of samples that is currently in the FIFO queue.

            /// @brief Gets the ratio of the amount of FIFO filled up.
            /// @return The amount of FIFO filled up (0 - completely empty, 1 - completely full).
            constexpr float ratio() const { return static_cast<float>(usedCount) / totalCount; }
        };

        StreamStats() { std::memset(this, 0, sizeof(StreamStats)); }
        uint64_t timestamp; ///< The current timestamp of the stream.
        int64_t bytesTransferred; ///< The total amount of bytes transferred.
        int64_t packets; ///< The total amount of packets transferred.
        FIFOStats FIFO; ///< The status of the FIFO queue.
        float dataRate_Bps; ///< The current data transmission rate.
        uint32_t overrun; ///< The amount of packets overrun.
        uint32_t underrun; ///< The amount of packets underrun.
        uint32_t loss; ///< The amount of packets that are lost.
        uint32_t late; ///< The amount of packets that arrived late for transmitting and were dropped.
    };

    /// @brief Describes the status of a global positioning system.
    struct GPS_Lock {
        /// @brief Enumerator describing the possible status of a positioning system.
        enum class LockStatus : uint8_t { Undefined, NotAvailable, Has2D, Has3D };

        LockStatus galileo; ///< Status for the Galileo system (European system).
        LockStatus beidou; ///< Status for the BeiDou system (Chinese system).
        LockStatus glonass; ///< Status for the GLONASS system (Russian system).
        LockStatus gps; ///< Status for the GPS system (American system).
    };

    /// @brief Configuration settings for a stream.
    struct StreamConfig {
        /// @brief Extra configuration settings for a stream.
        struct Extras {
            struct PacketTransmission {
                PacketTransmission();

                uint16_t samplesInPacket; ///< The amount of samples to transfer in a single packet.
                uint32_t packetsInBatch; ///< The amount of packets to send in a single transfer.
            };

            Extras();
            bool usePoll; ///< Whether to use a polling strategy for PCIe devices.

            PacketTransmission rx; ///< Configuration of the receive transfer direction.
            PacketTransmission tx; ///< Configuration of the transmit transfer direction.

            bool negateQ; ///< Whether to negate the Q element before sending the data or not.
            bool waitPPS; ///< Start sampling from next following PPS.
        };

        /// @brief The definition of the function that gets called whenever a stream status changes.
        typedef bool (*StatusCallbackFunc)(bool isTx, const StreamStats* stats, void* userData);

        /// @brief Enumerator describing the data format.
        enum class DataFormat : uint8_t {
            I16, ///< Data is in a form of two 16-bit integers.
            I12, ///< Data is in a form of two 12-bit integers.
            F32, ///< Data is in a form of two 32-bit floating-point values.
        };

        StreamConfig();

        std::unordered_map<TRXDir, std::vector<uint8_t>> channels;

        DataFormat format; ///< Samples format used for Read/Write functions
        DataFormat linkFormat; ///< Samples format used in transport layer Host<->FPGA

        /// @brief Memory size to allocate for each channel buffering.
        /// Default: 0 - allow to decide internally.
        uint32_t bufferSize;

        /// Optional: expected sampling rate for data transfer optimizations.
        /// Default: 0 - decide internally.
        float hintSampleRate;
        bool alignPhase; ///< Attempt to do phases alignment between paired channels

        StatusCallbackFunc statusCallback; ///< Function to call on a status change.
        void* userData; ///<  Data that will be supplied to statusCallback
        // TODO: callback for drops and errors

        Extras extraConfig; ///< Extra stream configuration settings.
    };

    /// @brief The metadata of a stream packet.
    struct StreamMeta {
        /**
         * Timestamp is a value of HW counter with a tick based on sample rate.
         * In RX: time when the first sample in the returned buffer was received.
         * In TX: time when the first sample in the submitted buffer should be send.
         */
        uint64_t timestamp;

        /**
         * In RX: not used/ignored.
         * In TX: wait for the specified HW timestamp before broadcasting data over the air.
         */
        bool waitForTimestamp;

        /**
         * In RX: not used/ignored.
         * In TX: send samples to HW even if packet is not completely filled (end TX burst).
         */
        bool flushPartialPacket;
    };

    /// @brief Configuration of a single channel.
    struct ChannelConfig {
        ChannelConfig()
            : rx()
            , tx()
        {
        }

        /// @brief Configuration for a direction in a channel.
        struct Direction {
            Direction()
                : centerFrequency(0)
                , NCOoffset(0)
                , sampleRate(0)
                , lpf(0)
                , path(0)
                , oversample(0)
                , gfir()
                , enabled(false)
                , calibrate(false)
                , testSignal{ false, false, TestSignal::Divide::Div8, TestSignal::Scale::Half }
            {
            }

            /// @brief Configuration of a general finite impulse response (FIR) filter.
            struct GFIRFilter {
                bool enabled; ///< Whether the filter is enabled or not.
                double bandwidth; ///< The bandwidth of the filter.
            };

            struct TestSignal {
                enum class Divide : uint8_t {
                    Div8 = 1U,
                    Div4 = 2U,
                };

                enum class Scale : uint8_t {
                    Half = 0U,
                    Full = 1U,
                };

                bool enabled;
                bool dcMode;
                Divide divide;
                Scale scale;

                TestSignal(bool enabled = false, bool dcMode = false, Divide divide = Divide::Div8, Scale scale = Scale::Half)
                    : enabled(enabled)
                    , dcMode(dcMode)
                    , divide(divide)
                    , scale(scale)
                {
                }
            };

            double centerFrequency; ///< The center frequency of the direction of this channel.
            double NCOoffset; ///< The offset from the channel's numerically controlled oscillator (NCO).
            double sampleRate; ///< The sample rate of this direction of a channel.
            std::unordered_map<eGainTypes, double> gain; ///< The gains and their current values for this direction.
            double lpf; ///< The bandwidth of the Low Pass Filter (LPF).
            uint8_t path; ///< The antenna being used for this direction.
            uint8_t oversample; ///< The oversample ratio of this direction.
            GFIRFilter gfir; ///< The general finite impulse response (FIR) filter settings of this direction.
            bool enabled; ///< Denotes whether this direction of a channel is enabled or not.
            bool calibrate; ///< Denotes whether the device will be calibrated or not.
            TestSignal testSignal; ///< Denotes whether the signal being sent is a test signal or not.
        };

        Direction& GetDirection(TRXDir direction)
        {
            switch (direction)
            {
            case TRXDir::Rx:
                return rx;
            case TRXDir::Tx:
                return tx;
            }
        }

        const Direction& GetDirection(TRXDir direction) const
        {
            switch (direction)
            {
            case TRXDir::Rx:
                return rx;
            case TRXDir::Tx:
                return tx;
            }
        }

        Direction rx; ///< Configuration settings for the Receive channel.
        Direction tx; ///< Configuration settings for the Transmit channel.
    };

    /// @brief Configuration of an SDR device.
    struct SDRConfig {
        SDRConfig()
            : referenceClockFreq(0)
            , skipDefaults(false){};
        double referenceClockFreq; ///< The reference clock frequency of the device.
        ChannelConfig channel[MAX_CHANNEL_COUNT]; ///< The configuration settings for each of the channels.
        // Loopback setup?
        bool skipDefaults; ///< Skip default values initialization and write on top of current config.
    };

    virtual ~SDRDevice(){};

    /// @brief Configures the device using the given configuration.
    /// @param config The configuration to set up the device with.
    /// @param moduleIndex The device index to configure.
    virtual void Configure(const SDRConfig& config, uint8_t moduleIndex) = 0;

    /// @brief Gets the Descriptor of the SDR Device.
    /// @return The Descriptor of the device.
    virtual const Descriptor& GetDescriptor() const = 0;

    /// @brief Initializes the device with initial settings.
    /// @return The success status of the initialization (0 on success).
    virtual int Init() = 0;

    /// @brief Resets the device.
    virtual void Reset() = 0;

    /// @brief Gets the current status of the GPS locks.
    /// @param status The pointer to which to output the GPS status.
    virtual void GetGPSLock(GPS_Lock* status) = 0;

    virtual void EnableChannel(uint8_t moduleIndex, TRXDir trx, uint8_t channel, bool enable) = 0;

    /// @brief Gets the frequency of a specified clock.
    /// @param clk_id The clock ID to get the frequency of.
    /// @param channel The channel to get the frequency of.
    /// @return The frequency of the specified clock (in Hz).
    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) = 0;

    /// @brief Sets the frequency of a specified clock.
    /// @param clk_id The clock ID to set the frequency of.
    /// @param freq The new frequency of the specified clock (in Hz).
    /// @param channel The channel to set the frequency of.
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) = 0;

    virtual double GetFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;
    virtual void SetFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double frequency) = 0;

    virtual double GetNCOFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t index) = 0;
    virtual void SetNCOFrequency(
        uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t index, double frequency, double phaseOffset = -1.0) = 0;

    virtual double GetNCOOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;

    virtual double GetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;
    virtual void SetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double sampleRate, uint8_t oversample) = 0;

    virtual int SetGain(uint8_t moduleIndex, TRXDir direction, uint8_t channel, eGainTypes gain, double value) = 0;
    virtual int GetGain(uint8_t moduleIndex, TRXDir direction, uint8_t channel, eGainTypes gain, double& value) = 0;

    virtual double GetLowPassFilter(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;
    virtual void SetLowPassFilter(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double lpf) = 0;

    virtual uint8_t GetAntenna(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;
    virtual void SetAntenna(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t path) = 0;

    virtual ChannelConfig::Direction::TestSignal GetTestSignal(uint8_t moduleIndex, TRXDir direction, uint8_t channel) = 0;
    virtual void SetTestSignal(uint8_t moduleIndex,
        TRXDir direction,
        uint8_t channel,
        ChannelConfig::Direction::TestSignal signalConfiguration,
        int16_t dc_i = 0,
        int16_t dc_q = 0) = 0;

    virtual bool GetDCOffsetMode(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;
    virtual void SetDCOffsetMode(uint8_t moduleIndex, TRXDir trx, uint8_t channel, bool isAutomatic) = 0;

    virtual complex64f_t GetDCOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;
    virtual void SetDCOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel, const complex64f_t& offset) = 0;

    virtual complex64f_t GetIQBalance(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;
    virtual void SetIQBalance(uint8_t moduleIndex, TRXDir trx, uint8_t channel, const complex64f_t& balance) = 0;

    virtual bool GetCGENLocked(uint8_t moduleIndex) = 0;
    virtual double GetTemperature(uint8_t moduleIndex) = 0;

    virtual bool GetSXLocked(uint8_t moduleIndex, TRXDir trx) = 0;

    virtual unsigned int ReadRegister(uint8_t moduleIndex, unsigned int address, bool useFPGA = false) = 0;
    virtual void WriteRegister(uint8_t moduleIndex, unsigned int address, unsigned int value, bool useFPGA = false) = 0;

    virtual void LoadConfig(uint8_t moduleIndex, const std::string& filename) = 0;
    virtual void SaveConfig(uint8_t moduleIndex, const std::string& filename) = 0;

    virtual uint16_t GetParameter(uint8_t moduleIndex, uint8_t channel, const std::string& parameterKey) = 0;
    virtual void SetParameter(uint8_t moduleIndex, uint8_t channel, const std::string& parameterKey, uint16_t value) = 0;

    virtual uint16_t GetParameter(uint8_t moduleIndex, uint8_t channel, uint16_t address, uint8_t msb, uint8_t lsb) = 0;
    virtual void SetParameter(uint8_t moduleIndex, uint8_t channel, uint16_t address, uint8_t msb, uint8_t lsb, uint16_t value) = 0;

    virtual void Calibrate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double bandwidth) = 0;
    virtual void ConfigureGFIR(uint8_t moduleIndex, TRXDir trx, uint8_t channel, ChannelConfig::Direction::GFIRFilter settings) = 0;

    virtual std::vector<double> GetGFIRCoefficients(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t gfirID) = 0;
    virtual void SetGFIRCoefficients(
        uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t gfirID, std::vector<double> coefficients) = 0;
    virtual void SetGFIR(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t gfirID, bool enabled) = 0;

    /// @brief Synchronizes the cached changed register values on the host with the real values on the device.
    /// @param toChip The direction in which to synchronize (true = uploads to the device).
    virtual void Synchronize(bool toChip) = 0;

    /// @brief Enable or disable register value caching on the host side.
    /// @param enable Whether to enable or disable the register value caching (true = enabled).
    virtual void EnableCache(bool enable) = 0;

    virtual uint64_t GetHardwareTimestamp(uint8_t moduleIndex) = 0;
    virtual void SetHardwareTimestamp(uint8_t moduleIndex, const uint64_t now) = 0;

    /// @brief Sets up all the streams on a device.
    /// @param config The configuration to use for setting the streams up.
    /// @param moduleIndex The index of the device to set up.
    /// @return Success status (0 on success).
    virtual int StreamSetup(const StreamConfig& config, uint8_t moduleIndex) = 0;

    /// @brief Starts all the set up streams on the device.
    /// @param moduleIndex The index of the device to start the streams on.
    virtual void StreamStart(uint8_t moduleIndex) = 0;

    /// @brief Stops all the set up streams on the device.
    /// @param moduleIndex The index of the device to stop the streams on.
    virtual void StreamStop(uint8_t moduleIndex) = 0;

    /// @brief Reveives samples from all the active streams in the device.
    /// @param moduleIndex The index of the device to receive the samples from.
    /// @param samples The buffer to put the received samples in.
    /// @param count The amount of samples to reveive.
    /// @param meta The metadata of the packets of the stream.
    /// @return The amount of samples received.
    virtual uint32_t StreamRx(uint8_t moduleIndex, lime::complex32f_t* const* samples, uint32_t count, StreamMeta* meta) = 0;
    /// @copydoc SDRDevice::StreamRx()
    virtual uint32_t StreamRx(uint8_t moduleIndex, lime::complex16_t* const* samples, uint32_t count, StreamMeta* meta) = 0;

    /// @brief Transmits packets from all the active streams in the device.
    /// @param moduleIndex The index of the device to transmit the samples with.
    /// @param samples The buffer of the samples to transmit.
    /// @param count The amount of samples to transmit.
    /// @param meta The metadata of the packets of the stream.
    /// @return The amount of samples transmitted.
    virtual uint32_t StreamTx(
        uint8_t moduleIndex, const lime::complex32f_t* const* samples, uint32_t count, const StreamMeta* meta) = 0;
    /// @copydoc SDRDevice::StreamTx()
    virtual uint32_t StreamTx(
        uint8_t moduleIndex, const lime::complex16_t* const* samples, uint32_t count, const StreamMeta* meta) = 0;

    /// @brief Retrieves the current stream statistics.
    /// @param moduleIndex The index of the device to retrieve the status from.
    /// @param rx The pointer (or nullptr if not needed) to store the receive statistics to.
    /// @param tx The pointer (or nullptr if not needed) to store the transmit statistics to.
    virtual void StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats* rx, SDRDevice::StreamStats* tx) = 0;

    /// @brief Uploads waveform to on board memory for later use.
    /// @param config The configuration of the stream.
    /// @param moduleIndex The index of the device to upload the waveform to.
    /// @param samples The samples to upload to the device.
    /// @param count The amount of samples to upload to the device.
    /// @return Operation status (0 on success).
    virtual int UploadTxWaveform(const StreamConfig& config, uint8_t moduleIndex, const void** samples, uint32_t count)
    {
        return -1;
    }

    /// @copydoc ISPI::SPI()
    /// @param spiBusAddress The SPI address of the device to use.
    virtual int SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) = 0;

    /// @copydoc II2C::I2CWrite()
    virtual int I2CWrite(int address, const uint8_t* data, uint32_t length) = 0;

    /// @copydoc II2C::I2CRead()
    virtual int I2CRead(int address, uint8_t* dest, uint32_t length) = 0;

    /***********************************************************************
     * GPIO API
     **********************************************************************/

    /// @copydoc IComms::GPIOWrite()
    virtual int GPIOWrite(const uint8_t* buffer, const size_t bufLength) { return -1; };

    /// @copydoc IComms::GPIORead()
    virtual int GPIORead(uint8_t* buffer, const size_t bufLength) { return -1; };

    /// @copydoc IComms::GPIODirWrite()
    virtual int GPIODirWrite(const uint8_t* buffer, const size_t bufLength) { return -1; };

    /// @copydoc IComms::GPIODirRead()
    virtual int GPIODirRead(uint8_t* buffer, const size_t bufLength) { return -1; };

    /***********************************************************************
     * Aribtrary settings API
     **********************************************************************/

    /// @copydoc IComms::CustomParameterWrite()
    virtual int CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) { return -1; };

    /// @copydoc IComms::CustomParameterRead()
    virtual int CustomParameterRead(std::vector<CustomParameterIO>& parameters) { return -1; };

    /// @brief The definition of a function to run when data is received.
    typedef void (*DataCallbackType)(bool, const uint8_t*, const uint32_t);

    /// @brief Sets callback function which gets called each time data is sent or received
    /// @param callback The callback to use from this point onwards.
    virtual void SetDataLogCallback(DataCallbackType callback){};

    /// @brief The definition of a function to call when a log message is generated.
    typedef void (*LogCallbackType)(LogLevel, const char*);

    /// @brief Sets callback function which gets called each a log message is received
    /// @param callback The callback to use from this point onwards.
    virtual void SetMessageLogCallback(LogCallbackType callback){};

    /// @brief Gets the pointer to an internal chip of the device.
    /// @param index The index of the device to retreive.
    /// @return The pointer to the internal device.
    virtual void* GetInternalChip(uint32_t index) = 0;

    /// @brief The definition of a function to call whenever memory is being uploaded.
    typedef bool (*UploadMemoryCallback)(size_t bsent, size_t btotal, const char* statusMessage);

    /// @brief Uploads the given memory into the specified device.
    /// @param device The memory device to upload the memory to.
    /// @param moduleIndex The index of the main device to upload the memory to.
    /// @param data The data to upload to the device.
    /// @param length The length of the memory to upload.
    /// @param callback The callback to call for status updates.
    /// @return The success status of the operation (0 on success).
    virtual int UploadMemory(
        eMemoryDevice device, uint8_t moduleIndex, const char* data, size_t length, UploadMemoryCallback callback)
    {
        return -1;
    };

    /// @brief Writes given data into a given memory address in EEPROM memory.
    /// @param storage The storage device to write to.
    /// @param region Information of the region in which to write the data to.
    /// @param data The data to write into the specified memory.
    /// @return The operation success state.
    virtual int MemoryWrite(std::shared_ptr<DataStorage> storage, Region region, const void* data) { return -1; };

    /// @brief Reads data from a given memory address in EEPROM memory.
    /// @param storage The storage device to read from.
    /// @param region Information of the region from which to read the memory.
    /// @param data The storage buffer for the data being read.
    /// @return The operation success state.
    virtual int MemoryRead(std::shared_ptr<DataStorage> storage, Region region, void* data) { return -1; };
};

} // namespace lime
#endif
