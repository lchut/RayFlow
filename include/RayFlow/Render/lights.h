#pragma once
#include <RayFlow/Core/light.h>
#include <RayFlow/Core/shape.h>
#include <RayFlow/Core/intersection.h>

namespace rayflow {

class PointLight : public Light {
public:
    PointLight(const Transform* wtl, const Transform* ltw, const Spectrum& I) :
               Light(wtl, ltw, LightType::DELTA_POSITION),
               pLight((*ltw)(Point3(0, 0, 0))),
               I(I) {

    }

    RAYFLOW_CPU_GPU Spectrum Le(const Intersection& ref, const Intersection& lightIsect = {}) const final;

    RAYFLOW_CPU_GPU rstd::optional<LightLiSample> SampleLi(const Intersection &ref, const Point2& sample) const final;

    RAYFLOW_CPU_GPU rstd::optional<LightLeSample> SampleLe(const Point2& posSample, const Point2& dirSample) const final;

    RAYFLOW_CPU_GPU Float PdfLi(const Intersection& ref, const Vector3& wi) const final;

    RAYFLOW_CPU_GPU void PdfLe(LightLeSample& ers) const final;

    RAYFLOW_CPU_GPU Spectrum Power() const final { return 4 * Pi * I; }

private:
    const Point3 pLight;
    // amount of power per unit solid angle
    const Spectrum I;
};

class SpotLight : public Light {
public:
    SpotLight(const Transform* wtl, const Transform* ltw, const Spectrum& I,
              Float maxTheta, Float fallOff) :
              Light(wtl, ltw, LightType::DELTA_POSITION),
              pLight((*ltw)(Point3(0, 0, 0))),
              I(I), cosMaxTheta(std::cos(maxTheta)),
              cosFallOff(std::cos(fallOff)) {

    }

    RAYFLOW_CPU_GPU Float FallOff(const Vector3& w) const {
        Float cosTheta = w.z;
        if (cosTheta < cosMaxTheta) { return 0; }
        if (cosTheta >= cosFallOff) { return 1; }

        Float delta = (cosTheta - cosMaxTheta) / (cosFallOff - cosMaxTheta);
        
        return delta * delta * delta * delta;
    }

    RAYFLOW_CPU_GPU Spectrum Le(const Intersection& ref, const Intersection& lightIsect = {}) const final;

    RAYFLOW_CPU_GPU rstd::optional<LightLiSample> SampleLi(const Intersection &ref, const Point2& sample) const final;

    RAYFLOW_CPU_GPU rstd::optional<LightLeSample> SampleLe(const Point2& posSample, const Point2& dirSample) const final;

    RAYFLOW_CPU_GPU Float PdfLi(const Intersection& ref, const Vector3& wi) const final;

    RAYFLOW_CPU_GPU void PdfLe(LightLeSample& ers) const final;

    RAYFLOW_CPU_GPU Spectrum Power() const final {
        return 2 * Pi * I * (1 - 0.5 * (cosFallOff + cosMaxTheta));
    }

private:
    const Point3 pLight;
    // amount of power per unit solid angle
    const Spectrum I;
    const Float cosMaxTheta;
    const Float cosFallOff;
};

class AreaLight : public Light {
public:
    AreaLight(const Transform* wtl, const Transform* ltw, const Shape* shape, const Spectrum& L) :
        Light(wtl, ltw, LightType::AREA),
        shape(shape),
        L(L),
        area(shape->Area()) {

    }

    RAYFLOW_CPU_GPU Spectrum Le(const Intersection& ref, const Intersection& lightIsect = {}) const final;

    RAYFLOW_CPU_GPU rstd::optional<LightLiSample> SampleLi(const Intersection &ref, const Point2& sample) const final;

    RAYFLOW_CPU_GPU rstd::optional<LightLeSample> SampleLe(const Point2& posSample, const Point2& dirSample) const final;

    RAYFLOW_CPU_GPU Float PdfLi(const Intersection& ref, const Vector3& wi) const final;

    RAYFLOW_CPU_GPU void PdfLe(LightLeSample& ers) const final;

    RAYFLOW_CPU_GPU Spectrum Power() const final {
        return Pi * area * L;
    }

private:
    const Shape* shape;
    const Spectrum L;
    const Float area;
    const bool twoside = false;
};


}