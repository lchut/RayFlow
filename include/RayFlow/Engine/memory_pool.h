#pragma once
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <tbb/tbb.h>

#include <RayFlow/Std/memory_resource.h>

namespace rayflow {

class TSMemoryPool : public rstd::pmr::memory_resource {
public:
    TSMemoryPool() {
        mFreeBlockList_ = new Block;
        mFreeBlockList_->next = mFreeBlockList_;
        mFreeBlockList_->prev = mFreeBlockList_;
    }

    void reset() {
        int threadId = tbb::this_task_arena::current_thread_index();

        std::shared_lock readCacheMapLock(mCacheMapMutex_);

        if (mThreadCacheMap_.find(threadId) != mThreadCacheMap_.end() &&
                !EmptyList(mThreadCacheMap_[threadId].blockList)) {
            std::lock_guard<std::mutex> freeBlockGuard(mFreeBlockMutex_);
            Block* ptr = mThreadCacheMap_[threadId].blockList;
            
            if (!EmptyList(ptr)) {
                InsertFront(mFreeBlockList_, ptr->next, ptr->prev);

                ptr->next = ptr;
                ptr->prev = ptr;
                mThreadCacheMap_[threadId].offset = 0;
            }
        }
    }

private:

    virtual void* do_allocate(size_t bytes, size_t alignment = alignof(std::max_align_t));

    virtual void do_deallocate(void* p, size_t bytes, size_t alignment = alignof(std::max_align_t));

    virtual bool do_is_equal(const memory_resource& other) const {
        return this == &other;
    }

private:
    struct Block {
        void* ptr = nullptr;
        Block* prev = nullptr;
        Block* next = nullptr;
    };

    struct ThreadCache {
        ThreadCache() : offset(0) {
            blockList = new Block;
            blockList->prev = blockList;
            blockList->next = blockList;
        }

        size_t offset = 0;
        Block* blockList;
    };

    size_t nBlock = 0;
    size_t mBlockSize_ = 4 * 1024;
    Block* mFreeBlockList_ = nullptr;

    std::mutex mFreeBlockMutex_;
    std::shared_mutex mCacheMapMutex_;

    std::unordered_map<int, ThreadCache> mThreadCacheMap_;
private:
    void InsertFront(Block* dest, Block* srcHead, Block* srcTail) {
        Block* destNext = dest->next;

        dest->next = srcHead;
        destNext->prev = srcTail;
        srcHead->prev = dest;
        srcTail->next = destNext;
    }

    Block* EraseFront(Block* head) {
        if (EmptyList(head)) {
            return nullptr;
        }

        Block* result = head->next;
        head->next->prev = head;
        head->next = head->next->next;

        result->next = result;
        result->prev = result;
        return result;
    }

    Block* AllocateBlock() {
        ++nBlock;
        Block* newBlock = static_cast<Block*>(allocateRaw(mBlockSize_ + sizeof(Block), alignof(Block)));
        newBlock->ptr = reinterpret_cast<char*>(newBlock) + sizeof(Block);
        newBlock->next = newBlock;
        newBlock->prev = newBlock;

        return newBlock;
    };

    bool EmptyList(Block* ptr) {
        return ptr == ptr->next;
    }

    void *allocateRaw(size_t size, size_t alignment) {
#if defined(RAYFLOW_HAVE_ALIGNED_MALLOC)
        return _aligned_malloc(size, alignment);
#elif defined(RAYFLOW_HAVE_POSIX_MEMALIGN)
        void *ptr;
        if (alignment < sizeof(void *))
            return malloc(size);
        if (posix_memalign(&ptr, alignment, size) != 0)
            ptr = nullptr;
        return ptr;
#else
        return memalign(alignment, size);
#endif
    }

    void deallocateRaw(void *ptr, size_t bytes, size_t alignment = alignof(std::max_align_t)) {
        if (!ptr)
            return;
#if defined(RAYFLOW_HAVE_ALIGNED_MALLOC)
        _aligned_free(ptr);
#else
        free(ptr);
#endif
    }
};

extern rstd::pmr::polymorphic_allocator<std::byte> GMalloc;

inline void ResetGMalloc() {
    dynamic_cast<TSMemoryPool*>(GMalloc.resource())->reset();
}

}