#include <RayFlow/Accelerate/bvh.h>

namespace rayflow
{

    BVH::BVH(const std::vector<Primitive> &primitives, int maxPrimitivesPerNode) : maxPrimitivesPerNode(maxPrimitivesPerNode)
    {
        if (primitives.empty())
        {
            return;
        }

        std::cout << "Begin constructing BVH" << std::endl;

        std::vector<BVHPrimitive> primInfo;

        for (int i = 0; i < primitives.size(); ++i)
        {
            primInfo.push_back(BVHPrimitive(primitives[i].GetBounds(), i));
        }

        int totalNode = 0;
        BVHBuildNode *root = BuildBVH(primitives, primInfo, 0, primInfo.size(), &totalNode);
        std::cout << "Finish build BVH" << std::endl;

        mNodes_.resize(totalNode);
        int offset = 0;
        ToLinearBVH(root, &offset);
        std::cout << "Finish linearize BVH" << std::endl;
    }

    rstd::optional<ShapeIntersection> BVH::Intersect(const Ray &ray, Float tMax) const
    {

        bool hit = false;
        rstd::optional<ShapeIntersection> result;

        Vector3 invDir(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
        int isDirNeg[3];
        isDirNeg[0] = ray.d.x < 0 ? 1 : 0;
        isDirNeg[1] = ray.d.y < 0 ? 1 : 0;
        isDirNeg[2] = ray.d.z < 0 ? 1 : 0;

        int currentNodeIndex = 0;
        int toVisitOffset = 0;
        int nodeToVisit[128];

        BVHNode node;

        while (true)
        {
            node = mNodes_[currentNodeIndex];

            if (rayflow::Intersect(node.bounds, ray.o, ray.d, invDir, isDirNeg, tMax))
            {
                if (node.type == BVHNode::NodeType::Interior)
                {
                    int leftChild = node.leftChild;
                    int rightChild = node.rightChild;
                    nodeToVisit[toVisitOffset++] = rightChild;
                    currentNodeIndex = leftChild;
                }
                else
                {
                    int startIdx = node.startIndex;
                    int nPrimitives = node.nPrimitives;

                    for (int i = 0; i < nPrimitives; ++i)
                    {
                        auto primSi = mOrderedPrimitives_[startIdx + i].Intersect(ray, tMax);
                        if (primSi)
                        {
                            hit = true;
                            primSi->isect.primitive = &mOrderedPrimitives_[startIdx + i];
                            result = primSi;
                            tMax = result->tHit;
                        }
                    }

                    if (toVisitOffset == 0)
                    {
                        break;
                    }

                    currentNodeIndex = nodeToVisit[--toVisitOffset];
                }
            }
            else
            {
                if (toVisitOffset == 0)
                {
                    break;
                }

                currentNodeIndex = nodeToVisit[--toVisitOffset];
            }
        }

        if (!hit)
        {
            return {};
        }
        return result;
    }

    BVHBuildNode *BVH::BuildBVH(const std::vector<Primitive> &primitives,
                                std::vector<BVHPrimitive>& primInfo,
                                int start, int end, int *totalNode)
    {
        BVHBuildNode *node = new BVHBuildNode;
        *totalNode += 1;
        int nPrimitives = end - start;

        if (nPrimitives <= maxPrimitivesPerNode)
        {
            AABB3 bounds;

            int startIdx = mOrderedPrimitives_.size();
            for (int i = start; i < end; ++i)
            {
                bounds = Union(bounds, primInfo[i].bounds);
                mOrderedPrimitives_.push_back(primitives[primInfo[i].pid]);
            }

            node->InitLeafNode(bounds, startIdx, nPrimitives);
        }
        else
        {
            AABB3 centroidBounds;

            for (int i = start; i < end; ++i)
            {
                const auto &primBounds = primInfo[i].bounds;
                Point3 centroid = 0.5f * (primBounds.pMax + primBounds.pMin);
                centroidBounds = Union(centroidBounds, centroid);
            }

            Vector3 diagonal = centroidBounds.pMax - centroidBounds.pMin;
            int dim = MaxComponentIndex(diagonal);

            constexpr int nBuckets = 12;
            struct BucketInfo
            {
                int cnt = 0;
                AABB3 bounds;
            };
            BucketInfo buckets[nBuckets];
            Float tmin = centroidBounds.pMin[dim];
            Float tmax = centroidBounds.pMax[dim];

            if (tmin == tmax)
            {
                AABB3 bounds;

                int startIdx = mOrderedPrimitives_.size();
                for (int i = start; i < end; ++i)
                {
                    bounds = Union(bounds, primInfo[i].bounds);
                    mOrderedPrimitives_.push_back(primitives[primInfo[i].pid]);
                }

                node->InitLeafNode(bounds, startIdx, nPrimitives);

                return node;
            }

            for (int i = start; i < end; ++i)
            {
                const auto &primBounds = primInfo[i].bounds;
                Float t = 0.5f * (primBounds.pMax[dim] + primBounds.pMin[dim]);
                int bucketIdx = std::min(int(nBuckets * (t - tmin) / (tmax - tmin)), nBuckets - 1);
                buckets[bucketIdx].cnt++;
                buckets[bucketIdx].bounds = Union(buckets[bucketIdx].bounds, primBounds);
            }

            float cost[nBuckets - 1];

            for (int i = 0; i < nBuckets - 1; ++i)
            {
                int leftCnt = 0;
                int rightCnt = 0;
                AABB3 leftBounds;
                AABB3 rightBounds;

                for (int j = 0; j <= i; ++j)
                {
                    leftCnt += buckets[j].cnt;
                    leftBounds = Union(leftBounds, buckets[j].bounds);
                }

                for (int j = i + 1; j < nBuckets; ++j)
                {
                    rightCnt += buckets[j].cnt;
                    rightBounds = Union(rightBounds, buckets[j].bounds);
                }

                cost[i] = 0.125f + (leftCnt * leftBounds.SurfaceArea() + rightCnt * rightBounds.SurfaceArea()) /
                                       centroidBounds.SurfaceArea();
            }

            int splitBucketIdx = 0;
            float minCost = cost[0];

            for (int i = 1; i < nBuckets - 1; ++i)
            {
                if (minCost > cost[i])
                {
                    minCost = cost[i];
                    splitBucketIdx = i;
                }
            }

            BVHPrimitive *midPtr = std::partition(&primInfo[start], &primInfo[end - 1] + 1,
                                                  [=](const BVHPrimitive &prim)
                                                  {
                                                      const auto &primBounds = prim.bounds;
                                                      Float t = 0.5f * (primBounds.pMin[dim] + primBounds.pMax[dim]);
                                                      int bucketIdx = std::min(int(nBuckets * (t - tmin) / (tmax - tmin)), nBuckets - 1);
                                                      return bucketIdx <= splitBucketIdx;
                                                  });

            int mid = static_cast<int>(midPtr - &primInfo[0]);

            node->InitInteriorNode(
                BuildBVH(primitives, primInfo, start, mid, totalNode),
                BuildBVH(primitives, primInfo, mid, end, totalNode),
                dim);
        }
        return node;
    }

    int BVH::ToLinearBVH(BVHBuildNode *root, int *offset)
    {
        if (root == nullptr)
        {
            return -1;
        }

        int nodeIdx = *offset;
        *offset += 1;
        int leftIdx = ToLinearBVH(root->leftChild, offset);
        int rightIdx = ToLinearBVH(root->rightChild, offset);

        mNodes_[nodeIdx].bounds = root->bounds;
        if (leftIdx == -1 && rightIdx == -1)
        {
            mNodes_[nodeIdx].type = BVHNode::NodeType::Leaf;
            mNodes_[nodeIdx].startIndex = root->startIdx;
            mNodes_[nodeIdx].nPrimitives = root->nPrimitives;
        }
        else
        {
            mNodes_[nodeIdx].type = BVHNode::NodeType::Interior;
            mNodes_[nodeIdx].leftChild = leftIdx;
            mNodes_[nodeIdx].rightChild = rightIdx;
        }

        return nodeIdx;
    }

}