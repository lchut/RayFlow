#include <RayFlow/Render/samplers.h>

namespace rayflow {

void StratifiedSampler::StartPixel(const Point2i& pos) {
    for (int i = 0; i < mSamples1D.size(); ++i) {
        Stratified1D(&mSamples1D[i][0], rng, xSamples * ySamples, jitter);
        Shuffle(&mSamples1D[i][0], xSamples * ySamples, 1, rng);
    }

    for (int i = 0; i < mSamples2D.size(); ++i) {
        Stratified2D(&mSamples2D[i][0], rng, xSamples, ySamples, jitter);
        Shuffle(&mSamples2D[i][0], xSamples * ySamples, 1, rng);
    }

    for (int i = 0; i < mSampleArray1DSize.size(); ++i) {
        const auto count = mSampleArray1DSize[i];
        for (int j = 0; j < mSampleCount_; ++j) {
            Stratified1D(&mSampleArray1D[i][j * count], rng, count, jitter);
            Shuffle(&mSampleArray1D[i][j * count], count, 1, rng);
        }
    }

    for (int i = 0; i < mSampleArray2DSize.size(); ++i) {
        const auto count = mSampleArray2DSize[i];
        for (int j = 0; j < mSampleCount_; ++j) {
            LatinHypercube(reinterpret_cast<Float*>(&mSampleArray2D[i][j + count]), rng, count, 2);
        }
    }

    Sampler::StartPixel(pos);
    mCurrent1DDimensions = 0;
    mCurrent2DDimensions = 0;
}

Sampler* StratifiedSampler::Clone(uint64_t seed) {
    Sampler* sampler = new StratifiedSampler(*this);
    rng.Reset(seed);
    return sampler;
}

Float StratifiedSampler::Get1D()  {
    if (mCurrent1DDimensions < mSamples1D.size()) {
        return mSamples1D[mCurrent1DDimensions++][mCurrentSampleIndex_];
    }
    else {
        return rng.UniformFloat();
    }
}

Point2 StratifiedSampler::Get2D() {
    if (mCurrent2DDimensions < mSamples2D.size()) {
        return mSamples2D[mCurrent2DDimensions++][mCurrentSampleIndex_];
    }
    else {
        return Point2(rng.UniformFloat(), rng.UniformFloat());
    }
}

void Stratified1D(Float* data, Rng& rng, int nSamples, bool jitter) {
    for (int i = 0; i < nSamples; ++i) {
        Float delta = jitter ? rng.UniformFloat() : 0.5f;
        data[i] = std::min((i + delta) / nSamples, OneMinusEpsilon);
    }
}

void Stratified2D(Point2* data, Rng& rng, int xSamples, int ySamples, bool jitter) {
    for (int i = 0; i < xSamples; ++i) {
        for (int j = 0; j < ySamples; ++j) {
            Float xDelta = jitter ? rng.UniformFloat() : 0.5f;
            Float yDelta = jitter ? rng.UniformFloat() : 0.5f;
            *data = Point2((i + xDelta) / xSamples, (j + yDelta) / ySamples);
            ++data;
        }
    }
}

void LatinHypercube(Float* data, Rng& rng, int nSamples, int nDims) {
    for (int i = 0; i < nSamples; ++i) {
        for (int j = 0; j < nDims; ++j) {
            data[i * nDims + j] = std::min((i + rng.UniformFloat()) / nSamples, OneMinusEpsilon);
        }
    }

    for (int i = 0; i < nDims; ++i) {
        for (int j = 0; j < nSamples; ++j) {
            int idx = j + rng.UniformUInt32(nSamples - j);
            std::swap(data[j * nDims + i], data[idx * nDims + i]);
        }
    }
}

template <typename T>
void Shuffle(T* data, int nSamples, int nDimensions, Rng& rng) {
    for (int i = 0; i < nSamples; ++i) {
        int idx = i + rng.UniformUInt32(nSamples - i);
        for (int j = 0; j < nDimensions; ++j) {
            std::swap(data[i * nDimensions + j], data[idx * nDimensions + j]);
        }
    }
}

}