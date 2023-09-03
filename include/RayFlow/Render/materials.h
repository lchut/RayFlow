#pragma once

#include <RayFlow/Core/material.h>
#include <RayFlow/Render/textures.h>
#include <RayFlow/Render/bxdfs.h>
#include <RayFlow/Std/optional.h>
#include <RayFlow/Engine/memory_pool.h>

namespace rayflow
{

	class DiffuseMaterial : public Material
	{
	public:
		explicit DiffuseMaterial(const Texture<Spectrum>* color) : color(color)
		{
		}

		RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const final
		{
			BSDF bsdf(isect.ns);
			bsdf.AddComponent(GMalloc.new_object<LambertianReflection>(color->Evaluate(isect)));
			return bsdf;
		}

	private:
		const Texture<Spectrum>* color;
	};

	class DielectricMaterial : public Material
	{
	public:
		DielectricMaterial(const Texture<Spectrum>* ks, const Texture<Spectrum>* kt, Float eta) : eta(eta),
			ks(ks),
			kt(kt)
		{
		}

		RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const final
		{
			BSDF bsdf(isect.ns);
			DielectricFresnel* fresnel = GMalloc.new_object<DielectricFresnel>(1.0f, eta);
			bsdf.AddComponent(GMalloc.new_object<DielectricBXDF>(ks->Evaluate(isect), kt->Evaluate(isect), fresnel, 1.0, eta));
			return bsdf;
		}

	private:
		Float eta;
		const Texture<Spectrum>* ks;
		const Texture<Spectrum>* kt;
	};

	class ConductorMaterial : public Material
	{
	public:
		ConductorMaterial(const Texture<Spectrum>* eta, const Texture<Spectrum>* etaK) : eta(eta),
			etaK(etaK)
		{
		}

		RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const final
		{
			BSDF bsdf(isect.ns);
			ConductorFresnel* fresnel = GMalloc.new_object<ConductorFresnel>(Spectrum(1.0f), eta->Evaluate(isect), etaK->Evaluate(isect));
			bsdf.AddComponent(GMalloc.new_object<SpecularReflection>(Spectrum(1.0f), fresnel));
			return bsdf;
		}

	private:
		const Texture<Spectrum>* eta;
		const Texture<Spectrum>* etaK;
	};

	class MirrorMaterial : public Material
	{
	public:
		explicit MirrorMaterial(const Texture<Spectrum>* ks) : ks(ks) {

		}

		RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const final
		{
			BSDF bsdf(isect.ns);
			bsdf.AddComponent(GMalloc.new_object<SpecularReflection>(ks->Evaluate(isect)));
			return bsdf;
		}

	private:
		const Texture<Spectrum>* ks;
	};

	class PhongMaterial : public Material
	{
	public:
		PhongMaterial(const Texture<Spectrum>* kd, const Texture<Spectrum>* ks, const Texture<Float>* exponent) : kd(kd),
			ks(ks),
			exponent(exponent)
		{
		}

		RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const final
		{
			BSDF bsdf(isect.ns);
			bsdf.AddComponent(GMalloc.new_object<Phong>(kd->Evaluate(isect), ks->Evaluate(isect), exponent->Evaluate(isect)));
			return bsdf;
		}

	private:
		const Texture<Spectrum>* kd;
		const Texture<Spectrum>* ks;
		const Texture<Float>* exponent;
	};

	class RoughMetalMaterial : public Material
	{
	public:
		RoughMetalMaterial(const Texture<Spectrum>* ks,
			const Texture<Float>* uRoughness,
			const Texture<Float>* vRoughness,
			const Texture<Spectrum>* eta,
			const Texture<Spectrum>* etaK) : ks(ks),
			uRoughness(uRoughness),
			vRoughness(vRoughness),
			eta(eta),
			etaK(etaK)
		{
		}

		RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const final
		{
			BSDF bsdf(isect.ns);
			Spectrum ksVal = ks->Evaluate(isect);
			Float uR = uRoughness->Evaluate(isect);
			Float vR = vRoughness->Evaluate(isect);
			Spectrum etaVal = eta->Evaluate(isect);
			Spectrum etaKVal = etaK->Evaluate(isect);
			Fresnel* fresnel = GMalloc.new_object<ConductorFresnel>(Spectrum(1.0f), etaVal, etaKVal);
			MicrofacetDistribution* distribution = GMalloc.new_object<TrowbridgeReitzDistribution>(uR, vR);
			bsdf.AddComponent(GMalloc.new_object<MicrofacetReflection>(distribution, fresnel, ksVal));
			return bsdf;
		}

	private:
		const Texture<Spectrum>* ks;
		const Texture<Float>* uRoughness;
		const Texture<Float>* vRoughness;
		const Texture<Spectrum>* eta;
		const Texture<Spectrum>* etaK;
	};

	class RoughDielectricMaterial : public Material
	{
	public:
		RoughDielectricMaterial(const Texture<Spectrum>* ks,
			const Texture<Spectrum>* kt,
			const Texture<Float>* uRoughness,
			const Texture<Float>* vRoughness,
			const Texture<Float>* eta) : ks(ks),
			kt(kt),
			uRoughness(uRoughness),
			vRoughness(vRoughness),
			eta(eta)
		{
		}

		RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const final
		{
			BSDF bsdf(isect.ns);
			Spectrum ksVal = ks->Evaluate(isect);
			Spectrum ktVal = kt->Evaluate(isect);
			Float uR = uRoughness->Evaluate(isect);
			Float vR = vRoughness->Evaluate(isect);
			Float etaVal = eta->Evaluate(isect);
			MicrofacetDistribution* distribution = GMalloc.new_object<TrowbridgeReitzDistribution>(uR, vR);
			DielectricFresnel* fresnel = GMalloc.new_object<DielectricFresnel>(1.0f, etaVal);
			bsdf.AddComponent(GMalloc.new_object<RoughDielectricBXDF>(ksVal, ktVal, 1.0f, etaVal, fresnel, distribution));
			return bsdf;
		}

	private:
		const Texture<Spectrum>* ks;
		const Texture<Spectrum>* kt;
		const Texture<Float>* uRoughness;
		const Texture<Float>* vRoughness;
		const Texture<Float>* eta;
	};

	class RoughPlasticMaterial : public Material
	{
	public:
		RoughPlasticMaterial(const Texture<Spectrum>* ks,
			const Texture<Spectrum>* kd,
			const Texture<Float>* roughness,
			const Texture<Float>* eta) : ks(ks),
			kd(kd),
			roughness(roughness),
			eta(eta)
		{
		}

		RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const final
		{
			BSDF bsdf(isect.ns);
			Spectrum ksVal = ks->Evaluate(isect);
			Spectrum kdVal = kd->Evaluate(isect);
			Float etaVal = eta->Evaluate(isect);
			DielectricFresnel* fresnel = GMalloc.new_object<DielectricFresnel>(1.0f, etaVal);
			Float roughnessVal = roughness->Evaluate(isect);
			MicrofacetDistribution* distribution = GMalloc.new_object<TrowbridgeReitzDistribution>(roughnessVal, roughnessVal);
			bsdf.AddComponent(GMalloc.new_object<RoughPlasticBXDF>(ksVal, kdVal, fresnel, distribution, etaVal));
			return bsdf;
		}

	private:
		const Texture<Spectrum>* kd;
		const Texture<Spectrum>* ks;
		const Texture<Float>* roughness;
		const Texture<Float>* eta;
	};

	class DisneyDiffuseMaterial : public Material
	{
	public:
		DisneyDiffuseMaterial(const Texture<Spectrum>* baseColor,
			const Texture<Float>* roughness,
			const Texture<Float>* subsurface) : baseColor(baseColor),
			roughness(roughness),
			subsurface(subsurface)
		{
		}

		RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const final;

	private:
		const Texture<Spectrum>* baseColor;
		const Texture<Float>* roughness;
		const Texture<Float>* subsurface;
	};

	class DisneyMetalMaterial : public Material
	{
	public:
		DisneyMetalMaterial(const Texture<Spectrum>* baseColor,
			const Texture<Float>* roughness,
			const Texture<Float>* anisotropic) : baseColor(baseColor),
			roughness(roughness),
			anisotropic(anisotropic)
		{
		}

		RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const final;

	private:
		const Texture<Spectrum>* baseColor;
		const Texture<Float>* roughness;
		const Texture<Float>* anisotropic;
	};

	class DisneyClearCoatMaterial : public Material
	{
	public:
		explicit DisneyClearCoatMaterial(const Texture<Float>* clearcoatGloss) : clearcoatGloss(clearcoatGloss)
		{
		}

		RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const final;

	private:
		const Texture<Float>* clearcoatGloss;
	};

	class DisneyGlassMaterial : public Material
	{
	public:
		DisneyGlassMaterial(const Texture<Spectrum>* baseColor,
			const Texture<Float>* roughness,
			const Texture<Float>* anisotropic,
			Float eta) : baseColor(baseColor),
			roughness(roughness),
			anisotropic(anisotropic),
			eta(eta)
		{
		}

		RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const final;

	private:
		const Texture<Spectrum>* baseColor;
		const Texture<Float>* roughness;
		const Texture<Float>* anisotropic;
		Float eta;
	};

	class DisneySheenMaterial : public Material
	{
	public:
		DisneySheenMaterial(const Texture<Spectrum>* baseColor,
			const Texture<Float>* sheenTint) : baseColor(baseColor),
			sheenTint(sheenTint)
		{
		}

		RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const final
		{
			Spectrum color = baseColor->Evaluate(isect);
			Float luminance = color.Luminance();
			Spectrum Ctint = luminance > 0 ? color / luminance : Spectrum(1.0f);

			Float st = sheenTint->Evaluate(isect);
			Spectrum Csheen = Spectrum(1 - st) + st * Ctint;

			BSDF bsdf(isect.ns);
			bsdf.AddComponent(GMalloc.new_object<DisneySheenBXDF>(Csheen));
			return bsdf;
		}

	private:
		const Texture<Spectrum>* baseColor;
		const Texture<Float>* sheenTint;
	};

	class DisneyMaterial : public Material
	{
	public:
		DisneyMaterial(const Texture<Spectrum>* baseColor,
			const Texture<Float>* subsurface,
			const Texture<Float>* metallic,
			const Texture<Float>* specular,
			const Texture<Float>* specularTint,
			const Texture<Float>* roughness,
			const Texture<Float>* anisotropic,
			const Texture<Float>* sheen,
			const Texture<Float>* sheenTint,
			const Texture<Float>* clearcoat,
			const Texture<Float>* clearcoatGloss,
			Float eta) : baseColor(baseColor),
			subsurface(subsurface),
			metallic(metallic),
			specular(specular),
			specularTint(specularTint),
			roughness(roughness),
			anisotropic(anisotropic),
			sheen(sheen),
			sheenTint(sheenTint),
			clearcoat(clearcoat),
			clearcoatGloss(clearcoatGloss),
			eta(eta)
		{
		}

		RAYFLOW_CPU_GPU rstd::optional<BSDF> EvaluateBSDF(const SurfaceIntersection& isect, TransportMode mode) const final;

	private:
		Float eta;
		const Texture<Spectrum>* baseColor;
		const Texture<Float>* subsurface;
		const Texture<Float>* metallic;
		const Texture<Float>* specular;
		const Texture<Float>* specularTint;
		const Texture<Float>* roughness;
		const Texture<Float>* anisotropic;
		const Texture<Float>* sheen;
		const Texture<Float>* sheenTint;
		const Texture<Float>* clearcoat;
		const Texture<Float>* clearcoatGloss;
	};

}