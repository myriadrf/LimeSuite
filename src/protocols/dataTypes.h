#ifndef LMS_DATA_TYPES_H
#define LMS_DATA_TYPES_H

#include <string.h>
#include <stdint.h>
#include <utility>
#include <complex>

namespace lime{

struct FPGA_DataPacket
{
    FPGA_DataPacket() {
        memset(this, 0, sizeof(FPGA_DataPacket));
    }

    inline bool txWasDropped() const {
        return header0 & (1 << 3);
    }
    inline void ignoreTimestamp(bool enabled) {
        const uint8_t mask = 1 << 4;
        header0 &= ~mask; //ignore timestamp
        header0 |= enabled ? mask : 0; //ignore timestamp
    }
    inline void ClearHeader() {
        memset(this, 0, 16);
    }
    // order matters
    uint8_t header0;
    uint8_t payloadSizeLSB;
    uint8_t payloadSizeMSB;
    uint8_t reserved[5];
    int64_t counter; // should be unsigned, but that's prone to underflow during arithmetic and would choke FPGA, packets would not be sent
    uint8_t data[4080];
};

struct complex16_t
{
    int16_t i;
    int16_t q;
};

struct complex32f_t
{
    float i;
    float q;
};

//typedef std::complex<float> complex32f_t;

const int samples12InPkt = 1360;
const int samples16InPkt = 1020;

class SamplesPacket
{
public:
    uint64_t timestamp; //timestamp of the packet
    uint32_t last; //end index of samples
    uint32_t flags;
    complex16_t* samples;

    SamplesPacket(int size = 0):
        timestamp(0),
        last(0),
        flags(0),
        samples(size ? new complex16_t[size]: nullptr)  {};

    SamplesPacket (SamplesPacket&& pkt)
    {
        timestamp = pkt.timestamp;
        last = pkt.last;
        flags = pkt.flags;
        samples = pkt.samples;
        pkt.samples = nullptr;
    };

    SamplesPacket& operator=(SamplesPacket&& pkt)
    {
        timestamp = pkt.timestamp;
        last = pkt.last;
        flags = pkt.flags;
        std::swap(samples, pkt.samples);
        return *this;
    }
    ~SamplesPacket()
    {
        if (samples)
            delete [] samples;
    };

    SamplesPacket (const SamplesPacket&) = delete;
    SamplesPacket& operator=(const SamplesPacket&) = delete;
};

}// namespace lime

#endif
