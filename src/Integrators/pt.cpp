#include <RayFlow/Integrators/pt.h>

namespace rayflow {

Spectrum PathTracerIntegrator::Li(const Ray& r, const Scene& scene, Sampler& sampler) const {
    Spectrum L(0.f);
    Spectrum beta(1.f);
    Ray ray = r;
    bool specularBounce = false;

    for (int bounce = 0; bounce < mMaxDepth_; ++bounce) {
        auto foundIntersection = scene.Intersect(ray);

        if (!foundIntersection) {
            break;
        }

        auto& si = foundIntersection->isect;

        if (bounce == 0 || specularBounce) {
            L += beta * si.Le(Intersection(ray.o));
        }

        Spectrum Ld(0.f);
        auto lightSample = scene.SampleLight(si.p, sampler.Get1D());
        auto bsdf = si.EvaluateBSDF();

        // sample direct lighting
        auto lightLiSample = lightSample.light->SampleLi(si, sampler.Get2D());

        if (lightLiSample && !lightLiSample->L.IsBlack()) {
            VisibilityTester vis(si, lightLiSample->pLight);

            if (vis.Visiable(scene)) {
                Spectrum f = bsdf->f(lightLiSample->wi, si.wo) * AbsDot(si.ns, lightLiSample->wi);
                Float pdfScattering = bsdf->Pdf(lightLiSample->wi, si.wo);
                Float pdfLightDir = lightLiSample->pdfDir;

                if (!f.IsBlack() && pdfLightDir != 0) {
                    if (IsDeltaLight(lightSample.light->type)) {
                        Ld += f * lightLiSample->L / pdfLightDir;
                    }
                    else {
                        Float weight = PowerHeuristic(1, pdfLightDir, 1, pdfScattering);
                        Ld += lightLiSample->L * f * weight / pdfLightDir;
                    }
                }
            }
        }
        // sample bsdf
        if (!IsDeltaLight(lightSample.light->type)) {
            auto bsdfSample = bsdf->SampleF(si.wo, &sampler);

            if (!HasSpecularComponent(bsdfSample->type)) {
                Spectrum f = bsdfSample->f * AbsDot(bsdfSample->wi, si.ns);
                Float pdfScattering = bsdfSample->pdf;
                bool sampledSpecular = ((int)bsdfSample->type & (int)BXDFType::SPECULAR) != 0;

                if (!f.IsBlack() && pdfScattering != 0) {
                    Float weight = 1;
                    Float pdfLightDir = 0;
                    if (!sampledSpecular) {
                        pdfLightDir = lightSample.light->PdfLi(si, bsdfSample->wi);
                        weight = PowerHeuristic(1, pdfScattering, 1, pdfLightDir);
                    }

                    if (pdfLightDir != 0) {
                        auto hitPoint = scene.Intersect(si.SpawnRay(bsdfSample->wi));

                        if (hitPoint && hitPoint->isect.GetAreaLight() == lightSample.light) {
                            Spectrum Le = lightSample.light->Le(si);
                            Ld += Le * f * weight / pdfScattering;
                        }
                    }
                }
            }
        }

        // add path contribution
        L += beta * Ld / lightSample.pdf;
        // sample next direction
        auto bsdfSample = bsdf->SampleF(si.wo, &sampler);
        specularBounce = ((int)bsdfSample->type & (int)BXDFType::SPECULAR) != 0;

        if (bsdfSample->f.IsBlack() || bsdfSample->pdf == 0) {
            break;
        }

        beta *= bsdfSample->f * AbsDot(si.ns, bsdfSample->wi) / bsdfSample->pdf;
        /*
        if (bounce > mRRDepth_) {
            Float q = std::max(Float(0.5), 1 - beta.Luminance());
            if (sampler.Get1D() < q) {
                break;
            }
            beta /= 1 - q;
        }*/
        ray = si.SpawnRay(bsdfSample->wi);
    }

    return L;
}

}