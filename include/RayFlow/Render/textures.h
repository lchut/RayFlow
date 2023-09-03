#pragma once
#include <RayFlow/Core/texture.h>
#include <RayFlow/Core/spectrum.h>
#include <RayFlow/Render/texturemappings.h>
#include <RayFlow/Util/bitmap.h>
#include <RayFlow/Util/half.h>

namespace rayflow {

template <typename T>
class ConstantTexture : public Texture<T> {
public:
    explicit ConstantTexture(const T& value) : value(value) {

    }

    RAYFLOW_CPU_GPU T Evaluate(const SurfaceIntersection& isect) const final {
        return value;
    }

private:
    const T value;
};

template <typename T, typename U>
class ScaleTexture : public Texture<U> {
public:
    ScaleTexture(const Texture<T>* tex1, const Texture<U>* tex2) :
        tex1(tex1), tex2(tex2) {

    }

    RAYFLOW_CPU_GPU U Evaluate(const SurfaceIntersection& isect) const final {
        return tex1->Evaluate(isect) * tex2->Evaluate(isect);
    }

private:
    const Texture<T>* tex1;
    const Texture<U>* tex2;
};

template <typename T>
class MixTexture : public Texture<T> {
public:     
    MixTexture(const Texture<T>* tex1, const Texture<T>* tex2, const Texture<Float>* coeff) :
        tex1(tex1), tex2(tex2), coeff(coeff) {

    }

    RAYFLOW_CPU_GPU T Evaluate(const SurfaceIntersection& isect) const final {
        Float u = coeff->Evaluate(isect);
        return (1 - u) * tex1->Evaluate(isect) + u * tex2->Evaluate(isect);
    }

private:
    const Texture<T>* tex1;
    const Texture<T>* tex2;
    const Texture<Float>* coeff;
};

template <typename T>
class BiLerpTexture : public Texture<T> {
public:
    BiLerpTexture(const TextureMapping* mapping, const T& v00,
                  const T& v01, const T& v10, const T& v11) :
                mapping(mapping), v00(v00), v01(v01), v10(v10), v11(v11) {

    }

    RAYFLOW_CPU_GPU T Evaluate(const SurfaceIntersection& isect) const final {
        Point2 st = mapping->Map(isect);
        return (1 - st[0]) * (1 - st[1]) * v00 + (1 - st[0]) * st[1] * v01 + 
               st[0] * (1 - st[1]) * v10 + st[0] * st[1] * v11;
    }

private:
    const TextureMapping* mapping;
    const T v00, v01, v10, v11;
};

enum class ImageTextureWrapMode {
    REPEAT,
    CLAMP
};

enum class ImageTextureFilterType {
    NEAREST,
    BILINEAR
};

template <typename T>
class ImageTexture : public Texture<T> {
public:
    ImageTexture(const TextureMapping* mapping, const BitMap* image,
                 ImageTextureWrapMode wrapmode = ImageTextureWrapMode::REPEAT, 
                 ImageTextureFilterType filterType = ImageTextureFilterType::BILINEAR) :
        mMapping_(mapping), 
        mImage_(image), 
        mResolution_(image->GetResolution()),
        mWrapMode_(wrapmode),
        mFilterType_(filterType) {

    }

    RAYFLOW_CPU_GPU T Evaluate(const SurfaceIntersection& isect) const final {
        Point2 pixelPos = mMapping_->Map(isect);
        return GetTexelValue(pixelPos);
    }

private:
    Point2i GetWrapTexCoord(const Point2i& st) const {
        Point2i result(st);
        
        if (mWrapMode_ == ImageTextureWrapMode::CLAMP) {
            result.x = Clamp(result.x, 0, mResolution_.x - 1);
            result.y = Clamp(result.y, 0, mResolution_.y - 1);
        }
        else if (mWrapMode_ == ImageTextureWrapMode::REPEAT) {
            result.x = Mod(result.x, mResolution_.x);
            result.y = Mod(result.y, mResolution_.y);
        }
        
        return result;
    }

    T GetTexelValue(const Point2& st) const {
        if (mFilterType_ == ImageTextureFilterType::BILINEAR) {
            Float stx = ::floor(st.x * mResolution_.x);
            Float sty = ::floor(st.y * mResolution_.y);

            int x = ::floor(stx);
            int y = ::floor(sty);

            Float dx = stx - x;
            Float dy = sty - y;

            return (1 - dx) * (1 - dy) * mImage_->GetPixel<T>(GetWrapTexCoord(Point2i(x, y))) +
                   dx * (1 - dy)       * mImage_->GetPixel<T>(GetWrapTexCoord(Point2i(x + 1, y))) + 
                   (1 - dx) * dy       * mImage_->GetPixel<T>(GetWrapTexCoord(Point2i(x, y + 1))) + 
                   dx * dy             * mImage_->GetPixel<T>(GetWrapTexCoord(Point2i(x + 1, y + 1)));
        }
        else {
            Point2i sst(::floor(st.x * mResolution_.x), ::floor(st.y * mResolution_.y));
            return mImage_->GetPixel<T>(GetWrapTexCoord(sst));
        }
    }

private:
    const TextureMapping* mMapping_;
    const BitMap* mImage_;
    Point2i mResolution_;
    ImageTextureWrapMode mWrapMode_;
    ImageTextureFilterType mFilterType_;
};

template <typename T>
class CheckBoard2DTexture : Texture<T> {
public:
    CheckBoard2DTexture(const TextureMapping* mapping, 
                        const Texture<T>* tex1, 
                        const Texture<T>* tex2) : 
                        mapping(mapping),
                        tex1(tex1),
                        tex2(tex2) {

    }

    RAYFLOW_CPU_GPU T Evaluate(const SurfaceIntersection& isect) const final {
        Point2 st = mapping->Map(isect);
        if ((int(::floor(st[0])) + int(::floor(st[1]))) % 2 == 0) {
            return tex1->Evaluate(isect);
        }
        else {
            return tex2->Evaluate(isect);
        }
    }


private:
    const TextureMapping* mapping;
    const Texture<T>* tex1;
    const Texture<T>* tex2;
};

}