#ifndef LIME_SDRDevice_H
#define LIME_SDRDevice_H

#include <vector>
#include <unordered_map>
#include <map>
#include <cstring>
#include <string>
#include <memory>

#include "limesuite/config.h"
#include "limesuite/commonTypes.h"
#include "limesuite/complex.h"
#include "limesuite/IComms.h"
#include "limesuite/MemoryDevices.h"
#include "limesuite/MemoryRegions.h"

namespace lime {

struct DeviceNode;

/** @brief SDRDevice can have multiple modules (RF chips), that can operate independently. */
class LIME_API SDRDevice
{
  public:
    static constexpr uint8_t MAX_CHANNEL_COUNT = 16;
    static constexpr uint8_t MAX_RFSOC_COUNT = 16;

    enum class LogLevel : uint8_t { CRITICAL, ERROR, WARNING, INFO, VERBOSE, DEBUG };
    typedef void (*DataCallbackType)(bool, const uint8_t*, const uint32_t);
    typedef void (*LogCallbackType)(LogLevel, const char*);

    typedef std::map<std::string, uint32_t> SlaveNameIds_t;

    /** @brief General information about the Radio-Frequency System-on-Chip (RFSoC). */
    struct RFSOCDescriptor {
        std::string name;
        uint8_t channelCount;
        std::vector<std::string> rxPathNames;
        std::vector<std::string> txPathNames;

        Range samplingRateRange;
        Range frequencyRange;
        std::unordered_map<TRXDir, std::unordered_map<std::string, Range>> antennaRange;
    };

    /** @brief Structure for the information of a custom parameter. */
    struct CustomParameter {
        std::string name;
        int32_t id;
        int32_t minValue;
        int32_t maxValue;
        bool readOnly;
    };

    /** @brief Structure for storing the information of a memory region. */
    struct Region {
        int32_t address;
        int32_t size;
    };

    /** @brief Describes a data storage of a certain type a device holds. */
    struct DataStorage {
        SDRDevice* ownerDevice;
        eMemoryDevice memoryDeviceType;
        std::unordered_map<eMemoryRegion, Region> regions;

        DataStorage(SDRDevice* device = nullptr,
            eMemoryDevice type = eMemoryDevice::COUNT,
            std::unordered_map<eMemoryRegion, Region> regions = {})
            : ownerDevice(device)
            , memoryDeviceType(type)
            , regions(regions)
        {
        }
    };

    /** @brief General information about device internals, static capabilities. */
    struct Descriptor {
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
        uint64_t serialNumber{ 0 }; /// A unique board serial number

        SlaveNameIds_t spiSlaveIds; // names and SPI bus numbers of internal chips
        std::vector<RFSOCDescriptor> rfSOC;
        std::vector<CustomParameter> customParameters;
        std::map<std::string, std::shared_ptr<DataStorage>> memoryDevices;
        std::shared_ptr<DeviceNode> socTree;

        static const char DEVICE_NUMBER_SEPARATOR_SYMBOL;
        static const char PATH_SEPARATOR_SYMBOL;
    };

    /** @brief Structure for holding the statistics of a stream */
    struct StreamStats {
        /** @brief Structure for storing the first in first out queue statistics */
        struct FIFOStats {
            std::size_t totalCount;
            std::size_t usedCount;

            float ratio() { return static_cast<float>(usedCount) / totalCount; }
        };

        StreamStats() { memset(this, 0, sizeof(StreamStats)); }
        uint64_t timestamp;
        int64_t bytesTransferred;
        int64_t packets;
        FIFOStats FIFO;
        float dataRate_Bps;
        uint32_t overrun;
        uint32_t underrun;
        uint32_t loss;
        uint32_t late;
    };

    /** @brief Describes the status of the GPS. */
    struct GPS_Lock {
        enum class LockStatus : uint8_t { Undefined, NotAvailable, Has2D, Has3D };

        LockStatus galileo;
        LockStatus beidou;
        LockStatus glonass;
        LockStatus gps;
    };

    /** @brief Configuration settings for a stream. */
    struct StreamConfig {
        /** @brief Extra configuration settings for a stream. */
        struct Extras {
            Extras();
            bool usePoll;
            uint16_t rxSamplesInPacket;
            uint32_t rxPacketsInBatch;
            uint32_t txMaxPacketsInBatch;
            uint16_t txSamplesInPacket;
            bool negateQ;
            bool waitPPS; // start sampling from next following PPS
        };
        typedef bool (*StatusCallbackFunc)(bool isTx, const StreamStats* stats, void* userData);
        enum class DataFormat : uint8_t {
            I16,
            I12,
            F32,
        };

        StreamConfig();
        ~StreamConfig();
        StreamConfig& operator=(const StreamConfig& srd);

        uint8_t rxCount;
        uint8_t rxChannels[MAX_CHANNEL_COUNT];
        uint8_t txCount;
        uint8_t txChannels[MAX_CHANNEL_COUNT];

        DataFormat format; // samples format used for Read/Write functions
        DataFormat linkFormat; // samples format used in transport layer Host<->FPGA

        /// memory size to allocate for each channel buffering
        /// Default: 0 - allow to decide internally
        uint32_t bufferSize;

        /// optional: expected sampling rate for data transfer optimizations.
        /// Default: 0 - deicide internally
        float hintSampleRate;
        bool alignPhase; // attempt to do phases alignment between paired channels

        StatusCallbackFunc statusCallback;
        void* userData; // will be supplied to statusCallback
        // TODO: callback for drops and errors

        Extras* extraConfig;
    };

    /** @brief The metadata of a stream packet. */
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

    /** @brief Configuration of a general finite impulse response (FIR) filter. */
    struct GFIRFilter {
        double bandwidth;
        bool enabled;
    };

    /** @brief Configuration of a single channel. */
    struct ChannelConfig {
        ChannelConfig() { memset(this, 0, sizeof(ChannelConfig)); }

        /** @brief Configuration for a direction in a channel. */
        struct Direction {
            double centerFrequency;
            double NCOoffset;
            double sampleRate;
            double gain;
            double lpf;
            uint8_t path;
            uint8_t oversample;
            GFIRFilter gfir;
            bool enabled;
            bool calibrate;
            bool testSignal;
        };
        Direction rx;
        Direction tx;
    };

    /** @brief Configuration of an SDR device. */
    struct SDRConfig {
        SDRConfig()
            : referenceClockFreq(0)
            , skipDefaults(false){};
        double referenceClockFreq;
        ChannelConfig channel[MAX_CHANNEL_COUNT];
        // Loopback setup?
        bool skipDefaults; // skip default values initialization and write on top of current config
    };

    virtual ~SDRDevice(){};

    virtual void Configure(const SDRConfig& config, uint8_t moduleIndex) = 0;

    /** @brief Returns SPI slave names and chip select IDs for use with SDRDevice::SPI() */
    virtual const Descriptor& GetDescriptor() = 0;

    virtual int Init() = 0;
    virtual void Reset() = 0;
    virtual void GetGPSLock(GPS_Lock* status) = 0;

    virtual double GetSampleRate(uint8_t moduleIndex, TRXDir trx) = 0;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) = 0;
    virtual void SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) = 0;

    virtual void Synchronize(bool toChip) = 0;
    virtual void EnableCache(bool enable) = 0;

    virtual int StreamSetup(const StreamConfig& config, uint8_t moduleIndex) = 0;
    virtual void StreamStart(uint8_t moduleIndex) = 0;
    virtual void StreamStop(uint8_t moduleIndex) = 0;

    virtual int StreamRx(uint8_t moduleIndex, lime::complex32f_t** samples, uint32_t count, StreamMeta* meta) = 0;
    virtual int StreamRx(uint8_t moduleIndex, lime::complex16_t** samples, uint32_t count, StreamMeta* meta) = 0;
    virtual int StreamTx(uint8_t moduleIndex, const lime::complex32f_t* const* samples, uint32_t count, const StreamMeta* meta) = 0;
    virtual int StreamTx(uint8_t moduleIndex, const lime::complex16_t* const* samples, uint32_t count, const StreamMeta* meta) = 0;
    virtual void StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats* rx, SDRDevice::StreamStats* tx) = 0;

    virtual int UploadTxWaveform(const StreamConfig& config, uint8_t moduleIndex, const void** samples, uint32_t count)
    {
        return -1;
    }

    virtual int SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) = 0;
    virtual int I2CWrite(int address, const uint8_t* data, uint32_t length) = 0;
    virtual int I2CRead(int addres, uint8_t* dest, uint32_t length) = 0;

    /***********************************************************************
     * GPIO API
     **********************************************************************/

    /** \copydoc IComms::GPIOWrite() */
    virtual int GPIOWrite(const uint8_t* buffer, const size_t bufLength) { return -1; };

    /** \copydoc IComms::GPIORead() */
    virtual int GPIORead(uint8_t* buffer, const size_t bufLength) { return -1; };

    /** \copydoc IComms::GPIODirWrite() */
    virtual int GPIODirWrite(const uint8_t* buffer, const size_t bufLength) { return -1; };

    /** \copydoc IComms::GPIODirRead() */
    virtual int GPIODirRead(uint8_t* buffer, const size_t bufLength) { return -1; };

    /***********************************************************************
     * Aribtrary settings API
     **********************************************************************/

    /** @brief Sets custom on board control to given value units
    @param parameters A vector of parameters describing the parameter to write
    @return The operation success state
    */
    virtual int CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) { return -1; };

    /** @brief Returns value of custom on board control
    @param parameters A vector of parameters describing the parameter to read
    @return The operation success state
    */
    virtual int CustomParameterRead(std::vector<CustomParameterIO>& parameters) { return -1; };

    /** @brief Sets callback function which gets called each time data is sent or received */
    virtual void SetDataLogCallback(DataCallbackType callback){};
    virtual void SetMessageLogCallback(LogCallbackType callback){};

    virtual void* GetInternalChip(uint32_t index) { return nullptr; };

    typedef bool (*UploadMemoryCallback)(size_t bsent, size_t btotal, const char* statusMessage);
    virtual bool UploadMemory(
        eMemoryDevice device, uint8_t moduleIndex, const char* data, size_t length, UploadMemoryCallback callback)
    {
        return -1;
    };

    virtual int MemoryWrite(std::shared_ptr<DataStorage> storage, Region region, const void* data) { return -1; };
    virtual int MemoryRead(std::shared_ptr<DataStorage> storage, Region region, void* data) { return -1; };
};

} // namespace lime
#endif
