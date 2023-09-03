#pragma once

#include <RayFlow/rayflow.h>

namespace rayflow {

#define HALF_SIGN_MASK 0x8000
#define HALF_EXPONENT_MASK 0x7c00
#define HALF_MANTISSA_MASK 0X03ff

#define GET_HALF_SIGN_BIT(x) ((x) >> 15)
#define GET_HALF_EXPONENT_BITS(x) (((x) & HALF_EXPONENT_MASK) >> 10)
#define GET_HALF_MANTISSA_BITS(x) (((x) & HALF_MANTISSA_MASK))

#define SET_HALF_SIGN_BIT(x, dest) ((dest) = ((((x) << 15) & HALF_SIGN_MASK) | ((dest) & (HALF_EXPONENT_MASK | HALF_MANTISSA_MASK))))
#define SET_HALF_EXPONENT_BITS(x, dest) ((dest) = ((((x << 10) & HALF_EXPONENT_MASK)) | ((dest) & (HALF_SIGN_MASK | HALF_MANTISSA_MASK))))
#define SET_HALF_MANTISSA_BITS(x, dest) ((dest) = (((x) & HALF_MANTISSA_MASK) | ((dest) & (HALF_SIGN_MASK | HALF_EXPONENT_MASK))))

#define MIN_HALF 5.96046448e-08f
#define MIN_NORM_HALF 6.10351562e-05f
#define MAX_HALF 65504.0f
#define HALF_EPSILON 0.00097656f

union FP16 {
    unsigned short u;
    struct {
        unsigned int Mantissa : 10;
        unsigned int Exponent : 5;
        unsigned int Sign : 1;
    };
};

union FP32 {
    uint32_t u;
    float f;
    struct {
        unsigned int Mantissa : 23;
        unsigned int Exponent : 8;
        unsigned int Sign : 1;
    };
};

class Half {
public:
    RAYFLOW_CPU_GPU Half() = default;

    RAYFLOW_CPU_GPU Half(float v);

    RAYFLOW_CPU_GPU Half& operator=(Half v) {
        bits = v.bits;
        return *this;
    }

    RAYFLOW_CPU_GPU Half& operator=(float v);

    RAYFLOW_CPU_GPU operator float() const;

    RAYFLOW_CPU_GPU Half operator-() const {
        Half half;
        half.bits = bits ^ 0x8000;
        return half;
    }

    RAYFLOW_CPU_GPU Half operator-(Half v) const {
        return Half(float(*this) - float(v));
    }

    RAYFLOW_CPU_GPU Half operator-(float v) const {
        return Half(float(*this) - v);
    }

    RAYFLOW_CPU_GPU Half operator+(Half v) const {
        return Half(float(*this) + float(v));
    }

    RAYFLOW_CPU_GPU Half operator+(float v) const {
        return Half(float(*this) + v);
    }

    RAYFLOW_CPU_GPU Half operator*(Half v) const {
        return Half(float(*this) * float(v));
    }

    RAYFLOW_CPU_GPU Half operator*(float v) const {
        return Half(float(*this) * v);
    }

    RAYFLOW_CPU_GPU Half operator/(Half v) const {
        return Half(float(*this) / float(v));
    }

    RAYFLOW_CPU_GPU Half operator/(float v) const {
        return Half(float(*this) / v);
    }

    RAYFLOW_CPU_GPU Half& operator-=(Half v) {
        *this = float(*this) - float(v);
        return *this;
    }

    RAYFLOW_CPU_GPU Half& operator-=(float v) {
        *this = float(*this) - v;
        return *this;
    }

    RAYFLOW_CPU_GPU Half& operator+=(Half v) {
        *this = float(*this) + float(v);
        return *this;
    }

    RAYFLOW_CPU_GPU Half& operator+=(float v) {
        *this = float(*this) + v;
        return *this;
    }

    RAYFLOW_CPU_GPU Half& operator*=(Half v) {
        *this = float(*this) * float(v);
        return *this;
    }

    RAYFLOW_CPU_GPU Half& operator*=(float v) {
        *this = float(*this) * v;
        return *this;
    }

    RAYFLOW_CPU_GPU Half& operator/=(Half v);

    RAYFLOW_CPU_GPU Half& operator/=(float v) {
        *this = float(*this) / v;
        return *this;
    }

    RAYFLOW_CPU_GPU bool IsFinite () const {
        unsigned short e = (bits >> 10) & 0x001f;
        return e < 31;
    }

    RAYFLOW_CPU_GPU bool IsNormalized () const {
        unsigned short e = (bits >> 10) & 0x001f;
        return e > 0 && e < 31;
    }

    RAYFLOW_CPU_GPU bool IsDenormalized () const {
        unsigned short e = (bits >> 10) & 0x001f;
        unsigned short m =  bits & 0x3ff;
        return e == 0 && m != 0;
    }

    RAYFLOW_CPU_GPU bool IsZero () const {
        return (bits & 0x7fff) == 0;
    }

    RAYFLOW_CPU_GPU bool IsNan () const {
        unsigned short e = (bits >> 10) & 0x001f;
        unsigned short m = bits & 0x3ff;
        return e == 31 && m != 0;
    }

    RAYFLOW_CPU_GPU bool IsInfinity () const {
        unsigned short e = (bits >> 10) & 0x001f;
        unsigned short m = bits & 0x3ff;
        return e == 31 && m == 0;
    }

    RAYFLOW_CPU_GPU bool IsNegative () const {
        return (bits & HALF_SIGN_MASK) != 0;
    }

    static Half PostiveInfinity() {
        return {0x7c00};
    }

    static Half NegtiveInfinity() {
        return {0xfc00};
    }

private:
    unsigned short bits;
}; 



}