#ifndef LIME_BUFFER_INTERLEAVING_H
#define LIME_BUFFER_INTERLEAVING_H

#include "TRXLooper.h"

namespace lime 
{

template<class DestType, bool mimo, bool compressed>
int ParseRxPayload(const uint8_t* buffer, int bufLen, DestType** samples);

struct DataConversion
{
    SDRDevice::StreamConfig::DataFormat srcFormat;
    SDRDevice::StreamConfig::DataFormat destFormat;
    uint8_t channelCount;
};

int Deinterleave(const DataConversion &fmt, const uint8_t* buffer, uint32_t length, TRXLooper::SamplesPacketType* output);
int Interleave(TRXLooper::SamplesPacketType* input, uint32_t count, const DataConversion &fmt, uint8_t* buffer);

}

#endif