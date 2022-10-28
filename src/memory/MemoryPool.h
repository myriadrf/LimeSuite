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

private:
    const char* name;
    int allocCnt;
    int freeCnt;
    int mBlockSize;
    std::mutex mLock;
    std::stack<void*> mFreeBlocks;
    std::set<void*> mUsedBlocks;
    std::set<void*> ownedAddresses;
};

}
#endif