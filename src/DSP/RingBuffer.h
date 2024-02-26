#pragma once

#include <string.h>
#include <vector>
#include <mutex>

template<class T> class RingBuffer
{
  public:
    RingBuffer(int32_t count)
        : headIndex(0)
        , tailIndex(0)
        , size(0)
        , capacity(count)
    {
        buffer.resize(count);
    }

    int Consume(T* dest, int32_t count)
    {
        std::lock_guard<std::mutex> lck(mMutex);
        int consumed = 0;
        count = std::min(count, size);

        if (headIndex + count >= capacity)
        {
            int toCopy = capacity - headIndex;
            memcpy(dest, &buffer[headIndex], toCopy * sizeof(T));
            dest += toCopy;
            headIndex = 0;
            size -= toCopy;
            count -= toCopy;
            consumed += toCopy;
        }
        memcpy(dest, &buffer[headIndex], count * sizeof(T));
        headIndex += count;
        size -= count;
        consumed += count;
        return consumed;
    }

    int Produce(const T* src, int32_t count)
    {
        std::lock_guard<std::mutex> lck(mMutex);
        count = std::min(count, capacity - size);
        int produced = 0;

        if (tailIndex + count >= capacity)
        {
            int toCopy = capacity - tailIndex;
            memcpy(&buffer[tailIndex], src, toCopy * sizeof(T));
            src += toCopy;
            count -= toCopy;
            size += toCopy;
            produced += toCopy;
            tailIndex = 0;
        }
        memcpy(&buffer[tailIndex], src, count * sizeof(T));
        tailIndex += count;
        size += count;
        produced += count;
        return produced;
    }

  private:
    std::mutex mMutex;
    std::vector<T> buffer;
    int headIndex;
    int tailIndex;
    int size;
    int capacity;
};
