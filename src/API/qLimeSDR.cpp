/*
 * File:   qLimeSDR.cpp
 * Author: Ignas J
 *
 * Created on April 27, 2018
 */
#include "qLimeSDR.h"
#include "FPGA_common.h"

LMS7_qLimeSDR::LMS7_qLimeSDR(lime::IConnection* conn, LMS7_Device *obj) : LMS7_Device(obj), dacRate(20e6), adcRate(20e6)
{
    fpga = new lime::FPGA();
    tx_channels.resize(GetNumChannels());
    rx_channels.resize(GetNumChannels());
  
    while (lms_list.size() < 2)
        lms_list.push_back(new lime::LMS7002M());

    fpga->SetConnection(conn);
    for (unsigned i = 0; i < 2; i++)
    {
        this->lms_list[i]->SetConnection(conn, i);
        mStreamers.push_back(new lime::Streamer(fpga,lms_list[i]));
        lms_list[i]->SetReferenceClk_SX(false, 30.72e6);
    }
    connection = conn;
}

unsigned LMS7_qLimeSDR::GetNumChannels(const bool tx) const
{
    return 5;
};

int LMS7_qLimeSDR::SetRate(unsigned ch, double rxRate, double txRate, unsigned oversample)
{
    if (ch == 4)
    {
        adcRate = rxRate;
        dacRate = txRate;
        return fpga->SetIntetfaceFreq(txRate,rxRate,0,0,2);
    }
    return LMS7_Device::SetRate(ch,rxRate,txRate,oversample);      
}

double LMS7_qLimeSDR::GetRate(bool tx, unsigned chan, double *rf_rate_Hz) const
{
    if (chan == 4)
        return tx ? dacRate : adcRate;
    return LMS7_Device::GetRate(tx, chan, rf_rate_Hz);
}