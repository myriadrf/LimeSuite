#ifndef LIME_MEMORYPOOL_H
#define LIME_MEMORYPOOL_H

#include <set>
#include <stack>
#include <mutex>

namespace lime {

/**
  @brief Class for having a preallocated memory pool for many allocations 
  and deallocations without actually having to allocate and deallocate memory every time.
 */
class MemoryPool
{
  public:
    MemoryPool(int blockCount, int blockSize, int alignment, const std::string& name);
    ~MemoryPool();

    void* Allocate(int size);
    void Free(void* ptr);
    int32_t MaxAllocSize() const { return mBlockSize; };

  private:
    std::string name;
    int allocCnt;
    int freeCnt;
    int mBlockSize;
    std::mutex mLock;
    std::stack<void*> mFreeBlocks;
    std::set<void*> mUsedBlocks;
    std::set<void*> ownedAddresses;
};

} // namespace lime
#endif