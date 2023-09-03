#pragma once

#include <RayFlow/rayflow.h>
#include <RayFlow/Util/math.h>

namespace rayflow {

enum SpectrumType {
    Reflectance,
    Illuminant
};

static constexpr int sampledLambdaStart = 400;
static constexpr int sampledLambdaEnd = 700;
static constexpr int nSpectralSamples = 60;

inline void XYZToRGB(const Float xyz[3], Float rgb[3]) {
    rgb[0] = 3.240479f * xyz[0] - 1.537150f * xyz[1] - 0.498535f * xyz[2];
    rgb[1] = -0.969256f * xyz[0] + 1.875991f * xyz[1] + 0.041556f * xyz[2];
    rgb[2] = 0.055648f * xyz[0] - 0.204043f * xyz[1] + 1.057311f * xyz[2];
}

inline void RGBToXYZ(const Float rgb[3], Float xyz[3]) {
    xyz[0] = 0.412453f * rgb[0] + 0.357580f * rgb[1] + 0.180423f * rgb[2];
    xyz[1] = 0.212671f * rgb[0] + 0.715160f * rgb[1] + 0.072169f * rgb[2];
    xyz[2] = 0.019334f * rgb[0] + 0.119193f * rgb[1] + 0.950227f * rgb[2];
}

static const int nCIESamples = 471;
extern const Float CIE_X_entries[nCIESamples];
extern const Float CIE_Y_entries[nCIESamples];
extern const Float CIE_Z_entries[nCIESamples];
extern const Float CIE_lambda[nCIESamples];
static const Float CIE_Y_integral = 106.856895;

template <int N>
class TSpectrum {
public:
    TSpectrum(Float val = 0.0f) {
        for (int i = 0; i < N; ++i) { values[i] = val; }
    }

    explicit TSpectrum(Float vals[N]) {
        for (int i = 0; i < N; ++i) { values[i] = vals[i]; }
    }

    RAYFLOW_CPU_GPU bool operator==(const TSpectrum& other) const {
        for (int i = 0; i < N; ++i) {
            if (values[i] != other.values[i]) {
                return false;
            }
        }
        return true;
    }

    RAYFLOW_CPU_GPU bool operator!=(const TSpectrum& other) const {
        return !(*this == other);
    }

    RAYFLOW_CPU_GPU TSpectrum operator+(const TSpectrum& spec) const {
        TSpectrum result;
        for (int i = 0; i < N; ++i) {
            result.values[i] = values[i] + spec.values[i];
        }
        return result;
    }

    RAYFLOW_CPU_GPU TSpectrum& operator+=(const TSpectrum& spec) {
        for (int i = 0; i < N; ++i) {
            values[i] += spec.values[i];
        }
        return *this;
    }

    RAYFLOW_CPU_GPU TSpectrum operator-(const TSpectrum& spec) const {
        TSpectrum result;
        for (int i = 0; i < N; ++i) {
            result.values[i] = values[i] - spec.values[i];
        }
        return result;
    }

    RAYFLOW_CPU_GPU TSpectrum& operator-=(const TSpectrum& spec) {
        for (int i = 0; i < N; ++i) {
            values[i] -= spec.values[i];
        }
        return *this;
    }

    RAYFLOW_CPU_GPU TSpectrum operator*(Float u) const {
        TSpectrum result;
        for (int i = 0; i < N; ++i) {
            result.values[i] = values[i] * u;
        }
        return result;
    }

    RAYFLOW_CPU_GPU TSpectrum& operator*=(Float u) {
        for (int i = 0; i < N; ++i) {
            values[i] *= u;
        }
        return *this;
    }

    RAYFLOW_CPU_GPU TSpectrum operator*(const TSpectrum& spec) const {
        TSpectrum result;
        for (int i = 0; i < N; ++i) {
            result.values[i] = values[i] * spec.values[i];
        }
        return result;
    }

    RAYFLOW_CPU_GPU TSpectrum& operator*=(const TSpectrum& spec) {
        for (int i = 0; i < N; ++i) {
            values[i] *= spec.values[i];
        }
        return *this;
    }

    RAYFLOW_CPU_GPU TSpectrum operator/(Float u) const {
        TSpectrum result;
        for (int i = 0; i < N; ++i) {
            result.values[i] = values[i] / u;
        }
        return result;
    }

    RAYFLOW_CPU_GPU TSpectrum& operator/=(Float u) {
        for (int i = 0; i < N; ++i) {
            values[i] /= u;
        }
        return *this;
    }

    RAYFLOW_CPU_GPU TSpectrum operator/(const TSpectrum& spec) const {
        TSpectrum result;
        for (int i = 0; i < N; ++i) {
            result.values[i] = values[i] / spec.values[i];
        }
        return result;
    }

    RAYFLOW_CPU_GPU TSpectrum& operator/=(const TSpectrum& spec) {
        for (int i = 0; i < N; ++i) {
            values[i] /= spec.values[i];
        }
        return *this;
    }

    RAYFLOW_CPU_GPU Float operator[](int idx) const {
        return values[idx];
    }

    RAYFLOW_CPU_GPU Float& operator[](int idx) {
        return values[idx];
    }

    RAYFLOW_CPU_GPU TSpectrum Abs() const {
        using std::abs;

        TSpectrum result;
        for (int i = 0; i < N; ++i) {
            result.values[i] = abs(values[i]);
        }
        return result;
    }

    RAYFLOW_CPU_GPU TSpectrum Sqrt() const {
        using std::sqrt;

        TSpectrum result;
        for (int i = 0; i < N; ++i) {
            result.values[i] = sqrt(values[i]);
        }
        return result;
    }

    RAYFLOW_CPU_GPU TSpectrum Log() const {
        using std::log;

        TSpectrum result;
        for (int i = 0; i < N; ++i) {
            result.values[i] = log(values[i]);
        }
        return result;
    }

    RAYFLOW_CPU_GPU TSpectrum Exp() const {
        using std::exp;

        TSpectrum result;
        for (int i = 0; i < N; ++i) {
            result.values[i] = exp(values[i]);
        }
        return result;
    }

    RAYFLOW_CPU_GPU TSpectrum Pow(Float e) const {
        using std::pow;

        TSpectrum result;
        for (int i = 0; i < N; ++i) {
            result.values[i] = pow(values[i], e);
        }
        return result;
    }

    RAYFLOW_CPU_GPU TSpectrum Clamp(Float low, Float high) const {
        TSpectrum result;
        for (int i = 0; i < N; ++i) {
            result.values[i] = Clamp(values[i], low, high);
        }
        return result;
    }

    RAYFLOW_CPU_GPU bool IsBlack() const {
        for (int i = 0; i < N; ++i) {
            if (values[i] != 0) {
                return false;
            }
        }

        return true;
    }

    RAYFLOW_CPU_GPU bool HasNaN() const {
        for (int i = 0; i < N; ++i) {
            if (::isnan(values[i])) {
                return true;
            }
        }

        return false;
    }

    RAYFLOW_CPU_GPU bool HasINF() const {
        for (int i = 0; i < N; ++i) {
            if (::isinf(values[i])) {
                return true;
            }
        }

        return false;
    }


    Float values[N];
};

template<int N>
inline TSpectrum<N> Lerp(const TSpectrum<N> &s1, const TSpectrum<N> &s2, Float t) {
    return (1 - t) * s1 + t * s2;
}

template <int N>
RAYFLOW_CPU_GPU inline TSpectrum<N> operator*(Float u, const TSpectrum<N>& spec) {
    return spec * u;
}

class RGBSpectrum : public TSpectrum<3> {
public:
    RGBSpectrum(Float v = 0.0f) : TSpectrum<3>(v) {}

    RGBSpectrum(Float v1, Float v2, Float v3) : TSpectrum<3>() {
        values[0] = v1;
        values[1] = v2;
        values[2] = v3;
    }


    RGBSpectrum(const TSpectrum<3>& v) : TSpectrum<3>(v) {}

    explicit RGBSpectrum(Float vals[3]) : TSpectrum<3>(vals) {}

    Float Luminance() const {
        const Float YWeight[3] = {0.212671f, 0.715160f, 0.072169f};
        return YWeight[0] * values[0] + YWeight[1] * values[1] + YWeight[2] * values[2];
    }

    void ToXYZ(Float xyz[3]) const {
        RGBToXYZ(values, xyz);
    }

    static RGBSpectrum FromXYZ(Float xyz[3], SpectrumType type = SpectrumType::Reflectance) {
        RGBSpectrum result;
        XYZToRGB(xyz, result.values);
        return result;
    }

    void ToRGB(Float rgb[3]) const {
        rgb[0] = values[0];
        rgb[1] = values[1];
        rgb[2] = values[2];
    }

    static RGBSpectrum FromRGB(Float rgb[3], SpectrumType type = SpectrumType::Reflectance) {
        RGBSpectrum result;
        result.values[0] = rgb[0];
        result.values[1] = rgb[1];
        result.values[2] = rgb[2];
        return result;
    }
};
}