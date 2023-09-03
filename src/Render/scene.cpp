#include <RayFlow/Render/scene.h>
#include <RayFlow/Render/primitive.h>


namespace rayflow {
rstd::optional<ShapeIntersection> Scene::Intersect(const Ray& ray, Float tMax) const {
    return  mBVH_.Intersect(ray, tMax);
}

SampledLight Scene::SampleLight(const Point3& p, Float u) const {
    return mLightSampler_->Sample(p, u);
}

const std::vector<Light*>& Scene::GetLights() const {
    return mLightSampler_->GetLights();
}

const LightSampler* Scene::GetLightSampler() const {
    return mLightSampler_;
}

}