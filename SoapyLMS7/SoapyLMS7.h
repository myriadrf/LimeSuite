/**
@file	SoapyLMS7.h
@brief	Header for Soapy SDR + IConnection bindings.
@author Lime Microsystems (www.limemicro.com)
*/

#include <SoapySDR/Device.hpp>

#include "limesuite/DeviceRegistry.h"
#include "limesuite/SDRDevice.h"

#include <array>
#include <chrono>
#include <map>
#include <mutex>
#include <set>
#include <vector>

struct IConnectionStream;

class SoapyLMS7 : public SoapySDR::Device
{
  public:
    SoapyLMS7(const lime::DeviceHandle& handle, const SoapySDR::Kwargs& args);

    ~SoapyLMS7(void);

    /*******************************************************************
     * Identification API
     ******************************************************************/

    std::string getDriverKey(void) const;

    std::string getHardwareKey(void) const;

    SoapySDR::Kwargs getHardwareInfo(void) const;

    /*******************************************************************
     * Channels API
     ******************************************************************/

    size_t getNumChannels(const int direction) const;

    bool getFullDuplex(const int direction, const size_t channel) const;

    /*******************************************************************
     * Stream API
     ******************************************************************/
    std::vector<std::string> getStreamFormats(const int direction, const size_t channel) const;

    std::string getNativeStreamFormat(const int direction, const size_t channel, double& fullScale) const;

    SoapySDR::ArgInfoList getStreamArgsInfo(const int direction, const size_t channel) const;

    SoapySDR::Stream* setupStream(const int direction,
        const std::string& format,
        const std::vector<size_t>& channels = std::vector<size_t>(),
        const SoapySDR::Kwargs& args = SoapySDR::Kwargs());

    void closeStream(SoapySDR::Stream* stream);

    size_t getStreamMTU(SoapySDR::Stream* stream) const;

    int activateStream(SoapySDR::Stream* stream, const int flags = 0, const long long timeNs = 0, const size_t numElems = 0);

    int deactivateStream(SoapySDR::Stream* stream, const int flags = 0, const long long timeNs = 0);

    int readStream(SoapySDR::Stream* stream,
        void* const* buffs,
        const size_t numElems,
        int& flags,
        long long& timeNs,
        const long timeoutUs = 100000);

    int writeStream(SoapySDR::Stream* stream,
        const void* const* buffs,
        const size_t numElems,
        int& flags,
        const long long timeNs = 0,
        const long timeoutUs = 100000);

    int readStreamStatus(SoapySDR::Stream* stream, size_t& chanMask, int& flags, long long& timeNs, const long timeoutUs = 100000);

    /*******************************************************************
     * Antenna API
     ******************************************************************/

    std::vector<std::string> listAntennas(const int direction, const size_t channel) const;

    void setAntenna(const int direction, const size_t channel, const std::string& name);

    std::string getAntenna(const int direction, const size_t channel) const;

    /*******************************************************************
     * Frontend corrections API
     ******************************************************************/

    bool hasDCOffsetMode(const int direction, const size_t channel) const;

    void setDCOffsetMode(const int direction, const size_t channel, const bool automatic);

    bool getDCOffsetMode(const int direction, const size_t channel) const;

    bool hasDCOffset(const int direction, const size_t channel) const;

    void setDCOffset(const int direction, const size_t channel, const std::complex<double>& offset);

    std::complex<double> getDCOffset(const int direction, const size_t channel) const;

    bool hasIQBalance(const int direction, const size_t channel) const;

    void setIQBalance(const int direction, const size_t channel, const std::complex<double>& balance);

    std::complex<double> getIQBalance(const int direction, const size_t channel) const;

    /*******************************************************************
     * Gain API
     ******************************************************************/

    std::vector<std::string> listGains(const int direction, const size_t channel) const;

    void setGain(const int direction, const size_t channel, const double value) override;

    double getGain(const int direction, const size_t channel) const;

    void setGain(const int direction, const size_t channel, const std::string& name, const double value);

    double getGain(const int direction, const size_t channel, const std::string& name) const;

    SoapySDR::Range getGainRange(const int direction, const size_t channel) const;

    SoapySDR::Range getGainRange(const int direction, const size_t channel, const std::string& name) const;

    /*******************************************************************
     * Frequency API
     ******************************************************************/

    SoapySDR::ArgInfoList getFrequencyArgsInfo(const int direction, const size_t channel) const;

    void setFrequency(
        const int direction, const size_t channel, const double frequency, const SoapySDR::Kwargs& args = SoapySDR::Kwargs());

    void setFrequency(const int direction,
        const size_t channel,
        const std::string& name,
        const double frequency,
        const SoapySDR::Kwargs& args = SoapySDR::Kwargs());

    double getFrequency(const int direction, const size_t channel, const std::string& name) const;

    double getFrequency(const int direction, const size_t channel) const override;

    std::vector<std::string> listFrequencies(const int direction, const size_t channel) const;

    SoapySDR::RangeList getFrequencyRange(const int direction, const size_t channel) const;

    SoapySDR::RangeList getFrequencyRange(const int direction, const size_t channel, const std::string& name) const;

    /*******************************************************************
     * Sample Rate API
     ******************************************************************/

    void setSampleRate(const int direction, const size_t channel, const double rate);

    double getSampleRate(const int direction, const size_t channel) const;

    SoapySDR::RangeList getSampleRateRange(const int direction, const size_t channel) const;

    /*******************************************************************
     * Bandwidth API
     ******************************************************************/

    void setBandwidth(const int direction, const size_t channel, const double bw);

    double getBandwidth(const int direction, const size_t channel) const;

    SoapySDR::RangeList getBandwidthRange(const int direction, const size_t channel) const;

    /*******************************************************************
     * Clocking API
     ******************************************************************/

    double getMasterClockRate(void) const;

    /*******************************************************************
     * Time API
     ******************************************************************/

    bool hasHardwareTime(const std::string& what = "") const;

    long long getHardwareTime(const std::string& what = "") const;

    void setHardwareTime(const long long timeNs, const std::string& what = "");

    /*******************************************************************
     * Sensor API
     ******************************************************************/

    std::vector<std::string> listSensors(void) const;

    SoapySDR::ArgInfo getSensorInfo(const std::string& name) const;

    std::string readSensor(const std::string& name) const;

    std::vector<std::string> listSensors(const int direction, const size_t channel) const;

    SoapySDR::ArgInfo getSensorInfo(const int direction, const size_t channel, const std::string& name) const;

    std::string readSensor(const int direction, const size_t channel, const std::string& name) const;

    /*******************************************************************
     * Register API
     ******************************************************************/

    std::vector<std::string> listRegisterInterfaces(void) const;

    void writeRegister(const std::string& name, const unsigned addr, const unsigned value);

    unsigned readRegister(const std::string& name, const unsigned addr) const;

    void writeRegister(const unsigned addr, const unsigned value);

    unsigned readRegister(const unsigned addr) const;

    /*******************************************************************
     * Settings API
     ******************************************************************/

    SoapySDR::ArgInfoList getSettingInfo(void) const;

    void writeSetting(const std::string& key, const std::string& value);

    SoapySDR::ArgInfoList getSettingInfo(const int direction, const size_t channel) const;

    void writeSetting(const int direction, const size_t channel, const std::string& key, const std::string& value);

    std::string readSetting(const std::string& key) const;

    std::string readSetting(const int direction, const size_t channel, const std::string& key) const;

    /*******************************************************************
     * GPIO API
     ******************************************************************/

    std::vector<std::string> listGPIOBanks(void) const;

    void writeGPIO(const std::string& bank, const unsigned value);

    unsigned readGPIO(const std::string& bank) const;

    void writeGPIODir(const std::string& bank, const unsigned dir);

    unsigned readGPIODir(const std::string& bank) const;

  private:
    struct SettingsCache {
        SettingsCache()
            : calibrationBandwidth(-1)
            , GFIRBandwidth(-1)
            , DCTestAmplitude(0){};
        double calibrationBandwidth;
        double GFIRBandwidth;
        int DCTestAmplitude;
    };

    const std::string _moduleName;
    lime::SDRDevice* sdrDevice;
    std::set<SoapySDR::Stream*> activeStreams;

    mutable std::recursive_mutex _accessMutex;

    std::array<double, 2> sampleRate; // sampleRate[direction]
    int oversampling;

    std::map<std::size_t, int> _interps;
    std::map<std::size_t, int> _decims;

    int _readStreamAligned(IConnectionStream* stream,
        void* const* buffs,
        std::size_t numElems,
        int64_t requestTime,
        lime::SDRDevice::StreamMeta& mdOut,
        const long timeoutMs);

    std::array<std::vector<SettingsCache>, 2> settingsCache; // settingsCache[direction]
};
