#include <RayFlow/Render/bxdfs.h>

namespace rayflow
{
    // https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/#more-1921
    Float FresnelDielectricToDieletric(Float eta, Float cosThetaI)
    {
        Float sin2ThetaI = 1 - cosThetaI * cosThetaI;

        if (sin2ThetaI * eta * eta > 1)
        {
            return 1;
        }

        Float t0 = ::sqrt(1 - (sin2ThetaI * (eta * eta)));
        Float t1 = eta * t0;
        Float t2 = eta * cosThetaI;

        Float rs = (t2 - t0) / (t2 + t0);
        Float rp = (t1 - cosThetaI) / (t1 + cosThetaI);

        return 0.5 * (rs * rs + rp * rp);
    }

    // https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/#more-1921
    Spectrum FresnelDielectricToConductor(const Spectrum &eta, const Spectrum &etaK, Float cosThetaI)
    {
        Float cos2ThetaI = cosThetaI * cosThetaI;
        Float sin2ThetaI = 1 - cos2ThetaI;
        Spectrum eta2 = eta * eta;
        Spectrum etaK2 = etaK * etaK;

        Spectrum t0 = eta2 - etaK2 - Spectrum(sin2ThetaI);
        Spectrum a2plusb2 = (t0 * t0 + 4 * eta2 * etaK2).Sqrt();
        Spectrum t1 = a2plusb2 + Spectrum(cos2ThetaI);
        Spectrum a = (0.5 * (a2plusb2 + t0)).Sqrt();
        Spectrum t2 = 2 * a * cosThetaI;
        Spectrum Rs = (t1 - t2) / (t1 + t2);

        Spectrum t3 = cos2ThetaI * a2plusb2 + Spectrum(sin2ThetaI * sin2ThetaI);
        Spectrum t4 = t2 * sin2ThetaI;
        Spectrum Rp = Rs * (t3 - t4) / (t3 + t4);

        return 0.5 * (Rp + Rs);
    }

    Float TrowbridgeReitzDistribution::D(const Vector3 &wh) const
    {
        Float tan2Theta = Tan2Theta(wh);

        if (::isinf(tan2Theta))
        {
            return 0;
        }

        Float cos2Theta = Cos2Theta(wh);
        Float sin2Theta = Sin2Theta(wh);
        Float cos4Theta = cos2Theta * cos2Theta;
        Float e = tan2Theta * (Cos2Phi(wh) / (alphaX * alphaX) + Sin2Phi(wh) / (alphaY * alphaY));

        return 1 / (Pi * alphaX * alphaY * cos4Theta * (1 + e) * (1 + e));
    }

    Vector3 TrowbridgeReitzDistribution::SampleWh(const Point2 &u) const
    {
        Float cosTheta = 0;
        Float phi = 2 * Pi * u[1];

        if (alphaX == alphaY)
        {
            Float tan2Theta = alphaX * alphaX * u[0] / (1 - u[0]);
            cosTheta = 1 / ::sqrt(1 + tan2Theta);
        }
        else
        {
            phi = std::atan(alphaY / alphaX * std::tan(2 * Pi * u[1] + .5f * Pi));
            if (u[1] > .5f)
            {
                phi += Pi;
            }
            Float sinPhi = std::sin(phi);
            Float cosPhi = std::cos(phi);
            Float k = 1 / (cosPhi * cosPhi / (alphaX * alphaX) + sinPhi * sinPhi / (alphaY * alphaY));
            Float tan2Theta = k * u[0] / (1 - u[0]);
            cosTheta = 1 / ::sqrt(1 + tan2Theta);
        }

        Float sinTheta = ::sin(std::max(Float(0), 1 - cosTheta * cosTheta));
        Vector3 wh(sinTheta * ::cos(phi), sinTheta * ::sin(phi), cosTheta);

        return wh;
    }

    Float TrowbridgeReitzDistribution::Lambda(const Vector3 &w) const
    {
        Float absTanTheta = ::abs(TanTheta(w));

        if (::isinf(absTanTheta))
        {
            return 0;
        }

        Float alpha = ::sqrt(Cos2Phi(w) * alphaX * alphaX + Sin2Phi(w) * alphaY * alphaY);
        Float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);

        return (-1 + ::sqrt(1 + alpha2Tan2Theta)) / 2;
    }

    rstd::optional<BXDFSample> BSDF::SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode) const
    {
        Vector3 wol = frame.ToLocal(wo);

        if (CosTheta(wol) == 0)
        {
            return {};
        }

        const int nBXDFs = nComponents;
        int componentIdx = std::min((int)::floor(sampler->Get1D() * nBXDFs), nBXDFs - 1);
        BXDF *sampleBxdf = mBXDFs_[componentIdx];

        rstd::optional<BXDFSample> result = sampleBxdf->SampleF(wol, sampler, mode);
        if (!result || result->pdf == 0)
        {
            return {};
        }

        Vector3 wil = result->wi;
        result->wi = frame.FromLocal(result->wi);

        if (!((int)result->type & (int)BXDFType::SPECULAR))
        {
            for (int i = 0; i < nComponents; ++i)
            {
                if (mBXDFs_[i] != sampleBxdf)
                {
                    result->pdf += mBXDFs_[i]->Pdf(wil, wol);
                }
            }
            result->pdf /= nBXDFs;

            bool reflect = Dot(result->wi, frame.n) * Dot(wo, frame.n) > 0;
            for (int i = 0; i < nComponents; ++i)
            {
                if (mBXDFs_[i] != sampleBxdf &&
                    (reflect && (int(mBXDFs_[i]->type) & int(BXDFType::REFLECTION)) ||
                     !reflect && (int(mBXDFs_[i]->type) & int(BXDFType::TRANSMISSION))))
                {
                    result->f += mBXDFs_[i]->f(wil, wol);
                }
            }
        }

        return result;
    }

    Spectrum BSDF::f(const Vector3 &wi, const Vector3 &wo, TransportMode mode) const
    {
        Vector3 wil = frame.ToLocal(wi);
        Vector3 wol = frame.ToLocal(wo);

        if (CosTheta(wol) == 0)
        {
            return 0;
        }

        bool reflect = Dot(wi, frame.n) * Dot(wo, frame.n) > 0;

        Spectrum result = Spectrum(0.0f);

        for (int i = 0; i < nComponents; ++i)
        {
            if ((reflect && (int(mBXDFs_[i]->type) & int(BXDFType::REFLECTION)) ||
                 !reflect && (int(mBXDFs_[i]->type) & int(BXDFType::TRANSMISSION))))
            {
                result += mBXDFs_[i]->f(wil, wol);
            }
        }

        return result;
    }

    Float BSDF::Pdf(const Vector3 &wi, const Vector3 &wo) const
    {
        const int nBXDFs = nComponents;
        Float pdf = 0;
        Vector3 wil = frame.ToLocal(wi);
        Vector3 wol = frame.ToLocal(wo);

        if (CosTheta(wol) == 0)
        {
            return 0;
        }

        for (int i = 0; i < nComponents; ++i)
        {
            pdf += mBXDFs_[i]->Pdf(wil, wol);
        }
        pdf /= nBXDFs;

        return pdf;
    }

    rstd::optional<BXDFSample> MicrofacetReflection::SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode) const
    {
        if (CosTheta(wo) <= 0)
        {
            return {};
        }

        Vector3 wh = Normalize(distribution->SampleWh(sampler->Get2D()));

        Vector3 wi = Normalize(Reflect(wo, wh));

        if (!InSameHemiSphere(wi, wo))
        {
            return {};
        }

        return BXDFSample{f(wi, wo, mode), wi, Pdf(wi, wo), type};
    }

    Spectrum MicrofacetReflection::f(const Vector3 &wi, const Vector3 &wo, TransportMode mode) const
    {
        Float cosThetaI = CosTheta(wi);
        Float cosThetaO = CosTheta(wo);

        Vector3 wh = Normalize(wi + wo);

        Float D = distribution->D(wh);
        Float G = distribution->G(wo, wi);
        Spectrum F = fresnel->Evalueate(Dot(wi, wh));

        return D * G * F * R / std::abs(4 * cosThetaI * cosThetaO);
    }

    Float MicrofacetReflection::Pdf(const Vector3 &wi, const Vector3 &wo) const
    {
        Vector3 wh = Normalize(wi + wo);
        return distribution->Pdf(wh) / (4 * AbsDot(wo, wh));
    }

    rstd::optional<BXDFSample> MicrofacetTransmission::SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode) const
    {
        if (CosTheta(wo) == 0)
        {
            return {};
        }

        Vector3 wh = Normalize(distribution->SampleWh(sampler->Get2D()));

        Float etaToverEtaI = CosTheta(wo) > 0 ? etaA / etaB : etaB / etaA;

        Vector3 wi;
        if (!Refract(wo, FaceForward(wh, wo), etaToverEtaI, &wi))
        {
            return {};
        }

        return BXDFSample{f(wi, wo, mode), wi, Pdf(wi, wo), type};
    }

    Spectrum MicrofacetTransmission::f(const Vector3 &wi, const Vector3 &wo, TransportMode mode) const
    {
        if (InSameHemiSphere(wi, wo))
        {
            return Spectrum(0.f);
        }

        Float cosThetaI = CosTheta(wi);
        Float cosThetaO = CosTheta(wo);

        if (cosThetaI == 0 || cosThetaO == 0) {
            return Spectrum(0.f);
        }

        Float eta = cosThetaI > 0 ? etaA / etaB : etaB / etaA;
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

        return factor * factor * eta * eta * D * G * (1 - F) * T *
                std::abs(Dot(wo, wh) * Dot(wi, wh) /
                (k * k * CosTheta(wi) * CosTheta(wo)));
    }

    Float MicrofacetTransmission::Pdf(const Vector3 &wi, const Vector3 &wo) const
    {
        Float eta = CosTheta(wo) > 0 ? etaB / etaA : etaA / etaB;

        Vector3 wh = Normalize(wo + eta * wi);

        if (CosTheta(wh) < 0) {
            wh = -wh;
        }

        if (Dot(wi, wh) * Dot(wo, wh) > 0)
        {
            return 0;
        }

        Float k = Dot(wo, wh) + eta * Dot(wi, wh);

        return distribution->Pdf(wh) * eta * eta * AbsDot(wi, wh) / (k * k);
    }

    Spectrum DisneyDiffuseBXDF::f(const Vector3 &wi, const Vector3 &wo, TransportMode mode) const
    {
        Vector3 wh = Normalize(wi + wo);

        Float Rd0 = 0.5 * 2 * roughness * Dot(wh, wi) * Dot(wh, wi);
        Spectrum fBaseDiffuse = baseColor * InvPi * FrSchlick(Rd0, CosTheta(wi)) * FrSchlick(Rd0, CosTheta(wo));

        Float Rss0 = roughness * Dot(wh, wi) * Dot(wh, wi);
        Spectrum fSubsurface = 1.25 * baseColor * InvPi * (FrSchlick(Rss0, AbsCosTheta(wi) * FrSchlick(Rss0, AbsCosTheta(wo))) * (1 / (AbsCosTheta(wi) + AbsCosTheta(wo)) - 0.5) + 0.5);

        return (1 - subsurface) * fBaseDiffuse + subsurface * fSubsurface;
    }

    rstd::optional<BXDFSample> DisneyMetalBXDF::SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode) const
    {
        if (CosTheta(wo) <= 0)
        {
            return {};
        }

        Vector3 wh = Normalize(distribution->SampleWh(sampler->Get2D()));
        Vector3 wi = Normalize(Reflect(wo, wh));

        if (!InSameHemiSphere(wi, wo))
        {
            return {};
        }

        return BXDFSample{f(wi, wo), wi, Pdf(wi, wo), type};
    }

    Spectrum DisneyMetalBXDF::f(const Vector3 &wi, const Vector3 &wo, TransportMode mode) const
    {
        Float cosThetaI = CosTheta(wi);
        Float cosThetaO = CosTheta(wo);

        Vector3 wh = Normalize(wi + wo);
        Float D = distribution->D(wh);
        Float G = distribution->G(wo, wi);
        Spectrum F = fresnel->Evalueate(Dot(wi, wh));

        return D * G * F * baseColor / std::abs(4 * cosThetaI * cosThetaO);
    }

    rstd::optional<BXDFSample> DisneyClearCoatBXDF::SampleF(const Vector3 &wo, Sampler* sampler, TransportMode mode) const
    {
        if (CosTheta(wo) <= 0)
        {
            return {};
        }

        Vector3 wh = Normalize(distribution->SampleWh(sampler->Get2D()));
        Vector3 wi = Normalize(Reflect(wo, wh));

        if (!InSameHemiSphere(wi, wo))
        {
            return {};
        }

        return BXDFSample{f(wi, wo), wi, Pdf(wi, wo), type};
    }

    Spectrum DisneyClearCoatBXDF::f(const Vector3 &wi, const Vector3 &wo, TransportMode mode) const
    {
        Float cosThetaI = CosTheta(wi);
        Float cosThetaO = CosTheta(wo);

        Vector3 wh = Normalize(wi + wo);
        Float D = distribution->D(wh);
        Float G = distribution->G(wo, wi);
        Spectrum F = FrSchlick(SchlickR0FromEta(1.5), Dot(wi, wh));

        return D * G * F / (4 * cosThetaI * cosThetaO);
    }

}