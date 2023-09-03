#pragma once

#include <RayFlow/Accelerate/bvh.h>
#include <RayFlow/Render/lights.h>
#include <RayFlow/Render/light_sampler.h>
#include <RayFlow/Core/material.h>
#include <RayFlow/Core/Shape.h>


namespace rayflow {

class Scene {
public:
    Scene(const std::vector<Primitive>& primitives, const std::vector<Light*> lights) :
        mBVH_(primitives, 4),
        mLightSampler_(new UniformLightSampler(lights)) {

    }

    rstd::optional<ShapeIntersection> Intersect(const Ray& ray, Float tMax = INFINITY) const;

    SampledLight SampleLight(const Point3& p, Float u) const;

    const std::vector<Light*>& GetLights() const;

    const LightSampler* GetLightSampler() const;

private:
    BVH mBVH_;
    LightSampler* mLightSampler_;
};

struct VisibilityTester {
    VisibilityTester() = default;

    VisibilityTester(const SurfaceIntersection& p0, const SurfaceIntersection& p1) :
        p0(p0),
        p1(p1) {

    }
    
    bool Visiable(const Scene& scene) const {
        Float dist2 = DistanceSquare(p0.p, p1.p);

        if (dist2 == 0) {
            return 0;
        }

        Ray ray = p0.SpawnRayTo(p1.p);
        
        return !scene.Intersect(ray, ::sqrt(dist2) - ShadowEpsilon);
    }

    SurfaceIntersection p0;
    SurfaceIntersection p1;
};
}
