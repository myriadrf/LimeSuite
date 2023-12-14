#include "MemoryPool.h"
#include <cstring>
#include <stdexcept>

namespace lime {
MemoryPool::MemoryPool(int blockCount, int blockSize, int alignment, const std::string& name)
    : name(name)
    , allocCnt(0)
    , freeCnt(0)
    , mBlockSize(blockSize)
{
    for (int i = 0; i < blockCount; ++i)
    {
        void* ptr = aligned_alloc(alignment, blockSize);
        if (!ptr)
        {
            throw std::runtime_error("Failed to allocate memory");
        }

        std::memset(ptr, 0, blockSize);
        mFreeBlocks.push(ptr);
        ownedAddresses.insert(ptr);
    }
}
MemoryPool::~MemoryPool()
{
    // if(mFreeBlocks.size() != ownedAddresses.size())
    //     throw std::runtime_error("Not all memory was freed");
    std::lock_guard<std::mutex> lock(mLock);
    while (!mFreeBlocks.empty())
    {
        void* ptr = mFreeBlocks.top();
        free(ptr);
        mFreeBlocks.pop();
    }
    for (auto ptr : mUsedBlocks)
        free(ptr);
}

void* MemoryPool::Allocate(int size)
{
    if (size > mBlockSize)
    {
        char ctemp[64];
        sprintf(ctemp, "%s Memory request too big(%i), max allowed(%i)", name.c_str(), size, mBlockSize);
        throw std::runtime_error(ctemp);
    }
    std::lock_guard<std::mutex> lock(mLock);
    if (mFreeBlocks.empty())
    {
        char ctemp[64];
        sprintf(ctemp, "%s No memory in pool", name.c_str());
        throw std::runtime_error(ctemp);
    }

    void* ptr = mFreeBlocks.top();
    mUsedBlocks.insert(ptr);
    mFreeBlocks.pop();
    ++allocCnt;
    return ptr;
}

void MemoryPool::Free(void* ptr)
{
    std::lock_guard<std::mutex> lock(mLock);
    if (mUsedBlocks.erase(ptr) == 0)
    {
        if (ownedAddresses.find(ptr) != ownedAddresses.end())
        {
            char ctemp[1024];
            sprintf(ctemp,
                "%s Double free?, allocs: %i , frees: %i, used: %li, free: %li\n ptr: %p",
                name.c_str(),
                allocCnt,
                freeCnt,
                mUsedBlocks.size(),
                mFreeBlocks.size(),
                ptr);
            for (auto adr : mUsedBlocks)
                printf("addrs: %p\n", adr);
            throw std::runtime_error(ctemp);
        }
        else
        {
            char ctemp[64];
            sprintf(ctemp, "%s Ptr does not belong to this pool", name.c_str());
            throw std::runtime_error(ctemp);
        }
    }
    ++freeCnt;
    mFreeBlocks.push(ptr);
}

} // namespace lime
