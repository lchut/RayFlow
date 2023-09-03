#pragma once
#include <RayFlow/Core/integrator.h>

namespace rayflow {

class DirectIntegrator : public SamplingIntegrator {
public:
    DirectIntegrator(Camera* camera, Sampler* sampler) :
        SamplingIntegrator(camera, sampler) {

    }

    virtual Spectrum Li(const Ray& ray, const Scene& scene, Sampler& sampler) const;
};

}