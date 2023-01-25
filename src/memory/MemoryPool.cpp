#include "MemoryPool.h"

namespace lime{
MemoryPool::MemoryPool(int blockCount, int blockSize, int alignment, const char* name) :
    name(name), allocCnt(0), freeCnt(0), mBlockSize(blockSize)
{
    for(int i=0; i<blockCount; ++i)
    {
        void* ptr = aligned_alloc(alignment, blockSize);
        if(!ptr)
            throw std::runtime_error("Failed to allocate memory");
        mFreeBlocks.push(ptr);
        ownedAddresses.insert(ptr);
    }
}
MemoryPool::~MemoryPool()
{
    // if(mFreeBlocks.size() != ownedAddresses.size())
    //     throw std::runtime_error("Not all memory was freed");
    std::lock_guard<std::mutex> lock(mLock);
    while(!mFreeBlocks.empty())
    {
        void* ptr = mFreeBlocks.top();
        free(ptr);
        mFreeBlocks.pop();
    }
    for(auto ptr : mUsedBlocks)
        free(ptr);
}

void* MemoryPool::Allocate(int size)
{
    if(size > mBlockSize)
        throw std::runtime_error("Memory request too big");
    std::lock_guard<std::mutex> lock(mLock);
    if(mFreeBlocks.empty())
        throw std::runtime_error("No memory in pool");

    void* ptr = mFreeBlocks.top();
    mUsedBlocks.insert(ptr);
    mFreeBlocks.pop();
    ++allocCnt;
    return ptr;
}

void MemoryPool::Free(void* ptr)
{
    std::lock_guard<std::mutex> lock(mLock);
    if(mUsedBlocks.erase(ptr) == 0)
    {
        if(ownedAddresses.find(ptr) != ownedAddresses.end())
        {
            char ctemp[1024];
            sprintf(ctemp, "%s Double free?, allocs: %i , frees: %i, used: %li, free: %li\n ptr: %p", name.c_str(), allocCnt, freeCnt, mUsedBlocks.size(), mFreeBlocks.size(), ptr);
            for(auto adr : mUsedBlocks)
                printf("addrs: %p\n", adr);
            throw std::runtime_error(ctemp);
        }
        else
            throw std::runtime_error("Ptr does not belong to this pool");
    }
    ++freeCnt;
    mFreeBlocks.push(ptr);
}

}
