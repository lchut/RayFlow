#pragma once

#include <RayFlow/Core/ray.h>
#include <RayFlow/Core/spectrum.h>
#include <RayFlow/Core/intersection.h>
#include <RayFlow/Util/vecmath.h>
#include <RayFlow/Util/transform.h>
#include <RayFlow/Std/optional.h>
#include <RayFlow/Render/film.h>


namespace rayflow {

struct CameraRaySample {
    Ray ray;
    Float pdfPos;
    Float pdfDir;
    Spectrum weight;
    Normal3 lenNormal;
};

struct CameraWeSample {
    CameraRaySample crs;
    Spectrum We;
};

struct CameraWiSample {
    Spectrum W;
    Vector3 wi;
    Float pdfPos;
    Float pdfDir;
    Point2f pRaster;
    Intersection pRef;
    Intersection pLen;
};

class Camera {
public:
    Camera(const Transform* ctw, const AABB2i& sampleBounds, Film* film) : 
        mCameraToWorld_(ctw), 
        mSampleBounds_(sampleBounds),
        mFilm_(film) {} 

    RAYFLOW_CPU_GPU virtual CameraRaySample GenerateRay(const Point2& pFilm, const Point2& sample) const = 0;
    
    RAYFLOW_CPU_GPU virtual Spectrum We(const Ray& ray, Point2f* pRaster) const = 0;

    RAYFLOW_CPU_GPU virtual rstd::optional<CameraWeSample> SampleWe(const Point2& pFilm, const Point2& sample) const = 0;

    RAYFLOW_CPU_GPU virtual rstd::optional<CameraWiSample> SampleWi(const Intersection& ref, const Point2& sample) const = 0;

    RAYFLOW_CPU_GPU virtual void PdfWe(const Ray& ray, Float* posPdf, Float* dirPdf) const = 0;
    
    const Transform* mCameraToWorld_;
    AABB2i mSampleBounds_;
    Film* mFilm_;
};


}

