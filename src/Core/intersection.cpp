#include <RayFlow/Core/intersection.h>
#include <RayFlow/Core/material.h>
#include <RayFlow/Render/lights.h>
#include <RayFlow/Render/primitive.h>

namespace rayflow {

Spectrum SurfaceIntersection::Le(const Intersection& ref) const {
    const AreaLight* areaLight = primitive->GetAreaLight();
    return areaLight ? areaLight->Le(ref, *this) : Spectrum(0.f);
}

rstd::optional<BSDF> SurfaceIntersection::EvaluateBSDF(TransportMode mode) const {
    const Material* material = primitive->GetMaterial();

    if (material == nullptr) {
        return {};
    }

    return material->EvaluateBSDF(*this, mode);
}

const AreaLight* SurfaceIntersection::GetAreaLight() const {
    return primitive ? primitive->GetAreaLight() : nullptr;
}

}