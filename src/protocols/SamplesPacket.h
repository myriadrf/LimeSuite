#ifndef LIME_SAMPLESPACKET_H
#define LIME_SAMPLESPACKET_H

#include <cassert>
#include <cstring>

#include "limesuite/SDRDevice.h"

namespace lime {

/**
  @brief Class for holding a buffer of samples for @p chCount channels.
  @tparam chCount The number of channels the packet holds.
 */
template<uint8_t chCount> class SamplesPacket
{
  public:
    /** The size of the structure that holds the sample packet information. */
    static constexpr int headerSize = sizeof(SamplesPacket);

    /**
      @brief Constructs the sample packet class.
      @param vptr Pointer to the sample data.
      @param samplesCount The amount of samples to store.
      @param frameSize The size of a single sample.
      @return A pointer to the SamplePacket class.
     */
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

    /**
      @brief Gets the remaining amount of samples in the packet.
      @return The amount of samples remaining.
     */
    inline constexpr uint32_t size() const
    {
        assert(length >= offset);
        return length - offset;
    };

    /**
      @brief Gets whether the packet is empty or not.
      @return True if empty.
      @return false if it has elements.
     */
    inline constexpr bool empty() const { return size() == 0; };

    /**
      @brief Gets the maximum amount of samples this packet can hold.
      @return The amount of samples the packet can hold.
     */
    inline constexpr int capacity() const { return mCapacity; };

    /**
      @brief Gets whether the packet is full or not.
      @return True if full.
      @return False if there is still space..
     */
    inline constexpr bool isFull() const { return mCapacity - length == 0; };

    /**
      @brief Gets the channel count of this packet.
      @return The amount of channels this packet holds the information for.
     */
    inline constexpr int channelCount() const { return chCount; };

    /**
      @brief Copies samples data to buffer.
      @tparam T The type of samples to copy.
      @param src The source array of the samples.
      @param count The amount of samples to copy.
      @return Actual copied samples count
     */
    template<class T> inline int push(const T* const* src, uint16_t count)
    {
        const uint16_t freeSamples = mCapacity - length;
        const uint16_t samplesToCopy = std::min(freeSamples, count);
        constexpr std::size_t alignment = sizeof(T);
        for (uint8_t i = 0; i < chCount; ++i)
        {
            if (src[i] == nullptr)
                continue;
            std::memcpy(tail[i], src[i], samplesToCopy * alignment);
            tail[i] += samplesToCopy * frameSize;
        }
        length += samplesToCopy;
        return samplesToCopy;
    }

    /**
      @brief Removes a given amount of samples from the packet.
      @param count The number of samples to remove.
      @return Number of samples actually removed.
     */
    inline uint32_t pop(uint32_t count)
    {
        const uint32_t toPop = std::min(count, length - offset);
        for (uint8_t i = 0; i < chCount; ++i)
            head[i] += toPop * frameSize;
        offset += toPop;
        metadata.timestamp += toPop; // Also offset timestamp
        return toPop;
    }

    /**
      @brief Gets a pointer to the front element of the packet.
      @return A pointer to the front of the packet elements.
     */
    inline constexpr void* const* front() const { return reinterpret_cast<void* const*>(head); }

    /**
      @brief Gets a pointer to the back element of the packet.
      @return A pointer to the back of the packet elements.
     */
    inline constexpr void* const* back() const { return reinterpret_cast<void* const*>(tail); }

    /** @brief Resets all the packet to be blank. */
    inline void Reset()
    {
        offset = 0;
        length = 0;
        for (uint8_t i = 0; i < chCount; ++i)
            tail[i] = head[i] = channel[i];
    }

    /**
      @brief Sets the amount of samples of this packet.
      @param sampleCount The amount of samples this packet has.
     */
    inline void SetSize(uint32_t sampleCount)
    {
        length = sampleCount;
        for (uint8_t i = 0; i < chCount; ++i)
            tail[i] = channel[i] + sampleCount * frameSize;
    }

    /**
      @brief Scales the packet's samples' I and Q values by the given multiplier. 
      @tparam T The type of samples the packet is holding.
      @param iScale The multiplier with which to multiply all the I values.
      @param qScale The multiplier with which to multiply all the Q values.
      @param channelCount The amount of channels to multiply the values for.
     */
    template<class T> void Scale(float iScale, float qScale, int channelCount)
    {
        assert(channelCount <= chCount);

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

    uint32_t offset;
    uint32_t length;
    uint32_t mCapacity;
    uint8_t frameSize;

  public:
    SDRDevice::StreamMeta metadata; ///< The stream metadata of the packet.
};

} // namespace lime
#endif
