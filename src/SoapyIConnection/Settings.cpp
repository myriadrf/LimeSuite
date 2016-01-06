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
#include <SoapySDR/Logger.hpp>

/*******************************************************************
 * Special LMS7002M with log forwarding
 ******************************************************************/
class LMS7002M_withLogging : public LMS7002M
{
public:
    LMS7002M_withLogging(void):
        LMS7002M()
    {
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
SoapyIConnection::SoapyIConnection(const ConnectionHandle &handle):
    _conn(nullptr)
{
    _moduleName = handle.module;
    _conn = ConnectionRegistry::makeConnection(handle);
    const auto devInfo = _conn->GetDeviceInfo();
    for (const auto &addr : devInfo.addrsLMS7002M)
    {
        SoapySDR::logf(SOAPY_SDR_INFO, "Init LMS7002M(%d)", addr);
        _rfics.push_back(new LMS7002M_withLogging());
        _rfics.back()->SetConnection(_conn, addr);
    }

    //enable all channels
    for (size_t i = 0; i < _rfics.size()*2; i++)
    {
        this->SetComponentsEnabled(i, true);
    }
}

SoapyIConnection::~SoapyIConnection(void)
{
    //power down all channels
    for (size_t i = 0; i < _rfics.size()*2; i++)
    {
        this->SetComponentsEnabled(i, false);
    }

    for (auto rfic : _rfics) delete rfic;
    ConnectionRegistry::freeConnection(_conn);
}

LMS7002M *SoapyIConnection::getRFIC(const size_t channel) const
{
    if (_rfics.size() <= channel/2)
    {
        throw std::out_of_range("SoapyIConnection::getRFIC("+std::to_string(channel)+") out of range");
    }
    auto rfic = _rfics.at(channel/2);
    rfic->Modify_SPI_Reg_bits(MAC, (channel%2) + 1);
    return rfic;
}

void SoapyIConnection::SetComponentsEnabled(const size_t channel, const bool enable)
{
    SoapySDR::logf(SOAPY_SDR_INFO, "%s LMS7002M::ch%d", enable?"Enable":"Disable", int(channel));

    auto rfic = getRFIC(channel);

    //--- ADC/DAC ---
    rfic->Modify_SPI_Reg_bits(EN_DIR_AFE, 1);
    rfic->Modify_SPI_Reg_bits(EN_G_AFE, enable?1:0);
    rfic->Modify_SPI_Reg_bits(PD_AFE, enable?0:1);
    if ((channel%2) == 0)
    {
        rfic->Modify_SPI_Reg_bits(PD_TX_AFE1, enable?0:1);
        rfic->Modify_SPI_Reg_bits(PD_RX_AFE1, enable?0:1);
    }
    else
    {
        rfic->Modify_SPI_Reg_bits(PD_TX_AFE2, enable?0:1);
        rfic->Modify_SPI_Reg_bits(PD_RX_AFE2, enable?0:1);
    }

    //--- digital ---
    rfic->Modify_SPI_Reg_bits(EN_RXTSP, enable?1:0);
    rfic->Modify_SPI_Reg_bits(EN_TXTSP, enable?1:0);

    //--- baseband ---
    rfic->Modify_SPI_Reg_bits(EN_DIR_RBB, 1);
    rfic->Modify_SPI_Reg_bits(EN_DIR_TBB, 1);
    rfic->Modify_SPI_Reg_bits(EN_G_RBB, enable?1:0);
    rfic->Modify_SPI_Reg_bits(EN_G_TBB, enable?1:0);

    //--- frontend ---
    rfic->Modify_SPI_Reg_bits(EN_DIR_RFE, 1);
    rfic->Modify_SPI_Reg_bits(EN_DIR_TRF, 1);
    rfic->Modify_SPI_Reg_bits(EN_G_RFE, enable?1:0);
    rfic->Modify_SPI_Reg_bits(EN_G_TRF, enable?1:0);

    //--- synthesizers ---
    rfic->Modify_SPI_Reg_bits(EN_DIR_SXRSXT, 1);
    rfic->Modify_SPI_Reg_bits(EN_G, enable?1:0);
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

    return "";
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
    return 0;
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
    return 0;
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
    return 0;
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
    auto rfic = getRFIC(channel);

    if (direction == SOAPY_SDR_RX and name == "LNA")
    {
        const double gmax = 30;
        double val = value - gmax;

        int g_lna_rfe = 0;
        if (val >= 0) g_lna_rfe = 15;
        else if (val >= -1) g_lna_rfe = 14;
        else if (val >= -2) g_lna_rfe = 13;
        else if (val >= -3) g_lna_rfe = 12;
        else if (val >= -4) g_lna_rfe = 11;
        else if (val >= -5) g_lna_rfe = 10;
        else if (val >= -6) g_lna_rfe = 9;
        else if (val >= -9) g_lna_rfe = 8;
        else if (val >= -12) g_lna_rfe = 7;
        else if (val >= -15) g_lna_rfe = 6;
        else if (val >= -18) g_lna_rfe = 5;
        else if (val >= -21) g_lna_rfe = 4;
        else if (val >= -24) g_lna_rfe = 3;
        else if (val >= -27) g_lna_rfe = 2;
        else g_lna_rfe = 1;

        rfic->Modify_SPI_Reg_bits(G_LNA_RFE, g_lna_rfe);
    }

    else if (direction == SOAPY_SDR_RX and name == "TIA")
    {
        const double gmax = 12;
        double val = value - gmax;

        int g_tia_rfe = 0;
        if (val >= 0) g_tia_rfe = 3;
        else if (val >= -3) g_tia_rfe = 2;
        else g_tia_rfe = 1;

        rfic->Modify_SPI_Reg_bits(G_TIA_RFE, g_tia_rfe);
    }

    else if (direction == SOAPY_SDR_RX and name == "PGA")
    {
        int g_pga_rbb = (int)(value + 12.5);
        if (g_pga_rbb > 0x1f) g_pga_rbb = 0x1f;
        if (g_pga_rbb < 0) g_pga_rbb = 0;
        rfic->Modify_SPI_Reg_bits(G_PGA_RBB, g_pga_rbb);

        //TODO set rcc_ctl_pga_rbb, c_ctl_pga_rbb
    }

    else if (direction == SOAPY_SDR_TX and name == "PAD")
    {
        const double pmax = 0;
        double loss = pmax-value;

        //different scaling realm
        if (loss > 10) loss = (loss+10)/2;

        //clip
        if (loss > 31) loss = 31;
        if (loss < 0) loss = 0;

        //integer round
        int loss_int = (int)(loss + 0.5);

        rfic->Modify_SPI_Reg_bits(LOSS_LIN_TXPAD_TRF, loss_int);
        rfic->Modify_SPI_Reg_bits(LOSS_MAIN_TXPAD_TRF, loss_int);
    }

    else throw std::runtime_error("SoapyIConnection::setGain("+name+") - unknown gain name");
}

double SoapyIConnection::getGain(const int direction, const size_t channel, const std::string &name) const
{
    auto rfic = getRFIC(channel);

    if (direction == SOAPY_SDR_RX and name == "LNA")
    {
        const double gmax = 30;
        auto g_lna_rfe = rfic->Get_SPI_Reg_bits(G_LNA_RFE);
        switch (g_lna_rfe)
        {
        case 15: return gmax-0;
        case 14: return gmax-1;
        case 13: return gmax-2;
        case 12: return gmax-3;
        case 11: return gmax-4;
        case 10: return gmax-5;
        case 9: return gmax-6;
        case 8: return gmax-9;
        case 7: return gmax-12;
        case 6: return gmax-15;
        case 5: return gmax-18;
        case 4: return gmax-21;
        case 3: return gmax-24;
        case 2: return gmax-27;
        case 1: return gmax-30;
        }
        return 0.0;
    }

    else if (direction == SOAPY_SDR_RX and name == "TIA")
    {
        const double gmax = 12;
        auto g_tia_rfe = rfic->Get_SPI_Reg_bits(G_TIA_RFE);
        switch (g_tia_rfe)
        {
        case 3: return gmax-0;
        case 2: return gmax-3;
        case 1: return gmax-12;
        }
        return 0.0;
    }

    else if (direction == SOAPY_SDR_RX and name == "PGA")
    {
        auto g_pga_rbb = rfic->Get_SPI_Reg_bits(G_PGA_RBB);
        return g_pga_rbb - 12;
    }

    else if (direction == SOAPY_SDR_TX and name == "PAD")
    {
        const double pmax = 0;
        auto loss_int = rfic->Get_SPI_Reg_bits(LOSS_LIN_TXPAD_TRF);
        if (loss_int > 10) return pmax-10-2*(loss_int-10);
        return pmax-loss_int;
    }

    else throw std::runtime_error("SoapyIConnection::getGain("+name+") - unknown gain name");
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
    SoapySDR::logf(SOAPY_SDR_INFO, "SoapyIConnection::setFrequency(%d, %s, %f MHz), ref %f MHz", direction, name.c_str(), frequency/1e6, ref_MHz);

    if (name == "RF")
    {
        rfic->SetFrequencySX(direction == SOAPY_SDR_TX, frequency/1e6, ref_MHz);
        return;
    }

    if (name == "BB")
    {
        switch (direction)
        {
        case SOAPY_SDR_RX: rfic->Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, (frequency == 0)?1:0);
        case SOAPY_SDR_TX: rfic->Modify_SPI_Reg_bits(CMIX_BYP_TXTSP, (frequency == 0)?1:0);
        }
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
    auto rfic = getRFIC(channel);

    const double dspRate = rfic->GetReferenceClk_TSP_MHz(direction == SOAPY_SDR_TX)*1e6;
    const double factor = dspRate/rate;
    SoapySDR::logf(SOAPY_SDR_INFO, "SoapyIConnection::setSampleRate(%d, %f MHz), baseRate %f MHz, factor %f", direction, rate/1e6, dspRate/1e6, factor);
    if (factor < 2.0) throw std::runtime_error("SoapyIConnection::setSampleRate() -- rate too high");
    int intFactor = 1 << int((std::log(factor)/std::log(2.0)) + 0.5);
    if (intFactor > 32) throw std::runtime_error("SoapyIConnection::setSampleRate() -- rate too low");

    if (std::abs(factor-intFactor) > 0.01) SoapySDR::logf(SOAPY_SDR_WARNING,
        "SoapyIConnection::setSampleRate(): not a power of two factor: TSP Rate = %f MHZ, Requested rate = %f MHz", dspRate/1e6, rate/1e6);

    switch (direction)
    {
    case SOAPY_SDR_TX: _interps[channel] = intFactor; break;
    case SOAPY_SDR_RX: _decims[channel] = intFactor; break;
    }

    rfic->SetInterfaceFrequency(
        this->getMasterClockRate()/1e6,
        int(std::log(double(_interps[channel]))/std::log(2.0)),
        int(std::log(double(_decims[channel]))/std::log(2.0)));
}

double SoapyIConnection::getSampleRate(const int direction, const size_t channel) const
{
    auto rfic = getRFIC(channel);
    const double dspRate = rfic->GetReferenceClk_TSP_MHz(direction == SOAPY_SDR_TX)*1e6;

    try
    {
        switch (direction)
        {
        case SOAPY_SDR_TX: return dspRate/_interps.at(channel);
        case SOAPY_SDR_RX: return dspRate/_decims.at(channel);
        }
    }
    catch (...)
    {
        return dspRate;
    }
    return dspRate;
}

std::vector<double> SoapyIConnection::listSampleRates(const int direction, const size_t channel) const
{
    auto rfic = getRFIC(channel);

    const double dspRate = rfic->GetReferenceClk_TSP_MHz(direction == SOAPY_SDR_TX)*1e6;
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

void SoapyIConnection::setBandwidth(const int direction, const size_t channel, const double bw)
{
    auto rfic = getRFIC(channel);
    auto &actual = _actualBw[direction][channel];
    actual = bw;

    if (direction == SOAPY_SDR_RX)
    {
        const bool hb = bw >= 37.0e6;
        const bool bypass = bw > 108.0e6;

        //run the calibration for this bandwidth setting
        auto status = rfic->CalibrateRx(bw/1e6);
        if (!bypass && status == LIBLMS7_SUCCESS)
        {
            LMS7002M::RxFilter filter;
            if (hb) filter = LMS7002M::RX_LPF_HIGHBAND;
            else filter = LMS7002M::RX_LPF_LOWBAND;

            status = rfic->TuneRxFilter(filter, bw/1e6);
        }
        if (status == LIBLMS7_SUCCESS)
        {
            status = rfic->TuneRxFilter(LMS7002M::RX_TIA, bw/1e6);
        }
        if (status != LIBLMS7_SUCCESS)
        {
            //TODO log failure
        }
    }

    if (direction == SOAPY_SDR_TX)
    {
        const bool hb = bw >= 18.5e6;
        const bool bypass = bw > 54.0e6;

        //run the calibration for this bandwidth setting
        auto status = rfic->CalibrateTx(bw/1e6);
        if (!bypass && status == LIBLMS7_SUCCESS)
        {
            LMS7002M::TxFilter filter;
            if (hb) filter = LMS7002M::TX_HIGHBAND;
            else if (bw >= 2.4e6) filter = LMS7002M::TX_LADDER;
            else filter = LMS7002M::TX_REALPOLE;

            status = rfic->TuneTxFilter(filter, bw/1e6);
        }
        if (status != LIBLMS7_SUCCESS)
        {
            //TODO log failure
        }
    }
}

double SoapyIConnection::getBandwidth(const int direction, const size_t channel) const
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
    return 0;
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
