#pragma once
#include <RayFlow/Util/transform.h>
#include <RayFlow/Core/texturemapping.h>
#include <RayFlow/Core/intersection.h>

namespace rayflow {

class UVMapping : public TextureMapping {
public:
    UVMapping(Float su = 1, Float sv = 1, Float du = 0, Float dv = 0) :
        su(su), sv(sv), du(du), dv(dv) {}

    RAYFLOW_CPU_GPU Point2 Map(const Intersection& isect) const final {
        return Point2(su * isect.uv[0] + du, sv * isect.uv[1] + dv);
    }

private:
    const Float su = 1;
    const Float sv = 1;
    const Float du = 0;
    const Float dv = 0;
};

class SphericalMapping : public TextureMapping {
public:
    SphericalMapping(const Transform* wtt) : mWorldToTexture_(wtt) {}

    RAYFLOW_CPU_GPU Point2 Map(const Intersection& isect) const final {
        Vector3 v = Normalize((*mWorldToTexture_)(isect.p) - Point3(0, 0, 0));
        Point2 phiTheta = CartesianToSphere(v);

        return { phiTheta.x * Inv2Pi, phiTheta.y * InvPi };
    }

private:
    const Transform* mWorldToTexture_;
};


}