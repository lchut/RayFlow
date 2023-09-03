#pragma once

#include <RayFlow/Core/sampler.h>
#include <RayFlow/Util/rng.h>

namespace rayflow {

class StratifiedSampler : public Sampler {
public:
    StratifiedSampler(int xSamples, int ySamples, int nDimensions, bool jitter = true) :
        Sampler(xSamples * ySamples),
        xSamples(xSamples), 
        ySamples(ySamples),
        jitter(jitter),
        mCurrent1DDimensions(0),
        mCurrent2DDimensions(0) {
        for (int i = 0; i < nDimensions; ++i) {
            mSamples1D.push_back(std::vector<Float>(xSamples * ySamples));
            mSamples2D.push_back(std::vector<Point2>(xSamples * ySamples));
        }
    }
    
    RAYFLOW_CPU_GPU void StartPixel(const Point2i& pos) final;

    RAYFLOW_CPU_GPU Sampler* Clone(uint64_t seed) final;

    RAYFLOW_CPU_GPU Float Get1D() final;

    RAYFLOW_CPU_GPU Point2 Get2D() final;

public:
    Rng rng;
private:
    const bool jitter;
    const int xSamples;
    const int ySamples;

    int mCurrent1DDimensions;
    int mCurrent2DDimensions;
    std::vector<std::vector<Float>> mSamples1D;
    std::vector<std::vector<Point2>> mSamples2D;
};

RAYFLOW_CPU_GPU void Stratified1D(Float* data, Rng& rng, int nSamples, bool jitter);

RAYFLOW_CPU_GPU void Stratified2D(Point2* data, Rng& rng, int xSamples, int ySamples, bool jitter);

RAYFLOW_CPU_GPU void LatinHypercube(Float* data, Rng& rng, int nSamples, int nDims);

template <typename T>
RAYFLOW_CPU_GPU void Shuffle(T* data, int nSamples, int nDimensions, Rng& rng);

}