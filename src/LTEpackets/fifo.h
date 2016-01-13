#ifndef LMS_FIFO_BUFFER_H
#define LMS_FIFO_BUFFER_H

#include <mutex>
#include <atomic>
#include <vector>
#include <thread>
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
    @return number of items inserted
    */
    uint32_t push_samples(const complex16_t **buffer, const uint32_t samplesCount, const uint8_t channelsCount, uint64_t timestamp, const uint32_t timeout_ms)
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
        @return number of samples popped
    */
    uint32_t pop_samples(complex16_t** buffer, const uint32_t samplesCount, const uint8_t channelsCount, uint64_t *timestamp, const uint32_t timeout_ms)
	{   
        assert(buffer != nullptr);
        uint32_t samplesFilled = 0;		
		*timestamp = 0;
        std::unique_lock<std::mutex> lck(readLock);
        while (samplesFilled < samplesCount)
        {   
            while (mElementsFilled.load() == 0) //buffer might be empty, wait for packets
            {
                if (canRead.wait_for(lck, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout)
                    return samplesFilled;
            }
			if(samplesFilled == 0)
                *timestamp = mBuffer[mHead.load()].timestamp + mBuffer[mHead.load()].first;
			
			while(mElementsFilled.load() > 0 && samplesFilled < samplesCount)
			{	
				int headIndex = mHead.load();
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
}
#endif
