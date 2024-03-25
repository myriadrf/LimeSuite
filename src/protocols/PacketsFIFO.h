/**
  @file:   PacketsFIFO.h
  @author: Sander Jobing

  Created on July 29, 2017, 5:17 PM

  This class implements a Single Producer - Single Consumer lock-free and
  wait-free queue. Only 1 thread can fill the queue, another thread can read
  from the queue, but no more threads are allowed. This lock-free queue
  is a fifo queue, the first element inserted is the first element which
  comes out.

  Thanks to Timur Doumler, Juce
  https://www.youtube.com/watch?v=qdrp6k4rcP4
 */

#ifndef SPSCLOCKFREEQUEUE_H
#define SPSCLOCKFREEQUEUE_H

#include <array>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "limesuite/Logger.h"

using namespace std::literals::string_literals;

namespace lime {

/**
  @brief Single Producer - Single Consumer lock-free and wait-free queue.
  @tparam T The type of the objects in the queue.
 */
template<class T> class PacketsFIFO
{
  public:
    ///---------------------------------------------------------------------------
    /// @brief Constructor. Asserts when the underlying type is not lock free.
    /// @param fixedSize The maximum size of the queue.
    PacketsFIFO(std::size_t fixedSize)
        : RingBufferSize(fixedSize + 1)
    {
        m_ringBuffer.resize(RingBufferSize);
#ifndef NDEBUG
        std::atomic<std::size_t> test;
        assert(test.is_lock_free());
#endif
    }

    PacketsFIFO(const PacketsFIFO& src) = delete;

    virtual ~PacketsFIFO()
    {
    }

    ///---------------------------------------------------------------------------
    /// @brief  Returns whether the queue is empty.
    /// @return True when empty.
    bool empty() const noexcept
    {
        bool isEmpty = false;
        const std::size_t readPosition = m_readPosition.load();
        const std::size_t writePosition = m_writePosition.load();

        if (readPosition == writePosition)
        {
            isEmpty = true;
        }

        return isEmpty;
    }

    ///---------------------------------------------------------------------------
    /// @brief Pushes an element to the queue.
    /// @param element  The element to add.
    /// @param wait Whether to wait or now.
    /// @param timeout The timeout (in ms) to wait for.
    /// @return True when the element was added, false when the queue is full.
    bool push(const T element, bool wait = false, int timeout = 250)
    {
        std::unique_lock<std::mutex> lk(mwr);
        const std::size_t oldWritePosition = m_writePosition.load();
        const std::size_t newWritePosition = getPositionAfter(oldWritePosition);
        const std::size_t readPosition = m_readPosition.load();

        if (newWritePosition == readPosition)
        {
            // The queue is full
            if (!wait)
                return false;

            // The queue is empty
            //std::unique_lock<std::mutex> lk(mwr);
            {
                if (canWrite.wait_for(lk, std::chrono::milliseconds(timeout)) == std::cv_status::timeout)
                {
                    lime::error("write fifo timeout"s);
                    return false;
                }
            }
        }

        m_ringBuffer[oldWritePosition] = element;
        m_writePosition.store(newWritePosition);
        canRead.notify_one();

        return true;
    }

    ///---------------------------------------------------------------------------
    /// @brief Pops an element from the queue.
    /// @param element The returned element.
    /// @param wait Whether to wait or now.
    /// @param timeout The timeout (in ms) to wait for.
    /// @return True when succeeded, false when the queue is empty.
    bool pop(T* element, bool wait = false, int timeout = 250)
    {
        std::unique_lock<std::mutex> lk(mwr);
        if (empty())
        {
            if (!wait)
                return false;
            // The queue is empty
            //std::unique_lock<std::mutex> lk(mrd);
            auto t1 = std::chrono::high_resolution_clock::now();
            {
                auto elapsed = std::chrono::high_resolution_clock::now() - t1;
                // if(elapsed >= std::chrono::milliseconds(timeout))
                //   return false;
                if (canRead.wait_for(lk, std::chrono::milliseconds(timeout)) == std::cv_status::timeout)
                {
                    //lime::error("pop fifo timeout"s);
                    return false;
                }
            }
        }

        const std::size_t readPosition = m_readPosition.load();
        *element = (m_ringBuffer[readPosition]);
        m_readPosition.store(getPositionAfter(readPosition));
        canWrite.notify_one();
        return true;
    }

    ///---------------------------------------------------------------------------
    /// @brief Clears the content from the queue.
    void clear() noexcept
    {
        const std::size_t readPosition = m_readPosition.load();
        const std::size_t writePosition = m_writePosition.load();

        if (readPosition != writePosition)
        {
            m_readPosition.store(writePosition);
        }
    }

    ///---------------------------------------------------------------------------
    /// @brief  Returns the maximum size of the queue.
    /// @return The maximum number of elements the queue can hold.
    constexpr std::size_t max_size() const noexcept
    {
        return RingBufferSize - 1;
    }

    ///---------------------------------------------------------------------------
    /// @brief  Returns the actual number of elements in the queue.
    /// @return The actual size or 0 when empty.
    std::size_t size() const noexcept
    {
        const std::size_t readPosition = m_readPosition.load();
        const std::size_t writePosition = m_writePosition.load();

        if (readPosition == writePosition)
        {
            return 0;
        }

        std::size_t size = 0;
        if (writePosition < readPosition)
        {
            size = RingBufferSize - readPosition + writePosition;
        }
        else
        {
            size = writePosition - readPosition;
        }

        return size;
    }

  private:
    // A lock-free queue is basically a ring buffer.
    std::size_t RingBufferSize;
    std::vector<T> m_ringBuffer;
    std::atomic<std::size_t> m_readPosition = { 0 };
    std::atomic<std::size_t> m_writePosition = { 0 };

    std::condition_variable canRead;
    std::condition_variable canWrite;
    std::mutex mwr;
    std::mutex mrd;

    constexpr std::size_t getPositionAfter(std::size_t pos) const noexcept
    {
        return ((pos + 1 == RingBufferSize) ? 0 : pos + 1);
    }
};

} // namespace lime

#endif /* SPSCLOCKFREEQUEUE_H */
