#include <RayFlow/Engine/memory_pool.h>

namespace rayflow {

rstd::pmr::polymorphic_allocator<std::byte> GMalloc(new TSMemoryPool());

void* TSMemoryPool::do_allocate(size_t bytes, size_t alignment) {
    int threadId = tbb::this_task_arena::current_thread_index();

    bool cacheInMap = false;
    {
        std::shared_lock readCacheMapLock(mCacheMapMutex_);
        if (mThreadCacheMap_.find(threadId) != mThreadCacheMap_.end()) {
            cacheInMap = true;
        }
    } 

    if (!cacheInMap) {
        std::unique_lock writeCacheMapLock(mCacheMapMutex_);
        mThreadCacheMap_[threadId] = ThreadCache();

    }
    
    {
        std::shared_lock readCacheMapLock(mCacheMapMutex_);
        ThreadCache& cache = mThreadCacheMap_[threadId];

        void* result = nullptr;
        if (bytes > mBlockSize_) {
            result = allocateRaw(bytes, alignment);
        }
        else {
            if (cache.offset % alignment != 0) {
                cache.offset += alignment - (cache.offset % alignment);
            }

            if (EmptyList(cache.blockList) || cache.offset + bytes > mBlockSize_) {
                std::lock_guard<std::mutex> freeBlockGuard(mFreeBlockMutex_);

                Block* freeBlock = nullptr;
                if (!EmptyList(mFreeBlockList_)) {
                    freeBlock = EraseFront(mFreeBlockList_);
                }
                else {
                    freeBlock = AllocateBlock();
                }
                InsertFront(cache.blockList, freeBlock, freeBlock);
                cache.offset = 0;
            }

            result = reinterpret_cast<std::byte*>(cache.blockList->next->ptr) + cache.offset;
            cache.offset += bytes;
        }
        return result;
    }
}

void TSMemoryPool::do_deallocate(void* p, size_t bytes, size_t alignment) {
    if (bytes > mBlockSize_) {
        deallocateRaw(p, bytes);
    }
}

}
