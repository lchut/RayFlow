#pragma once

#include <RayFlow/rayflow.h>

namespace rayflow {

class RGB {
public:
    RAYFLOW_CPU_GPU RGB() : r(0), g(0), b(0) {}
    
    RAYFLOW_CPU_GPU RGB(Float r, Float g, Float b) : r(r), g(g), b(b) {}

    RAYFLOW_CPU_GPU RGB operator+(const RGB& rgb) const {
        return RGB(r + rgb.r, g + rgb.g, b + rgb.b);
    }

    RAYFLOW_CPU_GPU RGB& operator+=(const RGB& rgb) {
        r += rgb.r;
        g += rgb.g;
        b += rgb.b;
        return *this;
    }

    RAYFLOW_CPU_GPU RGB operator-(const RGB& rgb) const {
        return RGB(r - rgb.r, g - rgb.g, b - rgb.b);
    }

    RAYFLOW_CPU_GPU RGB& operator-=(const RGB& rgb) {
        r -= rgb.r;
        g -= rgb.g;
        b -= rgb.b;
        return *this;
    }

    RAYFLOW_CPU_GPU RGB operator*(Float u) const {
        return RGB(u * r, u * g, u * b);
    }

    RAYFLOW_CPU_GPU RGB& operator*=(Float u) {
        r *= u;
        g *= u;
        b *= u;
        return *this;
    }

    RAYFLOW_CPU_GPU RGB operator/(Float u) const {
        return RGB(r / u, g / u, b / u);
    }

    RAYFLOW_CPU_GPU RGB& operator/=(Float u) {
        r /= u;
        g /= u;
        b /= u;
        return *this;
    }

    RAYFLOW_CPU_GPU RGB operator*(const RGB& rgb) const {
        return RGB(r * rgb.r, g * rgb.g, b * rgb.b);
    }
    
    RAYFLOW_CPU_GPU RGB& operator*=(const RGB& rgb) {
        r *= rgb.r;
        g *= rgb.g;
        b *= rgb.b;
        return *this;
    }

    RAYFLOW_CPU_GPU inline Float y() const {
        return 0.212671f * r + 0.715160f * g + 0.072169 * b;
    }

    RAYFLOW_CPU_GPU inline bool IsBlack() const {
        return r == 0 && g == 0 && b == 0;
    }

    Float r;
    Float g;
    Float b;
};

}