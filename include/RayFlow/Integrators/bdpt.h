#pragma once
#include <RayFlow/Core/integrator.h>

namespace rayflow {

inline Float CorrectNonsymmetryCauseByShadingNormal(const SurfaceIntersection& si, const Vector3& wi, TransportMode mode) {
    if (mode == TransportMode::Radiance) {
        return 1;
    }
    else {
        const Vector3& wo = si.wo;
        const Normal3& ng = si.ng;
        const Normal3& ns = si.ns;
        Float numerator = AbsDot(wo, ns) * AbsDot(wi, ng);
        Float denominator = AbsDot(wo, ng) * AbsDot(wi, ns);

        return denominator != 0 ? numerator / denominator : 0;
    }
}

class Vertex {
public:
    enum class VertexType {
        Camera,
        Surface,
        Light,
    };

    Vertex() = default;

    Vertex(const Camera* camera, const CameraWeSample& sample) : 
        camera(camera),
        si(sample.crs.ray.o, sample.crs.lenNormal),
        alpha(Spectrum(1.0f)),
        type(VertexType::Camera) {

    }

    Vertex(const Camera* camera, const CameraWiSample& sample) :
        camera(camera),
        si(sample.pLen),
        alpha(sample.W),
        pdfFwd(sample.pdfPos),
        type(VertexType::Camera) {

    }

    Vertex(const Light* light, const LightLeSample& sample, Float pdf) :
        emitter(light),
        si(sample.pLight),
        pdfFwd(pdf),
        type(VertexType::Light) {

    }

    Vertex(const Light* light, const LightLiSample& sample, Float pdf) :
        emitter(light),
        si(sample.pLight),
        pdfFwd(pdf),
        type(VertexType::Light) {

    }

    Vertex(const ShapeIntersection& si, const Spectrum& alpha, const Vertex& prev, Float pdfDir) :
        si(si.isect),
        alpha(alpha),
        type(VertexType::Surface) {
        pdfFwd = prev.ConvertPdf(*this, pdfDir);
    }

    const Point3& p() const { return si.p; }

    const Normal3& ns() const { return si.ns; }

    const Normal3& ng() const { return si.ng; }

    bool IsLight() const {
        return (type == VertexType::Light) || 
            (type == VertexType::Surface && si.GetAreaLight() != nullptr);
    }

    bool IsDeltaLight() const {
        return ((int)emitter->type & (int)LightType::DELTA_POSITION) ||
        ((int)emitter->type & (int)LightType::DELTA_DIRECTION);
    }

    bool IsOnSurface() const {
        return type == VertexType::Surface;
    }

    bool IsConnectible() const {
        switch (type) {
            case VertexType::Light:
                return ((int)emitter->type & (int)LightType::DELTA_DIRECTION) == 0;
            case VertexType::Camera:
                return true;
            case VertexType::Surface:
                return true;
        }
        return false;
    }
    
    // convert direction pdf to position pdf
    Float ConvertPdf(const Vertex& next, Float pdfW) const {
        Vector3 w = next.p() - p();
        Float dist2 = LengthSquare(w);

        if (dist2 == 0) {
            return 0;
        }

        w /= ::sqrt(dist2);

        return pdfW * AbsDot(next.ng(), w) / dist2;
    }

    Spectrum Le(const Scene& scene, const Vertex& next) const {
        if (!IsLight()) {
            return Spectrum(0.0f);
        }

        if (DistanceSquare(p(), next.p()) == 0) {
            return Spectrum(0.f);
        }

        const AreaLight* areaLight = si.GetAreaLight();

        return areaLight->Le(next.si, si);
    }

    Float PdfPnext(const Scene& scene, const Vertex* prev, const Vertex& next) const {
        if (type == VertexType::Light) {
            return PdfPLightNext(scene, next);
        }
        
        if (DistanceSquare(next.p(), p()) == 0) {
            return 0;
        }
        Vector3 wo = Normalize(next.p() - p());
        Vector3 wi;
        if (prev) {
            if (DistanceSquare(prev->p(), p()) == 0) {
                return 0;
            }
            wi = Normalize(prev->p() - p());
        }

        Float pdfDir = 0;
        Float pdfPos = 0;
        switch (type) {
            case VertexType::Camera: {
                camera->PdfWe(Ray(p(), wo), &pdfPos, &pdfDir);
                break;
            }
            case VertexType::Surface: {
                pdfDir = bsdf.Pdf(wi, wo);
                break;
            }
        }

        return ConvertPdf(next, pdfDir);
    }

    Float PdfPLightNext(const Scene& scene, const Vertex& next) const {
        if (DistanceSquare(p(), next.p()) == 0) {
            return 0;
        }

        Vector3 w = Normalize(next.p() - p());
        const Light* light = (type == VertexType::Light) ? emitter : si.GetAreaLight();
        LightLeSample emitSample(Ray(p(), w));
        emitSample.pLight = si;
        light->PdfLe(emitSample);
        Float pdf = emitSample.pdfDir / DistanceSquare(p(), next.p());

        if (IsOnSurface()) {
            pdf *= AbsDot(next.ng(), w);
        }

        return pdf;
    }

    Float PdfPlight(const Scene& scene, const Vertex& next) const {
        if (DistanceSquare(p(), next.p()) == 0) {
            return 0;
        }
        
        Vector3 w = Normalize(next.p() - p());
        const Light* light = (type == VertexType::Light) ? emitter : si.GetAreaLight();
        Float pdfSampleLight = scene.GetLightSampler()->Pdf(next.p(), light);
        LightLeSample emitSample(Ray(p(), w));
        emitSample.pLight = si;
        light->PdfLe(emitSample);

        return emitSample.pdfPos * pdfSampleLight;
    }

    Spectrum f(const Vertex& next, TransportMode mode = TransportMode::Radiance) const {
        if (DistanceSquare(p(), next.p()) == 0) {
            return Spectrum(0.f);
        }

        Vector3 wi = Normalize(next.p() - p());
        return bsdf.f(wi, si.wo, mode);
    }

    const BSDF& EvaluateBSDF(TransportMode mode) {
        bsdf = *(si.EvaluateBSDF(mode));
        return bsdf;
    }

    VertexType type;
    Spectrum alpha;
    Float pdfFwd = 0;
    Float pdfBwd = 0;
    SurfaceIntersection si;
    bool delta = false;
    const Camera* camera = nullptr;
    const Light* emitter = nullptr;
    BSDF bsdf;
};

template <typename T>
class BackUp {
public:
    BackUp() : mPtr_(nullptr) {

    }

    BackUp(T* ptr, T value = T()) :
        mPtr_(ptr) {
        if (mPtr_) {
            mBackUpData_ = *mPtr_;
            *mPtr_ = value;
        }
    }

    ~BackUp() {
        if (mPtr_) {
            *mPtr_ = mBackUpData_;
        }
    }

    BackUp(const BackUp &) = delete;
    BackUp& operator=(const BackUp &) = delete;
    BackUp& operator=(BackUp &&other) {
        if (mPtr_) {
            *mPtr_ = mBackUpData_;
        }
        mPtr_ = other.mPtr_;
        mBackUpData_ = other.mBackUpData_;
        other.mPtr_ = nullptr;
        return *this;
    }
private:
    T mBackUpData_;
    T* mPtr_;
};

inline bool Visable(const Scene& scene, const SurfaceIntersection& p0, const SurfaceIntersection& p1) {
    Float dist2 = DistanceSquare(p0.p, p1.p);

    if (dist2 == 0) {
        return false;
    }

    Ray ray = p0.SpawnRayTo(p1.p);
    if (scene.Intersect(ray, ::sqrt(dist2) - ShadowEpsilon)) {
        return false;
    }

    return true;
}

inline Float G(const Scene& scene, const SurfaceIntersection& p0, const SurfaceIntersection& p1) {
    Float dist2 = DistanceSquare(p0.p, p1.p);

    if (dist2 == 0) {
        return 0;
    }

    Ray ray = p0.SpawnRayTo(p1.p);
    if (scene.Intersect(ray, ::sqrt(dist2) - ShadowEpsilon)) {
        return 0;
    }

    return AbsDot(p0.ns, ray.d) * AbsDot(p1.ns, ray.d) / dist2;
}

class BDPTIntegrator : public MonteCarloIntegrator {
public:
    BDPTIntegrator(Camera* camera, Sampler* sampler, 
                   int maxDepth = 8, 
                   bool strictNormal = false) :
        MonteCarloIntegrator(camera, sampler, maxDepth, -1, strictNormal) {

    }

    virtual void Render(const Scene& scene);

private:
    using Path = std::vector<Vertex>;
    
    int ConstructCameraPath(const Scene& scene, Sampler& sampler, const Point2& pFilm, Path& path) const;

    int ConstructLightPath(const Scene& scene, Sampler& sampler, Path& path) const;

    Spectrum ConnectPath(const Scene& scene, Sampler& sampler, Path& cameraPath, Path& lightPath, int t, int s, Point2* pFilm) const;

    int RandomWalk(const Scene& scene, Sampler& sampler, Ray ray, Spectrum alpha, Float pdfDir, Path& path, TransportMode mode) const;

    Float MIS(const Scene& scene, Path& cameraPath, Path& lightPath, Vertex& vSample, int s, int t) const;
};


}
