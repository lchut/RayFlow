#pragma once
#include <RayFlow/rayflow.h>
#include <RayFlow/Std/memory.h>

#include <mutex>
#include <atomic>
#include <variant>

namespace rayflow {

// https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/float.h
RAYFLOW_CPU_GPU inline uint32_t FloatToBits(float v) {
#ifdef RAYFLOW_BUILD_GPU_RENDERER
        return __float_as_uint(f);
#else
        return rstd::bit_cast<uint32_t>(v);
#endif
}

RAYFLOW_CPU_GPU inline float BitsToFloat(uint32_t bits) {
#ifdef RAYFLOW_BUILD_GPU_RENDERER
        return __uint_as_float(bits);
#else
        return rstd::bit_cast<float>(bits);
#endif
}

// https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/parallel.h.h

class AtomicFloat {
public:
    RAYFLOW_CPU_GPU explicit AtomicFloat(float v = 0) {
#ifdef RAYFLOW_BUILD_GPU_RENDERER
        value = v;
#else
        bits = FloatToBits(v);
#endif
    }

    RAYFLOW_CPU_GPU operator float() const {
#ifdef RAYFLOW_BUILD_GPU_RENDERER
        return value;
#else
        return BitsToFloat(bits);
#endif
    }

    RAYFLOW_CPU_GPU Float operator=(float v) {
#ifdef RAYFLOW_BUILD_GPU_RENDERER
        value = v;
        return value;
#else
        bits = FloatToBits(v);
        return v;
#endif
    }

    RAYFLOW_CPU_GPU void Add(float v) {
#ifdef RAYFLOW_BUILD_GPU_RENDERER
        atomicAdd(&value, v);
#else
        FloatBits oldBits = bits, newBits;
        do {
            newBits = FloatToBits(BitsToFloat(oldBits) + v);
        } while (!bits.compare_exchange_weak(oldBits, newBits));
#endif
    }

private:
#if RAYFLOW_BUILD_GPU_RENDERER
    float value;
#else
    std::atomic<uint32_t> bits;
#endif
};


};