#pragma once
#include <RayFlow/Integrators/bdpt.h>

namespace rayflow {

void BDPTIntegrator::Render(const Scene& scene) {
    Preprocess(scene, *mSampler_);

    Film* film = mCamera_->mFilm_;
    Point2i resolution = film->resolution;
    AABB2i sampledBounds = mCamera_->mSampleBounds_;
    int filmTileWidth = 16;

    int tileXCount = ((resolution.x + filmTileWidth - 1) / filmTileWidth);
    int tileYCount = ((resolution.y + filmTileWidth - 1) / filmTileWidth);
    Point2i tileCount(tileXCount, tileYCount);

    Scheduler::Parallel2D(tileCount, 
        [&](Point2i tile) {
            ResetGMalloc();
    
            int seed = tile.y * tileCount.x + tile.x;
            Sampler* sampler = mSampler_->Clone(seed);
    
            int x0 = sampledBounds.pMin.x + tile.x * filmTileWidth;
            int x1 = std::min<int>(x0 + filmTileWidth, sampledBounds.pMax.x);
            int y0 = sampledBounds.pMin.y + tile.y * filmTileWidth;
            int y1 = std::min<int>(y0 + filmTileWidth, sampledBounds.pMax.y);
            FilmTile* filmTile = film->GetFilmTile(AABB2i(Point2i(x0, y0), Point2i(x1, y1)));

            Path cameraPath(mMaxDepth_ + 2);
            Path lightPath(mMaxDepth_ + 2);
            
            for (int y = y0; y < y1; ++y) {
                for (int x = x0; x < x1; ++x) {
                    Point2i pRaster(x, y);
                    size_t sampleCount = sampler->GetSampleCount();
                    sampler->StartPixel(pRaster);
                    //if (x == 305 && y == 487) {
                    //    std::cout << 1 << std::endl;
                    //}
                    //else {
                    //    continue;
                    //}
                    for (int i = 0; i < sampleCount; ++i) {
                        Point2 pFilm = Point2(pRaster) + sampler->Get2D();
                        Spectrum L(0.f);
                        int nCameraPathVertex = ConstructCameraPath(scene, *sampler,  pFilm, cameraPath);
                        int nLightPathVertex = ConstructLightPath(scene, *sampler, lightPath);
                        Float weisum = 0;
                        for (int t = 1; t <= nCameraPathVertex; ++t) {
                            for (int s = 0; s <= nLightPathVertex; ++s) {
                                int depth = s + t - 2;

                                if ((s == 1 && t == 1) || depth < 0 || depth > mMaxDepth_) {
                                    continue;
                                }

                                Point2f pFilmHit(-1, -1);
                                Spectrum Lpath = ConnectPath(scene, *sampler, cameraPath, lightPath, t, s, &pFilmHit);

                                if (t != 1) {
                                    L += Lpath;
                                }
                                else {
                                    film->AddSplat(pFilmHit, Lpath);
                                }
                            }

                        }

                        if (L.HasNaN()) {
                            L = Spectrum(0.f);
                        }
                        else if (L.HasINF()) {
                            L = Spectrum(0.f);
                        }
                        
                        filmTile->AddSample(pFilm, L);
                        
                        sampler->Advance();

                        //ResetGMalloc();
                    }
                }
            }
            film->MergeFilmTile(filmTile);
        }
    );

    film->Write(1.0f / mSampler_->GetSampleCount());
}

int BDPTIntegrator::ConstructCameraPath(const Scene& scene, Sampler& sampler, const Point2& pFilm, Path& path) const {
    auto cameraWeSample = mCamera_->SampleWe(pFilm, sampler.Get2D());
    path[0] = Vertex(mCamera_, *cameraWeSample);
    Spectrum alpha = Spectrum(1.0f);
    Float pdfDir = cameraWeSample->crs.pdfDir;
    return RandomWalk(scene, sampler, cameraWeSample->crs.ray, alpha, pdfDir, path, TransportMode::Radiance) + 1;
}

int BDPTIntegrator::ConstructLightPath(const Scene& scene, Sampler& sampler, Path& path) const {
    auto lightSample = scene.SampleLight(Point3(), sampler.Get1D());
    auto emitSample = lightSample.light->SampleLe(sampler.Get2D(), sampler.Get2D());
    if (!emitSample) {
        return 0;
    }
    const Ray& ray = emitSample->ray;
    Float pdfPos = emitSample->pdfPos;
    Float pdfDir = emitSample->pdfDir;
    path[0] = Vertex(lightSample.light, *emitSample, pdfPos * lightSample.pdf);
    Spectrum alpha = emitSample->L * AbsDot(emitSample->pLight.ng, ray.d) / 
            (lightSample.pdf * pdfPos * pdfDir);
    
    return RandomWalk(scene, sampler, ray, alpha, pdfDir, path, TransportMode::Importance) + 1;
}

int BDPTIntegrator::RandomWalk(const Scene& scene, Sampler& sampler, Ray ray, Spectrum alpha, Float pdfDir, Path& path, TransportMode mode) const {
    int bounce = 0;
    int count = 1;
    Float pdfFwd = pdfDir;
    Float pdfBwd = 0;

    for (; bounce < mMaxDepth_;) {
        if (alpha.IsBlack()) {
            break;
        }

        auto si = scene.Intersect(ray);

        if (!si) {
            break;
        }

        Vertex& prev = path[count - 1];
        path[count] = Vertex(*si, alpha, prev, pdfFwd);
        Vertex& current = path[count];

        ++bounce;
        ++count;

        auto& bsdf = current.EvaluateBSDF(mode);
        auto bsdfSample = bsdf.SampleF(si->isect.wo, &sampler, mode);
        
        if (bsdfSample->f.IsBlack() || bsdfSample->pdf == 0) {
            break;
        }

        pdfFwd = bsdfSample->pdf;
        pdfBwd = bsdf.Pdf(si->isect.wo, bsdfSample->wi);

        alpha *= bsdfSample->f * AbsDot(bsdfSample->wi, si->isect.ns) / pdfFwd;
        alpha *= CorrectNonsymmetryCauseByShadingNormal(si->isect, bsdfSample->wi, mode);

        if (HasSpecularComponent(bsdfSample->type)) {
            current.delta = true;
            pdfBwd = pdfFwd = 0;
        }
        
        prev.pdfBwd = current.ConvertPdf(prev, pdfBwd);
        ray = si->isect.SpawnRay(bsdfSample->wi);
    }

    return bounce;
}

Spectrum BDPTIntegrator::ConnectPath(const Scene& scene, Sampler& sampler, Path& cameraPath, Path& lightPath, int t, int s, Point2* pFilm) const {
    Spectrum L(0.f);
    Vertex vSample;

    if (t == 1) {
        Vertex& camVert = cameraPath[0];
        Vertex& ligVert = lightPath[s - 1];
        if (ligVert.IsConnectible()) {
            const auto& camera = *camVert.camera;
            auto cameraWiSample = camera.SampleWi(ligVert.si, sampler.Get2D());

            if (cameraWiSample && cameraWiSample->pdfDir > 0) {
                vSample = Vertex(camVert.camera, *cameraWiSample);
                const Spectrum& importance = cameraWiSample->W;
                const Normal3& lenNormal = cameraWiSample->pLen.ng;
                const Vector3& wi = cameraWiSample->wi;
                *pFilm = cameraWiSample->pRaster;
                Float pdfPos = cameraWiSample->pdfPos;

                Float invDist2 = 1.0 / DistanceSquare(cameraWiSample->pLen.p, ligVert.p());
                L = ligVert.alpha * ligVert.f(vSample, TransportMode::Importance) *
                    (importance * AbsDot(lenNormal, -wi) * AbsDot(ligVert.ns(), wi) * invDist2) / pdfPos;

                if (!L.IsBlack() && !Visable(scene, ligVert.si, vSample.si)) {
                    L = Spectrum(0.f);
                }
            }
        }
    }
    else if (s == 0) {
        const Vertex& camVert = cameraPath[t - 1];
        const Vertex& camVertPrev = cameraPath[t - 2];
        if (camVert.IsLight()) {
            L = camVert.Le(scene, camVertPrev) * camVert.alpha;
        }
    }
    else if (s == 1) {
        const Vertex& camVert = cameraPath[t - 1];
        SampledLight lightSample = scene.SampleLight(camVert.p(), sampler.Get1D());
        auto lightLiSample = lightSample.light->SampleLi(camVert.si, sampler.Get2D());

        if (lightLiSample) {
            vSample = Vertex(lightSample.light, *lightLiSample, lightLiSample->pdfPos * lightSample.pdf);
            Float pdfSampleLight = lightSample.pdf;
            const Spectrum& Le = lightLiSample->L;
            const Normal3& lightNormal = lightLiSample->pLight.ng;
            const Vector3& wi = lightLiSample->wi;
            Float pdfPos = lightLiSample->pdfPos;
            Float pdfDir = lightLiSample->pdfDir;
            vSample.pdfFwd = pdfPos * pdfSampleLight;

            if (pdfPos > 0 && !Le.IsBlack()) {
                Float invDist2 = 1.0 / DistanceSquare(lightLiSample->pLight.p, camVert.p());
                L = camVert.alpha * camVert.f(vSample, TransportMode::Radiance) * 
                    (Le * invDist2 * AbsDot(wi, lightNormal) * AbsDot(wi, camVert.ns())) / (pdfSampleLight * pdfPos);

                if (!L.IsBlack() && !Visable(scene, camVert.si, vSample.si)) {
                    L = Spectrum(0.f);
                }
            }
            
        }
    }
    else {
        const Vertex& camVert = cameraPath[t - 1];
        const Vertex& ligVert = lightPath[s - 1];
        if (camVert.IsConnectible() && ligVert.IsConnectible()) {
            L = camVert.alpha * camVert.f(ligVert, TransportMode::Radiance) * 
                ligVert.f(camVert, TransportMode::Importance) * ligVert.alpha;

            if (!L.IsBlack()) {
                L *= G(scene, camVert.si, ligVert.si);
            }
        }
    }

    if (L.IsBlack()) {
        return L;
    }
    
    Float weight = MIS(scene, cameraPath, lightPath, vSample, s, t);
    return weight * L;
}

Float BDPTIntegrator::MIS(const Scene& scene, Path& cameraPath, Path& lightPath, Vertex& vSample, int s, int t) const {
    if (s + t == 2) {
        return 1;
    }

    Vertex* camVert = t > 0 ? &cameraPath[t - 1] : nullptr;
    Vertex* camVertPrev = t > 1 ? &cameraPath[t - 2] : nullptr;
    Vertex* ligVert = s > 0 ? &lightPath[s - 1] : nullptr;
    Vertex* ligVertPrev = s > 1 ? &lightPath[s - 2] : nullptr;

    BackUp<Vertex> endPointBackUp;
    if (t == 1) {
        endPointBackUp = BackUp<Vertex>(camVert, vSample);
    }
    else if (s == 1) {
        endPointBackUp = BackUp<Vertex>(ligVert, vSample);
    }

    BackUp<Float> camVertBackUp, camVertPrevBackUp;
    if (camVert) {
        camVertBackUp = BackUp<Float>(&camVert->pdfBwd,
            s > 0 ? ligVert->PdfPnext(scene, ligVertPrev, *camVert) :
                    camVert->PdfPlight(scene, *camVertPrev));
    }
    if (camVertPrev) {
        camVertPrevBackUp = BackUp<Float>(&camVertPrev->pdfBwd,
            s > 0 ? camVert->PdfPnext(scene, ligVert, *camVertPrev) :
                    camVert->PdfPLightNext(scene, *camVertPrev));
    }

    BackUp<Float> ligVertBackUp, ligVertPrevBackUp;
    if (ligVert) {
        ligVertBackUp = BackUp<Float>(&ligVert->pdfBwd, camVert->PdfPnext(scene, camVertPrev, *ligVert));
    }
    if (ligVertPrev) {
        ligVertPrevBackUp = BackUp<Float>(&ligVertPrev->pdfBwd, ligVert->PdfPnext(scene, camVert, *ligVertPrev));
    }

    Float sumRi = 0;
    Float ri = 1;

    for (int i = t - 1; i > 0; --i) {
        Float pdfFwd = cameraPath[i].pdfFwd;
        Float pdfBwd = cameraPath[i].pdfBwd;
        Float tmp = 1;

        ri *= (pdfBwd > 0 ? pdfBwd : 1) / 
             (pdfFwd > 0 ? pdfFwd : 1);

        if (!cameraPath[i].delta && !cameraPath[i - 1].delta) {
            sumRi += ri;
        }
    }

    ri = 1;

    for (int i = s - 1; i >= 0; --i) {
        Float pdfFwd = lightPath[i].pdfFwd;
        Float pdfBwd = lightPath[i].pdfBwd;

        ri *= (pdfBwd > 0 ? pdfBwd : 1) / 
              (pdfFwd > 0 ? pdfFwd : 1);

        bool deltaPrev = i > 0 ? lightPath[i - 1].delta :
                                 lightPath[i].IsDeltaLight();
        if (!lightPath[i].delta && deltaPrev) {
            sumRi += ri;
        }
    }

    return 1 / (1 + sumRi);
}

}