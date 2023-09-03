#pragma once

#include <RayFlow/rayflow.h>
#include <RayFlow/Util/vecmath.h>
#include <RayFlow/Core/ray.h>
#include <RayFlow/Core/spectrum.h>
#include <RayFlow/Std/optional.h>

namespace rayflow {

enum class TransportMode {
    Importance = 0,
    Radiance
};

enum class BXDFType {
    REFLECTION = 1 << 0,
    TRANSMISSION = 1 << 1,
    DIFFUSE = 1 << 2,
    GLOSSY = 1 << 3,
    SPECULAR = 1 << 4,
    ALL = REFLECTION | TRANSMISSION | DIFFUSE | GLOSSY | SPECULAR
};

struct BXDFSample {
    BXDFSample() : f(0.0f), pdf(0) {

    }

    BXDFSample(const Spectrum& f, const Vector3& wi, Float pdf, BXDFType type) :
        f(f), wi(wi), pdf(pdf), type(type) {

    }
    
    Spectrum f;
    Vector3 wi;
    Float pdf;
    BXDFType type;
};

RAYFLOW_CPU_GPU inline bool HasSpecularComponent(BXDFType type) {
    return (int(type) & int(BXDFType::SPECULAR));
}

class BXDF {
public: 
    RAYFLOW_CPU_GPU BXDF(BXDFType type) : type(type) {

    } 

    RAYFLOW_CPU_GPU virtual ~BXDF() = default;
    
    RAYFLOW_CPU_GPU virtual rstd::optional<BXDFSample> SampleF(const Vector3& wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const = 0;

    RAYFLOW_CPU_GPU virtual Spectrum f(const Vector3& wi, const Vector3& wo, TransportMode mode = TransportMode::Radiance) const = 0;

    RAYFLOW_CPU_GPU virtual Float Pdf(const Vector3& wi, const Vector3& wo) const = 0;

    const BXDFType type;
};

}