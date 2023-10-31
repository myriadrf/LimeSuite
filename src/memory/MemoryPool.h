#ifndef LIME_MEMORYPOOL_H
#define LIME_MEMORYPOOL_H

#include <set>
#include <stack>
#include <mutex>

namespace lime {

class MemoryPool
{
  public:
    MemoryPool(int blockCount, int blockSize, int alignment, const char* name);
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