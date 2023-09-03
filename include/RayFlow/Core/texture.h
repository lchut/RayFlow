#pragma once

#include <RayFlow/Util/vecmath.h>
#include <RayFlow/Core/intersection.h>

namespace rayflow {

template <typename T>
class Texture {
public:
    RAYFLOW_CPU_GPU virtual ~Texture() = default;

    RAYFLOW_CPU_GPU virtual T Evaluate(const SurfaceIntersection& isect) const = 0;
};

}