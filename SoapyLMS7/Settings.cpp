/**
@file   Settings.cpp
@brief  Soapy SDR + IConnection config settings.
@author Lime Microsystems (www.limemicro.com)
*/

#include "SoapyLMS7.h"
#include "ErrorReporting.h"
#include "lms7_device.h"
#include <IConnection.h>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Time.hpp>
#include <cstdlib>
#include <algorithm>

using namespace lime;

#define dirName ((direction == SOAPY_SDR_RX)?"Rx":"Tx")

// arbitrary upper limit for CGEN automatic tune
#define MIN_CGEN_RATE 4e6
#define MAX_CGEN_RATE 640e6

//reasonable limits when advertising the rate
#define MIN_SAMP_RATE 1e5
#define MAX_SAMP_RATE 65e6

/*******************************************************************
 * Constructor/destructor
 ******************************************************************/
SoapyLMS7::SoapyLMS7(const ConnectionHandle &handle, const SoapySDR::Kwargs &args):
    _deviceArgs(args),
    _moduleName(handle.module),
    sampleRate(0.0)
{
    //connect
    SoapySDR::logf(SOAPY_SDR_INFO, "Make connection: '%s'", handle.ToString().c_str());
    auto conn = ConnectionRegistry::makeConnection(handle);
    if (conn == nullptr) throw std::runtime_error(
        "Failed to make connection with '" + handle.serialize() + "'");

    //device info
    lms7Device = LMS7_Device::CreateDevice(conn,nullptr);
    const auto devInfo = lms7Device->GetInfo();

    //quick summary
    SoapySDR::logf(SOAPY_SDR_INFO, "Device name: %s", devInfo->deviceName);
    SoapySDR::logf(SOAPY_SDR_INFO, "Reference: %g MHz", lms7Device->GetClockFreq(LMS_CLOCK_REF));

    lms7Device->Init();

    //enable all channels
    for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
    {
        lms7Device->EnableChannel(true, channel, true);   
        lms7Device->EnableChannel(false, channel, true);
    }

    //enable use of calibration value cache automatically
    //or specify args[cacheCalibrations] == 0 to disable
    const bool cacheEnable = args.count("cacheCalibrations") and std::stoi(args.at("cacheCalibrations")) != 0;
    SoapySDR::logf(SOAPY_SDR_INFO, "LMS7002M calibration values caching %s", cacheEnable?"Enable":"Disable");
    lms7Device->EnableCalibCache(cacheEnable);

    //give all RFICs a default state
    double defaultClockRate = DEFAULT_CLOCK_RATE;
    if (args.count("clock")) defaultClockRate = std::stod(args.at("clock"));
    this->setMasterClockRate(defaultClockRate);
    for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
    {
        this->setGain(SOAPY_SDR_RX, channel, "LNA", 0);
        this->setGain(SOAPY_SDR_TX, channel, "PAD", 0);
    }

    //reset flags for user calls
    _fixedClockRate = args.count("clock") != 0;
    _fixedRxSampRate.clear();
    _fixedTxSampRate.clear();
    _channelsToCal.clear();
}

SoapyLMS7::~SoapyLMS7(void)
{
    //power down all channels
    for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
    {
        lms7Device->EnableChannel(true, channel, false);   
        lms7Device->EnableChannel(false, channel, false);
    }
    delete lms7Device;
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
    return std::string(lms7Device->GetInfo()->deviceName);
}

SoapySDR::Kwargs SoapyLMS7::getHardwareInfo(void) const
{
    auto devinfo = lms7Device->GetInfo();
    SoapySDR::Kwargs info;
    if (std::string(devinfo->expansionName) != "UNSUPPORTED")
        info["expansionName"] = std::string(devinfo->expansionName);
    info["firmwareVersion"] = std::string(devinfo->firmwareVersion);
    info["hardwareVersion"] = std::string(devinfo->hardwareVersion);
    info["protocolVersion"] = std::string(devinfo->protocolVersion);
    info["gatewareVersion"] = std::string(devinfo->gatewareVersion);
    if (devinfo->boardSerialNumber!= unsigned(-1))
    {
        char buff[64]; sprintf(buff, "0x%lx", devinfo->boardSerialNumber);
        info["boardSerialNumber"] = buff;
    }
    return info;
}

/*******************************************************************
 * Channels API
 ******************************************************************/

size_t SoapyLMS7::getNumChannels(const int /*direction*/) const
{
    return lms7Device->GetNumChannels();
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
    return lms7Device->GetPathNames(direction == SOAPY_SDR_TX);
}

void SoapyLMS7::setAntenna(const int direction, const size_t channel, const std::string &name)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    
    SoapySDR::logf(SOAPY_SDR_DEBUG, "SoapyLMS7::setAntenna(%s, %d, %s)", dirName, int(channel), name.c_str());
    
    bool tx = direction == SOAPY_SDR_TX;
    std::vector<std::string> nameList = lms7Device->GetPathNames(tx);
    for (unsigned path = 0; path < nameList.size(); path++)
        if (nameList[path] == name)
        {
            lms7Device->SetPath(tx, channel, path);
            _channelsToCal.emplace(direction, channel);
            return;
        }
    
    throw std::runtime_error("SoapyLMS7::setAntenna(TX, "+name+") - unknown antenna name");
}

std::string SoapyLMS7::getAntenna(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    bool tx = direction == SOAPY_SDR_TX;
    int path = lms7Device->GetPath(tx,channel);
    if (path < 0)
        return "";
   
    std::vector<std::string> nameList = lms7Device->GetPathNames(tx);
    return (unsigned)path < nameList.size() ? nameList[path] : "";
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
    auto rfic = lms7Device->GetLMS(channel/2);

    if (direction == SOAPY_SDR_RX) rfic->SetRxDCRemoval(automatic);
}

bool SoapyLMS7::getDCOffsetMode(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = lms7Device->GetLMS(channel/2);
    
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
    auto rfic = lms7Device->GetLMS(channel/2);

    if (direction == SOAPY_SDR_TX) rfic->SetTxDCOffset(offset.real(), offset.imag());
    }

std::complex<double> SoapyLMS7::getDCOffset(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = lms7Device->GetLMS(channel/2);

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
    auto rfic = lms7Device->GetLMS(channel/2);
    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;
    
    double gain = std::abs(balance);
    double gainI = 1.0; if (gain < 1.0) gainI = gain/1.0;
    double gainQ = 1.0; if (gain > 1.0) gainQ = 1.0/gain;
    rfic->SetIQBalance(lmsDir, std::arg(balance), gainI, gainQ);
}

std::complex<double> SoapyLMS7::getIQBalance(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = lms7Device->GetLMS(channel/2);
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
        gains.push_back("TIA");
        gains.push_back("LNA");
        gains.push_back("PGA");
    }
    if (direction == SOAPY_SDR_TX)
    {
        gains.push_back("PAD");
        gains.push_back("IAMP");
    }
    return gains;
}

void SoapyLMS7::setGain(const int direction, const size_t channel, const double value)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);

    //Distribute Rx gain from elements in the direction of RFE to RBB
    //This differs from the default gain distribution in that it
    //does not scale the gain to the negative range of the PGA.
    //This keep the PGA in mid-range unless extreme values are used.
    double remaining(value);
    for (const auto &name : this->listGains(direction, channel))
    {
        this->setGain(direction, channel, name, remaining);
        remaining -= this->getGain(direction, channel, name);
    }
}

double SoapyLMS7::getGain(const int direction, const size_t channel) const
{
    return SoapySDR::Device::getGain(direction, channel)-12.0;
}

void SoapyLMS7::setGain(const int direction, const size_t channel, const std::string &name, const double value)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    SoapySDR::logf(SOAPY_SDR_DEBUG, "SoapyLMS7::setGain(%s, %d, %s, %g dB)", dirName, int(channel), name.c_str(), value);
    auto rfic = lms7Device->GetLMS(channel/2);

    if (direction == SOAPY_SDR_RX and name == "LNA")
    {
        rfic->SetRFELNA_dB(value);
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
    
    else if (direction == SOAPY_SDR_TX and name == "IAMP")
    {
        rfic->SetTBBIAMP_dB(value);
    }

    else if (direction == SOAPY_SDR_TX and name == "LB_PAD")
    {
        rfic->SetTRFLoopbackPAD_dB(value);
    }

    else throw std::runtime_error("SoapyLMS7::setGain("+name+") - unknown gain name");

    SoapySDR::logf(SOAPY_SDR_DEBUG, "Actual %s%s[%d] gain %g dB", dirName, name.c_str(), int(channel), this->getGain(direction, channel, name));
}

double SoapyLMS7::getGain(const int direction, const size_t channel, const std::string &name) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = lms7Device->GetLMS(channel/2);

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
    
    else if (direction == SOAPY_SDR_TX and name == "IAMP")
    {
        return rfic->GetTBBIAMP_dB();
    }

    else if (direction == SOAPY_SDR_TX and name == "LB_PAD")
    {
        return rfic->GetTRFLoopbackPAD_dB();
    }

    else throw std::runtime_error("SoapyLMS7::getGain("+name+") - unknown gain name");
}

SoapySDR::Range SoapyLMS7::getGainRange(const int direction, const size_t channel) const
{
    if (direction == SOAPY_SDR_RX)
    {
        //make it so gain of 0.0 sets PGA at its mid-range
        return SoapySDR::Range(-12.0, 19.0+12.0+30.0);
    }
    else
        return SoapySDR::Range(-12.0, 52.0+12.0);
}

SoapySDR::Range SoapyLMS7::getGainRange(const int direction, const size_t channel, const std::string &name) const
{
    if (direction == SOAPY_SDR_RX and name == "LNA") return SoapySDR::Range(0.0, 30.0);
    if (direction == SOAPY_SDR_RX and name == "LB_LNA") return SoapySDR::Range(0.0, 40.0);
    if (direction == SOAPY_SDR_RX and name == "TIA") return SoapySDR::Range(0.0, 12.0);
    if (direction == SOAPY_SDR_RX and name == "PGA") return SoapySDR::Range(-12.0, 19.0);
    if (direction == SOAPY_SDR_TX and name == "PAD") return SoapySDR::Range(0.0, 52.0);
    if (direction == SOAPY_SDR_TX and name == "IAMP") return SoapySDR::Range(-12.0, 12.0);
    if (direction == SOAPY_SDR_TX and name == "LB_PAD") return SoapySDR::Range(-4.3, 0.0);
    return SoapySDR::Device::getGainRange(direction, channel, name);
}

/*******************************************************************
 * Frequency API
 ******************************************************************/
SoapySDR::ArgInfoList SoapyLMS7::getFrequencyArgsInfo(const int direction, const size_t channel) const
{
    return SoapySDR::Device::getFrequencyArgsInfo(direction, channel);
}

void SoapyLMS7::setFrequency(const int direction, const size_t channel, const std::string &name, const double frequency, const SoapySDR::Kwargs &args)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    SoapySDR::logf(SOAPY_SDR_DEBUG, "SoapyLMS7::setFrequency(%s, %d, %s, %g MHz)", dirName, int(channel), name.c_str(), frequency/1e6);

    if (name == "RF")
    {
        if (direction == SOAPY_SDR_RX)
            lms7Device->SetTxFrequency(channel, frequency);
        else
            lms7Device->SetRxFrequency(channel, frequency);
        _channelsToCal.emplace(direction, channel);
        return;
    }

    if (name == "BB")
    {
        bool isTx = direction == SOAPY_SDR_TX;
        double ncoFreq[16] = {frequency};
        lms7Device->SetNCOFreq(isTx, channel, ncoFreq, 0);
        lms7Device->SetNCO(isTx, channel, 0, frequency < 0);
        return;
    }

    throw std::runtime_error("SoapyLMS7::setFrequency("+name+") unknown name");
}

double SoapyLMS7::getFrequency(const int direction, const size_t channel, const std::string &name) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = lms7Device->GetLMS(channel/2);


    if (name == "RF")
    {
        const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS_CLOCK_TXTSP:LMS_CLOCK_RXTSP;
        return lms7Device->GetClockFreq(lmsDir,channel);
    }

    if (name == "BB")
    {
        int sign = 0;
        int pos = 0, neg = 1;
        switch (direction)
        {
        case SOAPY_SDR_RX:
            if (rfic->Get_SPI_Reg_bits(LMS7_MASK, true) != 0) std::swap(pos, neg);
            sign = (rfic->Get_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP)) == pos) ? 1 : -1;
            break;
        case SOAPY_SDR_TX:
            sign = (rfic->Get_SPI_Reg_bits(LMS7param(CMIX_SC_TXTSP)) == pos) ? 1 : -1;
            break;
        }
        return rfic->GetNCOFrequency(direction == SOAPY_SDR_TX, 0) * sign;
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

    SoapySDR::RangeList ranges;
    if (name == "RF")
    {
        ranges.push_back(SoapySDR::Range(30e6, 3.8e9));
    }
    if (name == "BB")
    {
        const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS_CLOCK_TXTSP:LMS_CLOCK_RXTSP;
        const double dspRate = lms7Device->GetClockFreq(lmsDir,channel);
        ranges.push_back(SoapySDR::Range(-dspRate/2, dspRate/2));
    }
    return ranges;
}

SoapySDR::RangeList SoapyLMS7::getFrequencyRange(const int direction, const size_t channel) const
{
    SoapySDR::RangeList ranges;
    ranges.push_back(SoapySDR::Range(0.0, 3.8e9));
    return ranges;
}

/*******************************************************************
 * Sample Rate API
 ******************************************************************/

static bool cgenRatePossible(const double rate)
{
    if (rate > MAX_CGEN_RATE || rate < MIN_CGEN_RATE) return false;
    return true;
}

void SoapyLMS7::setSampleRate(const int direction, const size_t channel, const double rate)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    sampleRate = rate;
    lms7Device->SetRate(direction == SOAPY_SDR_TX,rate);
    return;
}

double SoapyLMS7::getSampleRate(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);

    return lms7Device->GetRate((direction == SOAPY_SDR_TX),channel);
}

std::vector<double> SoapyLMS7::_getEnumeratedRates(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = lms7Device->GetLMS(channel/2);
    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;
    std::vector<double> rates;

    const double clockRate = this->getMasterClockRate();
    const double dacFactor = clockRate/rfic->GetReferenceClk_TSP(LMS7002M::Tx);
    const double adcFactor = clockRate/rfic->GetReferenceClk_TSP(LMS7002M::Rx);
    const double dspRate = rfic->GetReferenceClk_TSP(lmsDir);
    const bool fixedRx = _fixedRxSampRate.count(channel) != 0 and _fixedRxSampRate.at(channel);
    const bool fixedTx = _fixedTxSampRate.count(channel) != 0 and _fixedTxSampRate.at(channel);

    //clock rate is fixed, only half-band chain is configurable
    if (_fixedClockRate)
    {
        for (int i = 32; i >= 2; i /= 2) rates.push_back(dspRate/i);
    }

    //special rates when looking for rx rates and tx is fixed
    //return all rates where the tx sample rate is achievable
    else if (direction == SOAPY_SDR_RX and fixedTx)
    {
        const double txRate = this->getSampleRate(SOAPY_SDR_TX, channel);
        for (int iTx = 32; iTx >= 2; iTx /= 2)
        {
            const double clockRate = txRate*dacFactor*iTx;
            if (not cgenRatePossible(clockRate)) continue;
            for (int iRx = 32; iRx >= 2; iRx /= 2)
            {
                const double rxRate = clockRate/(adcFactor*iRx);
                if (rxRate > MAX_SAMP_RATE) continue;
                if (rxRate < MIN_SAMP_RATE) continue;
                rates.push_back(rxRate);
            }
        }
    }

    //special rates when looking for tx rates and rx is fixed
    //return all rates where the rx sample rate is achievable
    else if (direction == SOAPY_SDR_TX and fixedRx)
    {
        const double rxRate = this->getSampleRate(SOAPY_SDR_RX, channel);
        for (int iRx = 32; iRx >= 2; iRx /= 2)
        {
            const double clockRate = rxRate*adcFactor*iRx;
            for (int iTx = 32; iTx >= 2; iTx /= 2)
            {
                const double txRate = clockRate/(dacFactor*iTx);
                if (txRate > MAX_SAMP_RATE) continue;
                if (txRate < MIN_SAMP_RATE) continue;
                rates.push_back(txRate);
            }
        }
    }

    std::sort(rates.begin(), rates.end());
    return rates;
}

std::vector<double> SoapyLMS7::listSampleRates(const int direction, const size_t channel) const
{
    auto rates = this->_getEnumeratedRates(direction, channel);

    //otherwise, the clock is the only limiting factor
    //just give a reasonable high and low
    if (rates.empty())
    {
        rates.push_back(MIN_SAMP_RATE);
        rates.push_back(MAX_SAMP_RATE);
    }

    return rates;
}

SoapySDR::RangeList SoapyLMS7::getSampleRateRange(const int direction, const size_t channel) const
{
    SoapySDR::RangeList ranges;
    auto rates = this->_getEnumeratedRates(direction, channel);

    //no enumerated rates? full continuous range is available
    if (rates.empty())
    {
        ranges.push_back(SoapySDR::Range(MIN_SAMP_RATE, MAX_SAMP_RATE));
    }

    //normal list of enumerated rates becomes ranges with start == stop
    for (const auto &rate : rates)
    {
        ranges.push_back(SoapySDR::Range(rate, rate));
    }

    return ranges;
}

/*******************************************************************
 * Bandwidth API
 ******************************************************************/

void SoapyLMS7::setBandwidth(const int direction, const size_t channel, const double bw)
{
    if (bw == 0.0) return; //special ignore value

    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    SoapySDR::logf(SOAPY_SDR_DEBUG, "SoapyLMS7::setBandwidth(%s, %d, %g MHz)", dirName, int(channel), bw/1e6);

    _actualBw[direction][channel] = bw;

    if (direction == SOAPY_SDR_RX)
    {        
        if (lms7Device->SetLPF(false,channel,true,true,bw) != 0)
        {
            SoapySDR::logf(SOAPY_SDR_ERROR, "setBandwidth(Rx, %d, %g MHz) Failed - %s", int(channel), bw/1e6, lime::GetLastErrorMessage());
            throw std::runtime_error(lime::GetLastErrorMessage());
        }
    }

    if (direction == SOAPY_SDR_TX)
    {
        if (lms7Device->SetLPF(true,channel,true,true,bw) != 0)
        {
            SoapySDR::logf(SOAPY_SDR_ERROR, "setBandwidth(Tx, %d, %g MHz) Failed - %s", int(channel), bw/1e6, lime::GetLastErrorMessage());
            throw std::runtime_error(lime::GetLastErrorMessage());
        }
    }

    _channelsToCal.emplace(direction, channel);
}

double SoapyLMS7::getBandwidth(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
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
    lms7Device->SetClockFreq(LMS_CLOCK_CGEN,rate);
    _fixedClockRate = true;
}

double SoapyLMS7::getMasterClockRate(void) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    return lms7Device->GetClockFreq(LMS_CLOCK_CGEN);;
}

SoapySDR::RangeList SoapyLMS7::getMasterClockRates(void) const
{
    SoapySDR::RangeList r;
    r.push_back(SoapySDR::Range(MIN_CGEN_RATE, MAX_CGEN_RATE));
    return r;
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
        auto ticks = lms7Device->GetHardwareTimestamp();
        return SoapySDR::ticksToTimeNs(ticks, sampleRate);
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
        auto ticks = SoapySDR::timeNsToTicks(timeNs, sampleRate);
        lms7Device->SetHardwareTimestamp(ticks);
    }
    else
    {
        throw std::invalid_argument("SoapyLMS7::setHardwareTime("+what+") unknown argument");
    }
}

/*******************************************************************
 * Sensor API
 ******************************************************************/

std::vector<std::string> SoapyLMS7::listSensors(void) const
{
    std::vector<std::string> sensors;
    sensors.push_back("clock_locked");
    sensors.push_back("lms7_temp");
    return sensors;
}

SoapySDR::ArgInfo SoapyLMS7::getSensorInfo(const std::string &name) const
{
    SoapySDR::ArgInfo info;
    if (name == "clock_locked")
    {
        info.key = "clock_locked";
        info.name = "Clock Locked";
        info.type = SoapySDR::ArgInfo::BOOL;
        info.value = "false";
        info.description = "CGEN clock is locked, good VCO selection.";
    }
    else if (name == "lms7_temp")
    {
        info.key = "lms7_temp";
        info.name = "LMS7 Temperature";
        info.type = SoapySDR::ArgInfo::FLOAT;
        info.value = "0.0";
        info.units = "C";
        info.description = "The temperature of the LMS7002M in degrees C.";
    }
    return info;
}

std::string SoapyLMS7::readSensor(const std::string &name) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);

    if (name == "clock_locked")
    {
        return lms7Device->GetLMS()->GetCGENLocked()?"true":"false";
    }
    if (name == "lms7_temp")
    {
        return std::to_string(lms7Device->GetChipTemperature());
    }

    throw std::runtime_error("SoapyLMS7::readSensor("+name+") - unknown sensor name");
}

std::vector<std::string> SoapyLMS7::listSensors(const int /*direction*/, const size_t /*channel*/) const
{
    std::vector<std::string> sensors;
    sensors.push_back("lo_locked");
    return sensors;
}

SoapySDR::ArgInfo SoapyLMS7::getSensorInfo(const int direction, const size_t channel, const std::string &name) const
{
    SoapySDR::ArgInfo info;
    if (name == "lo_locked")
    {
        info.key = "lo_locked";
        info.name = "LO Locked";
        info.type = SoapySDR::ArgInfo::BOOL;
        info.value = "false";
        info.description = "LO synthesizer is locked, good VCO selection.";
    }
    return info;
}

std::string SoapyLMS7::readSensor(const int direction, const size_t channel, const std::string &name) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = lms7Device->GetLMS(channel/2);
    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;

    if (name == "lo_locked")
    {
        return rfic->GetSXLocked(lmsDir)?"true":"false";
    }

    throw std::runtime_error("SoapyLMS7::readSensor("+name+") - unknown sensor name");
}

/*******************************************************************
 * Register API
 ******************************************************************/

std::vector<std::string> SoapyLMS7::listRegisterInterfaces(void) const
{
    std::vector<std::string> ifaces;
    ifaces.push_back("BBIC");
    for (size_t i = 0; i < lms7Device->GetLMSCnt(); i++)
    {
        ifaces.push_back("RFIC" + std::to_string(i));
    }
    return ifaces;
}

void SoapyLMS7::writeRegister(const std::string &name, const unsigned addr, const unsigned value)
{
    if (name == "BBIC") return this->writeRegister(addr, value);
    for (size_t i = 0; i < lms7Device->GetLMSCnt(); i++)
    {
        if (("RFIC" + std::to_string(i)) != name) continue;
        int st = lms7Device->WriteLMSReg(addr, value,i);
        if (st == 0) return;
        throw std::runtime_error("SoapyLMS7::WriteRegister("+name+", "+std::to_string(addr)+") FAIL");
    }
    throw std::runtime_error("SoapyLMS7::WriteRegister("+name+") unknown interface");
}

unsigned SoapyLMS7::readRegister(const std::string &name, const unsigned addr) const
{
    if (name == "BBIC") return this->readRegister(addr);
    for (size_t i = 0; i < lms7Device->GetLMSCnt(); i++)
    {
        if (("RFIC" + std::to_string(i)) != name) continue;
        uint16_t value;
        int st = lms7Device->ReadLMSReg(addr, &value, i);
        if (st == 0) return value;
        throw std::runtime_error("SoapyLMS7::readRegister("+name+", "+std::to_string(addr)+") FAIL");
    }
    throw std::runtime_error("SoapyLMS7::readRegister("+name+") unknown interface");
}

void SoapyLMS7::writeRegister(const unsigned addr, const unsigned value)
{
    auto st = lms7Device->GetConnection()->WriteRegister(addr, value);
    if (st != 0) throw std::runtime_error(
        "SoapyLMS7::WriteRegister("+std::to_string(addr)+") FAIL");
}

unsigned SoapyLMS7::readRegister(const unsigned addr) const
{
    unsigned readbackData = 0;
    auto st = lms7Device->GetConnection()->ReadRegister(addr, readbackData);
    if (st != 0) throw std::runtime_error(
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
        for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
        {
            this->writeSetting(SOAPY_SDR_RX, channel, "TSP_CONST", value);
        }
    }

    else if (key == "TXTSP_CONST")
    {
        for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
        {
            this->writeSetting(SOAPY_SDR_TX, channel, "TSP_CONST", value);
        }
    }

    else if (key == "CALIBRATE_TX")
    {
        for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
        {
            this->writeSetting(SOAPY_SDR_TX, channel, "CALIBRATE_TX", value);
        }
    }

    else if (key == "CALIBRATE_RX")
    {
        for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
        {
            this->writeSetting(SOAPY_SDR_RX, channel, "CALIBRATE_RX", value);
        }
    }

    else if (key == "CALIBRATE_TX_EXTLOOPBACK")
    {
        for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
        {
            this->writeSetting(SOAPY_SDR_TX, channel, "CALIBRATE_TX_EXTLOOPBACK", value);
        }
    }

    else if (key == "CALIBRATE_RX_EXTLOOPBACK")
    {
        for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
        {
            this->writeSetting(SOAPY_SDR_RX, channel, "CALIBRATE_RX_EXTLOOPBACK", value);
        }
    }

    else if (key == "ENABLE_RX_GFIR_LPF")
    {
        for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
        {
            this->writeSetting(SOAPY_SDR_RX, channel, "ENABLE_GFIR_LPF", value);
        }
    }

    else if (key == "ENABLE_TX_GFIR_LPF")
    {
        for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
        {
            this->writeSetting(SOAPY_SDR_TX, channel, "ENABLE_GFIR_LPF", value);
        }
    }

    else if (key == "DISABLE_RX_GFIR_LPF")
    {
        for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
        {
            this->writeSetting(SOAPY_SDR_RX, channel, "DISABLE_GFIR_LPF", value);
        }
    }

    else if (key == "DISABLE_TX_GFIR_LPF")
    {
        for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
        {
            this->writeSetting(SOAPY_SDR_TX, channel, "DISABLE_GFIR_LPF", value);
        }
    }

     else if (key == "RXTSG_NCO")
    {
        for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
        {
            this->writeSetting(SOAPY_SDR_RX, channel, "TSG_NCO", value);
        }
    }

    else if (key == "TXTSG_NCO")
    {
        for (size_t channel = 0; channel < lms7Device->GetNumChannels(); channel++)
        {
            this->writeSetting(SOAPY_SDR_TX, channel, "TSG_NCO", value);
        }
    }

    else if (key == "SAVE_CONFIG")
    {
        lms7Device->SaveConfig(value.c_str());
    }
    else if (key == "LOAD_CONFIG")
    {
        lms7Device->LoadConfig(value.c_str());
    }

    else throw std::runtime_error("unknown setting key: "+key);
}

SoapySDR::ArgInfoList SoapyLMS7::getSettingInfo(const int direction, const size_t channel) const
{
    SoapySDR::ArgInfoList infos;

    {
        SoapySDR::ArgInfo info;
        info.key = "TSP_CONST";
        info.name = "TSP DC Level";
        info.type = SoapySDR::ArgInfo::INT;
        info.description = "Digital DC level in LMS7002M TSP chain.";
        info.range = SoapySDR::Range(0, (1 << 15)-1);
        infos.push_back(info);
    }

    return infos;
}

void SoapyLMS7::writeSetting(const int direction, const size_t channel, const std::string &key, const std::string &value)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    const bool isTx = (direction == SOAPY_SDR_TX);

    if (key == "TSP_CONST")
    {
        const auto ampl = std::stoi(value);
        lms7Device->SetTestSignal(isTx, channel, LMS_TESTSIG_DC, ampl, 0);
    }

    else if (key == "CALIBRATE_TX")
    {
        float_type bw = std::stof(value);
        SoapySDR::logf(SOAPY_SDR_INFO, "Issuing CalibrateTx(%f, false)", bw);
        if (lms7Device->Calibrate(true, channel, bw, 0)!=0)
            throw std::runtime_error(lime::GetLastErrorMessage());
    }

    else if (key == "CALIBRATE_RX")
    {
        float_type bw = std::stof(value);
        SoapySDR::logf(SOAPY_SDR_INFO, "Issuing CalibrateRx(%f, false)", bw);
        if (lms7Device->Calibrate(false, channel, bw, 0)!=0)
            throw std::runtime_error(lime::GetLastErrorMessage());
    }

    else if (key == "ENABLE_GFIR_LPF")
    {

        //The value corresponding to this key is a CSV of the following format:
        //<channel>, <#coeffs>, <GFIR Index>, <Coeff1, Coeff2, Coeff3, ..., Coeff120>
        // Please note the following restrictions:
        // - GFIR_Index will be clipped to 8-bit
        // - Coefficients will be clipped to 16-bit
        // - Coefficient Count wil be clipped to 8-bit (really only 120 max)

        std::vector<int> vect;
        std::stringstream ss(value);

        int i;

        uint8_t GFIR_Index;
        double * Coeffs;
        uint8_t Coeff_Count;

        //CSV-to-int
        while (ss >> i)
        {
            vect.push_back(i);

            if (ss.peek() == ',')
                ss.ignore();
        }


        if((size_t)vect.at(0) == channel)
        {
            //The coeffs specified are for this channel

            Coeff_Count = (uint8_t) vect.at(1);
            GFIR_Index = (uint8_t) vect.at(2);

            if(GFIR_Index > 2)
                throw std::runtime_error("Invalid GFIR Index Specified: " + key);

            Coeffs = (double*) malloc(Coeff_Count * sizeof(double));
            for(size_t k = 0; k < Coeff_Count; k++)
            {
                Coeffs[k] = (double) vect.at(k+3);
            }

            SoapySDR::logf(SOAPY_SDR_INFO, "Issuing call to SetGFIRCoef; Channel = %d; isTx = %d; GFIR_Index = %d; #Coeffs = %d", channel, isTx, GFIR_Index, Coeff_Count);

            lms7Device->SetGFIRCoef(isTx,channel,lms_gfir_t(GFIR_Index),Coeffs,Coeff_Count);

            free(Coeffs);

            //Now we also want to enable the GFIR
            SoapySDR::logf(SOAPY_SDR_INFO, "Coefficients configured, now enabling appropriate GFIR");
            switch (GFIR_Index)
            {
                case 0:
                    lms7Device->WriteParam(isTx?LMS7param(GFIR1_BYP_TXTSP):LMS7param(GFIR1_BYP_RXTSP), 0, channel);
                    break;
                case 1:
                    lms7Device->WriteParam(isTx?LMS7param(GFIR2_BYP_TXTSP):LMS7param(GFIR2_BYP_RXTSP), 0, channel);
                    break;
                case 2:
                    lms7Device->WriteParam(isTx?LMS7param(GFIR3_BYP_TXTSP):LMS7param(GFIR3_BYP_RXTSP), 0, channel);
                    break;
            }

        }
        else if (vect.at(0) > 1)
        {
            //An invalid channel configuration has been specified
            throw std::runtime_error("Invalid channel specified: " + key);
        }
        else
        {
            //The coeffs specified are for a different channel
            return;
        }
    }

    else if  (key == "DISABLE_GFIR_LPF")
    {

        //The value corresponding to this key is a CSV of the following format:
        //<channel>, <GFIR Index>
        // Please note the following restrictions:
        // - GFIR_Index will be clipped to 8-bit

        std::vector<int> vect;
        std::stringstream ss(value);

        int i;

        uint8_t GFIR_Index;

        //CSV-to-int
        while (ss >> i)
        {
            vect.push_back(i);

            if (ss.peek() == ',')
                ss.ignore();
        }

        if((size_t)vect.at(0) == channel)
        {
            GFIR_Index = (uint8_t) vect.at(1);
            if(GFIR_Index > 2)
                throw std::runtime_error("Invalid GFIR Index Specified: " + key);

            //Disable the GFIR
            SoapySDR::logf(SOAPY_SDR_INFO, "Disabling GFIR; Channel %d, GFIR_Index %d", channel, GFIR_Index);
            switch (GFIR_Index)
            {
                case 0:
                    lms7Device->WriteParam(isTx?LMS7param(GFIR1_BYP_TXTSP):LMS7param(GFIR1_BYP_RXTSP), 1, channel);
                    break;
                case 1:
                    lms7Device->WriteParam(isTx?LMS7param(GFIR2_BYP_TXTSP):LMS7param(GFIR2_BYP_RXTSP), 1, channel);
                    break;
                case 2:
                    lms7Device->WriteParam(isTx?LMS7param(GFIR3_BYP_TXTSP):LMS7param(GFIR3_BYP_RXTSP), 1, channel);
                    break;
            }

        }
        else if (vect.at(0) > 1)
        {
            //An invalid channel configuration has been specified
            throw std::runtime_error("Invalid channel specified: " + key);
        }
        else
        {
            //The input specified is for a different channel
            return;
        }

    }

    else if (key == "TSG_NCO")
    {
        auto select = std::stoi(value);
        if (select == -1)
        {
            //Requested to disable the TSG
            lms7Device->SetTestSignal(isTx, channel, LMS_TESTSIG_NONE);
        }
        else if(select == 4)
        {
            lms7Device->SetTestSignal(isTx, channel, LMS_TESTSIG_NCODIV4F);
        }
        else if (select == 8)
        {
            lms7Device->SetTestSignal(isTx, channel, LMS_TESTSIG_NCODIV8F);
        }
        else
        {
            throw std::runtime_error("Invalid TSG_NCO option: " + value);
        }
    }

    else throw std::runtime_error("unknown setting key: "+key);
}
/*******************************************************************
 * GPIO API
 ******************************************************************/

std::vector<std::string> SoapyLMS7::listGPIOBanks(void) const
{
    std::vector<std::string> banks;
    banks.push_back("MAIN"); //just one associated with the connection
    return banks;
}

void SoapyLMS7::writeGPIO(const std::string &, const unsigned value)
{
    uint8_t buffer = uint8_t(value);
    int r = lms7Device->GetConnection()->GPIOWrite(&buffer, 1);
    if (r != 0) throw std::runtime_error("SoapyLMS7::writeGPIO() " + std::string(lime::GetLastErrorMessage()));
}

unsigned SoapyLMS7::readGPIO(const std::string &) const
{
    uint8_t buffer(0);
    int r = lms7Device->GetConnection()->GPIORead(&buffer, 1);
    if (r != 0) throw std::runtime_error("SoapyLMS7::readGPIO() " + std::string(lime::GetLastErrorMessage()));
    return unsigned(buffer);
}

void SoapyLMS7::writeGPIODir(const std::string &, const unsigned dir)
{
    uint8_t buffer = uint8_t(dir);
    int r = lms7Device->GetConnection()->GPIODirWrite(&buffer, 1);
    if (r != 0) throw std::runtime_error("SoapyLMS7::writeGPIODir() " + std::string(lime::GetLastErrorMessage()));
}

unsigned SoapyLMS7::readGPIODir(const std::string &) const
{
    uint8_t buffer(0);
    int r = lms7Device->GetConnection()->GPIODirRead(&buffer, 1);
    if (r != 0) throw std::runtime_error("SoapyLMS7::readGPIODir() " + std::string(lime::GetLastErrorMessage()));
    return unsigned(buffer);
}

/*******************************************************************
 * I2C API
 ******************************************************************/
void SoapyLMS7::writeI2C(const int addr, const std::string &data)
{
    auto st = lms7Device->GetConnection()->WriteI2C(addr, data);
    if (st != 0) throw std::runtime_error(
        "SoapyLMS7::writeI2C("+std::to_string(addr)+") FAIL");
}

std::string SoapyLMS7::readI2C(const int addr, const size_t numBytes)
{
    std::string result;
    auto st = lms7Device->GetConnection()->ReadI2C(addr, numBytes, result);
    if (st != 0) throw std::runtime_error(
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
    auto st = lms7Device->GetConnection()->TransactSPI(addr, &input, &readback, 1);
    if (st != 0) throw std::runtime_error(
        "SoapyLMS7::transactSPI("+std::to_string(addr)+") FAIL");
    return readback;
}
