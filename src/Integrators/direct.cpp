#include <RayFlow/Integrators/direct.h>

namespace rayflow {

Spectrum DirectIntegrator::Li(const Ray& ray, const Scene& scene, Sampler& sampler) const {
    Spectrum L(0.f);
    auto si = scene.Intersect(ray);
    auto& hitPoint = si->isect;

    if (!si) {
        return Spectrum(0.f);
    } 

    rstd::optional<BSDF> bsdf = hitPoint.EvaluateBSDF(TransportMode::Radiance);
    // area light contribution
    L += hitPoint.Le(Intersection(ray.o));
    // direct lighting
    SampledLight sampleLight = scene.SampleLight(hitPoint.p, sampler.Get1D());
    const Light* light = sampleLight.light;
    Float lightPdf = sampleLight.pdf;
    rstd::optional<LightLiSample> lightLiSample = light->SampleLi(hitPoint, sampler.Get2D());

    const Vector3& wi = lightLiSample->wi;
    const Spectrum& Ld = lightLiSample->L;
    const Intersection& pLight = lightLiSample->pLight;

    // sample direct lighting
    if (lightLiSample && !lightLiSample->L.IsBlack()) {
        VisibilityTester vis(hitPoint, pLight);

        if (vis.Visiable(scene)) {
            Spectrum f = bsdf->f(wi, hitPoint.wo) * AbsDot(wi, hitPoint.ns);
            Float scatteringPdf = bsdf->Pdf(wi, hitPoint.wo);
            Float pw = lightLiSample->pdfDir;

            if (!f.IsBlack() && pw != 0) {
                if (IsDeltaLight(light->type)) {
                    L += f * Ld / pw;
                }
                else {
                    Float weight = PowerHeuristic(1, pw, 1, scatteringPdf);
                    L += f * Ld * weight / pw;
                }
            }
        }
    }

    // sample bsdf 
    if (!IsDeltaLight(light->type)) {
        rstd::optional<BXDFSample> bsdfSample = bsdf->SampleF(hitPoint.wo, &sampler);
        
        if (!HasSpecularComponent(bsdfSample->type)) {
            Spectrum f = bsdfSample->f * AbsDot(bsdfSample->wi, hitPoint.ns);
            Float pw = bsdfSample->pdf;

            if (!f.IsBlack() && pw > 0) {
                Float lightLiPdf = light->PdfLi(hitPoint, bsdfSample->wi);

                if (lightLiPdf == 0) {
                    return L / lightPdf;
                }

                Float weight = PowerHeuristic(1, pw, 1, lightLiPdf);
                rstd::optional<ShapeIntersection> lightIsect = scene.Intersect(hitPoint.SpawnRay(bsdfSample->wi));
                if (lightIsect && lightIsect->isect.GetAreaLight() == light) {
                    Spectrum Ld = lightIsect->isect.Le(hitPoint);
                    L += f * Ld * weight / pw;
                }
            }
        }
    }
    
    return L / lightPdf;
}   

}