#include <RayFlow/Render/shapes.h>
#include <RayFlow/Render/samplers.h>

namespace rayflow {

rstd::optional<ShapeRefSample> Shape::Sample(const Intersection& ref, const Point2& sample) const {
    rstd::optional<ShapeSample> ss = Sample(sample);
    auto& si = ss->isect;
    auto wi = si.p - ref.p;
    Float pdfPos = ss->pdfPos;
    Float pdfDir = 0;

    if(LengthSquare(wi) == 0) {
        return {};
    }
    else {
        wi = Normalize(wi);
        pdfDir = pdfPos * DistanceSquare(ref.p, si.p) / AbsDot(si.ng, wi);
        if (::isnan(pdfDir)) {
            return {};
        }
    }

    return ShapeRefSample{si, pdfPos, pdfDir};
}

Float Shape::Pdf(const Intersection& ref, const Vector3& wi) const {
    Ray ray = ref.SpawnRay(wi);
    auto si = (*this).Intersect(ray);

    if (!si) { return 0; }

    const auto& hitPoint = si.value().isect;
    Float pdf = DistanceSquare(ref.p, hitPoint.p) /
                (AbsDot(hitPoint.ng, wi) * Area());

    if (::isnan(pdf)) { return 0; }

    return pdf;
}

rstd::optional<ShapeIntersection> Sphere::Intersect(const Ray& ray, Float tMax) const {
    auto worldOrigin = (*mLocalToWorld_)(Point3(0, 0, 0));
    const auto& o = ray.o;
    const auto& d = ray.d;

    Float a = Dot(d, d);
    Float b = 2 * Dot(d, o - worldOrigin);
    Float c = Dot(o - worldOrigin, o - worldOrigin) - radius * radius;

    Float x1, x2;
    if (!SolveQuadraticEquation(a, b, c, &x1, &x2)) {
        return {};
    }

    float tHit;
    if (x1 > 0) {
        tHit = x1;
    }
    else if (x2 > 0) {
        tHit = x2;
    }
    else {
        return {};
    }

    if (tHit < ShadowEpsilon || tHit >= tMax) {
        return {};
    }

    Point3 p = ray(tHit);
    Normal3 n = Normalize(Normal3(p - worldOrigin));
    
    SurfaceIntersection isect;
    isect.wo = -ray.d;
    isect.p = p;
    isect.ng = n;
    isect.ns = n;
    //isect.uv = CartesianToSphere(Inverse(*mLocalToWorld_)(Vector3(n)));
    
    return ShapeIntersection{ isect, tHit };
}

rstd::optional<ShapeSample> Sphere::Sample(const Point2& sample) const {
    Vector3 sv = UniformSampleSphere(sample);
    
    Point3 p = (*mLocalToWorld_)(Point3(sv));
    Normal3 ng = (*mLocalToWorld_)(Normal3(sv));
    Point2 texCoord = CartesianToSphere(Inverse(*mLocalToWorld_)(sv));

    return ShapeSample{ { p, ng, texCoord }, 1 / Area() };
}

rstd::optional<ShapeRefSample> Sphere::Sample(const Intersection& ref, const Point2& sample) const {
    Point3 pCenter = (*mLocalToWorld_)(Point3(0, 0, 0));
    Float dis = Distance(ref.p, pCenter);

    if (dis <= radius) {
        rstd::optional<ShapeSample> ss = Sample(sample);
        auto& si = ss->isect;
        Vector3 wi = si.p - ref.p;
        Float pdfPos = ss->pdfPos;
        Float pdfDir = 0;
        
        if (LengthSquare(wi) == 0) {
            return {};
        }
        else {
            wi = Normalize(wi);
            pdfDir = pdfPos * DistanceSquare(si.p, ref.p) / 
                    (AbsDot(wi, si.ng) * Area());
            if (::isnan(pdfDir)) {
                return {};
            }
        }
        return ShapeRefSample{si, pdfPos, pdfDir};
    }
    else {
        Float sinMaxTheta = radius / dis;
        Float cosMaxTheta = ::sqrt(1 - sinMaxTheta * sinMaxTheta);
        Vector3 sv = Normalize((*mLocalToWorld_)(UniformSampleCone(sample, cosMaxTheta)));

        Ray ray = ref.SpawnRay(sv);
        const auto& o = ray.o;
        const auto& d = ray.d;
        Vector3 co = o - pCenter;

        Float a = Dot(d, d);
        Float b = 2 * Dot(d, co);
        Float c = Dot(co, co) - radius * radius;

        Float x1, x2;
        SolveQuadraticEquation(a, b, c, &x1, &x2);

        Point3 p = ray(x1);
        Normal3 n = Normalize(Normal3(p - pCenter));
        //Point2 uv = CartesianToSphere(Inverse(*mLocalToWorld_)(Vector3(n)));
        
        Float pdfPos = Inv2Pi / (radius * radius * (1 - cosMaxTheta));
        Float pdfDir = pdfPos * DistanceSquare(ref.p, p) / AbsDot(n, ray.d);
        
        if (::isnan(pdfDir)) {
            return {};
        }
        
        return ShapeRefSample{ { p, n }, pdfPos, pdfDir };
    }
}

Float Sphere::Pdf(const Intersection& ref, const Vector3& wi) const {
    Point3 o = (*mLocalToWorld_)(Point3(0, 0, 0));
    Float dis = Distance(ref.p, o);

    if (dis <= radius) {
        return Shape::Pdf(ref, wi);
    }

    Float sinMaxTheta = radius / dis;
    Float cosMaxTheta = ::sqrt(1 - sinMaxTheta * sinMaxTheta);
    return UniformSampleConePdf(cosMaxTheta);
}

TriangleMeshObject::TriangleMeshObject(const Transform& trans, const std::vector<Point3>& pos,
                       const std::vector<Normal3>& n, const std::vector<Point2>& tex,
                       const std::vector<int>& fid, Allocator& alloc) {

    if (!fid.empty()) {
        faceIndices = (int*)alloc.allocate(sizeof(int) * fid.size());
        memcpy(faceIndices, fid.data(), sizeof(int) * fid.size());
    }

    positions = alloc.allocate_object<Point3>(pos.size());
    for (auto i = 0; i < pos.size(); ++i) {
        positions[i] = trans(pos[i]);
    }

    normals = alloc.allocate_object<Normal3>(n.size());
    for (auto i = 0; i < n.size(); ++i) {
        normals[i] = trans(Normalize(n[i]));
    }

    texCoords = alloc.allocate_object<Point2>(tex.size());
    for (auto i = 0; i < tex.size(); ++i) {
        texCoords[i] = tex[i];
    }
}

rstd::optional<ShapeIntersection> Triangle::Intersect(const Ray& ray, Float tMax) const {
    const auto& o = ray.o;
    const auto& d = ray.d;

    const int* v = &(mObject_->faceIndices[triIndex]);
    const auto& p1 = mObject_->positions[v[0]];
    const auto& p2 = mObject_->positions[v[3]];
    const auto& p3 = mObject_->positions[v[6]];
    
    Vector3 e1 = p2 - p1;
    Vector3 e2 = p3 - p1;

    // [vec3(p1 - p2), vec3(p1 - p3), d]
    MatrixNxN<3> A = MatrixNxN<3>(e1.x, e2.x, -d.x, 
                                  e1.y, e2.y, -d.y, 
                                  e1.z, e2.z, -d.z);

    if (Determinant(A) == 0) {
        return {};
    }

    Vector3 tmp = o - p1;
    rstd::optional<MatrixNxN<3>> invA = Inverse(A);
    
    if (!invA) {
        return {};
    }

    Vector3 x = (*invA) * tmp;
    float alpha = 1.0 - x[0] - x[1];
    float beta = x[0];
    float gamma = x[1];
    float tHit = x[2];

    if (tHit < ShadowEpsilon || tHit >= tMax) {
        return {};
    }

    if (alpha >= 0.0 && beta >= 0.0 && gamma >= 0.0) {
        const auto& n1 = mObject_->normals[v[1]];
        const auto& n2 = mObject_->normals[v[4]];
        const auto& n3 = mObject_->normals[v[7]];
        const auto& tex1 = mObject_->texCoords[v[2]];
        const auto& tex2 = mObject_->texCoords[v[5]];
        const auto& tex3 = mObject_->texCoords[v[8]];
        
        
        SurfaceIntersection isect;
        isect.wo = -ray.d;
        isect.p = alpha * p1 + beta * p2 + gamma * p3;
        isect.ng = Normal3(Normalize(Cross(e1, e2)));
        isect.ns = Normalize(alpha * n1 + beta * n2 + gamma * n3);
        isect.uv = alpha * tex1 + beta * tex2 + gamma * tex3;
        return ShapeIntersection{ isect, tHit };
    } 
    else {
        return {};
    }
}

rstd::optional<ShapeSample> Triangle::Sample(const Point2& sample) const {
    const int* v = &(mObject_->faceIndices[triIndex]);
    const auto& p1 = mObject_->positions[v[0]];
    const auto& p2 = mObject_->positions[v[3]];
    const auto& p3 = mObject_->positions[v[6]];
    const auto& n1 = mObject_->normals[v[1]];
    const auto& n2 = mObject_->normals[v[4]];
    const auto& n3 = mObject_->normals[v[7]];
    const auto& tex1 = mObject_->texCoords[v[2]];
    const auto& tex2 = mObject_->texCoords[v[5]];
    const auto& tex3 = mObject_->texCoords[v[8]];

    Point2 sp = UniformSampleTriangle(sample);

    Float alpha = 1 - sp[0] - sp[1];
    Float beta = sp[0];
    Float gamma = sp[1];

    Point3 p = alpha * p1 + beta * p2 + gamma * p3;
    Normal3 ng = Normalize(alpha * n1 + beta * n2 + gamma * n3);
    Point2 texCoord = alpha * tex1 + beta * tex2 + gamma * tex3;

    return ShapeSample{ { p, ng, texCoord }, 1 / Area() };
}

}