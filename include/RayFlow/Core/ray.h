#pragma once

#include <RayFlow/Util/vecmath.h>

namespace rayflow {

class Ray {
public:
	Ray() = default;
	Ray(const Point3& p, const Vector3& d) : o(p), d(d) {}
	Point3 operator()(Float t) const { return o + t * d; }

	Point3 o;
	Vector3 d;
};
}