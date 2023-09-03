#include <RayFlow/Render/materials.h>

namespace rayflow {

rstd::optional<BSDF> DisneyDiffuseMaterial::EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const {
    Spectrum color = baseColor->Evaluate(isect);
    Float rough = roughness->Evaluate(isect);
    Float ss = subsurface->Evaluate(isect);

    BSDF bsdf(isect.ns);
    bsdf.AddComponent(GMalloc.new_object<DisneyDiffuseBXDF>(color, rough, ss));
    return bsdf;
}

rstd::optional<BSDF> DisneyMetalMaterial::EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const {
    Spectrum color = baseColor->Evaluate(isect);
    Fresnel* fresnel = GMalloc.new_object<DisneyMetalFresnel>(color);

    Float rough = roughness->Evaluate(isect);
    Float anis = anisotropic->Evaluate(isect);

    Float aspect = ::sqrt(1 - 0.9 * anis);
    Float ax = std::max(Float(1e-4), rough * rough / aspect);
    Float ay = std::max(Float(1e-4), rough * rough * aspect);
    MicrofacetDistribution* distribution = GMalloc.new_object<TrowbridgeReitzDistribution>(ax, ay);

    BSDF bsdf(isect.ns);
    bsdf.AddComponent(GMalloc.new_object<DisneyMetalBXDF>(distribution, fresnel, color));
    return bsdf;
}

rstd::optional<BSDF> DisneyClearCoatMaterial::EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const {
    Float ccg = clearcoatGloss->Evaluate(isect);
    Float ag = (1 - ccg) * 0.1 + ccg * 0.001;
    MicrofacetDistribution* distribution = GMalloc.new_object<DisneyClearCoatGTR1>(ag);

    BSDF bsdf(isect.ns);
    bsdf.AddComponent(GMalloc.new_object<DisneyClearCoatBXDF>(distribution));
    return bsdf;
}

rstd::optional<BSDF> DisneyGlassMaterial::EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const {
    Spectrum colorR = baseColor->Evaluate(isect);

    Float rough = roughness->Evaluate(isect);
    MicrofacetDistribution* distribution = GMalloc.new_object<TrowbridgeReitzDistribution>(rough, rough);

    BSDF bsdf(isect.ns);
    DielectricFresnel* fresnel = GMalloc.new_object<DielectricFresnel>(1, eta);
    bsdf.AddComponent(GMalloc.new_object<DisneyGlassBXDF>(colorR, 1.0, eta, fresnel, distribution));
    return bsdf;
}

rstd::optional<BSDF> DisneyMaterial::EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const {
    BSDF bsdf(isect.ns);
    // diffuse
    Spectrum colorR = baseColor->Evaluate(isect);
    Float rough = roughness->Evaluate(isect);
    Float subsf = subsurface->Evaluate(isect);
    BXDF* fDiffuse = GMalloc.new_object<DisneyDiffuseBXDF>(colorR, rough, subsf);
    // metal
    Float specTint = specularTint->Evaluate(isect);
    Float luminance = colorR.Luminance();
    Spectrum Ctint = luminance > 0 ? colorR / luminance : Spectrum(1.f);

    Float spec = specular->Evaluate(isect);
    Spectrum Ks = Spectrum(1 - specTint) + specTint * Ctint;
    Float metal = metallic->Evaluate(isect);
    Spectrum C0 = spec * SchlickR0FromEta(eta) * (1 - metal) * Ks + metal * colorR;
    Fresnel* metalFresnel = GMalloc.new_object<DisneyMetalFresnel>(C0);
    Float anis = anisotropic->Evaluate(isect);
    Float aspect = ::sqrt(1 - 0.9 * anis);
    Float ax = std::max(Float(1e-4), rough * rough / aspect);
    Float ay = std::max(Float(1e-4), rough * rough * aspect);
    MicrofacetDistribution* metalDistribution = GMalloc.new_object<TrowbridgeReitzDistribution>(ax, ay);

    BXDF* fMetal = GMalloc.new_object<DisneyMetalBXDF>(metalDistribution, metalFresnel, colorR);
    // sheen
    Float st = sheenTint->Evaluate(isect);
    Spectrum Csheen = Spectrum(1 - st) + st * Ctint;
    BXDF* fSheen = GMalloc.new_object<DisneySheenBXDF>(Csheen);
    // clearcoat
    Float ccg = clearcoatGloss->Evaluate(isect);
    Float ag = (1 - ccg) * 0.1 + ccg * 0.001;
    MicrofacetDistribution* clearcoatDistribution = GMalloc.new_object<DisneyClearCoatGTR1>(ag);
    BXDF* fClearcoat = GMalloc.new_object<DisneyClearCoatBXDF>(clearcoatDistribution);
    // glass
    Spectrum colorT = colorR.Sqrt();
    Fresnel* glassFresnel = GMalloc.new_object<DielectricFresnel>(1, eta);
    Float alphaGlass = TrowbridgeReitzDistribution::RoughnessToAlpha(rough);
    MicrofacetDistribution* glassDistribution = GMalloc.new_object<TrowbridgeReitzDistribution>(alphaGlass, alphaGlass);
    BXDF* fGlassReflection = GMalloc.new_object<MicrofacetReflection>(glassDistribution, glassFresnel, colorR);
    BXDF* fGlassTransmission = GMalloc.new_object<MicrofacetTransmission>(glassDistribution, glassFresnel, colorT, 1.0f, eta);
    // scale bxdf
    Float diffuseWeight = (1 - specTint) * (1 - metal);
    Float sh = sheen->Evaluate(isect);
    Float sheenWeight = (1 - metal) * sh;
    Float metalWeight = (1 - specTint * ( 1 - metal));
    Float cc = clearcoat->Evaluate(isect);
    Float clearcoatWeight = 0.25f * cc;
    Float glassWeight = (1 - metal) * specTint;
    bsdf.AddComponent(GMalloc.new_object<ScaleBXDF>(fDiffuse, Spectrum(diffuseWeight)));
    bsdf.AddComponent(GMalloc.new_object<ScaleBXDF>(fSheen, Spectrum(sheenWeight)));
    bsdf.AddComponent(GMalloc.new_object<ScaleBXDF>(fMetal, Spectrum(metalWeight)));
    bsdf.AddComponent(GMalloc.new_object<ScaleBXDF>(fClearcoat, Spectrum(clearcoatWeight)));
    bsdf.AddComponent(GMalloc.new_object<ScaleBXDF>(fGlassReflection, Spectrum(glassWeight)));
    bsdf.AddComponent(GMalloc.new_object<ScaleBXDF>(fGlassTransmission, Spectrum(glassWeight)));
    return bsdf;
}

}