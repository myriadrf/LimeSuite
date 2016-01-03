/**
@file	Settings.h
@brief	Soapy SDR + IConnection config settings.
@author Lime Microsystems (www.limemicro.com)
*/

#include "SoapyIConnection.h"
#include <IConnection.h>
#include <stdexcept>
#include <LMS7002M.h>
#include <LMS7002M_RegistersMap.h>

/*******************************************************************
 * Constructor/destructor
 ******************************************************************/
SoapyIConnection::SoapyIConnection(const ConnectionHandle &handle):
    _conn(nullptr)
{
    _moduleName = handle.module;
    _conn = ConnectionRegistry::makeConnection(handle);
    const auto devInfo = _conn->GetDeviceInfo();
    for (const auto &addr : devInfo.addrsLMS7002M)
    {
        _rfics.push_back(new LMS7002M());
        _rfics.back()->SetConnection(_conn, addr);
    }
}

SoapyIConnection::~SoapyIConnection(void)
{
    for (auto rfic : _rfics) delete rfic;
    ConnectionRegistry::freeConnection(_conn);
}

LMS7002M *SoapyIConnection::getRFIC(const size_t channel) const
{
    if (_rfics.size() >= channel/2)
    {
        throw std::out_of_range("SoapyIConnection::getRFIC("+std::to_string(channel)+") out of range");
    }
    auto rfic = _rfics.at(channel/2);
    rfic->Modify_SPI_Reg_bits(MAC, (channel%2) + 1);
    return rfic;
}

/*******************************************************************
 * Identification API
 ******************************************************************/
std::string SoapyIConnection::getDriverKey(void) const
{
    return _moduleName;
}

std::string SoapyIConnection::getHardwareKey(void) const
{
    return _conn->GetDeviceInfo().deviceName;
}

SoapySDR::Kwargs SoapyIConnection::getHardwareInfo(void) const
{
    auto devinfo = _conn->GetDeviceInfo();
    SoapySDR::Kwargs info;
    info["expansionName"] = devinfo.expansionName;
    info["firmwareVersion"] = devinfo.firmwareVersion;
    info["hardwareVersion"] = devinfo.hardwareVersion;
    info["protocolVersion"] = devinfo.protocolVersion;
    return info;
}

/*******************************************************************
 * Channels API
 ******************************************************************/

size_t SoapyIConnection::getNumChannels(const int /*direction*/) const
{
    return _rfics.size()*2;
}

bool SoapyIConnection::getFullDuplex(const int /*direction*/, const size_t /*channel*/) const
{
    return true;
}

/*******************************************************************
 * Antenna API
 ******************************************************************/

std::vector<std::string> SoapyIConnection::listAntennas(const int direction, const size_t /*channel*/) const
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
        ants.push_back("BAND1");
        ants.push_back("BAND2");
    }
    return ants;
}

void SoapyIConnection::setAntenna(const int direction, const size_t channel, const std::string &name)
{
    auto rfic = getRFIC(channel);

    if (direction == SOAPY_SDR_RX)
    {
        int sel_path_rfe = 0;
        if (name == "NONE") sel_path_rfe = 0;
        else if (name == "LNAH") sel_path_rfe = 1;
        else if (name == "LNAL") sel_path_rfe = 2;
        else if (name == "LNAW") sel_path_rfe = 3;
        else if (name == "LB1") sel_path_rfe = 3;
        else if (name == "LB2") sel_path_rfe = 2;
        else throw std::runtime_error("SoapyIConnection::setAntenna(RX, "+name+") - unknown antenna name");

        int pd_lna_rfe = (name == "NONE");
        int pd_rloopb_1_rfe = (name != "LB1");
        int pd_rloopb_2_rfe = (name != "LB2");
        int en_inshsw_l_rfe = (name == "LNAL");
        int en_inshsw_w_rfe = (name == "LNAW");
        int en_inshsw_lb1_rfe = (name == "LB1");
        int en_inshsw_lb2_rfe =(name == "LB2");

        rfic->Modify_SPI_Reg_bits(PD_LNA_RFE, pd_lna_rfe);
        rfic->Modify_SPI_Reg_bits(PD_RLOOPB_1_RFE, pd_rloopb_1_rfe);
        rfic->Modify_SPI_Reg_bits(PD_RLOOPB_2_RFE, pd_rloopb_2_rfe);
        rfic->Modify_SPI_Reg_bits(EN_INSHSW_LB1_RFE, en_inshsw_lb1_rfe);
        rfic->Modify_SPI_Reg_bits(EN_INSHSW_LB2_RFE, en_inshsw_lb2_rfe);
        rfic->Modify_SPI_Reg_bits(EN_INSHSW_L_RFE, en_inshsw_l_rfe);
        rfic->Modify_SPI_Reg_bits(EN_INSHSW_W_RFE, en_inshsw_w_rfe);
        rfic->Modify_SPI_Reg_bits(SEL_PATH_RFE, sel_path_rfe);

        //TODO when loopback set: en_loopb_txpad_trf
    }

    if (direction == SOAPY_SDR_TX)
    {
        int band1 = 0, band2 = 0;
        if (name == "BAND1") band1 = 1;
        else if (name == "BAND2") band2 = 1;
        else throw std::runtime_error("SoapyIConnection::setAntenna(TX, "+name+") - unknown antenna name");

        rfic->Modify_SPI_Reg_bits(SEL_BAND1_TRF, band1);
        rfic->Modify_SPI_Reg_bits(SEL_BAND2_TRF, band2);
    }

    _conn->UpdateExternalBandSelect(
            rfic->Get_SPI_Reg_bits(SEL_BAND2_TRF),
            rfic->Get_SPI_Reg_bits(SEL_PATH_RFE));
}

std::string SoapyIConnection::getAntenna(const int direction, const size_t channel) const
{
    auto rfic = getRFIC(channel);

    if (direction == SOAPY_SDR_RX)
    {
        if (rfic->Get_SPI_Reg_bits(PD_LNA_RFE) != 0) return "NONE";
        if (rfic->Get_SPI_Reg_bits(EN_INSHSW_LB1_RFE) != 0) return "LB1";
        if (rfic->Get_SPI_Reg_bits(EN_INSHSW_LB2_RFE) != 0) return "LB2";
        if (rfic->Get_SPI_Reg_bits(EN_INSHSW_L_RFE) != 0) return "LNAL";
        if (rfic->Get_SPI_Reg_bits(EN_INSHSW_W_RFE) != 0) return "LNAW";
        return "LNAH";
    }

    if (direction == SOAPY_SDR_TX)
    {
        return (rfic->Get_SPI_Reg_bits(SEL_BAND2_TRF) == 1)?"BAND2":"BAND1";
    }
}

/*******************************************************************
 * Frontend corrections API
 ******************************************************************/

bool SoapyIConnection::hasDCOffsetMode(const int direction, const size_t /*channel*/) const
{
    return (direction == SOAPY_SDR_RX);
}

void SoapyIConnection::setDCOffsetMode(const int direction, const size_t channel, const bool automatic)
{
    //TODO set DC automatic removal on RX DSP chain only
}

bool SoapyIConnection::getDCOffsetMode(const int direction, const size_t channel) const
{
    
}

bool SoapyIConnection::hasDCOffset(const int direction, const size_t /*channel*/) const
{
    return (direction == SOAPY_SDR_TX);
}

void SoapyIConnection::setDCOffset(const int direction, const size_t channel, const std::complex<double> &offset)
{
    //TODO set DC offset registers TX DSP chan only
}

std::complex<double> SoapyIConnection::getDCOffset(const int direction, const size_t channel) const
{
    
}

bool SoapyIConnection::hasIQBalance(const int /*direction*/, const size_t /*channel*/) const
{
    return true;
}

void SoapyIConnection::setIQBalance(const int direction, const size_t channel, const std::complex<double> &balance)
{
    //TODO IQ balance on Tx and Rx DSP chains
}

std::complex<double> SoapyIConnection::getIQBalance(const int direction, const size_t channel) const
{
    
}

/*******************************************************************
 * Gain API
 ******************************************************************/

std::vector<std::string> SoapyIConnection::listGains(const int direction, const size_t /*channel*/) const
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

void SoapyIConnection::setGain(const int direction, const size_t channel, const std::string &name, const double value)
{
    //TODO set gain based on name
}

double SoapyIConnection::getGain(const int direction, const size_t channel, const std::string &name) const
{
    
}

SoapySDR::Range SoapyIConnection::getGainRange(const int direction, const size_t channel, const std::string &name) const
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

void SoapyIConnection::setFrequency(const int direction, const size_t channel, const std::string &name, const double frequency, const SoapySDR::Kwargs &args)
{
    auto rfic = getRFIC(channel);
    auto ref_MHz = _conn->GetReferenceClockRate()/1e6;

    if (name == "RF")
    {
        rfic->SetFrequencySX(direction == SOAPY_SDR_TX, frequency/1e6, ref_MHz);
        return;
    }

    if (name == "BB")
    {
        //TODO configure dsp bypass
        rfic->SetNCOFrequency(direction == SOAPY_SDR_TX, 0, frequency/1e6);
        return;
    }

    throw std::runtime_error("SoapyIConnection::getFrequency("+name+") unknown name");
}

double SoapyIConnection::getFrequency(const int direction, const size_t channel, const std::string &name) const
{
    auto rfic = getRFIC(channel);
    auto ref_MHz = _conn->GetReferenceClockRate()/1e6;

    if (name == "RF")
    {
        return rfic->GetFrequencySX_MHz(direction == SOAPY_SDR_TX, ref_MHz)*1e6;
    }

    if (name == "BB")
    {
        return rfic->GetNCOFrequency_MHz(direction == SOAPY_SDR_TX, 0, ref_MHz)*1e6;
    }

    throw std::runtime_error("SoapyIConnection::getFrequency("+name+") unknown name");
}

std::vector<std::string> SoapyIConnection::listFrequencies(const int /*direction*/, const size_t /*channel*/) const
{
    std::vector<std::string> opts;
    opts.push_back("RF");
    opts.push_back("BB");
    return opts;
}

SoapySDR::RangeList SoapyIConnection::getFrequencyRange(const int direction, const size_t channel, const std::string &name) const
{
    auto rfic = getRFIC(channel);

    SoapySDR::RangeList ranges;
    if (name == "RF")
    {
        ranges.push_back(SoapySDR::Range(100e3, 3.8e9));
    }
    if (name == "BB")
    {
        const double dspRate = rfic->GetReferenceClk_TSP_MHz(direction == SOAPY_SDR_TX)*1e6;
        ranges.push_back(SoapySDR::Range(-dspRate/2, dspRate/2));
    }
    return ranges;
}

/*******************************************************************
 * Sample Rate API
 ******************************************************************/

void SoapyIConnection::setSampleRate(const int direction, const size_t channel, const double rate)
{
    //TODO set interp/decim
}

double SoapyIConnection::getSampleRate(const int direction, const size_t channel) const
{
    
}

std::vector<double> SoapyIConnection::listSampleRates(const int direction, const size_t channel) const
{
    const double dspRate = 1e6; //TODO get from board
    std::vector<double> rates;
    for (int i = 5; i >= 0; i--)
    {
        rates.push_back(dspRate/(1 << i));
    }
    return rates;
}

void SoapyIConnection::setBandwidth(const int direction, const size_t channel, const double bw)
{
    //TODO set RBB/TBB BW + calibrate here?
}

double SoapyIConnection::getBandwidth(const int direction, const size_t channel) const
{
    
}

std::vector<double> SoapyIConnection::listBandwidths(const int direction, const size_t channel) const
{
    std::vector<double> bws;

    if (direction == SOAPY_SDR_RX)
    {
        bws.push_back(1.4e6);
        bws.push_back(3.0e6);
        bws.push_back(5.0e6);
        bws.push_back(10.0e6);
        bws.push_back(15.0e6);
        bws.push_back(20.0e6);
        bws.push_back(37.0e6);
        bws.push_back(66.0e6);
        bws.push_back(108.0e6);
    }
    if (direction == SOAPY_SDR_TX)
    {
        bws.push_back(2.4e6);
        bws.push_back(2.74e6);
        bws.push_back(5.5e6);
        bws.push_back(8.2e6);
        bws.push_back(11.0e6);
        bws.push_back(18.5e6);
        bws.push_back(38.0e6);
        bws.push_back(54.0e6);
    }

    return bws;
}

/*******************************************************************
 * Clocking API
 ******************************************************************/

void SoapyIConnection::setMasterClockRate(const double rate)
{
    for (auto rfic : _rfics)
    {
        rfic->SetFrequencyCGEN(rate/1e6);
    }
}

double SoapyIConnection::getMasterClockRate(void) const
{
    auto rfic = this->getRFIC(0); //same for all RFIC
    return rfic->GetFrequencyCGEN_MHz()*1e6;
}

/*******************************************************************
 * Time API
 ******************************************************************/

bool SoapyIConnection::hasHardwareTime(const std::string &what) const
{
    //TODO
    return false;
}

long long SoapyIConnection::getHardwareTime(const std::string &what) const
{
    //TODO put this call on iconnection...
}

void SoapyIConnection::setHardwareTime(const long long timeNs, const std::string &what)
{
    //TODO put this call on iconnection...
}

/*******************************************************************
 * I2C API
 ******************************************************************/
void SoapyIConnection::writeI2C(const int addr, const std::string &data)
{
    auto st = _conn->WriteI2C(addr, data);
    if (st != OperationStatus::SUCCESS) throw std::runtime_error(
        "SoapyIConnection::writeI2C("+std::to_string(addr)+") FAIL");
}

std::string SoapyIConnection::readI2C(const int addr, const size_t numBytes)
{
    std::string result;
    auto st = _conn->ReadI2C(addr, numBytes, result);
    if (st != OperationStatus::SUCCESS) throw std::runtime_error(
        "SoapyIConnection::readI2C("+std::to_string(addr)+") FAIL");
    return result;
}

/*******************************************************************
 * SPI API
 ******************************************************************/
unsigned SoapyIConnection::transactSPI(const int addr, const unsigned data, const size_t /*numBits*/)
{
    uint32_t input = data;
    uint32_t readback = 0;
    auto st = _conn->TransactSPI(addr, &input, &readback, 1);
    if (st != OperationStatus::SUCCESS) throw std::runtime_error(
        "SoapyIConnection::transactSPI("+std::to_string(addr)+") FAIL");
    return readback;
}
