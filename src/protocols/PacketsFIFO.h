#pragma once

#include <vector>
#include <stack>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <assert.h>

namespace lime {

template <class T> class PacketsFIFO // Single producer, single consumer FIFO
{
  public:
    PacketsFIFO(uint32_t elementsCount)
    {
        nodes.resize(elementsCount);
        for (int i = elementsCount - 1; i >= 0; --i)
            freeNodes.push(&nodes[i]);
    }
    ~PacketsFIFO()
    {
        assert((freeNodes.size() + queue.size()) == nodes.size());
    }

    // returns pointer to free FIFO node
    T *acquire()
    {
        std::lock_guard<std::mutex> lck(mLockAllocation);
        if (freeNodes.empty())
            return nullptr;

        T *node = freeNodes.top();
        freeNodes.pop();
        return node;
    }

    void release(T *node)
    {
        if (node == nullptr)
            return;
        std::lock_guard<std::mutex> lck(mLockAllocation);
        freeNodes.push(node);
    }

    void push(T *node)
    {
        std::lock_guard<std::mutex> lck(mLock);
        queue.push(node);
        hasItems.notify_one();
    }

    T *pop()
    {
        std::unique_lock<std::mutex> lck(mLock);
        T *node;
        if (queue.empty()) {
            if (hasItems.wait_for(lck, std::chrono::milliseconds(200)) == std::cv_status::timeout)
                return nullptr;
        }
        node = queue.front();
        queue.pop();
        lck.unlock();
        return node;
    }

    float Usage()
    {
        std::lock_guard<std::mutex> lck(mLockAllocation);
        std::lock_guard<std::mutex> lck2(mLock);
        return float(queue.size()) / nodes.size();
    }

  protected:
    std::vector<T> nodes; // memory allocation
    std::stack<T *> freeNodes;
    std::queue<T *> queue; // nodes order in queue
    std::mutex mLock;
    std::mutex mLockAllocation;
    std::condition_variable hasItems;
};

} // namespace lime
