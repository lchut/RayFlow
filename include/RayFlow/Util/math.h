#pragma once

#include <RayFlow/rayflow.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <climits>

namespace rayflow {

constexpr Float ShadowEpsilon = 0.001f;
constexpr Float Pi = 3.14159265358979323846;
constexpr Float InvPi = 0.31830988618379067154;
constexpr Float Inv2Pi = 0.15915494309189533577;
constexpr Float Inv4Pi = 0.07957747154594766788;
constexpr Float PiOver2 = 1.57079632679489661923;
constexpr Float PiOver4 = 0.78539816339744830961;
constexpr Float Sqrt2 = 1.41421356237309504880;
constexpr Float Infinity = std::numeric_limits<Float>::infinity();

template <typename T>
RAYFLOW_CPU_GPU inline void Swap(T& a, T& b) {
    T tmp = std::move(a);
    a = std::move(b);
    b = std::move(tmp);
}

RAYFLOW_CPU_GPU inline Float Lerp(Float x, Float y, Float t) {
	return (1.0 - t) * x + t * y;
}

template <typename T, typename U, typename V>
RAYFLOW_CPU_GPU inline constexpr T Clamp(T val, U leftEnd, V rightEnd) {
	return val < leftEnd ? T(leftEnd) : (val > rightEnd ? T(rightEnd) : val);
}

template <typename T>
RAYFLOW_CPU_GPU inline T Mod(T a, T b) {
    T result = a - (a / b) * b;
    return (T)((result < 0) ? result + b : result);
}

template <>
RAYFLOW_CPU_GPU inline Float Mod(Float a, Float b) {
    return std::fmod(a, b);
}

RAYFLOW_CPU_GPU inline Float Radians(Float deg) {
    return (Pi / 180) * deg;
}

RAYFLOW_CPU_GPU inline Float Degrees(Float rad) {
    return (180 / Pi) * rad;
}

template <int n>
RAYFLOW_CPU_GPU constexpr float Pow(float v) {
    if constexpr (n < 0) {
        return 1 / Pow<-n>(v);
    }
    float tmp = Pow<n / 2>(v);
    return tmp * tmp * Pow<n & 1>(v);
}

template <>
RAYFLOW_CPU_GPU constexpr float Pow<0>(float v) {
    return 1;
}

template <>
RAYFLOW_CPU_GPU constexpr float Pow<1>(float v) {
    return v;
}

template <int n>
RAYFLOW_CPU_GPU constexpr double Pow(double v) {
    if constexpr (n < 0) {
        return 1 / Pow<-n>(v);
    }
    double tmp = Pow<n / 2>(v);
    return tmp * tmp * Pow<n & 1>(v);
}

template <>
RAYFLOW_CPU_GPU constexpr double Pow<0>(double v) {
    return 1;
}

template <>
RAYFLOW_CPU_GPU constexpr double Pow<1>(double v) {
    return v;
}

RAYFLOW_CPU_GPU inline float SafeSqrt(float v) {
    return std::sqrt(std::max(0.0f, v));
}

RAYFLOW_CPU_GPU inline double SafeSqrt(double v) {
    return std::sqrt(std::max(0.0, v));
}

RAYFLOW_CPU_GPU inline float SafeAsin(float v) {
    return std::asin(Clamp(v, -1.0f, 1.0f));
}

RAYFLOW_CPU_GPU inline double SafeAsin(double v) {
    return std::asin(Clamp(v, -1.0, 1.0));
}

RAYFLOW_CPU_GPU inline float SafeAcos(float v) {
    return std::acos(Clamp(v, -1.0f, 1.0f));
}

RAYFLOW_CPU_GPU inline double SafeAcos(double v) {
    return std::acos(Clamp(v, -1.0, 1.0));
}

RAYFLOW_CPU_GPU inline Float SinXOverX(float x) {
    if (1 - x * x == 1) {
        return 1;
    }
    return std::sin(x) / x;
}

RAYFLOW_CPU_GPU inline bool SolveQuadraticEquation(float a, float b, float c, float* x0, float* x1) {
    if (a == 0) {
        if (b == 0) {
            return false;
        }
        *x0 = *x1 = -c / b;
        return true;
    }
    float discrim = b * b - 4 * a * c;
    if (discrim < 0) {
        return false;
    }
    float sqrtDiscrim = std::sqrt(discrim);
    float q = -0.5 * (b + std::copysign(sqrtDiscrim, b));
    *x0 = q / a;
    *x1 = c / q;

    if (*x0 > *x1) {
        Swap(*x0, *x1);
    }
    return true;
}

RAYFLOW_CPU_GPU inline bool SolveQuadraticEquation(double a, double b, double c, double* x0, double* x1) {
    if (a == 0) {
        if (b == 0) {
            return false;
        }
        *x0 = *x1 = -c / b;
        return true;
    }
    double discrim = b * b - 4 * a * c;
    if (discrim < 0) {
        return false;
    }
    double sqrtDiscrim = std::sqrt(discrim);
    double q = -0.5 * (b + std::copysign(sqrtDiscrim, b));
    *x0 = q / a;
    *x1 = c / q;

    if (*x0 > *x1) {
        Swap(*x0, *x1);
    }
    return true;
}

template <typename T>
RAYFLOW_CPU_GPU inline bool IsPowerOf2(T v) {
    return v && !(v & (v - 1));
}

RAYFLOW_CPU_GPU inline int32_t RoundUpPow2(int32_t v) {
    v--;
    v |= v >> 1;    v |= v >> 2;
    v |= v >> 4;    v |= v >> 8;
    v |= v >> 16;
    return v+1;
}


RAYFLOW_CPU_GPU inline Float Gaussian(Float x, Float u, Float sigma) {
    return 1 / ::sqrt(2 * Pi * sigma * sigma) *
        ::exp(-(x - u) * (x - u) / (2 * sigma * sigma));
}

// https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/math.h
RAYFLOW_CPU_GPU inline Float GaussianIntegral(Float x0, Float x1, Float u, Float sigma) {
    Float sigmaRoot2 = sigma * Float(1.414213562373095);
    return 0.5f * (::erf((u - x0) / sigmaRoot2) - ::erf((u - x1) / sigmaRoot2));
}

template <typename Pred>
inline int FindInterval(int size, const Pred& pred) {
    int left = 0;
    int right = size;
    while (left < right) {
        int mid = (left + right) >> 1;

        if (pred(mid)) {
            left = mid + 1;
        }
        else {
            right = mid;
        }
    }
    return Clamp(left - 1, 0, size - 2);
}

}
