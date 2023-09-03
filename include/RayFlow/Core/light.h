#pragma once

#include <RayFlow/Util/vecmath.h>
#include <RayFlow/Core/ray.h>
#include <RayFlow/Core/intersection.h>
#include <RayFlow/Core/spectrum.h>
#include <RayFlow/Std/optional.h>

namespace rayflow {

enum class LightType {
    DELTA_POSITION  = 1 << 0,
    DELTA_DIRECTION = 1 << 1,
    AREA = 1 << 2
};

inline bool IsDeltaLight(LightType type) {
    return ((int)type & (int)LightType::DELTA_POSITION) ||
        ((int)type & (int)LightType::DELTA_DIRECTION);
}

struct LightLiSample {
    LightLiSample() = default;

    RAYFLOW_CPU_GPU LightLiSample(const Spectrum& L, const Vector3& wi, Float pdfPos,
                                  Float pdfDir, const Intersection& pLight) :
        L(L), 
        wi(wi),
        pdfPos(pdfPos),
        pdfDir(pdfDir),
        pLight(pLight) {}
    
    Spectrum L;
    Vector3 wi;
    Float pdfPos;
    Float pdfDir;
    Intersection pLight;
};

struct LightLeSample {
    LightLeSample() = default;

    RAYFLOW_CPU_GPU explicit LightLeSample(const Ray& ray) :
        ray(ray) {
            
    }

    RAYFLOW_CPU_GPU LightLeSample(const Ray& ray, const Spectrum& L, Float pdfPos, Float pdfDir) :
        ray(ray), L(L), pdfPos(pdfPos), pdfDir(pdfDir) {}

    RAYFLOW_CPU_GPU LightLeSample(const Ray& ray, const Intersection& pLight, const Spectrum& L, Float pdfPos, Float pdfDir) :
        ray(ray), pLight(pLight), L(L), pdfPos(pdfPos), pdfDir(pdfDir) {}
    
    Spectrum L;
    Ray ray;
    Intersection pLight;
    Float pdfPos;
    Float pdfDir;
};

class Light {
public:
    Light(const Transform* wtl, const Transform* ltw, LightType type) : 
        mWorldToLocal_(wtl), mLocalToWorld_(ltw), type(type) {}

    RAYFLOW_CPU_GPU virtual ~Light() = default;
    
    RAYFLOW_CPU_GPU virtual Spectrum Le(const Intersection& ref, const Intersection& lightIsect = {}) const = 0;

    RAYFLOW_CPU_GPU virtual rstd::optional<LightLiSample> SampleLi(const Intersection&ref, const Point2& sample) const = 0;

    RAYFLOW_CPU_GPU virtual rstd::optional<LightLeSample> SampleLe(const Point2& posSample, const Point2& dirSample) const = 0;

    RAYFLOW_CPU_GPU virtual Float PdfLi(const Intersection& ref, const Vector3& wi) const = 0;

    RAYFLOW_CPU_GPU virtual void PdfLe(LightLeSample& ers) const = 0;

    RAYFLOW_CPU_GPU virtual Spectrum Power() const = 0;

    const Transform* mWorldToLocal_;
    const Transform* mLocalToWorld_;
    LightType type;
};
}