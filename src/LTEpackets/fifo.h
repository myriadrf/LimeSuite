#ifndef LMS_FIFO_BUFFER_H
#define LMS_FIFO_BUFFER_H

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <thread>
#include <queue>
#include <condition_variable>
#include "dataTypes.h"
#include <assert.h>

namespace lime{

/*  @brief Single producer, single consumer FIFO
*/
class LMS_SamplesFIFO
{
public:
    struct BufferInfo
    {
        uint32_t size;
        uint32_t itemsFilled;
    };

    /*  @brief Returns information about FIFO size and fullness
    */
    BufferInfo GetInfo()
    {
        //lock both reading and writing until information is collected
        std::unique_lock<std::mutex> lck2(readLock);
        std::unique_lock<std::mutex> lck(writeLock);
        BufferInfo stats;
        stats.size = (uint32_t)mBufferSize;
        stats.itemsFilled = mElementsFilled.load();
        return stats;
    }

	/**	@brief Initializes FIFO memory
		@param bufLength FIFO size, must be power of 2
	*/
	LMS_SamplesFIFO(uint32_t bufLength, uint8_t channelsCount)
	{   
        mBuffer = nullptr;
        mBufferSize = 0;
        Reset(bufLength, channelsCount);
	}
	
	~LMS_SamplesFIFO()
    {   
        delete []mBuffer;
    };
	
    /** @brief inserts samples to FIFO, operation is thread-safe
    @param buffer pointers to arrays containing samples data of each channel
    @param samplesCount number of samples to insert from each buffer channel
    @param channelsCount number of channels to insert
    @param timeout_ms timeout duration for operation
    @param flags optional flags associated with the samples
    @return number of items inserted
    */
    uint32_t push_samples(const complex16_t **buffer, const uint32_t samplesCount, const uint8_t channelsCount, uint64_t timestamp, const uint32_t timeout_ms, const uint32_t flags = 0)
    {
        assert(buffer != nullptr);
        uint32_t samplesTaken = 0;
        std::unique_lock<std::mutex> lck(writeLock);
        while (samplesTaken < samplesCount)
        {   
            while (mElementsFilled.load() >= mBufferSize) //buffer might be full, wait for free slots
            {
                if (canWrite.wait_for(lck, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout)
                    return samplesTaken;
            }

            int tailIndex = mTail.load(); //which element to fill
            while (mElementsFilled.load() < mBufferSize && samplesTaken < samplesCount) // not to release lock too often
            {
                mBuffer[tailIndex].timestamp = timestamp + samplesTaken;
                mBuffer[tailIndex].first = 0;
                mBuffer[tailIndex].last = 0;
                mBuffer[tailIndex].flags = flags;
                while (mBuffer[tailIndex].last < mBuffer[tailIndex].samplesCount && samplesTaken < samplesCount)
                {
                    const int sampleIndex = mBuffer[tailIndex].last;
                    for (int ch = 0; ch < channelsCount; ++ch)
                    { 
                        mBuffer[tailIndex].samples[ch][sampleIndex] = buffer[ch][samplesTaken];
                    }
                    ++samplesTaken;
                    ++mBuffer[tailIndex].last;
                }
                mTail.store((tailIndex + 1) & (mBufferSize - 1));//advance to next one
                tailIndex = mTail.load();
                mElementsFilled.fetch_add(1);
                canRead.notify_one();
            }
        }
        return samplesTaken;
    }
	
    /** @brief Takes samples out of FIFO, operation is thread-safe
        @param buffer pointers to destination arrays for each channel's samples data, each array must be big enough to contain \samplesCount number of samples.
        @param samplesCount number of samples to pop
        @param channelsCount number of channels to pop
		@param timestamp returns timestamp of the first sample in buffer
        @param timeout_ms timeout duration for operation
        @param flags optional flags associated with the samples
        @return number of samples popped
    */
    uint32_t pop_samples(complex16_t** buffer, const uint32_t samplesCount, const uint8_t channelsCount, uint64_t *timestamp, const uint32_t timeout_ms, uint32_t *flags = nullptr)
	{   
        assert(buffer != nullptr);
        uint32_t samplesFilled = 0;		
		*timestamp = 0;
        if (flags != nullptr) *flags = 0;
        std::unique_lock<std::mutex> lck(readLock);
        while (samplesFilled < samplesCount)
        {   
            while (mElementsFilled.load() == 0) //buffer might be empty, wait for packets
            {
                if (timeout_ms == 0) return samplesFilled;
                if (canRead.wait_for(lck, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout)
                    return samplesFilled;
            }
			if(samplesFilled == 0)
                *timestamp = mBuffer[mHead.load()].timestamp + mBuffer[mHead.load()].first;
			
			while(mElementsFilled.load() > 0 && samplesFilled < samplesCount)
			{	
				int headIndex = mHead.load();
                if (flags != nullptr) *flags |= mBuffer[headIndex].flags;
                while (mBuffer[headIndex].first < mBuffer[headIndex].last && samplesFilled < samplesCount)
				{
                    for (int ch = 0; ch < channelsCount; ++ch)
                    {   
                        buffer[ch][samplesFilled] = mBuffer[headIndex].samples[ch][mBuffer[headIndex].first];                        
                    }
                    ++mBuffer[headIndex].first;
                    ++samplesFilled;
				}
                if (mBuffer[headIndex].first == mBuffer[headIndex].last) //packet depleated
				{
                    mBuffer[headIndex].first = 0;
                    mBuffer[headIndex].last = 0;
                    mBuffer[headIndex].timestamp = 0;
					mHead.store( (headIndex + 1) & (mBufferSize - 1) );//advance to next one
                    headIndex = mHead.load();
					mElementsFilled.fetch_sub(1);
                    canWrite.notify_one();
				}
			}
        }
        return samplesFilled;
	}
	
    /** @brief Changes FIFO length and resets internal counter
		@param bufLength FIFO length, must be power of 2
    */
	void Reset(uint32_t bufLength, uint8_t channelsCount)
	{
        std::unique_lock<std::mutex> lck(writeLock);
        std::unique_lock<std::mutex> lck2(readLock);
        mChannelsCount = channelsCount;
        if (bufLength != mBufferSize)
        {
            delete[]mBuffer;
            mBuffer = new PacketFrame[bufLength];
            for (int i = 0; i < bufLength; ++i)
                mBuffer[i].Initialize(channelsCount);
            mBufferSize = bufLength;
        }
        
		mHead.store(0);
		mTail.store(0);
        mElementsFilled.store(0);
	}

    uint8_t GetChannelsCount()
    {
        return mChannelsCount;
    }
	
protected:
    int8_t mChannelsCount;
    int32_t mBufferSize;
	PacketFrame* mBuffer;
    std::atomic<uint32_t> mHead;
    std::atomic<uint32_t> mTail;
    std::mutex writeLock;
    std::mutex readLock;
	std::atomic<uint32_t> mElementsFilled;
    std::condition_variable canWrite;
    std::condition_variable canRead;
};

//https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
template <typename T>
class ConcurrentQueue
{
private:
    std::queue<T> mQueue;
    std::mutex mMutex;
    std::condition_variable mCond;
public:
    void push(T const& data)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mQueue.push(data);
        lock.unlock();
        mCond.notify_one();
    }

    bool empty() const
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return mQueue.empty();
    }

    bool try_pop(T& popped_value)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        if(mQueue.empty())
        {
            return false;
        }
        popped_value=mQueue.front();
        mQueue.pop();
        return true;
    }

    void wait_and_pop(T& popped_value)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        while(mQueue.empty())
        {
            mCond.wait(lock);
        }
        popped_value=mQueue.front();
        mQueue.pop();
    }

    bool wait_and_pop(T& popped_value, const int timeout_ms)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        while(mQueue.empty())
        {
            if (mCond.wait_for(lock, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout)
                return false;
        }
        popped_value=mQueue.front();
        mQueue.pop();
        return true;
    }
};

}
#endif
