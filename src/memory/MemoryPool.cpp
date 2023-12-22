#include "MemoryPool.h"
#include <cstring>
#include <stdexcept>

namespace lime {

/// @brief Constructs the Memory Pool and allocates the memory of the pool.
/// @param blockCount The amount of memory blocks to allocate.
/// @param blockSize The memory size of a single block.
/// @param alignment The alignment of the memory.
/// @param name The name of the memory pool.
MemoryPool::MemoryPool(int blockCount, int blockSize, int alignment, const std::string& name)
    : name(name)
    , allocCnt(0)
    , freeCnt(0)
    , mBlockSize(blockSize)
{
    for (int i = 0; i < blockCount; ++i)
    {
        void* ptr = std::aligned_alloc(alignment, blockSize);
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

/// @brief Gives a block of memory of a given size.
/// @param size The size of the memory to give. Must not be more than the maximum size.
/// @return The pointer to the allocated memory.
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

/// @brief Frees the given memory location.
/// @param ptr The pointer of the memory to free. Must belong to this memory pool.
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
            throw std::runtime_error("Pointer does not belong to pool " + name);
        }
    }
    ++freeCnt;
    mFreeBlocks.push(ptr);
}

} // namespace lime
