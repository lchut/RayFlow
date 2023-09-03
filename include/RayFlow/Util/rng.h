#pragma once

#include <RayFlow/rayflow.h>

namespace rayflow {

static const double DoubleOneMinusEpsilon = 0x1.fffffffffffffp-1;
static const float FloatOneMinusEpsilon = 0x1.fffffep-1;

#ifdef RAYFLOW_USE_FLOAT_AS_DOUBLE
static const Float OneMinusEpsilon = DoubleOneMinusEpsilon;
#else
static const Float OneMinusEpsilon = FloatOneMinusEpsilon;
#endif

#define PCG32_DEFAULT_STATE 0x853c49e6748fea9bULL
#define PCG32_DEFAULT_INC 0xda3e39cb94b95bdbULL
#define PCG32_MULT 0x5851f42d4c957f2dULL

// https://en.wikipedia.org/wiki/Permuted_congruential_generator
class Rng {
public:
    RAYFLOW_CPU_GPU Rng() : state(PCG32_DEFAULT_STATE), inc (PCG32_DEFAULT_INC) {}

    RAYFLOW_CPU_GPU Rng(uint64_t seed) : state(PCG32_DEFAULT_STATE), inc (PCG32_DEFAULT_INC) {
        state = seed + inc;
        UniformUInt32();
    }

    RAYFLOW_CPU_GPU void Reset(uint64_t seed) {
        state = PCG32_DEFAULT_STATE;
        inc = PCG32_DEFAULT_INC;
        state = seed + inc;
        UniformUInt32();
    }

    RAYFLOW_CPU_GPU uint32_t UniformUInt32() {
        uint64_t oldstate = state;
        state = oldstate * PCG32_MULT + inc;
        uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
        uint32_t rot = (uint32_t)(oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
    }
    
    RAYFLOW_CPU_GPU uint32_t UniformUInt32(uint32_t b) {
        uint32_t threshold = (~b + 1u) % b;
        while (true) {
            uint32_t r = UniformUInt32();
            if (r >= threshold) return r % b;
        }
    }

    RAYFLOW_CPU_GPU Float UniformFloat() {
        return std::min(OneMinusEpsilon, Float(UniformUInt32() * 0x1p-32f));
    }

private:
    uint64_t state;
    uint64_t inc;
};
}