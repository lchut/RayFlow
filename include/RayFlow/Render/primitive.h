#pragma once

#include <RayFlow/Render/shapes.h>
#include <RayFlow/Render/materials.h>
#include <RayFlow/Render/lights.h>
#include <RayFlow/Render/bxdfs.h>
#include <RayFlow/Std/optional.h>

namespace rayflow {

class Primitive {
public:
    Primitive() : mShape_(nullptr), 
                  mMaterial_(nullptr),
                  mAreaLight_(nullptr) {

    }

    Primitive(const Shape* shape, const Material* material, const AreaLight* areaLight = nullptr) :
        mShape_(shape),
        mMaterial_(material),
        mAreaLight_(areaLight) {

    }

    rstd::optional<ShapeIntersection> Intersect(const Ray& ray, Float tMax) const {
        return mShape_->Intersect(ray, tMax);
    }

    rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const {
        return mMaterial_->EvaluateBSDF(isect, mode);
    }

    AABB3 GetBounds() const {
        return mShape_->Bounds();
    }

    const AreaLight* GetAreaLight() const {
        return mAreaLight_;
    }

    const Material* GetMaterial() const {
        return mMaterial_;
    }


    const Shape* mShape_;
    const Material* mMaterial_;  
    const AreaLight* mAreaLight_;
};
}