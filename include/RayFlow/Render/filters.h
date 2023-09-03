#pragma once 

#include <RayFlow/Util/vecmath.h>

namespace rayflow {

enum class FilterType {
    Box,
    Gaussian
};


class Filter {
public:
    RAYFLOW_CPU_GPU Filter(const Vector2& radius) : radius(radius) {}

    RAYFLOW_CPU_GPU virtual ~Filter() = default;

    RAYFLOW_CPU_GPU Vector2 Radius() const { return radius; }
    
    RAYFLOW_CPU_GPU virtual Float Evaluate(const Point2& p) const = 0;

    RAYFLOW_CPU_GPU virtual Float Integral() const = 0;

protected:
    Vector2 radius;
};


class BoxFilter : public Filter {
public:
    BoxFilter(const Vector2& radius = Vector2f(0.5, 0.5)) : Filter(radius) {}

    RAYFLOW_CPU_GPU Float Evaluate(const Point2& p) const final {
        return (::abs(p.x) <= radius.x && ::abs(p.y) <= radius.y) ? 1 : 0;
    }

    RAYFLOW_CPU_GPU Float Integral() const final {
        return 2 * radius.x * 2 * radius.y;
    }
};

class GaussianFilter : public Filter {
public:
    GaussianFilter(const Vector2& radius, Float sigma = 0.5) :
        Filter(radius), 
        sigma(sigma),
        expX(Gaussian(radius.x, 0, sigma)),
        expY(Gaussian(radius.y, 0, sigma)) {

    }

    RAYFLOW_CPU_GPU Float Evaluate(const Point2& p) const final {
        return (std::max<Float>(0, Gaussian(p.x, 0, sigma) - expX) *
                std::max<Float>(0, Gaussian(p.y, 0, sigma) - expY));
    }

    RAYFLOW_CPU_GPU Float Integral() const final {
        return ((GaussianIntegral(-radius.x, radius.x, 0, sigma) - 2 * radius.x * expX) *
                (GaussianIntegral(-radius.y, radius.y, 0, sigma) - 2 * radius.y * expY));
    }

private:
    Float sigma;
    Float expX;
    Float expY;
};

}