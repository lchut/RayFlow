#pragma once

#include <cstddef>
#include <utility>
#include <new>

namespace rayflow {

namespace rstd {

namespace pmr {
class memory_resource;
 
bool operator==(const memory_resource& a, const memory_resource& b) noexcept;

// class template polymorphic_allocator
template<class Tp> class polymorphic_allocator;

template<class T1, class T2>
  bool operator==(const polymorphic_allocator<T1>& a,
                  const polymorphic_allocator<T2>& b) noexcept;

// global memory resources
memory_resource* new_delete_resource() noexcept;
memory_resource* null_memory_resource() noexcept;
memory_resource* set_default_resource(memory_resource* r) noexcept;
memory_resource* get_default_resource() noexcept;

// pool resource classes
struct pool_options;
class synchronized_pool_resource;
class unsynchronized_pool_resource;
class monotonic_buffer_resource;

class memory_resource {
public:
    memory_resource() = default;

    memory_resource(const memory_resource&) = default;

    virtual ~memory_resource() = default;

    void* allocate(size_t bytes, size_t alignment = alignof(std::max_align_t)) {
        return do_allocate(bytes, alignment);
    }

    void deallocate(void* p, size_t bytes, size_t alignment = alignof(std::max_align_t)) {
        if (!p) {
            return;
        }
        do_deallocate(p, bytes, alignment);
    }

    bool is_equal(const memory_resource& other) const {
        return do_is_equal(other);
    }

private:
    virtual void* do_allocate(size_t bytes, size_t alignment = alignof(std::max_align_t)) = 0;

    virtual void do_deallocate(void* p, size_t bytes, size_t alignment = alignof(std::max_align_t)) = 0;

    virtual bool do_is_equal(const memory_resource& other) const = 0;
};

struct pool_options {
  size_t max_blocks_per_chunk = 0;
  size_t largest_required_pool_block = 0;
};

class alignas(64) monotonic_buffer_resource : public memory_resource {
public:
    monotonic_buffer_resource() : mUpstream_(get_default_resource()) {}

    explicit monotonic_buffer_resource(memory_resource* upstream) : mUpstream_(upstream) {}
    
    explicit monotonic_buffer_resource(size_t initial_size) : monotonic_buffer_resource(initial_size, get_default_resource()) {}

    monotonic_buffer_resource(size_t initial_size, memory_resource* upstream) : mBlockSize_(initial_size), mUpstream_(upstream) {}

    monotonic_buffer_resource(void* buffer, size_t buffer_size) : monotonic_buffer_resource(buffer, buffer_size, get_default_resource()) {}

    monotonic_buffer_resource(void* buffer, size_t buffer_size, memory_resource* upstream) {

    }

    monotonic_buffer_resource(const monotonic_buffer_resource&) = delete;

    virtual ~monotonic_buffer_resource() { release(); }

    monotonic_buffer_resource& operator=(const monotonic_buffer_resource&) = delete;

    void release() {
        Block* ptr = mBlockList_;
        while (ptr != nullptr) {
            Block* next = ptr->next;
            mUpstream_->deallocate(ptr, ptr->size + sizeof(Block));
            ptr = next;
        }
        mBlockList_ = nullptr;
        mCurrentOffset_ = 0;
    }

    memory_resource* upstream_resource() const { return mUpstream_; }

protected:
    void* do_allocate(size_t bytes, size_t alignment = alignof(std::max_align_t)) override {
        if (bytes > mBlockSize_) {
            return mUpstream_->allocate(bytes, alignment);
        }
        if (mCurrentOffset_ % alignment != 0) {
            mCurrentOffset_ += alignment - (mCurrentOffset_ % alignment);
        }
        if (!mBlockList_ || mCurrentOffset_ + bytes > mBlockList_->size) {
            // allocate block
            Block* new_block = static_cast<Block*>(mUpstream_->allocate(mBlockSize_ + sizeof(Block), alignof(Block)));
            new_block->ptr = reinterpret_cast<char*>(new_block) + sizeof(Block);
            new_block->size = mBlockSize_;
            new_block->next = mBlockList_;
            mBlockList_ = new_block;
            mCurrentOffset_ = 0;
        }
        void* result = reinterpret_cast<std::byte*>(mBlockList_->ptr) + mCurrentOffset_;
        mCurrentOffset_ += bytes;
        return result;
    }

    void do_deallocate(void* p, size_t bytes, size_t alignment = alignof(std::max_align_t)) override {
        if (bytes > mBlockSize_) {
            mUpstream_->deallocate(p, bytes);
        }
    }

    bool do_is_equal(const memory_resource& other) const override {
        return this == &other;
    }
private:
    struct Block {
        void* ptr;
        size_t size;
        Block* next;
    };

    memory_resource* mUpstream_;
    size_t mBlockSize_ = 256 * 1024;
    size_t mCurrentOffset_ = 0;
    Block* mBlockList_ = nullptr;
};

template <typename T = std::byte>
class polymorphic_allocator {
public:
    polymorphic_allocator() : mMemoryResource_(get_default_resource()) {}

    polymorphic_allocator(const polymorphic_allocator& other) = default;

    template <typename U>
    polymorphic_allocator(const polymorphic_allocator<U>& other) : mMemoryResource_(other.mMemoryResource_) {}

    polymorphic_allocator(memory_resource* r) : mMemoryResource_(r) {}

    polymorphic_allocator& operator=(const polymorphic_allocator& other) = delete;

    T* allocate(size_t n) {
        return static_cast<T*>(mMemoryResource_->allocate(n * sizeof(T), alignof(T)));
    }

    void deallocate(T* p, size_t n) {
        mMemoryResource_->deallocate(p, n * sizeof(T));
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new ((void*)p) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p) { 
        p->~U();
    }

    void* allocate_bytes(size_t nbytes, size_t alignment = alignof(std::max_align_t)) {
        return mMemoryResource_->allocate(nbytes, alignment);
    }

    void deallocate_bytes(void* p, size_t nbytes, size_t alignment = alignof(std::max_align_t)) {
        mMemoryResource_->deallocate(p, nbytes, alignment);
    }

    template <typename U>
    U* allocate_object(size_t n = 1) {
        return static_cast<U*>(allocate_bytes(n * sizeof(U), alignof(U)));
    }

    template <typename U>
    void deallocate_object(U* p, size_t n = 1) {
        deallocate_bytes(p, n * sizeof(U), alignof(U));
    }

    template <typename U, typename... CtorArgs>
    U* new_object(CtorArgs... ctor_args) {
        U* p = allocate_object<U>();
        construct(p, std::forward<CtorArgs>(ctor_args)...);
        return p;
    }

    template <typename U>
    void delete_object(U* p) {
        p->~U();
        deallocate_object(p);
    }

    polymorphic_allocator select_on_container_copy_construction() const {
        return polymorphic_allocator(*this);
    }

    memory_resource* resource() const { return mMemoryResource_; }
private:
    memory_resource* mMemoryResource_;
};

template <typename T = std::byte>
inline bool operator==(const polymorphic_allocator<T>& lhs, const polymorphic_allocator<T>& rhs) {
    return lhs.resource() == rhs.resource();
}

template <typename T = std::byte>
inline bool operator!=(const polymorphic_allocator<T>& lhs, const polymorphic_allocator<T>& rhs) {
    return !(lhs.resource() == rhs.resource());
}

}

}
}