#include <RayFlow/Util/half.h>

namespace rayflow {
// https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/float.cpp
RAYFLOW_CPU_GPU Half::Half(float v) {
#ifdef RAYFLOW_GENERATE_GPU_CODE
        h = __half_as_ushort(__float2half(ff));
#else
        // Rounding ties to nearest even instead of towards +inf
        FP32 f;
        f.f = v;
        FP32 f32infty = {255 << 23};
        FP32 f16max = {(127 + 16) << 23};
        FP32 denorm_magic = {((127 - 15) + (23 - 10) + 1) << 23};
        unsigned int sign_mask = 0x80000000u;
        FP16 o = {0};

        unsigned int sign = f.u & sign_mask;
        f.u ^= sign;

        // NOTE all the integer compares in this function can be safely
        // compiled into signed compares since all operands are below
        // 0x80000000. Important if you want fast straight SSE2 code
        // (since there's no unsigned PCMPGTD).

        if (f.u >= f16max.u)  // result is Inf or NaN (all exponent bits set)
            o.u = (f.u > f32infty.u) ? 0x7e00 : 0x7c00;  // NaN->qNaN and Inf->Inf
        else {                                           // (De)normalized number or zero
            if (f.u < (113 << 23)) {  // resulting FP16 is subnormal or zero
                // use a magic value to align our 10 mantissa bits at the bottom
                // of the float. as long as FP addition is round-to-nearest-even
                // this just works.
                f.f += denorm_magic.f;

                // and one integer subtract of the bias later, we have our final
                // float!
                o.u = f.u - denorm_magic.u;
            } else {
                unsigned int mant_odd = (f.u >> 13) & 1;  // resulting mantissa is odd

                // update exponent, rounding bias part 1
                f.u += (uint32_t(15 - 127) << 23) + 0xfff;
                // rounding bias part 2
                f.u += mant_odd;
                // take the bits!
                o.u = f.u >> 13;
            }
        }

        o.u |= sign >> 16;
        bits = o.u;
#endif
}

RAYFLOW_CPU_GPU Half& Half::operator=(float v) {
    *this = Half(v);
    return *this;
}

// https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/float.cpp
RAYFLOW_CPU_GPU Half::operator float() const {
#ifdef RAYFLOW_GENERATE_GPU_CODE
        return __half2float(__ushort_as_half(h));
#else
        FP16 h;
        h.u = this->bits;
        static const FP32 magic = {113 << 23};
        static const unsigned int shifted_exp = 0x7c00
                                                << 13;  // exponent mask after shift
        FP32 o;

        o.u = (h.u & 0x7fff) << 13;            // exponent/mantissa bits
        unsigned int exp = shifted_exp & o.u;  // just the exponent
        o.u += (127 - 15) << 23;               // exponent adjust

        // handle exponent special cases
        if (exp == shifted_exp)       // Inf/NaN?
            o.u += (128 - 16) << 23;  // extra exp adjust
        else if (exp == 0) {          // Zero/Denormal?
            o.u += 1 << 23;           // extra exp adjust
            o.f -= magic.f;           // renormalize
        }

        o.u |= (h.u & 0x8000) << 16;  // sign bit
        return o.f;
#endif
}

}