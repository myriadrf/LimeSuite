#ifndef LIME_SAMPLESPACKET_H
#define LIME_SAMPLESPACKET_H

#include "dataTypes.h"

namespace lime {

template<uint8_t chCount> class SamplesPacket
{
  public:
    static constexpr int headerSize = sizeof(SamplesPacket);
    static SamplesPacket* ConstructSamplesPacket(void* vptr, uint32_t samplesCount, uint8_t frameSize)
    {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(vptr);
        SamplesPacket* pkt = reinterpret_cast<SamplesPacket*>(ptr);
        pkt->offset = 0;
        pkt->length = 0;
        pkt->mCapacity = samplesCount;
        pkt->frameSize = frameSize;
        for (int i = 0; i < chCount; ++i)
        {
            pkt->channel[i] = (ptr + headerSize) + (samplesCount * frameSize) * i;
            pkt->head[i] = pkt->tail[i] = pkt->channel[i];
        }
        return pkt;
    }
    SamplesPacket() = delete;
    ~SamplesPacket() = delete;

    inline int size() const
    {
        assert(length >= offset);
        return length - offset;
    };
    inline bool empty() const { return size() == 0; };
    inline int capacity() const { return mCapacity; };
    inline bool isFull() const { return mCapacity - length == 0; };
    inline constexpr int channelCount() const { return chCount; };

    // copies samples data to buffer, returns actual copied samples count
    template<class T> inline int push(const T* const* src, uint16_t count)
    {
        const uint16_t freeSamples = mCapacity - length;
        const int samplesToCopy = std::min(freeSamples, count);
        constexpr int alignment = sizeof(T);
        for (uint8_t i = 0; i < chCount; ++i)
        {
            if (src[i] == nullptr)
                continue;
            memcpy(tail[i], src[i], samplesToCopy * alignment);
            tail[i] += samplesToCopy * frameSize;
        }
        length += samplesToCopy;
        return samplesToCopy;
    }
    // returns number of samples removed
    inline int pop(int count)
    {
        const uint16_t toPop = std::min(count, length - offset);
        for (uint8_t i = 0; i < chCount; ++i)
            head[i] += toPop * frameSize;
        offset += toPop;
        timestamp += toPop; // also offset timestamp
        return toPop;
    }
    inline void* const* front() const { return reinterpret_cast<void* const*>(head); }
    inline void** back() { return reinterpret_cast<void**>(tail); }
    inline void Reset()
    {
        offset = 0;
        length = 0;
        for (uint8_t i = 0; i < chCount; ++i)
            tail[i] = head[i] = channel[i];
    }

    inline void SetSize(uint32_t sz)
    {
        length = sz;
        for (uint8_t i = 0; i < chCount; ++i)
            tail[i] = channel[i] + sz * frameSize;
    }

    template<class T> void Scale(float iScale, float qScale, int channelCount)
    {
        int samplesCount = size();
        for (int c = 0; c < channelCount; ++c)
        {
            T* samples = reinterpret_cast<T*>(head[c]);
            for (int i = 0; i < samplesCount; ++i)
            {
                samples[i].i *= iScale;
                samples[i].q *= qScale;
            }
        }
    }

  private:
    uint8_t* head[chCount];
    uint8_t* tail[chCount];
    uint8_t* channel[chCount];

  public:
    int64_t timestamp;
    uint16_t offset;
    uint16_t length;
    uint16_t mCapacity;
    uint16_t frameSize;
    bool useTimestamp;
    bool flush;
};

} // namespace lime
#endif
