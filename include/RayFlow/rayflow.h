#pragma once

#include <iostream>
#include <climits>
#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#define NOMINMAX
#undef min
#undef max

#undef RGB

#if defined(__CUDA_ARCH__)
#define RAYFLOW_GENERATE_GPU_CODE
#endif

#if defined(RAYFLOW_BUILD_GPU_RENDERER) && defined(__CUDACC__)
#define PBRT_CPU_GPU __host__ __device__
#define PBRT_GPU __device__
#else
#define RAYFLOW_CPU_GPU
#define RAYFLOW_GPU
#endif

#ifdef RAYFLOW_USE_FLOAT_AS_DOUBLE
using Float = double;
#else
using Float = float;
#endif // RAYFLOW_USE_FLOAT_AS_DOUBLE

#ifdef RAYFLOW_USE_FLOAT_AS_DOUBLE
using FloatBits = uint64_t;
#else
using FloatBits = uint32_t;
#endif // RAYFLOW_USE_FLOAT_AS_DOUBLE

namespace rayflow {

template <typename T>
class TVector2;
using Vector2 = TVector2<Float>;
using Vector2i = TVector2<int>;
using Vector2u = TVector2<unsigned int>;
using Vector2f = TVector2<float>;
using Vector2d = TVector2<double>;

template <typename T>
class TVector3;
using Vector3 = TVector3<Float>;
using Vector3i = TVector3<int>;
using Vector3u = TVector3<unsigned int>;
using Vector3f = TVector3<float>;
using Vector3d = TVector3<double>;

template <typename T>
class TVector4;
using Vector4 = TVector4<Float>;
using Vector4i = TVector4<int>;
using Vector4u = TVector4<unsigned int>;
using Vector4f = TVector4<float>;
using Vector4d = TVector4<double>;

template <typename T>
class TPoint2;
using Point2 = TPoint2<Float>;
using Point2i = TPoint2<int>;
using Point2u = TPoint2<unsigned int>;
using Point2f = TPoint2<float>;
using Point2d = TPoint2<double>;

template <typename T>
class TPoint3;
using Point3 = TPoint3<Float>;
using Point3i = TPoint3<int>;
using Point3u = TPoint3<unsigned int>;
using Point3f = TPoint3<float>;
using Point3d = TPoint3<double>;

template <typename T>
class TPoint4;
using Point4 = TPoint4<Float>;
using Point4i = TPoint4<int>;
using Point4u = TPoint4<unsigned int>;
using Point4f = TPoint4<float>;
using Point4d = TPoint4<double>;
template <typename T>
class TNormal3;
using Normal3 = TNormal3<Float>;
using Normal3f = TNormal3<float>;
using Normal3d = TNormal3<double>;

template <typename T>
class TAABB2;
using AABB2 = TAABB2<Float>;
using AABB2i = TAABB2<int>;
using AABB2u = TAABB2<unsigned int>;
using AABB2f = TAABB2<float>;
using AABB2d = TAABB2<double>;

template <typename T>
class TAABB3;
using AABB3 = TAABB3<Float>;
using AABB3i = TAABB3<int>;
using AABB3u = TAABB3<unsigned int>;
using AABB3f = TAABB3<float>;
using AABB3d = TAABB3<double>;

class Ray;

class Transform;
class Shape;
class TriangleMesh;
class BXDF;
class BSDF;
class Primitive;
class Material;
class Light;
class AreaLight;
class Rng;
class Scene;
class Sampler;
class LightSampler;
class Integrator;
class Intersection;
class SurfaceIntersection;

template <int N>
class TSpectrum;
class SampledSpectrum;
class RGBSpectrum;
#if defined(RAYFLOW_USE_SAMPLED_SPECTRUM)
    using Spectrum = SampledSpectrum;
#else 
    using Spectrum = RGBSpectrum;
#endif

namespace rstd {

namespace pmr {

template <typename Tp>
class polymorphic_allocator;
}
}

using Allocator = rstd::pmr::polymorphic_allocator<std::byte>;

}