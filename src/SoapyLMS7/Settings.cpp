/**
@file	Settings.cpp
@brief	Soapy SDR + IConnection config settings.
@author Lime Microsystems (www.limemicro.com)
*/

#include "SoapyLMS7.h"
#include <IConnection.h>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <LMS7002M.h>
#include <LMS7002M_RegistersMap.h>
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Time.hpp>
#include <cstdlib>

using namespace lime;

#define dirName ((direction == SOAPY_SDR_RX)?"Rx":"Tx")

//lazy fix for the const call issue -- FIXME
#define _accessMutex const_cast<std::recursive_mutex &>(_accessMutex)

bool skipRxCalibration = false;
bool skipTxCalibration = false;
bool calibrateOnce = false;

/*******************************************************************
 * Special LMS7002M with log forwarding
 ******************************************************************/
class LMS7002M_withLogging : public LMS7002M
{
public:
    LMS7002M_withLogging(void):
        LMS7002M()
    {
        //SoapySDR::setLogLevel(SOAPY_SDR_DEBUG);
        return;
    }

protected:
    void Log(const char* text, LogType type)
    {
        switch(type)
        {
        case LOG_INFO: SoapySDR::log(SOAPY_SDR_INFO, text); break;
        case LOG_WARNING: SoapySDR::log(SOAPY_SDR_WARNING, text); break;
        case LOG_ERROR: SoapySDR::log(SOAPY_SDR_ERROR, text); break;
        case LOG_DATA: SoapySDR::log(SOAPY_SDR_DEBUG, text); break;
        }
    }
};

/*******************************************************************
 * Constructor/destructor
 ******************************************************************/
SoapyLMS7::SoapyLMS7(const ConnectionHandle &handle, const SoapySDR::Kwargs &args):
    _conn(nullptr),
    _moduleName(handle.module)
{
    //connect
    SoapySDR::logf(SOAPY_SDR_INFO, "Make connection: '%s'", handle.ToString().c_str());
    _conn = ConnectionRegistry::makeConnection(handle);
    if (_conn == nullptr) throw std::runtime_error(
        "Failed to make connection with '" + handle.serialize() + "'");

    //device info
    const auto devInfo = _conn->GetDeviceInfo();
    const size_t numRFICs = devInfo.addrsLMS7002M.size();

    //quick summary
    SoapySDR::logf(SOAPY_SDR_INFO, "Device name: %s", devInfo.deviceName.c_str());
    SoapySDR::logf(SOAPY_SDR_INFO, "Reference: %f MHz", _conn->GetReferenceClockRate()/1e6);

    //disable cal hooks during setup
    std::map<size_t, std::shared_ptr<LMS7002M_SelfCalState>> calStates;

    //LMS7002M driver for each RFIC
    for (size_t i = 0; i < numRFICs; i++)
    {
        SoapySDR::logf(SOAPY_SDR_INFO, "Init LMS7002M(%d)", int(i));
        _rfics.push_back(new LMS7002M_withLogging());
        _rfics.back()->SetConnection(_conn, i);
        SoapySDR::logf(SOAPY_SDR_INFO, "Ver=%d, Rev=%d, Mask=%d",
            _rfics.back()->Get_SPI_Reg_bits(VER, true),
            _rfics.back()->Get_SPI_Reg_bits(REV, true),
            _rfics.back()->Get_SPI_Reg_bits(MASK, true));

        liblms7_status st;

        st = _rfics.back()->ResetChip();
        if (st != LIBLMS7_SUCCESS) throw std::runtime_error("ResetChip() failed");

        st = _rfics.back()->SoftReset();
        if (st != LIBLMS7_SUCCESS) throw std::runtime_error("SoftReset() failed");

        st = _rfics.back()->DownloadAll();
        if (st != LIBLMS7_SUCCESS) throw std::runtime_error("DownloadAll() failed");

        calStates[i].reset(new LMS7002M_SelfCalState(_rfics.back()));
    }

    //enable all channels
    for (size_t channel = 0; channel < _rfics.size()*2; channel++)
    {
        auto rfic = getRFIC(channel);
        rfic->EnableChannel(LMS7002M::Tx, true);
        rfic->EnableChannel(LMS7002M::Rx, true);
    }

    //enable use of calibration value cache
    if (args.count("cacheCalibrations"))
    {
        printf("Search cache parameter\n");
        bool enable = std::stoi(args.at("cacheCalibrations")) != 0;
        SoapySDR::logf(SOAPY_SDR_INFO, "LMS7002M calibration values caching %s", enable?"Enable":"Disable");
        for (size_t i=0; i<_rfics.size(); ++i)
            _rfics[i]->EnableValuesCache(enable);
    }

    //give all RFICs a default state
    double defaultClockRate = DEFAULT_CLOCK_RATE;
    if (args.count("clock")) defaultClockRate = std::stod(args.at("clock"));
    this->setMasterClockRate(defaultClockRate);
    for (size_t channel = 0; channel < _rfics.size()*2; channel++)
    {
        this->setFrequency(SOAPY_SDR_RX, channel, "BB", 0.0);
        this->setFrequency(SOAPY_SDR_TX, channel, "BB", 0.0);
        this->setAntenna(SOAPY_SDR_RX, channel, "LNAL");
        this->setAntenna(SOAPY_SDR_TX, channel, "BAND1");
        this->setGain(SOAPY_SDR_RX, channel, "PGA", 0);
        this->setGain(SOAPY_SDR_RX, channel, "LNA", 0);
        this->setGain(SOAPY_SDR_RX, channel, "TIA", 0);
        this->setGain(SOAPY_SDR_TX, channel, "PAD", -50);
        this->setSampleRate(SOAPY_SDR_RX, channel, defaultClockRate/8);
        this->setSampleRate(SOAPY_SDR_TX, channel, defaultClockRate/8);
        this->setBandwidth(SOAPY_SDR_RX, channel, 30e6);
        this->setBandwidth(SOAPY_SDR_TX, channel, 30e6);
    }

    if(args.count("calibrateOnce"))
    {
        calibrateOnce = std::stoi(args.at("calibrateOnce"));
        if(calibrateOnce)
            SoapySDR::logf(SOAPY_SDR_INFO, "Rx/Tx will be calibrated only once");
    }

    //also triggers internal stream threads ~ its hacky
    _conn->SetHardwareTimestamp(0);
}

SoapyLMS7::~SoapyLMS7(void)
{
    //power down all channels
    for (size_t channel = 0; channel < _rfics.size()*2; channel++)
    {
        auto rfic = getRFIC(channel);
        rfic->EnableChannel(LMS7002M::Tx, false);
        rfic->EnableChannel(LMS7002M::Rx, false);
    }

    for (auto rfic : _rfics) delete rfic;
    ConnectionRegistry::freeConnection(_conn);
}

LMS7002M *SoapyLMS7::getRFIC(const size_t channel) const
{
    if (_rfics.size() <= channel/2)
    {
        throw std::out_of_range("SoapyLMS7::getRFIC("+std::to_string(channel)+") out of range");
    }
    auto rfic = _rfics.at(channel/2);
    rfic->SetActiveChannel(((channel%2) == 0)?LMS7002M::ChA:LMS7002M::ChB);
    return rfic;
}

/*******************************************************************
 * Identification API
 ******************************************************************/
std::string SoapyLMS7::getDriverKey(void) const
{
    return _moduleName;
}

std::string SoapyLMS7::getHardwareKey(void) const
{
    return _conn->GetDeviceInfo().deviceName;
}

SoapySDR::Kwargs SoapyLMS7::getHardwareInfo(void) const
{
    auto devinfo = _conn->GetDeviceInfo();
    SoapySDR::Kwargs info;
    if (devinfo.expansionName != "UNSUPPORTED")
        info["expansionName"] = devinfo.expansionName;
    info["firmwareVersion"] = devinfo.firmwareVersion;
    info["hardwareVersion"] = devinfo.hardwareVersion;
    info["protocolVersion"] = devinfo.protocolVersion;
    if (devinfo.boardSerialNumber != unsigned(-1))
    {
        char buff[64]; sprintf(buff, "0x%x", devinfo.boardSerialNumber);
        info["boardSerialNumber"] = buff;
    }
    return info;
}

/*******************************************************************
 * Channels API
 ******************************************************************/

size_t SoapyLMS7::getNumChannels(const int /*direction*/) const
{
    return _rfics.size()*2;
}

bool SoapyLMS7::getFullDuplex(const int /*direction*/, const size_t /*channel*/) const
{
    return true;
}

/*******************************************************************
 * Antenna API
 ******************************************************************/

std::vector<std::string> SoapyLMS7::listAntennas(const int direction, const size_t /*channel*/) const
{
    std::vector<std::string> ants;
    if (direction == SOAPY_SDR_RX)
    {
        ants.push_back("NONE");
        ants.push_back("LNAH");
        ants.push_back("LNAL");
        ants.push_back("LNAW");
        ants.push_back("LB1");
        ants.push_back("LB2");
    }
    if (direction == SOAPY_SDR_TX)
    {
        ants.push_back("NONE");
        ants.push_back("BAND1");
        ants.push_back("BAND2");
    }
    return ants;
}

void SoapyLMS7::setAntenna(const int direction, const size_t channel, const std::string &name)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    SoapySDR::logf(SOAPY_SDR_INFO, "SoapyLMS7::setAntenna(%s, %d, %s)", dirName, int(channel), name.c_str());
    auto rfic = getRFIC(channel);

    if (direction == SOAPY_SDR_RX)
    {
        LMS7002M::PathRFE path = LMS7002M::PATH_RFE_NONE;
        if (name == "NONE") path = LMS7002M::PATH_RFE_NONE;
        else if (name == "LNAH") path = LMS7002M::PATH_RFE_LNAH;
        else if (name == "LNAL") path = LMS7002M::PATH_RFE_LNAL;
        else if (name == "LNAW") path = LMS7002M::PATH_RFE_LNAW;
        else if (name == "LB1") path = LMS7002M::PATH_RFE_LB1;
        else if (name == "LB2") path = LMS7002M::PATH_RFE_LB2;
        else throw std::runtime_error("SoapyLMS7::setAntenna(RX, "+name+") - unknown antenna name");

        rfic->SetPathRFE(path);
    }

    if (direction == SOAPY_SDR_TX)
    {
        int band = 0;
        if (name == "NONE") band = 0;
        else if (name == "BAND1") band = 1;
        else if (name == "BAND2") band = 2;
        else throw std::runtime_error("SoapyLMS7::setAntenna(TX, "+name+") - unknown antenna name");

        rfic->SetBandTRF(band);
    }
}

std::string SoapyLMS7::getAntenna(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);

    if (direction == SOAPY_SDR_RX)
    {
        switch (rfic->GetPathRFE())
        {
        case LMS7002M::PATH_RFE_NONE: return "NONE";
        case LMS7002M::PATH_RFE_LNAH: return "LNAH";
        case LMS7002M::PATH_RFE_LNAL: return "LNAL";
        case LMS7002M::PATH_RFE_LNAW: return "LNAW";
        case LMS7002M::PATH_RFE_LB1: return "LB1";
        case LMS7002M::PATH_RFE_LB2: return "LB2";
        }
    }

    if (direction == SOAPY_SDR_TX)
    {
        switch (rfic->GetBandTRF())
        {
        case 1: return "BAND1";
        case 2: return "BAND2";
        default: return "NONE";
        }
    }

    return "";
}

/*******************************************************************
 * Frontend corrections API
 ******************************************************************/

bool SoapyLMS7::hasDCOffsetMode(const int direction, const size_t /*channel*/) const
{
    return (direction == SOAPY_SDR_RX);
}

void SoapyLMS7::setDCOffsetMode(const int direction, const size_t channel, const bool automatic)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);

    if (direction == SOAPY_SDR_RX) rfic->SetRxDCRemoval(automatic);
}

bool SoapyLMS7::getDCOffsetMode(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);

    if (direction == SOAPY_SDR_RX) return rfic->GetRxDCRemoval();

    return false;
}

bool SoapyLMS7::hasDCOffset(const int direction, const size_t /*channel*/) const
{
    return (direction == SOAPY_SDR_TX);
}

void SoapyLMS7::setDCOffset(const int direction, const size_t channel, const std::complex<double> &offset)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);

    if (direction == SOAPY_SDR_TX) rfic->SetTxDCOffset(offset.real(), offset.imag());
}

std::complex<double> SoapyLMS7::getDCOffset(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);

    double I = 0.0, Q = 0.0;
    if (direction == SOAPY_SDR_TX) rfic->GetTxDCOffset(I, Q);
    return std::complex<double>(I, Q);
}

bool SoapyLMS7::hasIQBalance(const int /*direction*/, const size_t /*channel*/) const
{
    return true;
}

void SoapyLMS7::setIQBalance(const int direction, const size_t channel, const std::complex<double> &balance)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);
    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;

    double gain = std::abs(balance);
    double gainI = 1.0; if (gain < 1.0) gainI = gain/1.0;
    double gainQ = 1.0; if (gain > 1.0) gainQ = 1.0/gain;
    rfic->SetIQBalance(lmsDir, std::arg(balance), gainI, gainQ);
}

std::complex<double> SoapyLMS7::getIQBalance(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);
    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;

    double phase, gainI, gainQ;
    rfic->GetIQBalance(lmsDir, phase, gainI, gainQ);
    return (gainI/gainQ)*std::polar(1.0, phase);
}

/*******************************************************************
 * Gain API
 ******************************************************************/

std::vector<std::string> SoapyLMS7::listGains(const int direction, const size_t /*channel*/) const
{
    std::vector<std::string> gains;
    if (direction == SOAPY_SDR_RX)
    {
        gains.push_back("LNA");
        gains.push_back("TIA");
        gains.push_back("PGA");
    }
    if (direction == SOAPY_SDR_TX)
    {
        gains.push_back("PAD");
    }
    return gains;
}

void SoapyLMS7::setGain(const int direction, const size_t channel, const std::string &name, const double value)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    SoapySDR::logf(SOAPY_SDR_INFO, "SoapyLMS7::setGain(%s, %d, %s, %f dB)", dirName, int(channel), name.c_str(), value);
    auto rfic = getRFIC(channel);

    if (direction == SOAPY_SDR_RX and name == "LNA")
    {
        rfic->SetRFELNA_dB(value);
    }

    else if (direction == SOAPY_SDR_RX and name == "LB_LNA")
    {
        rfic->SetRFELoopbackLNA_dB(value);
    }

    else if (direction == SOAPY_SDR_RX and name == "LB_LNA")
    {
        rfic->SetRFELoopbackLNA_dB(value);
    }

    else if (direction == SOAPY_SDR_RX and name == "TIA")
    {
        rfic->SetRFETIA_dB(value);
    }

    else if (direction == SOAPY_SDR_RX and name == "PGA")
    {
        rfic->SetRBBPGA_dB(value);
    }

    else if (direction == SOAPY_SDR_TX and name == "PAD")
    {
        rfic->SetTRFPAD_dB(value);
    }

    else if (direction == SOAPY_SDR_TX and name == "LB_PAD")
    {
        rfic->SetTRFLoopbackPAD_dB(value);
    }

    else if (direction == SOAPY_SDR_TX and name == "LB_PAD")
    {
        rfic->SetTRFLoopbackPAD_dB(value);
    }

    else throw std::runtime_error("SoapyLMS7::setGain("+name+") - unknown gain name");

    SoapySDR::logf(SOAPY_SDR_DEBUG, "Actual %s%s[%d] gain %f dB", dirName, name.c_str(), int(channel), this->getGain(direction, channel, name));
}

double SoapyLMS7::getGain(const int direction, const size_t channel, const std::string &name) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);

    if (direction == SOAPY_SDR_RX and name == "LNA")
    {
        return rfic->GetRFELNA_dB();
    }

    else if (direction == SOAPY_SDR_RX and name == "LB_LNA")
    {
        return rfic->GetRFELoopbackLNA_dB();
    }

    else if (direction == SOAPY_SDR_RX and name == "TIA")
    {
        return rfic->GetRFETIA_dB();
    }

    else if (direction == SOAPY_SDR_RX and name == "PGA")
    {
        return rfic->GetRBBPGA_dB();
    }

    else if (direction == SOAPY_SDR_TX and name == "PAD")
    {
        return rfic->GetTRFPAD_dB();
    }

    else if (direction == SOAPY_SDR_TX and name == "LB_PAD")
    {
        return rfic->GetTRFLoopbackPAD_dB();
    }

    else throw std::runtime_error("SoapyLMS7::getGain("+name+") - unknown gain name");
}

SoapySDR::Range SoapyLMS7::getGainRange(const int direction, const size_t channel, const std::string &name) const
{
    if (direction == SOAPY_SDR_RX and name == "LNA") return SoapySDR::Range(0.0, 30.0);
    if (direction == SOAPY_SDR_RX and name == "LB_LNA") return SoapySDR::Range(0.0, 40.0);
    if (direction == SOAPY_SDR_RX and name == "TIA") return SoapySDR::Range(0.0, 12.0);
    if (direction == SOAPY_SDR_RX and name == "PGA") return SoapySDR::Range(-12.0, 19.0);
    if (direction == SOAPY_SDR_TX and name == "PAD") return SoapySDR::Range(-52.0, 0.0);
    if (direction == SOAPY_SDR_TX and name == "LB_PAD") return SoapySDR::Range(-4.3, 0.0);
    return SoapySDR::Device::getGainRange(direction, channel, name);
}

/*******************************************************************
 * Frequency API
 ******************************************************************/

void SoapyLMS7::setFrequency(const int direction, const size_t channel, const std::string &name, const double frequency, const SoapySDR::Kwargs &args)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);
    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;
    SoapySDR::logf(SOAPY_SDR_INFO, "SoapyLMS7::setFrequency(%s, %d, %s, %f MHz)", dirName, int(channel), name.c_str(), frequency/1e6);

    if (name == "RF")
    {
        //clip the frequency into the allowed range
        double targetRfFreq = frequency;
        if (targetRfFreq < 30e6) targetRfFreq = 30e6;
        if (targetRfFreq > 3.8e9) targetRfFreq = 3.8e9;
        rfic->SetFrequencySX(lmsDir, targetRfFreq/1e6);
        //TODO -- new corrections cache api here
        return;
    }

    if (name == "BB")
    {
        switch (direction)
        {
        case SOAPY_SDR_RX: rfic->Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, (frequency == 0)?1:0);
        case SOAPY_SDR_TX: rfic->Modify_SPI_Reg_bits(CMIX_BYP_TXTSP, (frequency == 0)?1:0);
        }
        rfic->SetNCOFrequency(lmsDir, 0, frequency/1e6);
        return;
    }

    throw std::runtime_error("SoapyLMS7::getFrequency("+name+") unknown name");
}

double SoapyLMS7::getFrequency(const int direction, const size_t channel, const std::string &name) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);
    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;

    if (name == "RF")
    {
        return rfic->GetFrequencySX_MHz(lmsDir)*1e6;
    }

    if (name == "BB")
    {
        return rfic->GetNCOFrequency_MHz(lmsDir, 0)*1e6;
    }

    throw std::runtime_error("SoapyLMS7::getFrequency("+name+") unknown name");
}

std::vector<std::string> SoapyLMS7::listFrequencies(const int /*direction*/, const size_t /*channel*/) const
{
    std::vector<std::string> opts;
    opts.push_back("RF");
    opts.push_back("BB");
    return opts;
}

SoapySDR::RangeList SoapyLMS7::getFrequencyRange(const int direction, const size_t channel, const std::string &name) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);
    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;

    SoapySDR::RangeList ranges;
    if (name == "RF")
    {
        ranges.push_back(SoapySDR::Range(30e6, 3.8e9));
    }
    if (name == "BB")
    {
        const double dspRate = rfic->GetReferenceClk_TSP_MHz(lmsDir)*1e6;
        ranges.push_back(SoapySDR::Range(-dspRate/2, dspRate/2));
    }
    return ranges;
}

/*******************************************************************
 * Sample Rate API
 ******************************************************************/

void SoapyLMS7::setSampleRate(const int direction, const size_t channel, const double rate)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);
    LMS7002M_SelfCalState state(rfic);
    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;

    const double dspRate = rfic->GetReferenceClk_TSP_MHz(lmsDir)*1e6;
    const double factor = dspRate/rate;
    int intFactor = 1 << int((std::log(factor)/std::log(2.0)) + 0.5);
    SoapySDR::logf(SOAPY_SDR_INFO, "SoapyLMS7::setSampleRate(%s, %d, %f MHz), baseRate %f MHz, factor %f", dirName, int(channel), rate/1e6, dspRate/1e6, factor);
    if (intFactor < 2) throw std::runtime_error("SoapyLMS7::setSampleRate() -- rate too high");
    if (intFactor > 32) throw std::runtime_error("SoapyLMS7::setSampleRate() -- rate too low");

    if (std::abs(factor-intFactor) > 0.01) SoapySDR::logf(SOAPY_SDR_WARNING,
        "SoapyLMS7::setSampleRate(): not a power of two factor: TSP Rate = %f MHZ, Requested rate = %f MHz", dspRate/1e6, rate/1e6);

    switch (direction)
    {
    case SOAPY_SDR_TX: _interps[channel] = intFactor; break;
    case SOAPY_SDR_RX: _decims[channel] = intFactor; break;
    }

    //when only setting the tx rate, also set rx if its not already
    //this is a workaround https://github.com/limemicro/lms7suite/issues/29
    if (direction == SOAPY_SDR_TX and _decims.count(channel) == 0)
    {
        this->setSampleRate(SOAPY_SDR_RX, channel, rate);
    }

    rfic->SetInterfaceFrequency(
        this->getMasterClockRate()/1e6,
        int(std::log(double(_interps[channel]))/std::log(2.0))-1,
        int(std::log(double(_decims[channel]))/std::log(2.0))-1);
}

double SoapyLMS7::getSampleRate(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);
    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;

    return rfic->GetSampleRate(lmsDir);
}

std::vector<double> SoapyLMS7::listSampleRates(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);
    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;

    const double dspRate = rfic->GetReferenceClk_TSP_MHz(lmsDir)*1e6;
    std::vector<double> rates;
    for (int i = 5; i >= 0; i--)
    {
        rates.push_back(dspRate/(1 << i));
    }
    return rates;
}

/*******************************************************************
 * Bandwidth API
 ******************************************************************/

void SoapyLMS7::setBandwidth(const int direction, const size_t channel, const double bw)
{
    SoapySDR::logf(SOAPY_SDR_INFO, "SoapyLMS7::setBandwidth(%s, %d, %f MHz)", dirName, int(channel), bw/1e6);

    //save dc offset mode
    auto saveDcMode = this->getDCOffsetMode(direction, channel);

    auto rfic = getRFIC(channel);
    LMS7002M_SelfCalState state(rfic);

    _actualBw[direction][channel] = bw;

    if (direction == SOAPY_SDR_RX)
    {
        const bool hb = bw >= 20.0e6;
        const bool bypass = bw > 60.0e6;

        //run the calibration for this bandwidth setting
        //SoapySDR::log(SOAPY_SDR_DEBUG, "CalibrateRx(...)");
        auto status = LIBLMS7_SUCCESS;//rfic->CalibrateRx(bw/1e6);
        if (status == LIBLMS7_SUCCESS)
        {
            SoapySDR::log(SOAPY_SDR_DEBUG, "TuneRxFilter(RX_TIA)");
            status = rfic->TuneRxFilter(LMS7002M::RX_TIA, bw/1e6);
        }
        if (!bypass && status == LIBLMS7_SUCCESS)
        {
            LMS7002M::RxFilter filter;
            if (hb)
            {
                SoapySDR::log(SOAPY_SDR_DEBUG, "TuneRxFilter(RX_LPF_HIGHBAND)");
                filter = LMS7002M::RX_LPF_HIGHBAND;
            }
            else
            {
                SoapySDR::log(SOAPY_SDR_DEBUG, "TuneRxFilter(RX_LPF_LOWBAND)");
                filter = LMS7002M::RX_LPF_LOWBAND;
            }

            status = rfic->TuneRxFilter(filter, bw/1e6);
        }
        if (status != LIBLMS7_SUCCESS)
        {
            SoapySDR::logf(SOAPY_SDR_ERROR, "setBandwidth(Rx, %d, %f MHz) Failed - %s", int(channel), bw/1e6, liblms7_status2string(status));
        }
    }

    if (direction == SOAPY_SDR_TX)
    {
        const bool hb = bw >= 18.5e6;
        const bool bypass = bw > 54.0e6;

        //run the calibration for this bandwidth setting
        //SoapySDR::log(SOAPY_SDR_DEBUG, "CalibrateTx(...)");
        auto status = LIBLMS7_SUCCESS;//rfic->CalibrateTx(bw/1e6);
        if (!bypass && status == LIBLMS7_SUCCESS)
        {
            LMS7002M::TxFilter filter;
            if (hb)
            {
                SoapySDR::log(SOAPY_SDR_DEBUG, "TuneTxFilter(TX_HIGHBAND)");
                filter = LMS7002M::TX_HIGHBAND;
            }
            else if (bw >= 2.4e6)
            {
                SoapySDR::log(SOAPY_SDR_DEBUG, "TuneTxFilter(TX_LADDER)");
                filter = LMS7002M::TX_LADDER;
            }
            else
            {
                SoapySDR::log(SOAPY_SDR_DEBUG, "TuneTxFilter(TX_REALPOLE)");
                filter = LMS7002M::TX_REALPOLE;
            }

            status = rfic->TuneTxFilter(filter, bw/1e6);
        }
        if (status != LIBLMS7_SUCCESS)
        {
            SoapySDR::logf(SOAPY_SDR_ERROR, "setBandwidth(Tx, %d, %f MHz) Failed - %s", int(channel), bw/1e6, liblms7_status2string(status));
        }
    }

    //restore dc offset mode
    this->setDCOffsetMode(direction, channel, saveDcMode);
}

double SoapyLMS7::getBandwidth(const int direction, const size_t channel) const
{
    try
    {
        return _actualBw.at(direction).at(channel);
    }
    catch (...)
    {
        return 1.0;
    }
}

SoapySDR::RangeList SoapyLMS7::getBandwidthRange(const int direction, const size_t channel) const
{
    SoapySDR::RangeList bws;

    if (direction == SOAPY_SDR_RX)
    {
        bws.push_back(SoapySDR::Range(1e6, 60e6));
    }
    if (direction == SOAPY_SDR_TX)
    {
        bws.push_back(SoapySDR::Range(0.8e6, 16e6));
        bws.push_back(SoapySDR::Range(28e6, 60e6));
    }

    return bws;
}

/*******************************************************************
 * Clocking API
 ******************************************************************/

void SoapyLMS7::setMasterClockRate(const double rate)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    for (auto rfic : _rfics)
    {
        //make tx rx rates equal
        rfic->Modify_SPI_Reg_bits(EN_ADCCLKH_CLKGN, 0);
        rfic->Modify_SPI_Reg_bits(CLKH_OV_CLKL_CGEN, 2);
        rfic->SetFrequencyCGEN(rate/1e6);
    }
}

double SoapyLMS7::getMasterClockRate(void) const
{
    auto rfic = this->getRFIC(0); //same for all RFIC
    return rfic->GetFrequencyCGEN_MHz()*1e6;
}

/*******************************************************************
 * Time API
 ******************************************************************/

bool SoapyLMS7::hasHardwareTime(const std::string &what) const
{
    //assume hardware time when no argument is specified
    //some boards may not ever support hw time, so TODO
    return what.empty();
}

long long SoapyLMS7::getHardwareTime(const std::string &what) const
{
    if (what.empty())
    {
        auto ticks = _conn->GetHardwareTimestamp();
        auto rate = _conn->GetHardwareTimestampRate();
        return SoapySDR::ticksToTimeNs(ticks, rate);
    }
    else
    {
        throw std::invalid_argument("SoapyLMS7::getHardwareTime("+what+") unknown argument");
    }
}

void SoapyLMS7::setHardwareTime(const long long timeNs, const std::string &what)
{
    if (what.empty())
    {
        auto rate = _conn->GetHardwareTimestampRate();
        auto ticks = SoapySDR::timeNsToTicks(timeNs, rate);
        _conn->SetHardwareTimestamp(ticks);
    }
    else
    {
        throw std::invalid_argument("SoapyLMS7::setHardwareTime("+what+") unknown argument");
    }
}

/*******************************************************************
 * Register API
 ******************************************************************/

void SoapyLMS7::writeRegister(const unsigned addr, const unsigned value)
{
    auto st = _conn->WriteRegister(addr, value);
    if (st != OperationStatus::SUCCESS) throw std::runtime_error(
        "SoapyLMS7::WriteRegister("+std::to_string(addr)+") FAIL");
}

unsigned SoapyLMS7::readRegister(const unsigned addr) const
{
    unsigned readbackData = 0;
    auto st = _conn->ReadRegister(addr, readbackData);
    if (st != OperationStatus::SUCCESS) throw std::runtime_error(
        "SoapyLMS7::ReadRegister("+std::to_string(addr)+") FAIL");
    return readbackData;
}

/*******************************************************************
 * Settings API
 ******************************************************************/
SoapySDR::ArgInfoList SoapyLMS7::getSettingInfo(void) const
{
    SoapySDR::ArgInfoList infos;

    return infos;
}

void SoapyLMS7::writeSetting(const std::string &key, const std::string &value)
{
    if (key == "RXTSP_CONST")
    {
        for (size_t channel = 0; channel < _rfics.size()*2; channel++)
        {
            this->writeSetting(SOAPY_SDR_RX, channel, "TSP_CONST", value);
        }
    }

    if (key == "TXTSP_CONST")
    {
        for (size_t channel = 0; channel < _rfics.size()*2; channel++)
        {
            this->writeSetting(SOAPY_SDR_TX, channel, "TSP_CONST", value);
        }
    }
}

void SoapyLMS7::writeSetting(const int direction, const size_t channel, const std::string &key, const std::string &value)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);
    const bool isTx = (direction == SOAPY_SDR_TX);

    if (key == "TSP_CONST")
    {
        const auto ampl = std::stoi(value);
        rfic->Modify_SPI_Reg_bits(isTx?TSGFC_TXTSP:TSGFC_RXTSP, 1); //Full-scale
        rfic->Modify_SPI_Reg_bits(isTx?TSGMODE_TXTSP:TSGMODE_RXTSP, 1); //DC
        rfic->Modify_SPI_Reg_bits(isTx?INSEL_TXTSP:INSEL_RXTSP, 1); //SIGGEN
        rfic->LoadDC_REG_IQ(isTx, ampl, 0);
    }
}

/*******************************************************************
 * I2C API
 ******************************************************************/
void SoapyLMS7::writeI2C(const int addr, const std::string &data)
{
    auto st = _conn->WriteI2C(addr, data);
    if (st != OperationStatus::SUCCESS) throw std::runtime_error(
        "SoapyLMS7::writeI2C("+std::to_string(addr)+") FAIL");
}

std::string SoapyLMS7::readI2C(const int addr, const size_t numBytes)
{
    std::string result;
    auto st = _conn->ReadI2C(addr, numBytes, result);
    if (st != OperationStatus::SUCCESS) throw std::runtime_error(
        "SoapyLMS7::readI2C("+std::to_string(addr)+") FAIL");
    return result;
}

/*******************************************************************
 * SPI API
 ******************************************************************/
unsigned SoapyLMS7::transactSPI(const int addr, const unsigned data, const size_t /*numBits*/)
{
    uint32_t input = data;
    uint32_t readback = 0;
    auto st = _conn->TransactSPI(addr, &input, &readback, 1);
    if (st != OperationStatus::SUCCESS) throw std::runtime_error(
        "SoapyLMS7::transactSPI("+std::to_string(addr)+") FAIL");
    return readback;
}
