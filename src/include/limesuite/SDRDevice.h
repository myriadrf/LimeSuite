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
/// SDRDevice can have multiple modules (RF chips), that can operate independently

class LIME_API SDRDevice
{
  public:
    static constexpr uint8_t MAX_CHANNEL_COUNT = 16;
    static constexpr uint8_t MAX_RFSOC_COUNT = 16;

    enum class LogLevel : uint8_t { CRITICAL, ERROR, WARNING, INFO, VERBOSE, DEBUG };
    typedef void (*DataCallbackType)(bool, const uint8_t*, const uint32_t);
    typedef void (*LogCallbackType)(LogLevel, const char*);

    typedef std::map<std::string, uint32_t> SlaveNameIds_t;

    struct GainValue {
        uint16_t hardwareRegisterValue;
        float actualGainValue;
    };

    struct RFSOCDescriptor {
        std::string name;
        uint8_t channelCount;
        std::unordered_map<TRXDir, std::vector<std::string>> pathNames;

        Range frequencyRange;
        Range samplingRateRange;

        std::unordered_map<TRXDir, std::unordered_map<std::string, Range>> antennaRange;
        std::unordered_map<TRXDir, Range> lowPassFilterRange;

        std::unordered_map<TRXDir, std::set<eGainTypes>> gains;
        std::unordered_map<TRXDir, std::unordered_map<eGainTypes, Range>> gainRange;
        std::unordered_map<TRXDir, std::unordered_map<eGainTypes, std::vector<GainValue>>> gainValues;
    };

    struct CustomParameter {
        std::string name;
        int32_t id;
        int32_t minValue;
        int32_t maxValue;
        bool readOnly;
    };

    struct Region {
        int32_t address;
        int32_t size;
    };

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

    struct StreamStats {
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

    struct GPS_Lock {
        enum class LockStatus : uint8_t { Undefined, NotAvailable, Has2D, Has3D };

        LockStatus galileo;
        LockStatus beidou;
        LockStatus glonass;
        LockStatus gps;
    };

    // channels order and data transmission formats setup
    struct StreamConfig {
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

        std::unordered_map<TRXDir, std::vector<uint8_t>> channels;

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

        Extras extraConfig;
    };

    struct StreamMeta {
        int64_t timestamp = 0;
        bool useTimestamp = false;
        bool flush = false; // submit data to hardware without waiting for full buffer
    };

    struct ChannelConfig {
        ChannelConfig()
            : rx()
            , tx()
        {
        }

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

            struct GFIRFilter {
                bool enabled;
                double bandwidth;
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

            double centerFrequency;
            double NCOoffset;
            double sampleRate;
            std::unordered_map<eGainTypes, double> gain;
            double lpf;
            uint8_t path;
            uint8_t oversample;
            GFIRFilter gfir;
            bool enabled;
            bool calibrate;
            TestSignal testSignal;
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

        Direction rx;
        Direction tx;
    };

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

    virtual OpStatus Configure(const SDRConfig& config, uint8_t moduleIndex) = 0;

    /** @brief Returns SPI slave names and chip select IDs for use with SDRDevice::SPI() */
    virtual const Descriptor& GetDescriptor() const = 0;

    virtual OpStatus Init() = 0;
    virtual OpStatus Reset() = 0;
    virtual OpStatus GetGPSLock(GPS_Lock* status) = 0;

    virtual OpStatus EnableChannel(uint8_t moduleIndex, TRXDir trx, uint8_t channel, bool enable) = 0;

    virtual double GetClockFreq(uint8_t clk_id, uint8_t channel) = 0;
    virtual OpStatus SetClockFreq(uint8_t clk_id, double freq, uint8_t channel) = 0;

    virtual double GetFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;
    virtual OpStatus SetFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double frequency) = 0;

    virtual double GetNCOFrequency(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t index) = 0;
    virtual OpStatus SetNCOFrequency(
        uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t index, double frequency, double phaseOffset = -1.0) = 0;

    virtual double GetNCOOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;

    virtual double GetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;
    virtual OpStatus SetSampleRate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double sampleRate, uint8_t oversample) = 0;

    virtual OpStatus SetGain(uint8_t moduleIndex, TRXDir direction, uint8_t channel, eGainTypes gain, double value) = 0;
    virtual OpStatus GetGain(uint8_t moduleIndex, TRXDir direction, uint8_t channel, eGainTypes gain, double& value) = 0;

    virtual double GetLowPassFilter(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;
    virtual OpStatus SetLowPassFilter(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double lpf) = 0;

    virtual uint8_t GetAntenna(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;
    virtual OpStatus SetAntenna(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t path) = 0;

    virtual ChannelConfig::Direction::TestSignal GetTestSignal(uint8_t moduleIndex, TRXDir direction, uint8_t channel) = 0;
    virtual OpStatus SetTestSignal(uint8_t moduleIndex,
        TRXDir direction,
        uint8_t channel,
        ChannelConfig::Direction::TestSignal signalConfiguration,
        int16_t dc_i = 0,
        int16_t dc_q = 0) = 0;

    virtual bool GetDCOffsetMode(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;
    virtual OpStatus SetDCOffsetMode(uint8_t moduleIndex, TRXDir trx, uint8_t channel, bool isAutomatic) = 0;

    virtual complex64f_t GetDCOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;
    virtual OpStatus SetDCOffset(uint8_t moduleIndex, TRXDir trx, uint8_t channel, const complex64f_t& offset) = 0;

    virtual complex64f_t GetIQBalance(uint8_t moduleIndex, TRXDir trx, uint8_t channel) = 0;
    virtual OpStatus SetIQBalance(uint8_t moduleIndex, TRXDir trx, uint8_t channel, const complex64f_t& balance) = 0;

    virtual bool GetCGENLocked(uint8_t moduleIndex) = 0;
    virtual double GetTemperature(uint8_t moduleIndex) = 0;

    virtual bool GetSXLocked(uint8_t moduleIndex, TRXDir trx) = 0;

    virtual unsigned int ReadRegister(uint8_t moduleIndex, unsigned int address, bool useFPGA = false) = 0;
    virtual OpStatus WriteRegister(uint8_t moduleIndex, unsigned int address, unsigned int value, bool useFPGA = false) = 0;

    virtual OpStatus LoadConfig(uint8_t moduleIndex, const std::string& filename) = 0;
    virtual OpStatus SaveConfig(uint8_t moduleIndex, const std::string& filename) = 0;

    virtual uint16_t GetParameter(uint8_t moduleIndex, uint8_t channel, const std::string& parameterKey) = 0;
    virtual OpStatus SetParameter(uint8_t moduleIndex, uint8_t channel, const std::string& parameterKey, uint16_t value) = 0;

    virtual uint16_t GetParameter(uint8_t moduleIndex, uint8_t channel, uint16_t address, uint8_t msb, uint8_t lsb) = 0;
    virtual OpStatus SetParameter(
        uint8_t moduleIndex, uint8_t channel, uint16_t address, uint8_t msb, uint8_t lsb, uint16_t value) = 0;

    virtual OpStatus Calibrate(uint8_t moduleIndex, TRXDir trx, uint8_t channel, double bandwidth) = 0;
    virtual OpStatus ConfigureGFIR(
        uint8_t moduleIndex, TRXDir trx, uint8_t channel, ChannelConfig::Direction::GFIRFilter settings) = 0;

    virtual std::vector<double> GetGFIRCoefficients(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t gfirID) = 0;
    virtual OpStatus SetGFIRCoefficients(
        uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t gfirID, std::vector<double> coefficients) = 0;
    virtual OpStatus SetGFIR(uint8_t moduleIndex, TRXDir trx, uint8_t channel, uint8_t gfirID, bool enabled) = 0;

    virtual OpStatus Synchronize(bool toChip) = 0;
    virtual void EnableCache(bool enable) = 0;

    virtual uint64_t GetHardwareTimestamp(uint8_t moduleIndex) = 0;
    virtual OpStatus SetHardwareTimestamp(uint8_t moduleIndex, const uint64_t now) = 0;

    virtual OpStatus StreamSetup(const StreamConfig& config, uint8_t moduleIndex) = 0;
    virtual void StreamStart(uint8_t moduleIndex) = 0;
    virtual void StreamStop(uint8_t moduleIndex) = 0;

    virtual int StreamRx(uint8_t moduleIndex, lime::complex32f_t* const* samples, uint32_t count, StreamMeta* meta) = 0;
    virtual int StreamRx(uint8_t moduleIndex, lime::complex16_t* const* samples, uint32_t count, StreamMeta* meta) = 0;
    virtual int StreamTx(uint8_t moduleIndex, const lime::complex32f_t* const* samples, uint32_t count, const StreamMeta* meta) = 0;
    virtual int StreamTx(uint8_t moduleIndex, const lime::complex16_t* const* samples, uint32_t count, const StreamMeta* meta) = 0;
    virtual void StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats* rx, SDRDevice::StreamStats* tx) = 0;

    virtual OpStatus UploadTxWaveform(const StreamConfig& config, uint8_t moduleIndex, const void** samples, uint32_t count)
    {
        return OpStatus::NOT_IMPLEMENTED;
    }

    virtual OpStatus SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count);
    virtual OpStatus I2CWrite(int address, const uint8_t* data, uint32_t length);
    virtual OpStatus I2CRead(int addres, uint8_t* dest, uint32_t length);

    /***********************************************************************
     * GPIO API
     **********************************************************************/

    /** \copydoc IComms::GPIOWrite() */
    virtual OpStatus GPIOWrite(const uint8_t* buffer, const size_t bufLength);

    /** \copydoc IComms::GPIORead() */
    virtual OpStatus GPIORead(uint8_t* buffer, const size_t bufLength);

    /** \copydoc IComms::GPIODirWrite() */
    virtual OpStatus GPIODirWrite(const uint8_t* buffer, const size_t bufLength);

    /** \copydoc IComms::GPIODirRead() */
    virtual OpStatus GPIODirRead(uint8_t* buffer, const size_t bufLength);

    /***********************************************************************
     * Aribtrary settings API
     **********************************************************************/

    /** @brief Sets custom on board control to given value units
    @param parameters A vector of parameters describing the parameter to write
    @return The operation success state
    */
    virtual OpStatus CustomParameterWrite(const std::vector<CustomParameterIO>& parameters);

    /** @brief Returns value of custom on board control
    @param parameters A vector of parameters describing the parameter to read
    @return The operation success state
    */
    virtual OpStatus CustomParameterRead(std::vector<CustomParameterIO>& parameters);

    /** @brief Sets callback function which gets called each time data is sent or received */
    virtual void SetDataLogCallback(DataCallbackType callback){};
    virtual void SetMessageLogCallback(LogCallbackType callback){};

    virtual void* GetInternalChip(uint32_t index) = 0;

    typedef bool (*UploadMemoryCallback)(size_t bsent, size_t btotal, const char* statusMessage);
    virtual OpStatus UploadMemory(
        eMemoryDevice device, uint8_t moduleIndex, const char* data, size_t length, UploadMemoryCallback callback)
    {
        return OpStatus::NOT_IMPLEMENTED;
    };

    virtual OpStatus MemoryWrite(std::shared_ptr<DataStorage> storage, Region region, const void* data)
    {
        return OpStatus::NOT_IMPLEMENTED;
    };
    virtual OpStatus MemoryRead(std::shared_ptr<DataStorage> storage, Region region, void* data)
    {
        return OpStatus::NOT_IMPLEMENTED;
    };
};

} // namespace lime
#endif
