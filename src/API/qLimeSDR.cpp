/*
 * File:   qLimeSDR.cpp
 * Author: Ignas J
 *
 * Created on April 27, 2018
 */
#include "qLimeSDR.h"

LMS7_qLimeSDR::LMS7_qLimeSDR(LMS7_Device *obj) : LMS7_Device(obj)
{
}

size_t LMS7_qLimeSDR::GetNumChannels(const bool tx) const
{
    return 4;
};