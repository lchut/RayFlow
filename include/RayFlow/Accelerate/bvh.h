#pragma once

#include <RayFlow/Util/vecmath.h>
#include <RayFlow/Core/ray.h>
#include <RayFlow/Core/intersection.h>
#include <RayFlow/Core/shape.h>
#include <RayFlow/Render/primitive.h>
#include <RayFlow/Std/vector.h>

#include <vector>

namespace rayflow {

struct BVHPrimitive;
struct BVHBuildNode;
struct BVHNode;

struct BVHPrimitive {
    BVHPrimitive() = default;

    BVHPrimitive(const AABB3& bounds, int idx) :
        pid(idx),
        bounds(bounds),
        centorid(0.5f * (bounds.pMax + bounds.pMin)) {
        
    }

    int pid;
    AABB3 bounds;
    Vector3 centorid;
};

struct BVHBuildNode {
    BVHBuildNode() : 
        leftChild(nullptr), rightChild(nullptr), splitAxis(0), startIdx(0), nPrimitives(0) { }

    void InitLeafNode(const AABB3& box, int sid, int n) {
        bounds = box;
        startIdx = sid;
        nPrimitives = n;
        leftChild = rightChild = nullptr;
    }

    void InitInteriorNode(BVHBuildNode* left, BVHBuildNode* right, int axis) {
        leftChild = left;
        rightChild = right;
        splitAxis = axis;
        nPrimitives = 0;
        bounds = Union(left->bounds, right->bounds);
    }

    AABB3 bounds;
    BVHBuildNode* leftChild;
    BVHBuildNode* rightChild;
    int splitAxis, startIdx, nPrimitives;
};

struct BVHNode {
public:
    enum class NodeType { Interior, Leaf };
public:
    BVHNode() : type(NodeType::Leaf), leftChild(-1), rightChild(-1) { }

    AABB3 bounds;
    NodeType type;
    union {
        struct {
            int leftChild;
            int rightChild;
        };
        struct {
            int startIndex;
            int nPrimitives;
        };
    };
};

class BVH {
public:
    BVH(const std::vector<Primitive>& primitives, int maxPrimitivesPerNode = 4);

    rstd::optional<ShapeIntersection> Intersect(const Ray& ray, Float tMax = Infinity) const;
    
private:

    BVHBuildNode* BuildBVH(const std::vector<Primitive>& primitives, 
                        std::vector<BVHPrimitive>& primInfo,
                        int start, int end, int* totalNode);


    int ToLinearBVH(BVHBuildNode* root, int* offset);

    std::vector<BVHNode> mNodes_;
    std::vector<Primitive> mOrderedPrimitives_;
    const int maxPrimitivesPerNode;
};

}