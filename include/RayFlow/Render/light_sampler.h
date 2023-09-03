#pragma once

#include <RayFlow/Std/vector.h>
#include <RayFlow/Std/optional.h>
#include <RayFlow/Core/light.h>

namespace rayflow {

struct SampledLight {
    Light* light;
    Float pdf;
};

class LightSampler {
public:
    LightSampler(const std::vector<Light*>& lights) :
        mLights_(lights) {

    }

    RAYFLOW_CPU_GPU virtual ~LightSampler() = default;

    RAYFLOW_CPU_GPU virtual SampledLight Sample(const Point3& p, Float u) const = 0;

    RAYFLOW_CPU_GPU virtual Float Pdf(const Point3& p, const Light* light) const = 0;

    RAYFLOW_CPU_GPU const std::vector<Light*>& GetLights() const {
        return mLights_;
    }

protected:
    std::vector<Light*> mLights_;
};

class UniformLightSampler final : public LightSampler {
public:
    RAYFLOW_CPU_GPU UniformLightSampler(const std::vector<Light*>& lights) : 
        LightSampler(lights) {

    }

    RAYFLOW_CPU_GPU SampledLight Sample(const Point3& p, Float u) const override {
        int nLights = mLights_.size();
        int idx = std::min(int(u * nLights), nLights - 1);
        return SampledLight{mLights_[idx], 1.0f / mLights_.size()};
    }

    RAYFLOW_CPU_GPU Float Pdf(const Point3& p, const Light* light) const override {
        return 1.0f / mLights_.size();
    }

};  

class PowerLightSampler final : public LightSampler {
public:
    RAYFLOW_CPU_GPU PowerLightSampler(const std::vector<Light*>& lights) : 
            LightSampler(lights),
            mPowerPdf_(lights.size()),
            mPowerCdf_(lights.size() + 1) {
        int n = mLights_.size();
        Float totalPower = 0;
        rstd::vector<Float> powers(n);
        for (int i = 0; i < n; ++i) {
            powers[i] = mLights_[i]->Power().Luminance();
            totalPower += powers[i];
        }

        for (int i = 0; i < n; ++i) {
            mPowerPdf_[i] = powers[i] / totalPower;
        }

        mPowerCdf_[0] = 0;
        for (int i = 1; i < n + 1; ++i) {
            mPowerCdf_[i] = mPowerCdf_[i - 1] + mPowerPdf_[i - 1];
        }
        mPowerCdf_[n] = 1;
    }

    RAYFLOW_CPU_GPU SampledLight Sample(const Point3& p, Float u) const override {
        int offset = FindInterval((int)mPowerCdf_.size(), 
                                  [&](int idx)->bool {
                                    return mPowerCdf_[idx] <= u;});
        return SampledLight{mLights_[offset], mPowerPdf_[offset]};
    }

    RAYFLOW_CPU_GPU Float Pdf(const Point3& p, const Light* light) const override {
        for (int i = 0; i < mLights_.size(); ++i) {
            if (light == mLights_[i]) {
                return mPowerPdf_[i];
            }
        }
        return 0;
    }   
    
private:
    rstd::vector<Float> mPowerPdf_;
    rstd::vector<Float> mPowerCdf_;
};

}