#pragma once

#include <RayFlow/Render/scene.h>
#include <RayFlow/Util/parallel.h>
#include <RayFlow/Core/camera.h>

namespace rayflow {

class Integrator {
public:
    virtual ~Integrator() = default;
    
    virtual void Render(const Scene& scene) = 0;
};

class SamplingIntegrator : public Integrator {
public:
    SamplingIntegrator(Camera* camera, Sampler* sampler) :
        mCamera_(camera),
        mSampler_(sampler) {

    }

    virtual void Preprocess(const Scene& scene, Sampler& sampler) {}

    virtual void Render(const Scene& scene);
    
    virtual Spectrum Li(const Ray& ray, const Scene& scene, Sampler& sampler) const {
        return Spectrum(0.f);
    }

protected:
    Camera* mCamera_;
    Sampler* mSampler_;
};

class MonteCarloIntegrator : public SamplingIntegrator {
public:
    MonteCarloIntegrator(Camera* camera, Sampler* sampler, 
                         int maxDepth = 8, int rrDepth = 3, 
                         bool strictNormal = false) :
        SamplingIntegrator(camera, sampler),
        mMaxDepth_(maxDepth), 
        mRRDepth_(rrDepth), 
        mStrictNormal_(strictNormal) {

    }

protected:
    // Longest visualized path depth
    int mMaxDepth_;

    // depth to begin using russian roulette
    int mRRDepth_;

    // whether to use shading normal
    bool mStrictNormal_;
};

}