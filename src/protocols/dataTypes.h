#ifndef LMS_DATA_TYPES_H
#define LMS_DATA_TYPES_H

#include <string.h>

namespace lime{

typedef struct
{
    uint8_t reserved[8];
    uint64_t counter;
    uint8_t data[4080];
} FPGA_DataPacket;

typedef struct
{
    int16_t i;
    int16_t q;
} complex16_t;

class SamplesPacket
{
public:
    static const int16_t maxSamplesInPacket = 1360; //total number of samples in all channels combined
    uint64_t timestamp; //timestamp of the packet
    uint16_t first; //index of first unused sample in samples
    uint16_t last; //end index of samples
    complex16_t samples[1360];
    uint32_t flags;

    SamplesPacket()
    {
        timestamp = 0;
        first = 0;
        last = 0;
        flags = 0;
    }
};

}// namespace lime

#endif
