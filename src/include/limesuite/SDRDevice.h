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
#include "limesuite/OpStatus.h"
#include "limesuite/GainTypes.h"
#include "limesuite/IComms.h"
#include "limesuite/MemoryDevices.h"
#include "limesuite/MemoryRegions.h"

namespace lime {

struct DeviceNode;
class OEMTestReporter;

/// @brief Class for holding information about an SDR (Software Defined Radio) device.
/// SDRDevice can have multiple modules (RF chips), that can operate independently.
class LIME_API SDRDevice
{
  public:
    static constexpr uint8_t MAX_CHANNEL_COUNT = 16; ///< Maximum amount of channels an SDR Device can hold
    static constexpr uint8_t MAX_RFSOC_COUNT = 16; ///< Maximum amount of Radio-Frequency System-on-Chips

    /// @brief Enumerator to define the log level of a log message.
    enum class LogLevel : uint8_t { CRITICAL, ERROR, WARNING, INFO, VERBOSE, DEBUG };

    /// @brief Information about possible gain values.
    struct GainValue {
        uint16_t hardwareRegisterValue; ///< The value that is written to the hardware
        float actualGainValue; ///< The actual meaning of the value (in dB)
    };

    /// @brief General information about the Radio-Frequency System-on-Chip (RFSoC).
    struct RFSOCDescriptor {
        std::string name; ///< The name of the system
        uint8_t channelCount; ///< The available channel count of the system
        std::unordered_map<TRXDir, std::vector<std::string>> pathNames; ///< The available antenna names

        Range frequencyRange; ///< Deliverable frequency capabilities of the device
        Range samplingRateRange; ///< Sampling rate capabilities of the device

        std::unordered_map<TRXDir, std::unordered_map<std::string, Range>> antennaRange; ///< Antenna recommended bandwidths
        std::unordered_map<TRXDir, Range> lowPassFilterRange; ///< The ranges of the low pass filter

        std::unordered_map<TRXDir, std::set<eGainTypes>> gains; ///< The types of gains available
        std::unordered_map<TRXDir, std::unordered_map<eGainTypes, Range>> gainRange; ///< The available ranges of each gain
        std::unordered_map<TRXDir, std::unordered_map<eGainTypes, std::vector<GainValue>>> gainValues; ///< The possible gain values
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
    struct LIME_API StreamConfig {
        /// @brief Extra configuration settings for a stream.
        struct Extras {
            /// @brief The settings structure for a packet transmission.
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

        std::unordered_map<TRXDir, std::vector<uint8_t>> channels; ///< The channels to set up for the stream.

        DataFormat format; ///< Samples format used for Read/Write functions
        DataFormat linkFormat; ///< Samples format used in transport layer Host<->FPGA

        /// @brief Memory size to allocate for each channel buffering.
        /// Default: 0 - allow to decide internally.
        uint32_t bufferSize;

        /// Optional: expected sampling rate for data transfer optimizations (in Hz).
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
                double bandwidth; ///< The bandwidth of the filter (in Hz).
            };

            /// @brief The structure holding the status of the test signal the device can produce.
            struct TestSignal {
                /// @brief The enumeration describing the divide mode of the test signal.
                enum class Divide : uint8_t { Div8, Div4 };

                /// @brief The enumeration describing the scale of the test signal.
                enum class Scale : uint8_t { Full, Half };

                complex16_t dcValue; ///< The value to use when in DC mode.
                Divide divide; ///< The current divide of the test signal.
                Scale scale; ///< The current scale of the test signal.
                bool enabled; ///< Denotes whether test mode is enabled or not.
                bool dcMode; ///< The DC mode of the test mode.

                /// @brief The constructor for the Test Signal storage class.
                /// @param enabled Whether the test signal is enabled or not.
                /// @param dcMode Whether the DC mode is enabled or not.
                /// @param divide The divide mode of the test signal.
                /// @param scale The scale of the dest signal.
                TestSignal(bool enabled = false, bool dcMode = false, Divide divide = Divide::Div8, Scale scale = Scale::Half)
                    : dcValue(0, 0)
                    , divide(divide)
                    , scale(scale)
                    , enabled(enabled)
                    , dcMode(dcMode)
                {
                }
            };

            double centerFrequency; ///< The center frequency of the direction of this channel (in Hz).
            double NCOoffset; ///< The offset from the channel's numerically controlled oscillator (NCO) (in Hz).
            double sampleRate; ///< The sample rate of this direction of a channel (in Hz).
            std::unordered_map<eGainTypes, double> gain; ///< The gains and their current values for this direction.
            double lpf; ///< The bandwidth of the Low Pass Filter (LPF) (in Hz).
            uint8_t path; ///< The antenna being used for this direction.
            uint8_t oversample; ///< The oversample ratio of this direction.
            GFIRFilter gfir; ///< The general finite impulse response (FIR) filter settings of this direction.
            bool enabled; ///< Denotes whether this direction of a channel is enabled or not.
            bool calibrate; ///< Denotes whether the device will be calibrated or not.
            TestSignal testSignal; ///< Denotes whether the signal being sent is a test signal or not.
        };

        /// @brief Gets the reference to the direction settings.
        /// @param direction The direction to get it for.
        /// @return The reference to the direction.
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

        /// @brief Gets the const reference to the direction settings.
        /// @param direction The direction to get it for.
        /// @return The const reference to the direction.
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
    /// @return The status of the operation.
    virtual OpStatus Configure(const SDRConfig& config, uint8_t moduleIndex) = 0;

    /// @brief Gets the Descriptor of the SDR Device.
    /// @return The Descriptor of the device.
    virtual const Descriptor& GetDescriptor() const = 0;

    /// @brief Initializes the device with initial settings.
    /// @return The success status of the initialization.
    virtual OpStatus Init() = 0;

    /// @brief Resets the device.
    /// @return The status of the operation.
    virtual OpStatus Reset() = 0;

    /// @brief Gets the current status of the GPS locks.
    /// @param status The pointer to which to output the GPS status.
    /// @return The status of the operation.
    virtual OpStatus GetGPSLock(GPS_Lock* status) = 0;

    /// @brief Enables or disables the specified channel.
    /// @param moduleIndex The device index to configure.
    /// @param trx The direction of the channel to configure.
    /// @param channel The channel to configure.
    /// @param enable Whether to enable the channel or not.
    /// @return The status of the operation.
    virtual OpStatus EnableChannel(uint8_t moduleIndex, TRXDir trx, uint8_t channel, bool enable) = 0;

    /// @brief Gets the frequency of a specified clock.
    /// @param clk_id The clock ID to get the frequency of.
    /// @param channel The channel to get the frequency of.
    /// @return The frequency of the specified clock (in Hz).
    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) = 0;

    /// @brief Sets the frequency of a specified clock.
    /// @param clk_id The clock ID to set the frequency of.
    /// @param freq The new frequency of the specified clock (in Hz).
    /// @param channel The channel to set the frequency of.
    /// @return The status of the operation.
    virtual OpStatus SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) = 0;

    /// @brief Gets the current frequency of the given channel.
    /// @param moduleIndex The device index to read from.
    /// @param trx The direction to read from.
    /// @param channel The channel to read from.
    /// @return The current radio frequency of the channel (in Hz).
    virtual double GetFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;

    /// @brief Sets the radio frequency of the given channel.
    /// @param moduleIndex The device index to configure.
    /// @param trx The direction to configure.
    /// @param channel The channel to configure.
    /// @param frequency The frequency to set the channel to (in Hz).
    /// @return The status of the operation.
    virtual OpStatus SetFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double frequency) = 0;

    /// @brief Gets the current frequency of the NCO.
    /// @param moduleIndex The device index to read from.
    /// @param trx The direction to read from.
    /// @param channel The channel to read from.
    /// @param index The index of the NCO to read from.
    /// @return The current frequency of the NCO (in Hz)
    virtual double GetNCOFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t index) = 0;

    /// @brief Sets the frequency and the phase angle of the NCO.
    /// @param moduleIndex The device index to configure.
    /// @param trx The direction to configure.
    /// @param channel The channel to configure.
    /// @param index The index of the NCO to use.
    /// @param frequency The frequency of the NCO to set (in Hz).
    /// @param phaseOffset Phase offset angle (in degrees)
    /// @return The status of the operation.
    virtual OpStatus SetNCOFrequency(
        uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t index, double frequency, double phaseOffset = -1.0) = 0;

    /// @brief Gets the current offset of the NCO compared to the main frequency.
    /// @param moduleIndex The device index to read from.
    /// @param trx The direction to read from.
    /// @param channel The channel to read from.
    /// @return The delta between the current device frequency and the current device NCO frequency (in Hz).
    virtual double GetNCOOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;

    /// @brief Gets the current sample rate of the device.
    /// @param moduleIndex The device index to read from.
    /// @param trx The direction to read from.
    /// @param channel The channel to read from.
    /// @return The currend device sample rate (in Hz)
    virtual double GetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;

    /// @brief Sets the sample rate of the device.
    /// @param moduleIndex The device index to configure.
    /// @param trx The direction to configure.
    /// @param channel The channel to configure.
    /// @param sampleRate The target sample rate (in Hz)
    /// @param oversample The RF oversampling ratio.
    /// @return The status of the operation.
    virtual OpStatus SetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double sampleRate, uint8_t oversample) = 0;

    /// @brief Gets the current value of the specified gain.
    /// @param moduleIndex The device index to read from.
    /// @param direction The direction to read from.
    /// @param channel The channel to read from.
    /// @param gain The type of gain to get the data of.
    /// @param value The value of the gain (in dB).
    /// @return The status code of the operation.
    virtual OpStatus GetGain(uint8_t moduleIndex, TRXDir direction, uint8_t channel, eGainTypes gain, double& value) = 0;

    /// @brief Sets the gain level of a specified gain.
    /// @param moduleIndex The device index to configure.
    /// @param direction The direction to configure.
    /// @param channel The channel to configure.
    /// @param gain The type of gain to set.
    /// @param value The amount of gain to set (in dB).
    /// @return The status code of the operation.
    virtual OpStatus SetGain(uint8_t moduleIndex, TRXDir direction, uint8_t channel, eGainTypes gain, double value) = 0;

    /// @brief Gets the current frequency of the Low Pass Filter.
    /// @param moduleIndex The device index to read from.
    /// @param trx The direction to read from.
    /// @param channel The channel to read from.
    /// @return The current frequency of the Low Pass Filter (in Hz).
    virtual double GetLowPassFilter(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;

    /// @brief Sets the Low Pass Filter to a specified frequency.
    /// @param moduleIndex The device index to configure.
    /// @param trx The direction to configure.
    /// @param channel The channel to configure.
    /// @param lpf The bandwidth of the Low Pass Filter to set it to (in Hz).
    /// @return The status of the operation.
    virtual OpStatus SetLowPassFilter(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double lpf) = 0;

    /// @brief Gets the currently set antenna of the device.
    /// @param moduleIndex The device index to read from.
    /// @param trx The direction to read from.
    /// @param channel The channel to read from.
    /// @return The ID of the currently set antenna.
    virtual uint8_t GetAntenna(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;

    /// @brief Sets the current antenna of the device.
    /// @param moduleIndex The device index to configure.
    /// @param trx The direction to configure.
    /// @param channel The channel to configure.
    /// @param path The ID of the antenna to set the device to use.
    /// @return The status of the operation.
    virtual OpStatus SetAntenna(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t path) = 0;

    /// @brief Gets the current status of the test signal mode.
    /// @param moduleIndex The device index to read from.
    /// @param direction The direction to read from.
    /// @param channel The channel to read from.
    /// @return The current status of the test signal mode.
    virtual ChannelConfig::Direction::TestSignal GetTestSignal(uint8_t moduleIndex, TRXDir direction, uint8_t channel) = 0;

    /// @brief Sets the test signal mode.
    /// @param moduleIndex The device index to configure.
    /// @param direction The direction to configure.
    /// @param channel The channel to configure.
    /// @param signalConfiguration The configuration of the test mode to set.
    /// @param dc_i The I value of the test mode to send (0 for defaults)
    /// @param dc_q The Q value of the test mode to send (0 for defaults)
    /// @return The status of the operation.
    virtual OpStatus SetTestSignal(uint8_t moduleIndex,
        TRXDir direction,
        uint8_t channel,
        ChannelConfig::Direction::TestSignal signalConfiguration,
        int16_t dc_i = 0,
        int16_t dc_q = 0) = 0;

    /// @brief Gets if the DC corrector bypass is enabled or not.
    /// @param moduleIndex The device index to read from.
    /// @param trx The direction to read from.
    /// @param channel The channel to read from.
    /// @return Whether the DC corrector bypassis enabled or not (false = bypass the corrector, true = use the corrector)
    virtual bool GetDCOffsetMode(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;

    /// @brief Enables or disables the DC corrector bypass.
    /// @param moduleIndex The device index to configure.
    /// @param trx The direction to configure.
    /// @param channel The channel to configure.
    /// @param isAutomatic Whether to use the DC corrector bypass or not (false = bypass the corrector, true = use the corrector)
    /// @return The status of the operation.
    virtual OpStatus SetDCOffsetMode(uint8_t moduleIndex, TRXDir trx, uint8_t channel, bool isAutomatic) = 0;

    /// @brief Gets the DC I and Q corrector values.
    /// @param moduleIndex The device index to read from.
    /// @param trx The direction to read from.
    /// @param channel The channel to read from.
    /// @return The current DC I and Q corrector values.
    virtual complex64f_t GetDCOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;

    /// @brief Sets the DC I and Q corrector values.
    /// @param moduleIndex The device index to configure.
    /// @param trx The direction to configure.
    /// @param channel The channel to configure.
    /// @param offset The offsets of the I and Q channels.
    /// @return The status of the operation.
    virtual OpStatus SetDCOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel, const complex64f_t& offset) = 0;

    /// @brief Gets the current I and Q gain corrector values.
    /// @param moduleIndex The device index to read from.
    /// @param trx The direction to read from.
    /// @param channel The channel to read from.
    /// @return The current I and Q gain corrector values.
    virtual complex64f_t GetIQBalance(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;

    /// @brief Sets the I and Q gain corrector values.
    /// @param moduleIndex The device index to configure.
    /// @param trx The direction to configure.
    /// @param channel The channel to configure.
    /// @param balance The I and Q corrector values to set.
    /// @return The status of the operation.
    virtual OpStatus SetIQBalance(uint8_t moduleIndex, TRXDir trx, uint8_t channel, const complex64f_t& balance) = 0;

    /// @brief Gets whether the VCO comparators of the clock generator are locked or not.
    /// @param moduleIndex The device index to read from.
    /// @return A value indicating whether the VCO comparators of the clock generator are locked or not.
    virtual bool GetCGENLocked(uint8_t moduleIndex) = 0;

    /// @brief Gets the temperature of the device.
    /// @param moduleIndex The device index to get the temperature of.
    /// @return The temperature of the device (in degrees Celsius)
    virtual double GetTemperature(uint8_t moduleIndex) = 0;

    /// @brief Gets whether the VCO comparators of the LO synthesizer are locked or not.
    /// @param moduleIndex The device index to read from.
    /// @param trx The direction to read from.
    /// @return A value indicating whether the VCO comparators of the clock generator are locked or not.
    virtual bool GetSXLocked(uint8_t moduleIndex, TRXDir trx) = 0;

    /// @brief Reads the value of the given register.
    /// @param moduleIndex The device index to read from.
    /// @param address The memory address to read from.
    /// @param useFPGA Whether to read memory from the FPGA or not.
    /// @return The value read from the register.
    virtual unsigned int ReadRegister(uint8_t moduleIndex, unsigned int address, bool useFPGA = false) = 0;

    /// @brief Writes the given register value to the given address.
    /// @param moduleIndex The device index to configure.
    /// @param address The address of the memory to write to.
    /// @param value The value to write to the device's memory.
    /// @param useFPGA Whether to write to the FPGA or not (default false)
    /// @return The status of the operation.
    virtual OpStatus WriteRegister(uint8_t moduleIndex, unsigned int address, unsigned int value, bool useFPGA = false) = 0;

    /// @brief Loads the configuration of a device from a given file.
    /// @param moduleIndex The device index to write the configuration into.
    /// @param filename The file to read the data from.
    /// @return The status of the operation.
    virtual OpStatus LoadConfig(uint8_t moduleIndex, const std::string& filename) = 0;

    /// @brief Saves the current configuration of the device into a given file.
    /// @param moduleIndex The device index to save the data from.
    /// @param filename The file to save the information to.
    /// @return The status of the operation.
    virtual OpStatus SaveConfig(uint8_t moduleIndex, const std::string& filename) = 0;

    /// @brief Gets the given parameter from the device.
    /// @param moduleIndex The device index to configure.
    /// @param channel The channel to configure.
    /// @param parameterKey The key of the paremeter to read from.
    /// @return The value read from the parameter.
    virtual uint16_t GetParameter(uint8_t moduleIndex, uint8_t channel, const std::string& parameterKey) = 0;

    /// @brief Sets the given parameter in the device.
    /// @param moduleIndex The device index to configure.
    /// @param channel The channel to configure.
    /// @param parameterKey The key of the paremeter to write to.
    /// @param value The value to write to the address.
    /// @return The status of the operation.
    virtual OpStatus SetParameter(uint8_t moduleIndex, uint8_t channel, const std::string& parameterKey, uint16_t value) = 0;

    /// @brief Gets the given parameter from the device.
    /// @param moduleIndex The device index to get the data from.
    /// @param channel The channel to get the data from.
    /// @param address The memory address of the device to read.
    /// @param msb The index of the most significant bit of the address to read. (16-bit register)
    /// @param lsb The index of the least significant bit of the address to read. (16-bit register)
    /// @return The value read from the parameter.
    virtual uint16_t GetParameter(uint8_t moduleIndex, uint8_t channel, uint16_t address, uint8_t msb, uint8_t lsb) = 0;

    /// @brief Sets the given parameter in the device.
    /// @param moduleIndex The device index to configure.
    /// @param channel The channel to configure.
    /// @param address The memory address in the device to change.
    /// @param msb The index of the most significant bit of the address to modify. (16-bit register)
    /// @param lsb The index of the least significant bit of the address to modify. (16-bit register)
    /// @param value The value to write to the address.
    /// @return The status of the operation.
    virtual OpStatus SetParameter(
        uint8_t moduleIndex, uint8_t channel, uint16_t address, uint8_t msb, uint8_t lsb, uint16_t value) = 0;

    /// @brief Calibrates the given channel for a given bandwidth.
    /// @param moduleIndex The device index to configure.
    /// @param trx The direction of the channel to configure.
    /// @param channel The channel to configure.
    /// @param bandwidth The bandwidth of the channel to calibrate for (in Hz).
    /// @return The status of the operation.
    virtual OpStatus Calibrate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double bandwidth) = 0;

    /// @brief Configures the GFIR with the settings.
    /// @param moduleIndex The device index to configure.
    /// @param trx The direction of the channel to configure.
    /// @param channel The channel to configure.
    /// @param settings The settings of the GFIR to set.
    /// @return The status of the operation.
    virtual OpStatus ConfigureGFIR(
        uint8_t moduleIndex, TRXDir trx, uint8_t channel, ChannelConfig::Direction::GFIRFilter settings) = 0;

    /// @brief Gets the current coefficients of a GFIR.
    /// @param moduleIndex The device index to get the coefficients from.
    /// @param trx The direction of the channel to get the data from.
    /// @param channel The channel to get the data from.
    /// @param gfirID The ID of the GFIR to get the coefficients from.
    /// @return The current coefficients (normalized in the range [-1; 1]) of the GFIR.
    virtual std::vector<double> GetGFIRCoefficients(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t gfirID) = 0;

    /// @brief Sets the coefficients of a given GFIR
    /// @param moduleIndex The device index to configure.
    /// @param trx The direction of the channel to configure.
    /// @param channel The channel to set the filter of.
    /// @param gfirID The ID of the GFIR to set.
    /// @param coefficients The coefficients (normalized in the range [-1; 1]) to set the GFIR to.
    /// @return The status of the operation.
    virtual OpStatus SetGFIRCoefficients(
        uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t gfirID, std::vector<double> coefficients) = 0;

    /// @brief Sets the GFIR to use.
    /// @param moduleIndex The device index to configure.
    /// @param trx The direction of the channel to configure.
    /// @param channel The channel to set the filter of.
    /// @param gfirID The ID of the GFIR to set.
    /// @param enabled Whether the specifed GFIR should be enabled or disabled.
    /// @return The status of the operation.
    virtual OpStatus SetGFIR(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t gfirID, bool enabled) = 0;

    /// @brief Synchronizes the cached changed register values on the host with the real values on the device.
    /// @param toChip The direction in which to synchronize (true = uploads to the device).
    /// @return The status of the operation.
    virtual OpStatus Synchronize(bool toChip) = 0;

    /// @brief Enable or disable register value caching on the host side.
    /// @param enable Whether to enable or disable the register value caching (true = enabled).
    virtual void EnableCache(bool enable) = 0;

    /// @brief Gets the hardware timestamp with the applied offset.
    /// @param moduleIndex The device index to configure.
    /// @return The current timestamp of the hardware.
    virtual uint64_t GetHardwareTimestamp(uint8_t moduleIndex) = 0;

    /// @brief Sets the hardware timestamp to the provided one by applying a constant offset.
    /// @param moduleIndex The device index to configure.
    /// @param now What the definition of the current time should be.
    /// @return The status of the operation.
    virtual OpStatus SetHardwareTimestamp(uint8_t moduleIndex, const uint64_t now) = 0;

    /// @brief Sets up all the streams on a device.
    /// @param config The configuration to use for setting the streams up.
    /// @param moduleIndex The index of the device to set up.
    /// @return The status code of the operation.
    virtual OpStatus StreamSetup(const StreamConfig& config, uint8_t moduleIndex) = 0;

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
    /// @return Operation status.
    virtual OpStatus UploadTxWaveform(const StreamConfig& config, uint8_t moduleIndex, const void** samples, uint32_t count)
    {
        return OpStatus::NOT_IMPLEMENTED;
    }

    /// @copydoc ISPI::SPI()
    /// @param spiBusAddress The SPI address of the device to use.
    virtual OpStatus SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count);

    /// @copydoc II2C::I2CWrite()
    virtual OpStatus I2CWrite(int address, const uint8_t* data, uint32_t length);

    /// @copydoc II2C::I2CRead()
    virtual OpStatus I2CRead(int address, uint8_t* dest, uint32_t length);

    /***********************************************************************
     * GPIO API
     **********************************************************************/

    /// @copydoc IComms::GPIOWrite()
    virtual OpStatus GPIOWrite(const uint8_t* buffer, const size_t bufLength);

    /// @copydoc IComms::GPIORead()
    virtual OpStatus GPIORead(uint8_t* buffer, const size_t bufLength);

    /// @copydoc IComms::GPIODirWrite()
    virtual OpStatus GPIODirWrite(const uint8_t* buffer, const size_t bufLength);

    /// @copydoc IComms::GPIODirRead()
    virtual OpStatus GPIODirRead(uint8_t* buffer, const size_t bufLength);

    /***********************************************************************
     * Aribtrary settings API
     **********************************************************************/

    /// @copydoc IComms::CustomParameterWrite()
    virtual OpStatus CustomParameterWrite(const std::vector<CustomParameterIO>& parameters);

    /// @copydoc IComms::CustomParameterRead()
    virtual OpStatus CustomParameterRead(std::vector<CustomParameterIO>& parameters);

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
    /// @return The success status of the operation.
    virtual OpStatus UploadMemory(
        eMemoryDevice device, uint8_t moduleIndex, const char* data, size_t length, UploadMemoryCallback callback);

    /// @brief Writes given data into a given memory address in EEPROM memory.
    /// @param storage The storage device to write to.
    /// @param region Information of the region in which to write the data to.
    /// @param data The data to write into the specified memory.
    /// @return The operation success state.
    virtual OpStatus MemoryWrite(std::shared_ptr<DataStorage> storage, Region region, const void* data);

    /// @brief Reads data from a given memory address in EEPROM memory.
    /// @param storage The storage device to read from.
    /// @param region Information of the region from which to read the memory.
    /// @param data The storage buffer for the data being read.
    /// @return The operation success state.
    virtual OpStatus MemoryRead(std::shared_ptr<DataStorage> storage, Region region, void* data);
};

} // namespace lime
#endif
