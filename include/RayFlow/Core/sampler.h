#pragma once

#include <vector>

#include <RayFlow/Util/rng.h>
#include <RayFlow/Util/vecmath.h>
#include <RayFlow/Std/vector.h>

namespace rayflow {

RAYFLOW_CPU_GPU inline Vector3 UniformSampleHemiSphere(const Point2& sample) {
    Float cosTheta = sample[0];
    Float phi = 2 * Pi * sample[1];
    Float sinTheta = ::sqrt(1 - cosTheta * cosTheta);
    
    return Vector3(::cos(phi) * sinTheta, ::sin(phi) * sinTheta, cosTheta);
}

RAYFLOW_CPU_GPU inline Float UniformSampleHemiSpherePdf() {
    return Inv2Pi;
}

RAYFLOW_CPU_GPU inline Vector3 CosineWeightedSampleHemiSphere(const Point2& sample) {
    Float sinTheta = ::sqrt(sample[0]);
    Float cosTheta = ::sqrt(1 - sample[0]);
    Float phi = 2 * Pi * sample[1];
    
    return Vector3(::cos(phi) * sinTheta, ::sin(phi) * sinTheta, cosTheta); 
}

RAYFLOW_CPU_GPU inline Float CosineWeightedSampleHemiSpherePdf(Float cosTheta) {
    return cosTheta * InvPi;
}

RAYFLOW_CPU_GPU inline Vector3 UniformSampleSphere(const Point2& sample) {
    Float cosTheta = Clamp(1 - 2 * sample[0], -1, 1);
    Float sinTheta = ::sqrt(1 - cosTheta * cosTheta);
    Float phi = 2 * Pi * sample[1];
    
    return Vector3(::cos(phi) * sinTheta, ::sin(phi) * sinTheta, cosTheta);
}

RAYFLOW_CPU_GPU inline Float UniformSampleSpherePdf() {
    return Inv4Pi;
}

RAYFLOW_CPU_GPU inline Vector3 UniformSampleCone(const Point2& sample, Float cosMaxTheta) {
    Float cosTheta = 1 - sample[0] + sample[0] * cosMaxTheta;
    Float sinTheta = ::sqrt(1 - cosTheta * cosTheta);
    Float phi = 2 * Pi * sample[1];

    return Vector3(::cos(phi) * sinTheta, ::sin(phi) * sinTheta, cosTheta);
}

RAYFLOW_CPU_GPU inline Float UniformSampleConePdf(Float cosMaxTheta) {
    return Inv2Pi / (1 - cosMaxTheta);
}

RAYFLOW_CPU_GPU inline Point2 UniformSampleTriangle(const Point2& sample) {
    Float su0 = ::sqrt(sample[0]);
    return Point2(1 - su0, su0 * sample[1]);
}

RAYFLOW_CPU_GPU inline Point2 UniformSampleDisk(const Point2& sample) {
    Float r = ::sqrt(sample[0]);
    Float theta = 2 * Pi * sample[1];
    return Point2f(r * ::cos(theta), r * ::sin(theta));
}

RAYFLOW_CPU_GPU inline Float UniformSampleDiskPdf() {
    return Inv2Pi;
}

RAYFLOW_CPU_GPU inline Float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf) {
    return (nf * fPdf) / (nf * fPdf + ng * gPdf);
}

inline Float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf) {
    Float f = nf * fPdf, g = ng * gPdf;
    return (f * f) / (f * f + g * g);
}

class Sampler {
public:
    Sampler(size_t sampleCount) : 
        mSampleCount_(sampleCount),
        mCurrentSampleIndex_(0),
        mSampleArray1DCurrentOffset_(0),
        mSampleArray2DCurrentOffset_(0) {}
    
    virtual ~Sampler() = default;

    virtual Sampler* Clone(uint64_t seed) = 0;

    RAYFLOW_CPU_GPU virtual void StartPixel(const Point2i& pos) {
        mCurrentPixel = pos;
        mCurrentSampleIndex_ = 0;
        mSampleArray1DCurrentOffset_ = 0;
        mSampleArray2DCurrentOffset_ = 0;
    }

    RAYFLOW_CPU_GPU virtual void Advance() {
        ++mCurrentSampleIndex_;
        mSampleArray1DCurrentOffset_ = 0;
        mSampleArray2DCurrentOffset_ = 0;
    }

    RAYFLOW_CPU_GPU virtual Float Get1D() = 0;

    RAYFLOW_CPU_GPU virtual Point2 Get2D() = 0;

    RAYFLOW_CPU_GPU Float* Get1DArray(size_t size) {
        if (mSampleArray1DCurrentOffset_ == mSampleArray1D.size()) { return nullptr; }
        return &mSampleArray1D[mSampleArray1DCurrentOffset_++][mCurrentSampleIndex_ * size];
    }

    RAYFLOW_CPU_GPU Point2* Get2DArray(size_t size) {
        if (mSampleArray2DCurrentOffset_ == mSampleArray2D.size()) { return nullptr; }
        return &mSampleArray2D[mSampleArray2DCurrentOffset_++][mCurrentSampleIndex_ * size];
    }

    RAYFLOW_CPU_GPU void Request1DArray(size_t size) {
        mSampleArray1DSize.push_back(size);
        mSampleArray1D.push_back(rstd::vector<Float>(size * mSampleCount_));
    }

    RAYFLOW_CPU_GPU void Request2DArray(size_t size) {
        mSampleArray2DSize.push_back(size);
        mSampleArray2D.push_back(rstd::vector<Point2>(size * mSampleCount_));
    }

    RAYFLOW_CPU_GPU size_t GetSampleCount() const {
        return mSampleCount_;
    }

    RAYFLOW_CPU_GPU size_t GetCurrentSampleNumber() const {
        return mCurrentSampleIndex_;
    }

protected:
    Point2i mCurrentPixel;
    size_t mSampleCount_;
    size_t mCurrentSampleIndex_;
    size_t mSampleArray1DCurrentOffset_;
    size_t mSampleArray2DCurrentOffset_;
    rstd::vector<size_t> mSampleArray1DSize;
    rstd::vector<size_t> mSampleArray2DSize;
    rstd::vector<rstd::vector<Float>> mSampleArray1D;
    rstd::vector<rstd::vector<Point2>> mSampleArray2D;
};


}