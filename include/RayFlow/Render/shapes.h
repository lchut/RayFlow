#pragma once

#include <RayFlow/Core/shape.h>
#include <RayFlow/Std/vector.h>

namespace rayflow {

class Sphere : public Shape {
public:
    RAYFLOW_CPU_GPU Sphere(const Transform* ltw, Float r) : 
        Shape(ltw),
        radius(r) {}

    RAYFLOW_CPU_GPU AABB3 Bounds() const final {
        return (*mLocalToWorld_)(AABB3(Point3(-radius, -radius, -radius), 
                                       Point3(radius, radius, radius)));
    }

    RAYFLOW_CPU_GPU rstd::optional<ShapeIntersection> Intersect(const Ray& ray, Float tMax = Infinity) const final;

	RAYFLOW_CPU_GPU Float Area() const final { return 4 * Pi * radius * radius; }

	RAYFLOW_CPU_GPU rstd::optional<ShapeSample> Sample(const Point2& sample) const final;

	RAYFLOW_CPU_GPU rstd::optional<ShapeRefSample> Sample(const Intersection& ref, const Point2& sample) const final;

	RAYFLOW_CPU_GPU Float Pdf(const Intersection& ref, const Vector3& wi) const final;

private:
    Float radius;
};

class TriangleMeshObject {
public:
    TriangleMeshObject(const Transform& trans, const std::vector<Point3>& pos,
                       const std::vector<Normal3>& n, const std::vector<Point2>& tex,
                       const std::vector<int>& fid, Allocator& alloc);
    
    Point3* positions;
    Normal3* normals;
    Point2* texCoords;
    int* faceIndices;
};

class Triangle : public Shape {
public:
    RAYFLOW_CPU_GPU Triangle(TriangleMeshObject* object, int idx) : 
        Shape(nullptr),
        mObject_(object),
        triIndex(idx) {}

    RAYFLOW_CPU_GPU AABB3 Bounds() const final {
        const int* v = &(mObject_->faceIndices[triIndex]);
        const auto& p0 = mObject_->positions[v[0]];
        const auto& p1 = mObject_->positions[v[3]];
        const auto& p2 = mObject_->positions[v[6]];

        return AABB3(Min(Min(p0, p1), p2),
                                 Max(Max(p0, p1), p2));
    }

	RAYFLOW_CPU_GPU rstd::optional<ShapeIntersection> Intersect(const Ray& ray, Float tMax = Infinity) const final;

	RAYFLOW_CPU_GPU Float Area() const final {
        const int* v = &(mObject_->faceIndices[triIndex]);
        const auto& p0 = mObject_->positions[v[0]];
        const auto& p1 = mObject_->positions[v[3]];
        const auto& p2 = mObject_->positions[v[6]];

        return 0.5 * Length(Cross(p1 - p0, p2 - p0));
    }

	RAYFLOW_CPU_GPU rstd::optional<ShapeSample> Sample(const Point2& sample) const final;

private:
    TriangleMeshObject* mObject_;
    int triIndex = -1;
};

}