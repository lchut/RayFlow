#include <RayFlow/Core/sampler.h>
#include <RayFlow/Render/cameras.h>

namespace rayflow {

// Perspective Camera

CameraRaySample PerspectiveCamera::GenerateRay(const Point2& pFilm, const Point2& sample) const {
    Point3 pCamera = mRasterToCamera_(Point3(pFilm.x, pFilm.y, 0));
    Ray ray(Point3(0, 0, 0), Normalize(pCamera - Point3(0, 0, 0)));
    Float cosTheta = Dot(ray.d, Vector3(0, 0, 1));
    Float posPdf = 1;
    Float dirPdf = 1;

    if(mLenRadius_ > 0) {
        Point2 sp = UniformSampleDisk(sample);
        Point3 pLen = Point3(sp.x, sp.y, 0) * mLenRadius_;
        
        Point3 pFocus = ray(mFocalDistance_ / cosTheta);

        ray.o = Point3(pLen.x, pLen.y, 0);
        ray.d = Normalize(pFocus - ray.o);

        posPdf = 1 / (Pi * mLenRadius_ * mLenRadius_);
    }
    dirPdf = 1 / (A * cosTheta * cosTheta * cosTheta);
    ray = (*mCameraToWorld_)(ray);

    return { ray, posPdf, dirPdf, Spectrum(1.0f), (*mCameraToWorld_)(Normal3(0, 0, 1)) };
}

Spectrum PerspectiveCamera::We(const Ray& ray, Point2f* pRaster) const  {
    Vector3 lenNormal = (*mCameraToWorld_)(Vector3(0, 0, 1));
    Float cosTheta = Dot(lenNormal, ray.d);
    
    if (cosTheta <= 0) { return 0; }

    Point3 pFocus = ray(((mLenRadius_ == 0) ? 1 : mFocalDistance_) / cosTheta);
    Point3 pRaster3d = Inverse(mRasterToCamera_)(Inverse(*mCameraToWorld_)(pFocus));

    if (pRaster3d.x < mSampleBounds_.pMin.x || pRaster3d.y < mSampleBounds_.pMin.y ||
        pRaster3d.x >= mSampleBounds_.pMax.x || pRaster3d.y >= mSampleBounds_.pMax.y) {
        return 0;
    }

    if (pRaster) {
        pRaster->x = pRaster3d.x;
        pRaster->y = pRaster3d.y;
    }

    Float ALen = mLenRadius_ == 0 ? 1 : Pi * mLenRadius_ * mLenRadius_;

    return 1 / (ALen * A * cosTheta * cosTheta * cosTheta * cosTheta);
}

rstd::optional<CameraWeSample> PerspectiveCamera::SampleWe(const Point2& pFilm, const Point2& sample) const {
    CameraRaySample rs = GenerateRay(pFilm, sample);
    return CameraWeSample{ rs, We(rs.ray, nullptr) };
}

rstd::optional<CameraWiSample> PerspectiveCamera::SampleWi(const Intersection& ref, const Point2& sample) const {
    Vector3 lenNormal = (*mCameraToWorld_)(Vector3(0, 0, 1));
    Point3 pLenLocal = Point3(0, 0, 0);
    Float posPdf = 0;

    if (mLenRadius_ > 0) {
        Point2 sp = UniformSampleDisk(sample);
        pLenLocal = Point3(sp.x, sp.y, 0) * mLenRadius_;
        posPdf = 1 / (Pi * mLenRadius_ * mLenRadius_);
    }
    else {
        posPdf = 1;
    }

    Point3 pLen = (*mCameraToWorld_)(pLenLocal);

    if (DistanceSquare(pLen, ref.p) == 0) {
        return {};
    }

    Vector3 wi = Normalize(ref.p - pLen);
    Ray ray(pLen, wi);
    Point2f pRaster;
    Spectrum W = We(ray, &pRaster);

    if (W.IsBlack()) {
        return {};
    }

    Float dirPdf = mLenRadius_ > 0 ? posPdf * DistanceSquare(pLen, ref.p) / AbsDot(wi, lenNormal) : 1;

    return CameraWiSample{ W, wi, posPdf, dirPdf, pRaster, ref, Intersection(-wi, pLen, Normal3(lenNormal)) };
}

void PerspectiveCamera::PdfWe(const Ray& ray, Float* posPdf, Float* dirPdf) const {
    Vector3 lenNormal = (*mCameraToWorld_)(Vector3(0, 0, 1));
    Float cosTheta = Dot(lenNormal, ray.d);
    
    if (cosTheta <= 0) {
        *posPdf = 0;
        *dirPdf = 0;
        return;
    }

    Point3 pFocus = ray(((mLenRadius_ == 0) ? 1 : mFocalDistance_) / cosTheta);
    Point3 pRaster = Inverse(mRasterToCamera_)(Inverse(*mCameraToWorld_)(pFocus));

    if (pRaster.x < mSampleBounds_.pMin.x || pRaster.y < mSampleBounds_.pMin.y ||
        pRaster.x >= mSampleBounds_.pMax.x || pRaster.y >= mSampleBounds_.pMax.y) {
        *posPdf = 0;
        *dirPdf = 0;
    }

    Float ALen = mLenRadius_ == 0 ? 1 : (Pi * mLenRadius_ * mLenRadius_);
    *posPdf = 1 / ALen;
    *dirPdf = 1 / (A * cosTheta * cosTheta * cosTheta);
}

}