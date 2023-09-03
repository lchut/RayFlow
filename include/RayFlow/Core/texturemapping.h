#pragma once

#include <RayFlow/rayflow.h>

namespace rayflow {

class TextureMapping {
public: 
    RAYFLOW_CPU_GPU virtual ~TextureMapping() = default;

    RAYFLOW_CPU_GPU virtual Point2 Map(const Intersection& isect) const = 0;
};


}