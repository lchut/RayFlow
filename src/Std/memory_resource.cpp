#include <RayFlow/Std/memory_resource.h>

namespace rayflow {

namespace rstd {

namespace pmr {

class NewDeleteMemoryResource : public memory_resource {
    void *do_allocate(size_t size, size_t alignment) {
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

    void do_deallocate(void *ptr, size_t bytes, size_t alignment) {
        if (!ptr)
            return;
#if defined(RAYFLOW_HAVE_ALIGNED_MALLOC)
        _aligned_free(ptr);
#else
        free(ptr);
#endif
    }

    bool do_is_equal(const memory_resource &other) const noexcept {
        return this == &other;
    }

};

static NewDeleteMemoryResource *ndr;

memory_resource *new_delete_resource() noexcept {
    if (!ndr)
        ndr = new NewDeleteMemoryResource;
    return ndr;
}

static memory_resource *rfDefaultMemoryResource = new_delete_resource();

memory_resource *set_default_resource(memory_resource *r) noexcept {
    memory_resource *orig = rfDefaultMemoryResource;
    rfDefaultMemoryResource = r;
    return orig;
}

memory_resource *get_default_resource() noexcept {
    return rfDefaultMemoryResource;
}

}

}
}