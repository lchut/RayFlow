#pragma once

#include <RayFlow/rayflow.h>
#include <RayFlow/Util/vecmath.h>
#include <RayFlow/Core/ray.h>
#include <RayFlow/Core/bxdf.h>
#include <RayFlow/Std/optional.h>

namespace rayflow {

class Intersection {
public:
    Intersection() = default;

    RAYFLOW_CPU_GPU virtual ~Intersection() {}
    
    RAYFLOW_CPU_GPU Intersection(const Vector3& wo, const Point3& p, const Normal3& n, const Point2& uv) :
        wo(wo), p(p), ng(n), uv(uv) {}
    
    RAYFLOW_CPU_GPU Intersection(const Vector3& wo, const Point3& p, const Normal3& n) :
        wo(wo), p(p), ng(n) {}

    RAYFLOW_CPU_GPU Intersection(const Vector3& wo, const Point3& p) : wo(wo), p(p) {}
    
    RAYFLOW_CPU_GPU Intersection(const Point3& p, const Normal3& n) : p(p), ng(n) {}

    RAYFLOW_CPU_GPU Intersection(const Point3& p, const Point2& uv) : p(p), uv(uv) {}

    RAYFLOW_CPU_GPU Intersection(const Point3& p, const Normal3& n, const Point2& uv) :
        p(p),
        ng(n),
        uv(uv) {}

    RAYFLOW_CPU_GPU explicit Intersection(const Point3& p) : p(p) {}

    RAYFLOW_CPU_GPU inline Ray SpawnRay(const Vector3& dir) const {
        return Ray(p, dir);
    }

    RAYFLOW_CPU_GPU inline Ray SpawnRayTo(const Point3& target) const {
        return Ray(p, Normalize(target - p));
    }

    Vector3 wo;
    Point3 p;
    Normal3 ng;
    Point2 uv;
};

class SurfaceIntersection : public Intersection {
public:
    using Intersection::Intersection;

    SurfaceIntersection() = default;

    SurfaceIntersection(const Intersection& isect) : Intersection(isect) {

    }
    
    RAYFLOW_CPU_GPU SurfaceIntersection(const Vector3& wo, const Point3& p, const Normal3& n, const Point2& texCoord, 
                                        const Normal3& ns, const Primitive* primitive) :
                                        Intersection(wo, p, n, texCoord),
                                        ns(ns),
                                        primitive(primitive) {}

    RAYFLOW_CPU_GPU Spectrum Le(const Intersection& ref) const;

    RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(TransportMode mode = TransportMode::Radiance) const;

    RAYFLOW_CPU_GPU const AreaLight* GetAreaLight() const;

    Normal3 ns;
    const Primitive* primitive = nullptr;
};

}