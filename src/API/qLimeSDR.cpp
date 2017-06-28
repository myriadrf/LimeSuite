/*
 * File:   qLimeSDR.cpp
 * Author: Ignas J
 *
 * Created on April 27, 2018
 */
#include "qLimeSDR.h"

LMS7_qLimeSDR::LMS7_qLimeSDR(LMS7_Device *obj) : LMS7_Device(obj)
{
    dacRate = 20e6;
    adcRate = 20e6;
}

size_t LMS7_qLimeSDR::GetNumChannels(const bool tx) const
{
    return 5;
};

int LMS7_qLimeSDR::SetRate(unsigned ch, double rxRate, double txRate, unsigned oversample)
{
    if (ch ==4)
    {
        adcRate = rxRate;
        dacRate = txRate;
        return connection->UpdateExternalDataRate(2,txRate,rxRate,0,0);
    }
    return LMS7_Device::SetRate(ch,rxRate,txRate,oversample);      
}

double LMS7_qLimeSDR::GetRate(bool tx, unsigned chan, double *rf_rate_Hz)
{
    if (chan == 4)
        return tx ? dacRate : adcRate;
    return LMS7_Device::GetRate(tx, chan, rf_rate_Hz);
}