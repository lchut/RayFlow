#include <RayFlow/Render/lights.h>
#include <RayFlow/Core/sampler.h>

namespace rayflow {

Spectrum PointLight::Le(const Intersection& ref, const Intersection& lightIsect) const {
    Float dist2 = DistanceSquare(pLight, ref.p);
    
    if (dist2 == 0) {
        return 0;
    }

    return I / dist2;
}

rstd::optional<LightLiSample> PointLight::SampleLi(const Intersection &ref, const Point2& sample) const {
    Float dist2 = DistanceSquare(ref.p, pLight);

    if (dist2 == 0) {
        return {};
    }

    Vector3 wi = (pLight - ref.p) / dist2;
    Spectrum L = I / dist2;
    Float pdfDir = 1;
    Float pdfPos = 1;

    return LightLiSample(L, wi, pdfPos, pdfDir, Intersection(pLight, Normal3(-wi)));
}

rstd::optional<LightLeSample> PointLight::SampleLe(const Point2& posSample, const Point2& dirSample) const {
    Vector3 wo = (*mLocalToWorld_)(UniformSampleSphere(dirSample));
    Float posPdf = 1;
    Float dirPdf = UniformSampleSpherePdf();
    
    return LightLeSample(Ray(pLight, wo), Intersection(pLight, Normal3(wo)), I, posPdf, dirPdf);
}

Float PointLight::PdfLi(const Intersection& ref, const Vector3& wi) const {
    return 0;
}

void PointLight::PdfLe(LightLeSample& ers) const {
    ers.pdfPos = 0;
    ers.pdfDir = UniformSampleSpherePdf();
}

Spectrum SpotLight::Le(const Intersection& ref, const Intersection& lightIsect) const {
    Float dist2 = DistanceSquare(pLight, ref.p);
    
    if (dist2 == 0) {
        return 0;
    }

    Vector3 w = (*mWorldToLocal_)((ref.p - pLight) / dist2);
    Float coeff = FallOff(w);
    
    return coeff * I / dist2;
}

rstd::optional<LightLiSample> SpotLight::SampleLi(const Intersection &ref, const Point2& sample) const {
    Float dist2 = DistanceSquare(ref.p, pLight);

    if (dist2 == 0) {
        return {};
    }

    Vector3 wi = (pLight - ref.p) / dist2;
    Spectrum L = FallOff((*mWorldToLocal_)(-wi)) * I / dist2;
    Float pdfPos = 1;
    Float pdfDir = 1;

    return LightLiSample(L, wi, pdfPos, pdfDir, Intersection(pLight, Normal3(-wi)));
}

rstd::optional<LightLeSample> SpotLight::SampleLe(const Point2& posSample, const Point2& dirSample) const {
    Ray ray(pLight, (*mLocalToWorld_)(UniformSampleCone(dirSample, cosMaxTheta)));
    Float posPdf = 1;
    Float dirPdf = UniformSampleConePdf(cosMaxTheta);

    return LightLeSample(ray, Intersection(pLight, Normal3(ray.d)), I * FallOff(ray.d),  posPdf, dirPdf);
}

Float SpotLight::PdfLi(const Intersection& ref, const Vector3& wi) const {
    return 0;
}

void SpotLight::PdfLe(LightLeSample& ers) const {
    ers.pdfPos = 0;
    ers.pdfDir = Dot(ers.ray.d, (*mLocalToWorld_)(Vector3(0, 0, 1))) ? UniformSampleConePdf(cosMaxTheta) : 0;
}

Spectrum AreaLight::Le(const Intersection& ref, const Intersection& lightIsect) const {
    Vector3 w = Normalize(ref.p - lightIsect.p);
    return Dot(w, lightIsect.ng) > 0 ? L : 0;
}

rstd::optional<LightLiSample> AreaLight::SampleLi(const Intersection &ref, const Point2& sample) const {
    auto ss = shape->Sample(ref, sample);
    
    if (!ss) {
        return {};
    }

    const auto& si = ss->isect;
    Vector3 wi = Normalize(si.p - ref.p);

    return LightLiSample(Le(ref, si), wi, ss->pdfPos, ss->pdfDir, si);
}

rstd::optional<LightLeSample> AreaLight::SampleLe(const Point2& posSample, const Point2& dirSample) const {
    auto sp = shape->Sample(posSample);
    const auto& si = sp->isect;
    Float pdfPos = sp->pdfPos;
    
    Vector3 sw = CosineWeightedSampleHemiSphere(dirSample);
    Frame lcs = Frame::FromZ(si.ng);
    Ray ray(si.p, lcs.FromLocal(sw));
    Float pdfDir = CosineWeightedSampleHemiSpherePdf(sw.z);
    Spectrum Lemit = Dot(ray.d, si.ng) > 0 ? L : Spectrum(0.f);
    
    return LightLeSample(ray, si, Lemit, pdfPos, pdfDir);
}

Float AreaLight::PdfLi(const Intersection& ref, const Vector3& wi) const {
    return shape->Pdf(ref, wi);
}

void AreaLight::PdfLe(LightLeSample& ers) const {
    ers.pdfPos = shape->Pdf(ers.pLight);
    ers.pdfDir = CosineWeightedSampleHemiSpherePdf(Dot(ers.ray.d, ers.pLight.ng));
}

}