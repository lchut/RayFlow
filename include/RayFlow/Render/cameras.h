#pragma once

#include <RayFlow/Core/camera.h>

namespace rayflow {

class ProjectiveCamera : public Camera {
public:
    ProjectiveCamera(const Transform* ctw, const Transform& cameraToScreen, const AABB2i& sampleBounds,
                    const Point2i& resolution, const AABB2& screenWindow,
                    Float lenRadius, Float focalDistance,
                    Film* film) :
                    Camera(ctw, sampleBounds, film),
                    mCameraToScreen_(cameraToScreen),
                    mLenRadius_(lenRadius),
                    mFocalDistance_(focalDistance) {
        mScreenToRaster_ = Scale(Vector3(resolution.x, resolution.y, 1)) *
                           Scale(Vector3(1 / (screenWindow.pMax.x - screenWindow.pMin.x),
                                         1 / (screenWindow.pMin.y - screenWindow.pMax.y), 1)) *
                           Translate(Vector3(screenWindow.pMax.x, screenWindow.pMin.y, 0));

        mRasterToScreen_ = Inverse(mScreenToRaster_);

        mRasterToCamera_ = Inverse(mCameraToScreen_) * mRasterToScreen_;
    }
protected:
    Transform mCameraToScreen_;
    Transform mRasterToCamera_;
    Transform mRasterToScreen_;
    Transform mScreenToRaster_;
    Float mLenRadius_;
    Float mFocalDistance_;
};

class PerspectiveCamera : public ProjectiveCamera {
public:
    PerspectiveCamera(const Transform* ctw, const Point2i& resolution,
                      const AABB2i& sampleBounds,
                      const AABB2& screenWindow, Float lenRadius,
                      Float focalDistance, Float fov,
                      Film* film) :
                      ProjectiveCamera(ctw, Perspective(fov, 0.1f, 1000.0f), sampleBounds,
                      resolution, screenWindow, lenRadius, focalDistance, film) {
        Point3 pMin = mRasterToCamera_(Point3(0, 0, 0));
        Point3 pMax = mRasterToCamera_(Point3(resolution.x, resolution.y, 0));

        pMin /= pMin.z;
        pMax /= pMax.z;

        A = ::abs((pMax.x - pMin.x) * (pMax.y - pMin.y));
    }

    RAYFLOW_CPU_GPU CameraRaySample GenerateRay(const Point2& pFilm, const Point2& sample) const final;

    RAYFLOW_CPU_GPU Spectrum We(const Ray& ray, Point2f* pRaster) const final;

    RAYFLOW_CPU_GPU rstd::optional<CameraWeSample> SampleWe(const Point2& pFilm, const Point2& sample) const final;

    RAYFLOW_CPU_GPU rstd::optional<CameraWiSample> SampleWi(const Intersection& ref, const Point2& sample) const final;

    RAYFLOW_CPU_GPU void PdfWe(const Ray& ray, Float* posPdf, Float* dirPdf) const final;

private:
    Float A;
};

}