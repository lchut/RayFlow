#pragma once

#include <RayFlow/Core/bxdf.h>
#include <RayFlow/Core/spectrum.h>
#include <RayFlow/Core/intersection.h>
#include <RayFlow/Core/sampler.h>
#include <RayFlow/Std/vector.h>
#include <RayFlow/Std/optional.h>

namespace rayflow
{

    RAYFLOW_CPU_GPU inline Vector3 Reflect(const Vector3 &wi, const Vector3 &n)
    {
        return -wi + 2 * Dot(wi, n) * n;
    }

    RAYFLOW_CPU_GPU inline bool Refract(const Vector3 &wi, const Vector3 &n, Float eta, Vector3 *wo)
    {
        Float cosThetaI = CosTheta(wi);
        Float sinThetaI = SinTheta(wi);
        Float determinant = 1 - eta * eta * sinThetaI * sinThetaI;
        if (determinant < 0)
        {
            return false;
        }
        Float cosThetaO = ::sqrt(determinant);
        *wo = -wi * eta + (eta * cosThetaI - cosThetaO) * n;
        return true;
    }

    RAYFLOW_CPU_GPU inline bool InSameHemiSphere(const Vector3 &w1, const Vector3 &w2)
    {
        return w1.z * w2.z > 0;
    }

    RAYFLOW_CPU_GPU inline bool InSameHemiSphere(const Vector3 &w, const Normal3 &n)
    {
        return w.z * n.z > 0;
    }

    RAYFLOW_CPU_GPU Float FresnelDielectricToDieletric(Float eta, Float cosThetaI);
    RAYFLOW_CPU_GPU Spectrum FresnelDielectricToConductor(const Spectrum &etaT, const Spectrum &etaK, Float cosThetaI);

    class BSDF
    {
    public:
        RAYFLOW_CPU_GPU BSDF() = default;
        RAYFLOW_CPU_GPU explicit BSDF(const Normal3 &n) : frame(Frame::FromZ(n))
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const;

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const;

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const;

        RAYFLOW_CPU_GPU void AddComponent(BXDF *bxdf)
        {
            mBXDFs_[nComponents++] = bxdf;
        }

    private:
        RAYFLOW_CPU_GPU Vector3 ToLocal(const Vector3 &v) const
        {
            return frame.ToLocal(v);
        }

        RAYFLOW_CPU_GPU Vector3 ToWorld(const Vector3 &v) const
        {
            return frame.FromLocal(v);
        }

        Frame frame;
        static constexpr int MaxBXDFComponents = 8;
        int nComponents = 0;
        BXDF *mBXDFs_[MaxBXDFComponents];
    };

    class Fresnel
    {
    public:
        RAYFLOW_CPU_GPU virtual ~Fresnel() = default;

        RAYFLOW_CPU_GPU virtual Spectrum Evalueate(Float cosThetaI) const = 0;
    };

    class ConductorFresnel : public Fresnel
    {
    public:
        ConductorFresnel(const Spectrum &etaI, const Spectrum &etaT, const Spectrum &etaK) : etaI(etaI),
                                                                                             etaT(etaT),
                                                                                             etaK(etaK)
        {
        }

        RAYFLOW_CPU_GPU Spectrum Evalueate(Float cosThetaI) const final
        {
            if (cosThetaI <= 0)
            {
                return 0;
            }
            return FresnelDielectricToConductor(etaT / etaI, etaK / etaI, ::abs(cosThetaI));
        }

        Spectrum etaI;
        Spectrum etaT;
        Spectrum etaK;
    };

    class DielectricFresnel : public Fresnel
    {
    public:
        DielectricFresnel(Float etaI, Float etaT) : etaI(etaI),
                                                    etaT(etaT)
        {
        }

        RAYFLOW_CPU_GPU Spectrum Evalueate(Float cosThetaI) const final
        {
            Float eta = 1;
            if (cosThetaI > 0)
            {
                eta = etaT / etaI;
            }
            else
            {
                eta = etaI / etaT;
                cosThetaI = -cosThetaI;
            }
            return FresnelDielectricToDieletric(eta, cosThetaI);
        }

        Float etaI;
        Float etaT;
    };

    class ScaleBXDF : public BXDF
    {
    public:
        ScaleBXDF(const BXDF *bxdf, const Spectrum &scale) : BXDF(bxdf->type),
                                                             bxdf(bxdf),
                                                             scale(scale)
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const final
        {
            rstd::optional<BXDFSample> result = bxdf->SampleF(wo, sampler, mode);
            if (!result)
            {
                return {};
            }

            result->f *= scale;

            return result;
        }

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const final
        {
            return scale * bxdf->f(wi, wo, mode);
        }

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const final
        {
            return bxdf->Pdf(wi, wo);
        }

    private:
        const Spectrum scale;
        const BXDF *bxdf;
    };

    class SpecularReflection : public BXDF
    {
    public:
        SpecularReflection(const Spectrum &R, Fresnel *fresnel = nullptr) : BXDF(BXDFType((int)BXDFType::REFLECTION | (int)BXDFType::SPECULAR)),
                                                                            R(R),
                                                                            fresnel(fresnel)
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const final
        {
            Vector3 wi = Reflect(wo, Vector3(0, 0, 1));
            Spectrum F = fresnel ? fresnel->Evalueate(CosTheta(wi)) : Spectrum(1.0f);

            return BXDFSample{F * R / AbsCosTheta(wi), wi, 1, type};
        }

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const final
        {
            return Spectrum(0.0f);
        }

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const final
        {
            return 0;
        }

    private:
        const Spectrum R;
        Fresnel *fresnel = nullptr;
    };

    class SpecularTransmission : public BXDF
    {
    public:
        SpecularTransmission(const Spectrum &T, Float etaA, Float etaB, DielectricFresnel *fresnel) : BXDF(BXDFType((int)BXDFType::TRANSMISSION | (int)BXDFType::SPECULAR)),
                                                                                                      T(T),
                                                                                                      etaA(etaA),
                                                                                                      etaB(etaB),
                                                                                                      fresnel(fresnel)
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const final
        {
            Float cosTheta = CosTheta(wo);
            Vector3 n = cosTheta > 0 ? Vector3(0, 0, 1) : Vector3(0, 0, -1);

            Float etaI = cosTheta > 0 ? etaB : etaA;
            Float etaT = cosTheta > 0 ? etaA : etaB;

            Vector3 wi;
            if (!Refract(wo, n, etaT / etaI, &wi))
            {
                return {};
            }

            wi = Normalize(wi);
            Float factor = 1;

            if (mode == TransportMode::Radiance)
            {
                factor = etaT / etaI;
            }

            return BXDFSample{factor * factor * (Spectrum(1.0f) - fresnel->Evalueate(CosTheta(wi))) * T / AbsCosTheta(wi), wi, 1, type};
        }

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const final
        {
            return Spectrum(0.0f);
        }

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const final
        {
            return 0;
        }

    private:
        const Spectrum T;
        const Float etaA;
        const Float etaB;
        const DielectricFresnel *fresnel;
    };

    class LambertianReflection : public BXDF
    {
    public:
        explicit LambertianReflection(const Spectrum &R) : BXDF(BXDFType((int)BXDFType::REFLECTION | (int)BXDFType::DIFFUSE)),
                                                           R(R)
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const final
        {
            Vector3 wi = CosineWeightedSampleHemiSphere(sampler->Get2D());

            if (wo.z < 0)
            {
                wi.z = -wi.z;
            }

            return BXDFSample{f(wi, wo, mode), wi, Pdf(wi, wo), type};
        }

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const final
        {
            return R * InvPi;
        }

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const final
        {
            return InSameHemiSphere(wi, wo) ? CosineWeightedSampleHemiSpherePdf(AbsCosTheta(wi)) : 0;
        }

    private:
        const Spectrum R;
    };

    class LambertianTransmission : public BXDF
    {
    public:
        LambertianTransmission(const Spectrum &T, Float etaA, Float etaB) : BXDF(BXDFType((int)BXDFType::TRANSMISSION | (int)BXDFType::DIFFUSE)),
                                                                            T(T),
                                                                            etaA(etaA),
                                                                            etaB(etaB)
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const final
        {
            Vector3 wi = CosineWeightedSampleHemiSphere(sampler->Get2D());

            if (wo.z > 0)
            {
                wi.z = -wi.z;
            }

            return BXDFSample{f(wi, wo, mode), wi, Pdf(wi, wo), type};
        }

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const final
        {
            return T * InvPi;
        }

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const final
        {
            return !InSameHemiSphere(wi, wo) ? CosineWeightedSampleHemiSpherePdf(AbsCosTheta(wi)) : 0;
        }

    private:
        const Spectrum T;
        const Float etaA;
        const Float etaB;
    };

    class Phong : public BXDF
    {
    public:
        Phong(const Spectrum &kd, const Spectrum &ks, Float exponent) : BXDF(BXDFType((int)BXDFType::REFLECTION | (int)BXDFType::GLOSSY | (int)BXDFType::DIFFUSE)),
                                                                        kd(kd),
                                                                        ks(ks),
                                                                        exponent(exponent)
        {
            Float diffuseLum = kd.Luminance();
            Float specularLum = ks.Luminance();
            specularSamplingWeight = specularLum / (diffuseLum + specularLum);
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const final
        {
            if (CosTheta(wo) < 0)
            {
                return {};
            }

            Point2 sample = sampler->Get2D();
            bool chooseSpecular = true;

            if (sample.x <= specularSamplingWeight)
            {
                sample.x /= specularSamplingWeight;
            }
            else
            {
                chooseSpecular = false;
                sample.x = (sample.x - specularSamplingWeight) / (1 - specularSamplingWeight);
            }

            Vector3 wi;
            Float pdf = 0;
            BXDFType sampleType;

            if (chooseSpecular)
            {
                sampleType = BXDFType::GLOSSY;
                Vector3 w = Reflect(wo, Vector3(0, 0, 1));

                Float sinTheta = ::sqrt(1 - std::pow(sample.y, 2 / (exponent + 1)));
                Float cosTheta = std::pow(sample.y, 1 / (exponent + 1));
                Float phi = 2 * Pi * sample.x;

                Vector3 localDir = Vector3(::cos(phi) * sinTheta, ::sin(phi) * sinTheta, cosTheta);
                wi = Frame::FromZ(w).FromLocal(localDir);
            }
            else
            {
                sampleType = BXDFType::DIFFUSE;
                wi = CosineWeightedSampleHemiSphere(sample);
            }

            pdf = Pdf(wi, wo);

            if (pdf == 0)
            {
                return {};
            }

            return BXDFSample{f(wi, wo, mode), wi, pdf, sampleType};
        }

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const final
        {
            if (CosTheta(wo) < 0)
            {
                return 0;
            }

            Spectrum result(0.f);
            Float alpha = Dot(wo, Reflect(wi, Vector3(0, 0, 1)));

            if (alpha > 0)
            {
                result += ((exponent + 2) * Inv2Pi * std::pow(alpha, exponent)) * ks;
            }

            result += kd * InvPi;

            return result;
        }

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const final
        {
            if (!InSameHemiSphere(wi, wo))
            {
                return 0;
            }

            Float result = 0;
            Float alpha = Dot(wo, Reflect(wi, Vector3(0, 0, 1)));

            if (alpha > 0)
            {
                result += specularSamplingWeight * std::pow(alpha, exponent) * (exponent + 1) * Inv2Pi;
            }

            result += (1 - specularSamplingWeight) * CosineWeightedSampleHemiSpherePdf(AbsCosTheta(wi));

            return result;
        }

    private:
        const Spectrum kd;
        const Spectrum ks;
        const Float exponent;
        Float specularSamplingWeight = 0;
    };

    class DielectricBXDF : public BXDF
    {
    public:
        DielectricBXDF(const Spectrum &ks, const Spectrum &kt, const DielectricFresnel *fresnel, Float etaA, Float etaB) : BXDF(BXDFType((int)BXDFType::SPECULAR | (int)BXDFType::REFLECTION | (int)(BXDFType::TRANSMISSION))),
                                                                                                                           ks(ks),
                                                                                                                           kt(kt),
                                                                                                                           fresnel(fresnel),
                                                                                                                           etaA(etaA),
                                                                                                                           etaB(etaB)
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const final
        {
            Point2 sample = sampler->Get2D();

            Float cosTheta = CosTheta(wo);
            Vector3 n = cosTheta > 0 ? Vector3(0, 0, 1) : Vector3(0, 0, -1);

            Float etaI = cosTheta > 0 ? etaB : etaA;
            Float etaT = cosTheta > 0 ? etaA : etaB;
            Float F = FresnelDielectricToDieletric(etaT / etaI, AbsCosTheta(wo));

            Vector3 wi;
            // total reflection
            if (!Refract(wo, n, etaT / etaI, &wi))
            {
                wi = Reflect(wo, n);
                return BXDFSample{ F * ks / AbsCosTheta(wi), wi, 1, BXDFType((int)BXDFType::SPECULAR | (int)BXDFType::REFLECTION) };
            }

            if (sample.x <= F)
            {
                wi = Reflect(wo, n);
                return BXDFSample{ F * ks / AbsCosTheta(wi), wi, F, BXDFType((int)BXDFType::SPECULAR | (int)BXDFType::REFLECTION) };
            }
            else
            {
                Float factor = 1;

                if (mode == TransportMode::Radiance)
                {
                    factor = etaT / etaI;
                }

                return BXDFSample{ factor * factor * (1 - F) * kt / AbsCosTheta(wi), wi, 1 - F, BXDFType((int)BXDFType::SPECULAR | (int)BXDFType::TRANSMISSION) };
            }
        }

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const final
        {
            return Spectrum(0.f);
        }

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const final
        {
            return 0;
        }

    private:
        const Spectrum ks;
        const Spectrum kt;
        const DielectricFresnel *fresnel;
        const Float etaA;
        const Float etaB;
    };

    class MicrofacetDistribution
    {
    public:
        RAYFLOW_CPU_GPU virtual ~MicrofacetDistribution() = default;

        RAYFLOW_CPU_GPU virtual Float D(const Vector3 &wh) const = 0;

        RAYFLOW_CPU_GPU virtual Vector3 SampleWh(const Point2 &u) const = 0;

        RAYFLOW_CPU_GPU virtual Float Lambda(const Vector3 &w) const = 0;

        RAYFLOW_CPU_GPU Float G(const Vector3 &wi, const Vector3 &wo) const
        {
            return 1 / (1 + Lambda(wi) + Lambda(wo));
        }

        RAYFLOW_CPU_GPU Float G1(const Vector3 &w) const
        {
            return 1 / (1 + Lambda(w));
        }

        RAYFLOW_CPU_GPU Float Pdf(const Vector3f &wh) const
        {
            return D(wh) * AbsCosTheta(wh);
        }
    };

    class TrowbridgeReitzDistribution : public MicrofacetDistribution
    {
    public:
        static inline Float RoughnessToAlpha(Float roughness)
        {
            roughness = std::max(roughness, (Float)1e-4);
            Float x = ::log(roughness);
            return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x +
                   0.000640711f * x * x * x * x;
        }

        TrowbridgeReitzDistribution(Float alphaX, Float alphaY) : alphaX(alphaX),
                                                                  alphaY(alphaY)
        {
        }

        RAYFLOW_CPU_GPU Float D(const Vector3 &wh) const final;

        RAYFLOW_CPU_GPU Vector3 SampleWh(const Point2 &u) const final;

        RAYFLOW_CPU_GPU Float Lambda(const Vector3 &w) const final;

    private:
        const Float alphaX;
        const Float alphaY;
    };

    class MicrofacetReflection : public BXDF
    {
    public:
        MicrofacetReflection(const MicrofacetDistribution *distribution, const Fresnel *fresnel, const Spectrum &R) : BXDF(BXDFType((int)BXDFType::REFLECTION | (int)BXDFType::GLOSSY)),
                                                                                                                      R(R),
                                                                                                                      distribution(distribution),
                                                                                                                      fresnel(fresnel)
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const final;

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const final;

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const final;

    private:
        const Spectrum R;
        const MicrofacetDistribution *distribution;
        const Fresnel *fresnel;
    };

    class MicrofacetTransmission : public BXDF
    {
    public:
        MicrofacetTransmission(const MicrofacetDistribution *distribution, const Fresnel *fresnel,
                               const Spectrum &T, Float etaA, Float etaB) : BXDF(BXDFType((int)BXDFType::TRANSMISSION | (int)BXDFType::GLOSSY)),
                                                                            T(T),
                                                                            etaA(etaA),
                                                                            etaB(etaB),
                                                                            distribution(distribution),
                                                                            fresnel(fresnel)
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const final;

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const final;

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const final;

    private:
        const Spectrum T;
        const Float etaA;
        const Float etaB;
        const MicrofacetDistribution *distribution;
        const Fresnel *fresnel;
    };

    RAYFLOW_CPU_GPU inline Float SchlickWeight(Float cosTheta)
    {
        Float m = Clamp(1 - cosTheta, 0, 1);
        return (m * m) * (m * m) * m;
    }

    RAYFLOW_CPU_GPU inline Float FrSchlick(Float R0, Float cosTheta)
    {
        return Lerp(1.0f, R0, SchlickWeight(cosTheta));
    }

    RAYFLOW_CPU_GPU inline Spectrum FrSchlick(const Spectrum &R0, Float cosTheta)
    {
        return Lerp(Spectrum(1.), R0, SchlickWeight(cosTheta));
    }

    RAYFLOW_CPU_GPU inline Float SchlickR0FromEta(Float eta) { return (eta - 1) * (eta - 1) / (eta + 1) * (eta + 1); }

    class GTR1 : public MicrofacetDistribution
    {
    public:
        explicit GTR1(Float alpha) : alpha(alpha)
        {
            C = (alpha * alpha - 1) / (2 * Pi * std::log(alpha));
        }

        RAYFLOW_CPU_GPU Float D(const Vector3 &wh) const final
        {
            return C / (alpha * alpha * Cos2Theta(wh) + Sin2Theta(wh));
        }

        RAYFLOW_CPU_GPU Vector3 SampleWh(const Point2 &u) const final
        {
            Float phi = u[1];
            Float cosTheta = ::sqrt((::pow(alpha, 2 * (1 - u[1])) - 1) / (alpha * alpha - 1));
            Float sinTheta = ::sqrt(std::max(Float(0), 1 - cosTheta * cosTheta));
            return Vector3(::cos(phi) * sinTheta, ::sin(phi) * sinTheta, cosTheta);
        }

        RAYFLOW_CPU_GPU Float Lambda(const Vector3 &w) const final
        {
            Float absTanTheta = ::abs(TanTheta(w));

            if (::isinf(absTanTheta))
            {
                return 0;
            }

            Float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);

            return (-1 + ::sqrt(1 + alpha2Tan2Theta)) / 2;
        }

    private:
        Float C;
        const Float alpha;
    };

    class RoughDielectricBXDF : public BXDF
    {
    public:
        RoughDielectricBXDF(const Spectrum &ks, const Spectrum &kt, Float etaA, Float etaB, const DielectricFresnel *fresnel, const MicrofacetDistribution *distribution) : BXDF(BXDFType((int)BXDFType::GLOSSY | (int)BXDFType::REFLECTION | (int)BXDFType::TRANSMISSION)),
                                                                                                                                                                            ks(ks),
                                                                                                                                                                            kt(kt),
                                                                                                                                                                            etaA(etaA),
                                                                                                                                                                            etaB(etaB),
                                                                                                                                                                            fresnel(fresnel),
                                                                                                                                                                            distribution(distribution)
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const
        {
            bool entering = CosTheta(wo) < 0;
            Float etaI = entering ? etaA : etaB;
            Float etaT = entering ? etaB : etaA;
            bool sampleReflection = true;
            Vector3 wh = distribution->SampleWh(sampler->Get2D());
            Float F = FresnelDielectricToDieletric(etaT / etaI, AbsDot(wo, wh));

            if (sampler->Get1D() > F) {
                sampleReflection = false;
            }

            if (sampleReflection) {
                Vector3 wi = Normalize(Reflect(wo, wh));
                Float D = distribution->D(wh);
                Float G = distribution->G(wi, wo);
                Float pdf = distribution->Pdf(wh) / (4 * AbsDot(wo, wh));

                return BXDFSample{ D * G * F * ks / std::abs(4 * CosTheta(wi) * CosTheta(wo)), wi, F * pdf, BXDFType((int)BXDFType::GLOSSY | (int)(BXDFType::REFLECTION)) };
            }
            else {
                Vector3 wi;
                Float eta = etaI / etaT;

                if (!Refract(wo, FaceForward(wh, wo), etaT / etaI, &wi)) {
                    return {};
                }

                Float D = distribution->D(wh);
                Float G = distribution->G(wi, wo);
                Float k = Dot(wo, wh) + eta * Dot(wi, wh);
                Float pdf = distribution->Pdf(wh) * eta * eta * AbsDot(wi, wh) / (k * k);
                Float factor = 1;

                if (mode == TransportMode::Radiance) {
                    factor = 1 / eta;
                }

                return BXDFSample{ factor * factor * D * G * (1 - F) * eta * eta * kt * std::abs(Dot(wo, wh) * Dot(wi, wh) /
                                   (k * k * CosTheta(wi) * CosTheta(wo))), wi, (1 - F) * pdf,
                                   BXDFType((int)BXDFType::GLOSSY | (int)(BXDFType::TRANSMISSION)) };

            }
        }

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const
        {
            bool reflect = InSameHemiSphere(wi, wo);
            Float cosThetaI = CosTheta(wi);
            Float cosThetaO = CosTheta(wo);

            if (cosThetaI == 0 || cosThetaO == 0) {
                return Spectrum(0.f);
            }

            Float eta = cosThetaI > 0 ? etaA / etaB : etaB / etaA;

            if (reflect) {
                Vector3 wh = Normalize(wi + wo);

                if (CosTheta(wh) < 0) {
                    wh = -wh;
                }

                Float D = distribution->D(wh);
                Float G = distribution->G(wi, wo);
                Float F = FresnelDielectricToDieletric(eta, AbsDot(wi, wh));

                return D * G * F * ks / std::abs(4 * CosTheta(wi) * CosTheta(wo));
            }
            else {

                Vector3 wh = Normalize(wo + eta * wi);

                if (CosTheta(wh) < 0) {
                    wh = -wh;
                }

                if (Dot(wi, wh) * Dot(wo, wh) > 0) {
                    return Spectrum(0.f);
                }

                Float D = distribution->D(wh);
                Float G = distribution->G(wo, wi);
                Float F = FresnelDielectricToDieletric(eta, AbsDot(wi, wh));
                Float k = Dot(wo, wh) + eta * Dot(wi, wh);

                Float factor = 1;
                if (mode == TransportMode::Radiance)
                {
                    factor = 1 / eta;
                }

                return factor * factor * eta * eta * D * G * (1 - F) * kt *
                    std::abs(Dot(wo, wh) * Dot(wi, wh) /
                        (k * k * CosTheta(wi) * CosTheta(wo)));
            }
        }

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const
        {
            bool reflect = InSameHemiSphere(wi, wo);
            Float eta = CosTheta(wi) > 0 ? etaA / etaB : etaB / etaA;

            if (reflect) {
                Vector3 wh = Normalize(wi + wo);

                if (CosTheta(wh) < 0) {
                    wh = -wh;
                }

                Float F = FresnelDielectricToDieletric(eta, AbsDot(wi, wh));
                return F * distribution->Pdf(wh) / (4 * AbsDot(wo, wh));
            }
            else {

                Vector3 wh = Normalize(wo + eta * wi);

                if (CosTheta(wh) < 0) {
                    wh = -wh;
                }

                Float F = FresnelDielectricToDieletric(eta, AbsDot(wi, wh));
                Float k = Dot(wo, wh) + eta * Dot(wi, wh);

                return (1 - F) * distribution->Pdf(wh) * eta * eta * AbsDot(wi, wh) / (k * k);
            }
        }

    private:
        const Spectrum ks;
        const Spectrum kt;
        const Float etaA;
        const Float etaB;
        const DielectricFresnel *fresnel;
        const MicrofacetDistribution *distribution;
    };

    class RoughPlasticBXDF : public BXDF
    {
    public:
        RoughPlasticBXDF(const Spectrum &ks, const Spectrum &kd, const DielectricFresnel *fresnel, const MicrofacetDistribution *distribution, Float eta) : BXDF(BXDFType((int)BXDFType::REFLECTION | (int)BXDFType::DIFFUSE | (int)BXDFType::GLOSSY)),
                                                                                                                                                            ks(ks),
                                                                                                                                                            kd(kd),
                                                                                                                                                            fresnel(fresnel),
                                                                                                                                                            distribution(distribution),
                                                                                                                                                            eta(eta)
        {
            Float specularProb = ks.Luminance();
            Float diffuseProb = kd.Luminance();
            specularSampleWeight = specularProb / (specularProb + diffuseProb);
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const
        {
            if (CosTheta(wo) <= 0)
            {
                return {};
            }

            Point2 sample = sampler->Get2D();
            bool chooseSpecular = true;
            BXDFType sampleType;

            if (sample.x < specularSampleWeight)
            {
                sampleType = BXDFType::GLOSSY;
                sample.x /= specularSampleWeight;
            }
            else
            {
                sampleType = BXDFType::DIFFUSE;
                chooseSpecular = false;
                sample.x = (sample.x - specularSampleWeight) / (1 - specularSampleWeight);
            }

            Vector3 wi;
            if (chooseSpecular)
            {
                Vector3 wh = distribution->SampleWh(sample);
                wi = Normalize(Reflect(wo, wh));
            }
            else
            {
                wi = CosineWeightedSampleHemiSphere(sample);
            }

            Float pdf = Pdf(wi, wo);

            if (pdf == 0)
            {
                return {};
            }

            return BXDFSample{f(wi, wo), wi, pdf, sampleType};
        }

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const
        {
            Float cosThetaI = CosTheta(wi);
            Float cosThetaO = CosTheta(wo);

            Vector3 wh = Normalize(wi + wo);
            Float D = distribution->D(wh);
            Float G = distribution->G(wo, wi);
            Spectrum F = fresnel->Evalueate(Dot(wh, wi));

            return D * G * F * ks / std::abs(4 * cosThetaI * cosThetaO) + (Spectrum(1.0f) - F) * kd * InvPi;
        }

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const
        {
            if (!InSameHemiSphere(wi, wo))
            {
                return 0;
            }

            Vector3 wh = Normalize(wi + wo);
            Float result = 0;

            result += specularSampleWeight * distribution->Pdf(wh) / (4 * AbsCosTheta(wo));
            result += (1 - specularSampleWeight) * CosineWeightedSampleHemiSpherePdf(AbsCosTheta(wi));

            return result;
        }

    private:
        const Spectrum ks;
        const Spectrum kd;
        const DielectricFresnel *fresnel;
        const MicrofacetDistribution *distribution;
        Float eta;
        Float specularSampleWeight;
    };

    class DisneyClearCoatGTR1 : public MicrofacetDistribution
    {
    public:
        explicit DisneyClearCoatGTR1(Float alpha) : alpha(alpha)
        {
            C = (alpha * alpha - 1) / (2 * Pi * std::log(alpha));
        }

        RAYFLOW_CPU_GPU Float D(const Vector3 &wh) const final
        {
            return C / (alpha * alpha * Cos2Theta(wh) + Sin2Theta(wh));
        }

        RAYFLOW_CPU_GPU Vector3 SampleWh(const Point2 &u) const final
        {
            Float phi = u[1];
            Float cosTheta = ::sqrt((::pow(alpha, 2 * (1 - u[1])) - 1) / (alpha * alpha - 1));
            Float sinTheta = ::sqrt(std::max(Float(0), 1 - cosTheta * cosTheta));
            return Vector3(::cos(phi) * sinTheta, ::sin(phi) * sinTheta, cosTheta);
        }

        RAYFLOW_CPU_GPU Float Lambda(const Vector3 &w) const final
        {
            Float absTanTheta = ::abs(TanTheta(w));

            if (::isinf(absTanTheta))
            {
                return 0;
            }

            Float alpha2Tan2Theta = (0.25 * absTanTheta) * (0.25 * absTanTheta);

            return (-1 + ::sqrt(1 + alpha2Tan2Theta)) / 2;
        }

    private:
        Float C;
        const Float alpha;
    };

    class DisneyDiffuseBXDF : public BXDF
    {
    public:
        DisneyDiffuseBXDF(const Spectrum &baseColor, Float roughness, Float subsurface) : BXDF(BXDFType((int)BXDFType::REFLECTION | (int)BXDFType::DIFFUSE)),
                                                                                          baseColor(baseColor),
                                                                                          roughness(roughness),
                                                                                          subsurface(subsurface)
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const final
        {
            Vector3 wi = CosineWeightedSampleHemiSphere(sampler->Get2D());

            if (wo.z < 0)
            {
                wi.z = -wi.z;
            }

            return BXDFSample{f(wi, wo, mode), wi, Pdf(wi, wo), type};
        }

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const final;

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const final
        {
            return InSameHemiSphere(wi, wo) ? CosineWeightedSampleHemiSpherePdf(AbsCosTheta(wi)) : 0;
        }

    private:
        const Spectrum baseColor;
        const Float roughness;
        const Float subsurface;
    };

    class DisneyMetalFresnel : public Fresnel
    {
    public:
        explicit DisneyMetalFresnel(const Spectrum &baseColor) : baseColor(baseColor)
        {
        }

        RAYFLOW_CPU_GPU Spectrum Evalueate(Float cosThetaI) const final
        {
            return baseColor + (Spectrum(1.0f) - baseColor) * SchlickWeight(cosThetaI);
        }

    private:
        const Spectrum baseColor;
    };

    class DisneyMetalBXDF : public BXDF
    {
    public:
        DisneyMetalBXDF(const MicrofacetDistribution *distribution, const Fresnel *fresnel, const Spectrum &baseColor) : BXDF(BXDFType((int)BXDFType::REFLECTION | (int)BXDFType::GLOSSY)),
                                                                                                                         fresnel(fresnel),
                                                                                                                         distribution(distribution),
                                                                                                                         baseColor(baseColor)
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const final;

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const final;

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const final
        {
            Vector3 wh = Normalize(wi + wo);
            return distribution->Pdf(wh) / (4 * AbsDot(wo, wh));
        }

    private:
        const Fresnel *fresnel;
        const MicrofacetDistribution *distribution;
        const Spectrum baseColor;
    };

    class DisneyGlassBXDF : public BXDF {
    public:
        DisneyGlassBXDF(const Spectrum& baseColor, Float etaA, Float etaB, const DielectricFresnel* fresnel, const MicrofacetDistribution* distribution) : BXDF(BXDFType((int)BXDFType::GLOSSY | (int)BXDFType::REFLECTION | (int)BXDFType::TRANSMISSION)),
            baseColor(baseColor),
            etaA(etaA),
            etaB(etaB),
            fresnel(fresnel),
            distribution(distribution)
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3& wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const
        {
            bool entering = CosTheta(wo) < 0;
            Float etaI = entering ? etaA : etaB;
            Float etaT = entering ? etaB : etaA;
            bool sampleReflection = true;
            Vector3 wh = distribution->SampleWh(sampler->Get2D());
            Float F = FresnelDielectricToDieletric(etaT / etaI, AbsDot(wo, wh));

            if (sampler->Get1D() > F) {
                sampleReflection = false;
            }

            if (sampleReflection) {
                Vector3 wi = Normalize(Reflect(wo, wh));
                Float D = distribution->D(wh);
                Float G = distribution->G(wi, wo);
                Float pdf = distribution->Pdf(wh) / (4 * AbsDot(wo, wh));

                return BXDFSample{ D * G * F * baseColor / std::abs(4 * CosTheta(wi) * CosTheta(wo)), wi, F * pdf, BXDFType((int)BXDFType::GLOSSY | (int)(BXDFType::REFLECTION)) };
            }
            else {
                Vector3 wi;
                Float eta = etaI / etaT;

                if (!Refract(wo, FaceForward(wh, wo), etaT / etaI, &wi)) {
                    return {};
                }

                Float D = distribution->D(wh);
                Float G = distribution->G(wi, wo);
                Float k = Dot(wo, wh) + eta * Dot(wi, wh);
                Float pdf = distribution->Pdf(wh) * eta * eta * AbsDot(wi, wh) / (k * k);
                Float factor = 1;

                if (mode == TransportMode::Radiance) {
                    factor = 1 / eta;
                }

                return BXDFSample{ factor * factor * D * G * (1 - F) * eta * eta * baseColor.Sqrt() * std::abs(Dot(wo, wh) * Dot(wi, wh) /
                                   (k * k * CosTheta(wi) * CosTheta(wo))), wi, (1 - F) * pdf,
                                   BXDFType((int)BXDFType::GLOSSY | (int)(BXDFType::TRANSMISSION)) };

            }
        }

        RAYFLOW_CPU_GPU Spectrum f(const Vector3& wi, const Vector3& wo, TransportMode mode = TransportMode::Radiance) const
        {
            bool reflect = InSameHemiSphere(wi, wo);
            Float cosThetaI = CosTheta(wi);
            Float cosThetaO = CosTheta(wo);

            if (cosThetaI == 0 || cosThetaO == 0) {
                return Spectrum(0.f);
            }

            Float eta = cosThetaI > 0 ? etaA / etaB : etaB / etaA;

            if (reflect) {
                Vector3 wh = Normalize(wi + wo);

                if (CosTheta(wh) < 0) {
                    wh = -wh;
                }

                Float D = distribution->D(wh);
                Float G = distribution->G(wi, wo);
                Float F = FresnelDielectricToDieletric(eta, AbsDot(wi, wh));

                return D * G * F * baseColor / std::abs(4 * CosTheta(wi) * CosTheta(wo));
            }
            else {
                Vector3 wh = Normalize(wo + eta * wi);

                if (CosTheta(wh) < 0) {
                    wh = -wh;
                }

                if (Dot(wi, wh) * Dot(wo, wh) > 0) {
                    return Spectrum(0.f);
                }

                Float D = distribution->D(wh);
                Float G = distribution->G(wo, wi);
                Float F = FresnelDielectricToDieletric(eta, AbsDot(wi, wh));
                Float k = Dot(wo, wh) + eta * Dot(wi, wh);

                Float factor = 1;
                if (mode == TransportMode::Radiance)
                {
                    factor = 1 / eta;
                }

                return factor * factor * eta * eta * D * G * (1 - F) * baseColor.Sqrt() *
                    std::abs(Dot(wo, wh) * Dot(wi, wh) /
                        (k * k * CosTheta(wi) * CosTheta(wo)));
            }
        }

        RAYFLOW_CPU_GPU Float Pdf(const Vector3& wi, const Vector3& wo) const
        {
            bool reflect = InSameHemiSphere(wi, wo);
            Float eta = CosTheta(wi) > 0 ? etaA / etaB : etaB / etaA;

            if (reflect) {
                Vector3 wh = Normalize(wi + wo);

                if (CosTheta(wh) < 0) {
                    wh = -wh;
                }

                Float F = FresnelDielectricToDieletric(eta, AbsDot(wi, wh));
                return F * distribution->Pdf(wh) / (4 * AbsDot(wo, wh));
            }
            else {

                Vector3 wh = Normalize(wo + eta * wi);

                if (CosTheta(wh) < 0) {
                    wh = -wh;
                }

                Float F = FresnelDielectricToDieletric(eta, AbsDot(wi, wh));
                Float k = Dot(wo, wh) + eta * Dot(wi, wh);
                return (1 - F) * distribution->Pdf(wh) * eta * eta * AbsDot(wi, wh) / (k * k);
            }
        }

    private:
        const Spectrum baseColor;
        const Float etaA;
        const Float etaB;
        const DielectricFresnel* fresnel;
        const MicrofacetDistribution* distribution;
    };

    class DisneyClearCoatBXDF : public BXDF
    {
    public:
        explicit DisneyClearCoatBXDF(const MicrofacetDistribution *distribution) : BXDF(BXDFType((int)BXDFType::REFLECTION | (int)BXDFType::GLOSSY)),
                                                                                   distribution(distribution)
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const final;

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const final;

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const final
        {
            Vector3 wh = Normalize(wi + wo);
            return distribution->Pdf(wh) / (4 * AbsDot(wo, wh));
        }

    private:
        const MicrofacetDistribution *distribution;
    };

    class DisneySheenBXDF : public BXDF
    {
    public:
        explicit DisneySheenBXDF(const Spectrum Csheen) : BXDF(BXDFType((int)BXDFType::REFLECTION | (int)BXDFType::DIFFUSE)),
                                                          Csheen(Csheen)
        {
        }

        RAYFLOW_CPU_GPU rstd::optional<BXDFSample> SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode = TransportMode::Radiance) const final
        {
            Vector3 wi = CosineWeightedSampleHemiSphere(sampler->Get2D());

            if (wo.z < 0)
            {
                wi.z = -wi.z;
            }

            return BXDFSample{f(wi, wo), wi, Pdf(wi, wo), type};
        }

        RAYFLOW_CPU_GPU Spectrum f(const Vector3 &wi, const Vector3 &wo, TransportMode mode = TransportMode::Radiance) const final
        {
            Vector3 wh = Normalize(wi + wo);
            Float cosTheta = Dot(wi, wh);
            return Csheen * SchlickWeight(cosTheta);
        }

        RAYFLOW_CPU_GPU Float Pdf(const Vector3 &wi, const Vector3 &wo) const final
        {
            return InSameHemiSphere(wi, wo) ? CosineWeightedSampleHemiSpherePdf(AbsCosTheta(wi)) : 0;
        }

    private:
        const Spectrum Csheen;
    };

}