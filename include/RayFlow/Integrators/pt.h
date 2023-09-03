#pragma once
#include <RayFlow/Core/integrator.h>

namespace rayflow {

class PathTracerIntegrator : public MonteCarloIntegrator {
public:
    PathTracerIntegrator(Camera* camera, Sampler* sampler, 
                         int maxDepth = 8, int rrDepth = 3, 
                         bool strictNormal = false) :
        MonteCarloIntegrator(camera, sampler, maxDepth, rrDepth, strictNormal) {

    }

    virtual Spectrum Li(const Ray& ray, const Scene& scene, Sampler& sampler) const;
};

}