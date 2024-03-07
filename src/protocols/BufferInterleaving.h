#ifndef LIME_BUFFER_INTERLEAVING_H
#define LIME_BUFFER_INTERLEAVING_H

#include "TRXLooper.h"

namespace lime {

struct DataConversion {
    SDRDevice::StreamConfig::DataFormat srcFormat;
    SDRDevice::StreamConfig::DataFormat destFormat;
    uint8_t channelCount;
};

int Deinterleave(void** dest, const uint8_t* buffer, uint32_t length, const DataConversion& fmt);
int Interleave(uint8_t* dest, const void* const* src, uint32_t count, const DataConversion& fmt);

} // namespace lime

#endif