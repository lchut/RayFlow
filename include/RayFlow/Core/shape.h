#pragma once

#include <RayFlow/Util/transform.h>
#include <RayFlow/Core/intersection.h>
#include <RayFlow/Std/optional.h>

namespace rayflow {

struct ShapeSample {
	Intersection isect;
	Float pdfPos;
};

struct ShapeRefSample {
	Intersection isect;
	Float pdfPos;
	Float pdfDir;
};

struct ShapeIntersection {
	SurfaceIntersection isect;
	Float tHit;
};

class Shape {
public:
	RAYFLOW_CPU_GPU Shape(const Transform* ltw) : 
		mLocalToWorld_(ltw) {}

	RAYFLOW_CPU_GPU virtual ~Shape() = default;

	RAYFLOW_CPU_GPU virtual AABB3 Bounds() const = 0;

	RAYFLOW_CPU_GPU virtual rstd::optional<ShapeIntersection> Intersect(const Ray& ray, Float tMax = Infinity) const = 0;

	RAYFLOW_CPU_GPU virtual Float Area() const = 0;

	RAYFLOW_CPU_GPU virtual rstd::optional<ShapeSample> Sample(const Point2& sample) const = 0;

	RAYFLOW_CPU_GPU virtual Float Pdf(const Intersection& isect) const { return 1 / Area(); }

	RAYFLOW_CPU_GPU virtual rstd::optional<ShapeRefSample> Sample(const Intersection& ref, const Point2& sample) const;

	RAYFLOW_CPU_GPU virtual Float Pdf(const Intersection& ref, const Vector3& wi) const;

	const Transform* mLocalToWorld_;
};
}