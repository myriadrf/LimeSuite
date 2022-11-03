#ifndef LMS_DATA_TYPES_H
#define LMS_DATA_TYPES_H

#include <string.h>
#include <stdint.h>
#include <utility>
#include <complex>
#include <chrono>
#include <type_traits>
#include <assert.h>

namespace lime{

template<typename T, uint8_t chCount>
class StagingPacket
{
public:
    StagingPacket(uint16_t samplesCount) : timestamp(0), useTimestamp(false), flush(false),
        offset(0), length(0), mCapacity(samplesCount)
    {
        assert(samplesCount > 0);
        constexpr int alignment = sizeof(T);
        for(uint8_t i=0; i<chCount; ++i)
        {
            uint8_t* mem = (uint8_t*)aligned_alloc(alignment, alignment*samplesCount);
            tail[i] = head[i] = channel[i] = reinterpret_cast<T*>(mem);
        }

    };
    ~StagingPacket()
    {
        for(uint8_t i=0; i<chCount; ++i)
            free(channel[i]);
    };
    int64_t timestamp;
    bool useTimestamp;
    bool flush;

    inline int size() const { return length-offset; };
    inline bool empty() const { return size() == 0; };
    inline int capacity() const { return mCapacity; };
    inline bool isFull() const { return mCapacity-length == 0; };
    inline constexpr int channelCount() const { return chCount; };

    // copies samples data to buffer, returns actual copied samples count
    inline int push(const T* const * src, uint16_t count)
    {
        const uint16_t freeSamples = mCapacity-length;
        const int samplesToCopy = std::min(freeSamples, count);
        constexpr int alignment = sizeof(T);
        for(uint8_t i=0; i<chCount; ++i)
        {
            if(src[i] == nullptr)
                continue;
            memcpy(tail[i], src[i], samplesToCopy * alignment);
            tail[i] += samplesToCopy;
        }
        length += samplesToCopy;
        return samplesToCopy;
    }
    // returns number of samples removed
    inline int pop(int count)
    {
        const uint16_t toPop = std::min(count, length-offset);
        for(uint8_t i=0; i<chCount; ++i)
            head[i] += toPop;
        offset += toPop;
        timestamp += toPop; // also offset timestamp
        return toPop;
    }
    inline T* const * front() const { return head; }
    inline void Reset() {
        offset = 0;
        length = 0;
        for(uint8_t i=0; i<chCount; ++i)
            tail[i] = head[i] = channel[i];
    }
private:
    T* head[chCount];
    T* tail[chCount];
    T* channel[chCount];
    uint16_t offset;
    uint16_t length;
    uint16_t mCapacity;
};

template<class T>
struct PartialPacket
{
    PartialPacket() : timestamp(0), start(0), end(0), useTimestamp(false) {};
    static constexpr int samplesStorage = 510*8;
    T chA[samplesStorage];
    T chB[samplesStorage];
    int64_t timestamp;
    int16_t start;
    int16_t end;
    bool useTimestamp;
    bool flush;
};

struct DataBlock
{
    DataBlock() : size(0), usage(0), offset(0), ptr(nullptr) {};
    inline bool isValid() const {
        return (ptr != nullptr) && (usage <= size) && (offset <= usage);
    }
    // ~DataBlock() { if(ptr != nullptr) delete ptr; };
    std::chrono::time_point<std::chrono::high_resolution_clock> genTime;
    int32_t size;
    int32_t usage;
    int32_t offset;
    void* ptr;
};

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
        header0 &= ~mask; //clear ignore timestamp
        header0 |= enabled ? mask : 0; //ignore timestamp
    }
    inline bool getIgnoreTimestamp() {
        const uint8_t mask = 1 << 4;
        return header0 & mask; //ignore timestamp
    }
    inline void ClearHeader() {
        memset(this, 0, 16);
    }
    inline float RxFIFOFill() {
        return (header0 & 0x7) * 0.125;
    }
    inline void SetPayloadSize(uint16_t size) {
        payloadSizeLSB = size & 0xFF;
        payloadSizeMSB = (size >> 8) & 0xFF;
    }
    inline int16_t GetPayloadSize() const {
        return (payloadSizeMSB << 8) | payloadSizeLSB;
    }
    // order matters
    uint8_t header0;
    // payload size specifies how many bytes are valid samples data, 0-full packet is valid
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
    int64_t timestamp; //timestamp of the packet
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
