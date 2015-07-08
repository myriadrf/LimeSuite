/**
@file 	LMS_StreamBoard_FIFO.h
@author Lime Microsystems (www.limemicro.com)
@brief 	blocking FIFO for packets storing
*/

#ifndef LMS_STREAMBOARD_FIFO_H
#define LMS_STREAMBOARD_FIFO_H

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <mutex>
#include <chrono>
#include <condition_variable>

template<class T>
class LMS_StreamBoard_FIFO
{
private:
    LMS_StreamBoard_FIFO(){};
public:
    struct Status
    {
        uint32_t maxElements;
        uint32_t filledElements;
        uint32_t head;
        uint32_t tail;
    };

    LMS_StreamBoard_FIFO(uint64_t FIFO_length)
    {        
        mHead = 0;
        mTail = 0;
		mElements.resize(FIFO_length);    
		mElementsFilled = 0;
    }

    ~LMS_StreamBoard_FIFO()
    {
        
    }

    /** @brief Copies given src element to queue.
        @param src Source data
        @return true if element was inserted

        Copies element to queue. This function blocks until element is copied to
        queue or certain amount of time has passed.
    */
    bool push_back(const T &src, unsigned int timeout_ms = 200)
    {        
        std::unique_lock<std::mutex> lck(mElementsLock);         
        while (mElementsFilled == mElements.size())
        {
            if (canWrite.wait_for(lck, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout)
                return false;            
        }
        memcpy(&mElements[mTail], &src, sizeof(T));		
		mTail = (mTail+1) % mElements.size();
		++mElementsFilled;
        canRead.notify_one();
		return true;		
    }

    /** @brief Copies and removes first element from queue to dest.
        @param dest destination container for data
        @return true if element was copied
        Copies element from queue to destination and then removes it.
        This function blocks until element is returned, or certain amount of time
        has passed.
    */
    bool pop_front(T *dest, unsigned int timeout_ms = 200)
    {
        assert(dest != nullptr);   
        std::unique_lock<std::mutex> lck(mElementsLock);
        while (mElementsFilled == 0)
        {
            if (canRead.wait_for(lck, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout)
                return false;
        }                
        if (mElementsFilled == 0)
            return false;
        *dest = mElements[mHead];
        memcpy(dest, &mElements[mHead], sizeof(T));
        mHead = (mHead + 1) % mElements.size();
        --mElementsFilled;
        canWrite.notify_one();
        return true;        
    }
    void reset()
    {
        std::unique_lock<std::mutex> lck(mElementsLock);
        mElementsFilled = 0;
        mHead = 0;
        mTail = 0;
    }    

    Status GetStatus()
    {
        Status stats;
        std::unique_lock<std::mutex> lck(mElementsLock);
        stats.maxElements = mElements.size();
        stats.filledElements = mElementsFilled;
        stats.head = mHead;
        stats.tail = mTail;
        return stats;
    };

protected:
    uint32_t mHead; //reading pos
    uint32_t mTail; //writing pos
	uint32_t mElementsFilled;

    std::mutex mElementsLock; // condition variable for critical section  	
    std::vector<T> mElements;
    std::condition_variable canWrite;
    std::condition_variable canRead;
};

#endif // LMS_STREAMBOARD_FIFO_H
