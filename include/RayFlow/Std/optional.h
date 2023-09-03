#pragma once

#include <RayFlow/rayflow.h>

#include <new>
#include <type_traits>

namespace rayflow {

namespace rstd {

template <typename T>
class optional {
public:
    optional() = default;

    RAYFLOW_CPU_GPU optional(const optional& other) : mSet_(other.mSet_) {
        if (other.has_value()) {
            new (ptr()) T(other.value());
        }
    }

    RAYFLOW_CPU_GPU optional(optional&& other) : mSet_(true) {
        if (other.has_value()) {
            new (ptr()) T(std::move(other.value()));
            other.reset();
        }
    }

    RAYFLOW_CPU_GPU optional(const T& v) : mSet_(true) {
        new (ptr()) T(v);
    }

    RAYFLOW_CPU_GPU optional(T&& v) : mSet_(true) {
        new (ptr()) T(std::move(v));
    }

    RAYFLOW_CPU_GPU optional& operator=(const optional& other) {
        reset();
        if (other.has_value()) {
            mSet_ = true;
            new (ptr()) T(other.value());
        }
        return *this;
    }

    RAYFLOW_CPU_GPU optional& operator=(optional&& other) {
        reset();
        if (other.has_value()) {
            mSet_ = true;
            new (ptr()) T(std::move(other.value()));
            other.reset();
        }
        return *this;
    }

    RAYFLOW_CPU_GPU optional& operator=(const T& v) {
        reset();
        mSet_ = true;
        new (ptr()) T(v);
        return *this;
    }

    RAYFLOW_CPU_GPU optional& operator=(T&& v) {
        reset();
        mSet_ = true;
        new (ptr()) T(v);
        return *this;
    }

    RAYFLOW_CPU_GPU ~optional() { reset(); }

    RAYFLOW_CPU_GPU T* operator->() { return &value(); }

    RAYFLOW_CPU_GPU const T* operator->() const { return &value(); }

    RAYFLOW_CPU_GPU T& operator*() { return value(); }

    RAYFLOW_CPU_GPU const T& operator*() const { return value(); }

    RAYFLOW_CPU_GPU operator bool() const { return mSet_; }

    RAYFLOW_CPU_GPU bool has_value() const { return mSet_; }

    RAYFLOW_CPU_GPU T& value() { return *ptr(); }

    RAYFLOW_CPU_GPU const T& value() const { return *ptr(); }

    template <typename U>
    RAYFLOW_CPU_GPU T value_or(U&& default_value) const { return mSet_ ? value() : T(default_value); }

    RAYFLOW_CPU_GPU void reset() {
        if (mSet_) {
            mSet_ = false;
            value().~T();
        }
    }

private:
#ifdef __NVCC__
    RAYFLOW_CPU_GPU T *ptr() { return reinterpret_cast<T *>(&mObject_); }
    RAYFLOW_CPU_GPU const T *ptr() const { return reinterpret_cast<const T *>(&mObject_); }
#else
    RAYFLOW_CPU_GPU T *ptr() { return std::launder(reinterpret_cast<T *>(&mObject_)); }
    RAYFLOW_CPU_GPU const T *ptr() const {
        return std::launder(reinterpret_cast<const T *>(&mObject_));
    }
#endif

    std::aligned_storage_t<sizeof(T), alignof(T)> mObject_;
    bool mSet_ = false;
};
}
} 
