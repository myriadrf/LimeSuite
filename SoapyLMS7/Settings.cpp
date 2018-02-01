/**
@file   Settings.cpp
@brief  Soapy SDR + IConnection config settings.
@author Lime Microsystems (www.limemicro.com)
*/

#include "SoapyLMS7.h"
#include "ErrorReporting.h"
#include <IConnection.h>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <LMS7002M.h>
#include <LMS7002M_RegistersMap.h>
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Time.hpp>
#include <cstdlib>
#include <algorithm>

using namespace lime;

#define dirName ((direction == SOAPY_SDR_RX)?"Rx":"Tx")

// arbitrary upper limit for CGEN automatic tune
#define MIN_CGEN_RATE 6.4e6
#define MAX_CGEN_RATE 640e6
#define CGEN_DEADZONE_LO 450e6
#define CGEN_DEADZONE_HI 491.5e6

//reasonable limits when advertising the rate
#define MIN_SAMP_RATE 1e5
#define MAX_SAMP_RATE 65e6

/*******************************************************************
 * Constructor/destructor
 ******************************************************************/
SoapyLMS7::SoapyLMS7(const ConnectionHandle &handle, const SoapySDR::Kwargs &args):
    _deviceArgs(args),
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
    SoapySDR::logf(SOAPY_SDR_INFO, "Reference: %g MHz", _conn->GetReferenceClockRate()/1e6);

    //LMS7002M driver for each RFIC
    for (size_t i = 0; i < numRFICs; i++)
    {
        SoapySDR::logf(SOAPY_SDR_INFO, "Init LMS7002M(%d)", int(i));
        _rfics.push_back(new LMS7002M());
        _rfics.back()->SetConnection(_conn, i);
        SoapySDR::logf(SOAPY_SDR_INFO, "Ver=%d, Rev=%d, Mask=%d",
            _rfics.back()->Get_SPI_Reg_bits(LMS7param(VER), true),
            _rfics.back()->Get_SPI_Reg_bits(LMS7param(REV), true),
            _rfics.back()->Get_SPI_Reg_bits(LMS7param(MASK), true));

        int st;

        st = _rfics.back()->ResetChip();
        if (st != 0) throw std::runtime_error("ResetChip() failed");

        st = _rfics.back()->SoftReset();
        if (st != 0) throw std::runtime_error("SoftReset() failed");
        if (devInfo.deviceName.find("LimeSDR-mini")!=std::string::npos)
        {
            st = _rfics.back()->SPI_write(0xA6,0x0001);
            if (st != 0) throw std::runtime_error("SPI_write() failed");
            st = _rfics.back()->SPI_write(0x92,0xFFFF);
            if (st != 0) throw std::runtime_error("SPI_write() failed");
            st = _rfics.back()->SPI_write(0x93,0x03FF);
            if (st != 0) throw std::runtime_error("SPI_write() failed");
            st = _rfics.back()->SPI_write(0x8B,0x338E);
            if (st != 0) throw std::runtime_error("SPI_write() failed");
        }

        st = _rfics.back()->UploadAll();
        if (st != 0) throw std::runtime_error("UploadAll() failed");
    }

    //enable all channels
    for (size_t channel = 0; channel < _rfics.size()*2; channel++)
    {
        auto rfic = getRFIC(channel);
        rfic->EnableChannel(LMS7002M::Tx, true);
        rfic->EnableChannel(LMS7002M::Rx, true);
    }

    //enable use of calibration value cache automatically
    //or specify args[cacheCalibrations] == 0 to disable
    const bool cacheEnable = args.count("cacheCalibrations") and std::stoi(args.at("cacheCalibrations")) != 0;
    SoapySDR::logf(SOAPY_SDR_INFO, "LMS7002M calibration values caching %s", cacheEnable?"Enable":"Disable");
    for (auto rfic : _rfics) rfic->EnableValuesCache(cacheEnable);

    //give all RFICs a default state
    double defaultClockRate = DEFAULT_CLOCK_RATE;
    if (args.count("clock")) defaultClockRate = std::stod(args.at("clock"));
    this->setMasterClockRate(defaultClockRate);
    for (size_t channel = 0; channel < _rfics.size()*2; channel++)
    {
        this->setFrequency(SOAPY_SDR_RX, channel, "BB", 0.0);
        this->setFrequency(SOAPY_SDR_TX, channel, "BB", 0.0);
        this->setAntenna(SOAPY_SDR_RX, channel, devInfo.deviceName.find("LimeSDR-mini")!=std::string::npos ? "LNAH" :  "LNAL");
        this->setAntenna(SOAPY_SDR_TX, channel, "BAND1");
        this->setGain(SOAPY_SDR_RX, channel, "PGA", 0);
        this->setGain(SOAPY_SDR_RX, channel, "LNA", 0);
        this->setGain(SOAPY_SDR_RX, channel, "TIA", 9);
        this->setGain(SOAPY_SDR_TX, channel, "PAD", 0);
        this->setGain(SOAPY_SDR_TX, channel, "IAMP", 0);
        this->setSampleRate(SOAPY_SDR_RX, channel, defaultClockRate/8);
        this->setSampleRate(SOAPY_SDR_TX, channel, defaultClockRate/8);
        this->setBandwidth(SOAPY_SDR_RX, channel, 30e6);
        this->setBandwidth(SOAPY_SDR_TX, channel, 30e6);
        this->setDCOffsetMode(SOAPY_SDR_RX, channel, true);
        this->setDCOffset(SOAPY_SDR_TX, channel, 0.0);
        this->setIQBalance(SOAPY_SDR_RX, channel, 1.0);
        this->setIQBalance(SOAPY_SDR_TX, channel, 1.0);
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
        char buff[64]; sprintf(buff, "0x%lx", devinfo.boardSerialNumber);
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
    SoapySDR::logf(SOAPY_SDR_DEBUG, "SoapyLMS7::setAntenna(%s, %d, %s)", dirName, int(channel), name.c_str());
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

    _channelsToCal.emplace(direction, channel);
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
    auto rfic = getRFIC(channel);

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

void SoapyLMS7::setFrequency(const int direction, const size_t channel, const double frequency, const SoapySDR::Kwargs &args)
{
    //less than 30? use distributed algorithm with NCO to help tuning
    if (frequency < 30e6)
    {
        SoapySDR::Device::setFrequency(direction, channel, frequency, args);
    }

    //otherwise tune the RF center frequency keeping any small errors
    //the baseband NCO will be set to zero as part of tuning overall
    //this allows the DC component for RX chain to be removed
    //by the DC removal filter by ensuring the NCO is kept at 0
    else
    {
        this->setFrequency(direction, channel, "RF", frequency, args);
        this->setFrequency(direction, channel, "BB", 0.0,       args);
    }
}

void SoapyLMS7::setFrequency(const int direction, const size_t channel, const std::string &name, const double frequency, const SoapySDR::Kwargs &args)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);
    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;
    SoapySDR::logf(SOAPY_SDR_DEBUG, "SoapyLMS7::setFrequency(%s, %d, %s, %g MHz)", dirName, int(channel), name.c_str(), frequency/1e6);

    if (name == "RF")
    {
        //clip the frequency into the allowed range
        double targetRfFreq = frequency;
        if (targetRfFreq < 30e6) targetRfFreq = 30e6;
        if (targetRfFreq > 3.8e9) targetRfFreq = 3.8e9;
        rfic->SetFrequencySX(lmsDir, targetRfFreq);
        _channelsToCal.emplace(direction, channel);
        return;
    }

    if (name == "BB")
    {
        int pos = 0, neg = 1;
        switch (direction)
        {
        case SOAPY_SDR_RX:
            if (rfic->Get_SPI_Reg_bits(LMS7_MASK, true) != 0) std::swap(pos, neg);
            rfic->Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), (frequency == 0)?1:0);
            rfic->Modify_SPI_Reg_bits(LMS7param(CMIX_SC_RXTSP), (frequency < 0)?neg:pos);
            break;
        case SOAPY_SDR_TX:
            rfic->Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_TXTSP), (frequency == 0)?1:0);
            rfic->Modify_SPI_Reg_bits(LMS7param(CMIX_SC_TXTSP), (frequency < 0)?neg:pos);
            break;
        }
        if (rfic->SetNCOFrequency(lmsDir, 0, abs(frequency)) != 0)
        {
            //rate was out of bounds, clip to the maximum frequency
            const double dspRate = rfic->GetReferenceClk_TSP(lmsDir);
            rfic->SetNCOFrequency(lmsDir, 0, dspRate/2);
        }
        return;
    }

    throw std::runtime_error("SoapyLMS7::setFrequency("+name+") unknown name");
}

double SoapyLMS7::getFrequency(const int direction, const size_t channel, const std::string &name) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);
    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;

    if (name == "RF")
    {
        return rfic->GetFrequencySX(lmsDir);
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
        return rfic->GetNCOFrequency(lmsDir, 0) * sign;
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
        const double dspRate = rfic->GetReferenceClk_TSP(lmsDir);
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
    if (rate > MAX_CGEN_RATE) return false;
    if (rate > CGEN_DEADZONE_LO and rate < CGEN_DEADZONE_HI) return false; //avoid range where CGEN does not lock
    return true;
}

static double calculateClockRate(
    const int adcFactorRx,
    const int dacFactorTx,
    const double rateRx,
    const double rateTx,
    int &dspFactorRx,
    int &dspFactorTx)
{
    double bestClockRate = 0.0;

    for (int decim = 2; decim <= 32; decim *= 2)
    {
        const double rateClock = rateRx*decim*adcFactorRx;
        if (not cgenRatePossible(rateClock)) continue;
        if (rateClock < bestClockRate) continue;
        for (int interp = 2; interp <= 32; interp *= 2)
        {
            const double actualRateTx = rateClock/(interp*dacFactorTx);

            //good if we got the same output rate with small margin of error
            if (std::abs(actualRateTx-rateTx) < 10.0)
            {
                bestClockRate = rateClock;
                dspFactorRx = decim;
                dspFactorTx = interp;
            }
        }
    }

    //return the best possible match
    if (bestClockRate != 0.0) return bestClockRate;

    SoapySDR::logf(SOAPY_SDR_ERROR, "setSampleRate(Rx %g MHz, Tx %g MHz) Failed -- no common clock rate", rateRx/1e6, rateTx/1e6);
    throw std::runtime_error("SoapyLMS7::setSampleRate() -- no common clock rate");
}

void SoapyLMS7::setSampleRate(const int direction, const size_t channel, const double rate)
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);

    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;

    double clockRate = this->getMasterClockRate();
    const double dspFactor = clockRate/rfic->GetReferenceClk_TSP(lmsDir);

    //select automatic clock rate
    if (not _fixedClockRate)
    {
        double rxRate = rate, txRate = rate;
        if (direction != SOAPY_SDR_RX and _fixedRxSampRate[channel]) rxRate = this->getSampleRate(SOAPY_SDR_RX, channel);
        if (direction != SOAPY_SDR_TX and _fixedTxSampRate[channel]) txRate = this->getSampleRate(SOAPY_SDR_TX, channel);
        clockRate = calculateClockRate(
            clockRate/rfic->GetReferenceClk_TSP(LMS7002M::Rx),
            clockRate/rfic->GetReferenceClk_TSP(LMS7002M::Tx),
            rxRate, txRate, _decims[channel], _interps[channel]
        );
    }

    const double dspRate = clockRate/dspFactor;
    const double factor = dspRate/rate;
    int intFactor = 1 << int((std::log(factor)/std::log(2.0)) + 0.5);
    SoapySDR::logf(SOAPY_SDR_DEBUG, "SoapyLMS7::setSampleRate(%s, %d, %g MHz), CGEN=%g MHz, %s=%g MHz, %s=%g",
        dirName, int(channel), rate/1e6, clockRate/1e6,
        (direction == SOAPY_SDR_RX)?"ADC":"DAC", dspRate/1e6,
        (direction == SOAPY_SDR_RX)?"decim":"interp", factor);
    if (intFactor < 2) throw std::runtime_error("SoapyLMS7::setSampleRate() -- rate too high");
    if (intFactor > 32) throw std::runtime_error("SoapyLMS7::setSampleRate() -- rate too low");

    if (std::abs(factor-intFactor) > 0.01) SoapySDR::logf(SOAPY_SDR_WARNING,
        "SoapyLMS7::setSampleRate(): not a power of two factor: TSP Rate = %g MHZ, Requested rate = %g MHz", dspRate/1e6, rate/1e6);

    switch (direction)
    {
    case SOAPY_SDR_TX:
        _fixedTxSampRate[channel] = true;
        _interps[channel] = intFactor;
        break;
    case SOAPY_SDR_RX:
        _fixedRxSampRate[channel] = true;
        _decims[channel] = intFactor;
        break;
    }

    int status = 0;
    status = rfic->SetInterfaceFrequency(clockRate,
        int(std::log(double(_interps[channel]))/std::log(2.0))-1,
        int(std::log(double(_decims[channel]))/std::log(2.0))-1);
    if(status != 0)
        SoapySDR::logf(SOAPY_SDR_ERROR, GetLastErrorMessage());

    status = _conn->UpdateExternalDataRate(
        rfic->GetActiveChannelIndex(),
        rfic->GetSampleRate(LMS7002M::Tx, rfic->GetActiveChannel()),
        rfic->GetSampleRate(LMS7002M::Rx, rfic->GetActiveChannel()));
    if(status != 0)
        SoapySDR::logf(SOAPY_SDR_ERROR, GetLastErrorMessage());
}

double SoapyLMS7::getSampleRate(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);
    const auto lmsDir = (direction == SOAPY_SDR_TX)?LMS7002M::Tx:LMS7002M::Rx;

    return rfic->GetSampleRate(lmsDir, rfic->GetActiveChannel());
}

std::vector<double> SoapyLMS7::_getEnumeratedRates(const int direction, const size_t channel) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    auto rfic = getRFIC(channel);
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
        ranges.push_back(SoapySDR::Range(MIN_SAMP_RATE, CGEN_DEADZONE_LO/8));
        ranges.push_back(SoapySDR::Range(CGEN_DEADZONE_HI/8, MAX_SAMP_RATE));
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

    //save dc offset mode
    auto saveDcMode = this->getDCOffsetMode(direction, channel);

    auto rfic = getRFIC(channel);

    _actualBw[direction][channel] = bw;

    if (direction == SOAPY_SDR_RX)
    {
        if (rfic->TuneRxFilterWithCaching(bw) != 0)
        {
            SoapySDR::logf(SOAPY_SDR_ERROR, "setBandwidth(Rx, %d, %g MHz) Failed - %s", int(channel), bw/1e6, lime::GetLastErrorMessage());
            throw std::runtime_error(lime::GetLastErrorMessage());
        }
    }

    if (direction == SOAPY_SDR_TX)
    {
        if (rfic->TuneTxFilterWithCaching(bw) != 0)
        {
            SoapySDR::logf(SOAPY_SDR_ERROR, "setBandwidth(Tx, %d, %g MHz) Failed - %s", int(channel), bw/1e6, lime::GetLastErrorMessage());
            throw std::runtime_error(lime::GetLastErrorMessage());
        }
    }

    //restore dc offset mode
    this->setDCOffsetMode(direction, channel, saveDcMode);

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
        bws.push_back(SoapySDR::Range(1.4e6, 130e6));
    }
    if (direction == SOAPY_SDR_TX)
    {
        bws.push_back(SoapySDR::Range(5e6, 40e6));
        bws.push_back(SoapySDR::Range(50e6, 130e6));
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
        rfic->Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0);
        rfic->Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2);
        rfic->SetFrequencyCGEN(rate);
    }
    _fixedClockRate = true;
}

double SoapyLMS7::getMasterClockRate(void) const
{
    std::unique_lock<std::recursive_mutex> lock(_accessMutex);
    //assume same rate for all RFIC in this wrapper
    return _rfics.front()->GetFrequencyCGEN();
}

SoapySDR::RangeList SoapyLMS7::getMasterClockRates(void) const
{
    SoapySDR::RangeList r;
    r.push_back(SoapySDR::Range(MIN_CGEN_RATE, CGEN_DEADZONE_LO));
    r.push_back(SoapySDR::Range(CGEN_DEADZONE_HI, MAX_CGEN_RATE));
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
        return _rfics.front()->GetCGENLocked()?"true":"false";
    }
    if (name == "lms7_temp")
    {
        return std::to_string(_rfics.front()->GetTemperature());
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
    auto rfic = getRFIC(channel);
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
    for (size_t i = 0; i < _rfics.size(); i++)
    {
        ifaces.push_back("RFIC" + std::to_string(i));
    }
    return ifaces;
}

void SoapyLMS7::writeRegister(const std::string &name, const unsigned addr, const unsigned value)
{
    if (name == "BBIC") return this->writeRegister(addr, value);
    for (size_t i = 0; i < _rfics.size(); i++)
    {
        if (("RFIC" + std::to_string(i)) != name) continue;
        int st = _rfics[i]->SPI_write(addr, value);
        if (st == 0) return;
        throw std::runtime_error("SoapyLMS7::WriteRegister("+name+", "+std::to_string(addr)+") FAIL");
    }
    throw std::runtime_error("SoapyLMS7::WriteRegister("+name+") unknown interface");
}

unsigned SoapyLMS7::readRegister(const std::string &name, const unsigned addr) const
{
    if (name == "BBIC") return this->readRegister(addr);
    for (size_t i = 0; i < _rfics.size(); i++)
    {
        if (("RFIC" + std::to_string(i)) != name) continue;
        int st(0);
        int value = _rfics[i]->SPI_read(addr, true, &st);
        if (st == 0) return value;
        throw std::runtime_error("SoapyLMS7::readRegister("+name+", "+std::to_string(addr)+") FAIL");
    }
    throw std::runtime_error("SoapyLMS7::readRegister("+name+") unknown interface");
}

void SoapyLMS7::writeRegister(const unsigned addr, const unsigned value)
{
    auto st = _conn->WriteRegister(addr, value);
    if (st != 0) throw std::runtime_error(
        "SoapyLMS7::WriteRegister("+std::to_string(addr)+") FAIL");
}

unsigned SoapyLMS7::readRegister(const unsigned addr) const
{
    unsigned readbackData = 0;
    auto st = _conn->ReadRegister(addr, readbackData);
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
        for (size_t channel = 0; channel < _rfics.size()*2; channel++)
        {
            this->writeSetting(SOAPY_SDR_RX, channel, "TSP_CONST", value);
        }
    }

    else if (key == "TXTSP_CONST")
    {
        for (size_t channel = 0; channel < _rfics.size()*2; channel++)
        {
            this->writeSetting(SOAPY_SDR_TX, channel, "TSP_CONST", value);
        }
    }

    else if (key == "CALIBRATE_TX")
    {
        for (size_t channel = 0; channel < _rfics.size()*2; channel++)
        {
            this->writeSetting(SOAPY_SDR_TX, channel, "CALIBRATE_TX", value);
        }
    }

    else if (key == "CALIBRATE_RX")
    {
        for (size_t channel = 0; channel < _rfics.size()*2; channel++)
        {
            this->writeSetting(SOAPY_SDR_RX, channel, "CALIBRATE_RX", value);
        }
    }

    else if (key == "CALIBRATE_TX_EXTLOOPBACK")
    {
        for (size_t channel = 0; channel < _rfics.size()*2; channel++)
        {
            this->writeSetting(SOAPY_SDR_TX, channel, "CALIBRATE_TX_EXTLOOPBACK", value);
        }
    }

    else if (key == "CALIBRATE_RX_EXTLOOPBACK")
    {
        for (size_t channel = 0; channel < _rfics.size()*2; channel++)
        {
            this->writeSetting(SOAPY_SDR_RX, channel, "CALIBRATE_RX_EXTLOOPBACK", value);
        }
    }

    else if (key == "ENABLE_RX_GFIR_LPF")
    {
        for (size_t channel = 0; channel < _rfics.size()*2; channel++)
        {
            this->writeSetting(SOAPY_SDR_RX, channel, "ENABLE_GFIR_LPF", value);
        }
    }

    else if (key == "ENABLE_TX_GFIR_LPF")
    {
        for (size_t channel = 0; channel < _rfics.size()*2; channel++)
        {
            this->writeSetting(SOAPY_SDR_TX, channel, "ENABLE_GFIR_LPF", value);
        }
    }

    else if (key == "DISABLE_RX_GFIR_LPF")
    {
        for (size_t channel = 0; channel < _rfics.size()*2; channel++)
        {
            this->writeSetting(SOAPY_SDR_RX, channel, "DISABLE_GFIR_LPF", value);
        }
    }

    else if (key == "DISABLE_TX_GFIR_LPF")
    {
        for (size_t channel = 0; channel < _rfics.size()*2; channel++)
        {
            this->writeSetting(SOAPY_SDR_TX, channel, "DISABLE_GFIR_LPF", value);
        }
    }

     else if (key == "RXTSG_NCO")
    {
        for (size_t channel = 0; channel < _rfics.size()*2; channel++)
        {
            this->writeSetting(SOAPY_SDR_RX, channel, "TSG_NCO", value);
        }
    }

    else if (key == "TXTSG_NCO")
    {
        for (size_t channel = 0; channel < _rfics.size()*2; channel++)
        {
            this->writeSetting(SOAPY_SDR_TX, channel, "TSG_NCO", value);
        }
    }

    else if (key == "SAVE_CONFIG")
    {
        auto rfic = getRFIC(0);
        rfic->SaveConfig(value.c_str());
    }
    else if (key == "LOAD_CONFIG")
    {
        auto rfic = getRFIC(0);
        rfic->LoadConfig(value.c_str());
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
    auto rfic = getRFIC(channel);
    const bool isTx = (direction == SOAPY_SDR_TX);

    if (key == "TSP_CONST")
    {
        const auto ampl = std::stoi(value);
        rfic->Modify_SPI_Reg_bits(isTx?LMS7param(TSGFC_TXTSP):LMS7param(TSGFC_RXTSP), 1); //Full-scale
        rfic->Modify_SPI_Reg_bits(isTx?LMS7param(TSGMODE_TXTSP):LMS7param(TSGMODE_RXTSP), 1); //DC
        rfic->Modify_SPI_Reg_bits(isTx?LMS7param(INSEL_TXTSP):LMS7param(INSEL_RXTSP), 1); //SIGGEN
        rfic->LoadDC_REG_IQ(isTx, ampl, 0);
    }

    else if (key == "CALIBRATE_TX")
    {
        float_type bw = std::stof(value);
        SoapySDR::logf(SOAPY_SDR_INFO, "Issuing CalibrateTx(%f, false)", bw);
        if(rfic->CalibrateTx(bw, false) != 0)
            throw std::runtime_error(lime::GetLastErrorMessage());
    }

    else if (key == "CALIBRATE_RX")
    {
        float_type bw = std::stof(value);
        SoapySDR::logf(SOAPY_SDR_INFO, "Issuing CalibrateRx(%f, false)", bw);
        if(rfic->CalibrateRx(bw, false) != 0)
            throw std::runtime_error(lime::GetLastErrorMessage());
    }


    else if (key == "CALIBRATE_TX_EXTLOOPBACK")
    {
        float_type bw = std::stof(value);
        SoapySDR::logf(SOAPY_SDR_INFO, "Issuing CalibrateTx(%f, true)", bw);
        if(rfic->CalibrateTx(bw, true) != 0)
            throw std::runtime_error(lime::GetLastErrorMessage());
    }

    else if (key == "CALIBRATE_RX_EXTLOOPBACK")
    {
        float_type bw = std::stof(value);
        SoapySDR::logf(SOAPY_SDR_INFO, "Issuing CalibrateRx(%f, true)", bw);
        if(rfic->CalibrateRx(bw, true) != 0)
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
        int16_t * Coeffs;
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

            Coeffs = (int16_t*) malloc(Coeff_Count * sizeof(int16_t));
            for(size_t k = 0; k < Coeff_Count; k++)
            {
                Coeffs[k] = (int16_t) vect.at(k+3);
            }

            SoapySDR::logf(SOAPY_SDR_INFO, "Issuing call to SetGFIRCoefficients; Channel = %d; isTx = %d; GFIR_Index = %d; #Coeffs = %d", channel, isTx, GFIR_Index, Coeff_Count);

            rfic->SetGFIRCoefficients(isTx, GFIR_Index, (const int16_t*) Coeffs, Coeff_Count);

            free(Coeffs);

            //Now we also want to enable the GFIR
            SoapySDR::logf(SOAPY_SDR_INFO, "Coefficients configured, now enabling appropriate GFIR");
            switch (GFIR_Index)
            {
                case 0:
                    rfic->Modify_SPI_Reg_bits(isTx?LMS7param(GFIR1_BYP_TXTSP):LMS7param(GFIR1_BYP_RXTSP), 0);
                    break;
                case 1:
                    rfic->Modify_SPI_Reg_bits(isTx?LMS7param(GFIR2_BYP_TXTSP):LMS7param(GFIR2_BYP_RXTSP), 0);
                    break;
                case 2:
                    rfic->Modify_SPI_Reg_bits(isTx?LMS7param(GFIR3_BYP_TXTSP):LMS7param(GFIR3_BYP_RXTSP), 0);
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
                    rfic->Modify_SPI_Reg_bits(isTx?LMS7param(GFIR1_BYP_TXTSP):LMS7param(GFIR1_BYP_RXTSP), 1);
                    break;
                case 1:
                    rfic->Modify_SPI_Reg_bits(isTx?LMS7param(GFIR2_BYP_TXTSP):LMS7param(GFIR2_BYP_RXTSP), 1);
                    break;
                case 2:
                    rfic->Modify_SPI_Reg_bits(isTx?LMS7param(GFIR3_BYP_TXTSP):LMS7param(GFIR3_BYP_RXTSP), 1);
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
            rfic->Modify_SPI_Reg_bits(isTx?LMS7param(INSEL_TXTSP):LMS7param(INSEL_RXTSP), 0, true);
        }
        else
        {
            //Requested to enable the TSG
            rfic->Modify_SPI_Reg_bits(isTx?LMS7param(TSGFC_TXTSP):LMS7param(TSGFC_RXTSP), 1, true); //Full Scale Control
            rfic->Modify_SPI_Reg_bits(isTx?LMS7param(TSGMODE_TXTSP):LMS7param(TSGMODE_RXTSP), 0, true);
            rfic->Modify_SPI_Reg_bits(isTx?LMS7param(TSGSWAPIQ_TXTSP):LMS7param(TSGSWAPIQ_RXTSP), 0, true);

            if(select == 4)
            {
                rfic->Modify_SPI_Reg_bits(isTx?LMS7param(TSGFCW_TXTSP):LMS7param(TSGFCW_RXTSP), 2, true);
                rfic->Modify_SPI_Reg_bits(isTx?LMS7param(INSEL_TXTSP):LMS7param(INSEL_RXTSP), 1, true);
            }
            else if (select == 8)
            {
                rfic->Modify_SPI_Reg_bits(isTx?LMS7param(TSGFCW_TXTSP):LMS7param(TSGFCW_RXTSP), 1, true);
                rfic->Modify_SPI_Reg_bits(isTx?LMS7param(INSEL_TXTSP):LMS7param(INSEL_RXTSP), 1, true);
            }
            else
            {
                throw std::runtime_error("Invalid TSG_NCO option: " + value);
            }
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
    int r = _conn->GPIOWrite(&buffer, 1);
    if (r != 0) throw std::runtime_error("SoapyLMS7::writeGPIO() " + std::string(lime::GetLastErrorMessage()));
}

unsigned SoapyLMS7::readGPIO(const std::string &) const
{
    uint8_t buffer(0);
    int r = _conn->GPIORead(&buffer, 1);
    if (r != 0) throw std::runtime_error("SoapyLMS7::readGPIO() " + std::string(lime::GetLastErrorMessage()));
    return unsigned(buffer);
}

void SoapyLMS7::writeGPIODir(const std::string &, const unsigned dir)
{
    uint8_t buffer = uint8_t(dir);
    int r = _conn->GPIODirWrite(&buffer, 1);
    if (r != 0) throw std::runtime_error("SoapyLMS7::writeGPIODir() " + std::string(lime::GetLastErrorMessage()));
}

unsigned SoapyLMS7::readGPIODir(const std::string &) const
{
    uint8_t buffer(0);
    int r = _conn->GPIODirRead(&buffer, 1);
    if (r != 0) throw std::runtime_error("SoapyLMS7::readGPIODir() " + std::string(lime::GetLastErrorMessage()));
    return unsigned(buffer);
}

/*******************************************************************
 * I2C API
 ******************************************************************/
void SoapyLMS7::writeI2C(const int addr, const std::string &data)
{
    auto st = _conn->WriteI2C(addr, data);
    if (st != 0) throw std::runtime_error(
        "SoapyLMS7::writeI2C("+std::to_string(addr)+") FAIL");
}

std::string SoapyLMS7::readI2C(const int addr, const size_t numBytes)
{
    std::string result;
    auto st = _conn->ReadI2C(addr, numBytes, result);
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
    auto st = _conn->TransactSPI(addr, &input, &readback, 1);
    if (st != 0) throw std::runtime_error(
        "SoapyLMS7::transactSPI("+std::to_string(addr)+") FAIL");
    return readback;
}
