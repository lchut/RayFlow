#pragma once

#include <RayFlow/Core/intersection.h>

namespace rayflow {

class Material {
public:
    RAYFLOW_CPU_GPU virtual ~Material() = default;

    RAYFLOW_CPU_GPU virtual rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const = 0;    
};

}