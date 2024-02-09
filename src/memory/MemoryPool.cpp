#include "MemoryPool.h"

#include <cstring>
#include <stdexcept>
#include <string>
#include <sstream>

#include "Logger.h"

namespace lime {
MemoryPool::MemoryPool(int blockCount, int blockSize, int alignment, const std::string& name)
    : name(name)
    , allocCnt(0)
    , freeCnt(0)
    , mBlockSize(blockSize)
{
    for (int i = 0; i < blockCount; ++i)
    {
#if __unix__
        void* ptr = aligned_alloc(alignment, blockSize);
#else
        void* ptr = _aligned_malloc(alignment, blockSize);
#endif
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
        const std::string ctemp = "Memory request for " + name + " is too big (" + std::to_string(size) + "), max allowed is " +
                                  std::to_string(mBlockSize);
        throw std::runtime_error(ctemp);
    }
    std::lock_guard<std::mutex> lock(mLock);
    if (mFreeBlocks.empty())
    {
        throw std::runtime_error("No memory in pool " + name);
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
                "%s Double free?, allocs: %i , frees: %i, used: %li, free: %li, ptr: %p",
                name.c_str(),
                allocCnt,
                freeCnt,
                mUsedBlocks.size(),
                mFreeBlocks.size(),
                ptr);
            for (auto adr : mUsedBlocks)
                lime::error("addrs: %p", adr);
            throw std::runtime_error(ctemp);
        }
        else
        {
            std::stringstream ss;
            ss << ptr;

            throw std::runtime_error("Pointer " + ss.str() + " does not belong to pool " + name);
        }
    }
    ++freeCnt;
    mFreeBlocks.push(ptr);
}

} // namespace lime
