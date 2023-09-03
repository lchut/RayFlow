#pragma once

#include <RayFlow/Util/vecmath.h>
#include <RayFlow/Util/bitmap.h>
#include <RayFlow/Render/filters.h>
#include <RayFlow/Std/memory_resource.h>
#include <RayFlow/Std/vector.h>
#include <RayFlow/Util/atomic_float.h>

namespace rayflow {

struct Pixel {
    Pixel() {
        rgb[0] = rgb[1] = rgb[2] = 0;
        splatRGB[0] = splatRGB[1] = splatRGB[2] = 0;
        weightSum = 0;
    }

    Float rgb[3];
    AtomicFloat splatRGB[3];
    Float weightSum;
    int padding;
};

struct FilmTilePixel {
    Spectrum L;
    Float weightSum;
};

class FilmTile;

class Film {
public:
    Film(const AABB2i& bounds, const Point2i& resolution, 
         const Filter* filter, const std::string& filename,
         Allocator alloc) :
        bounds(bounds),
        resolution(resolution),
        mFilter_(filter),
        mFilename_(filename),
        mAlloc_(alloc) {
        int pixelCount = resolution.x * resolution.y;

        mBitMap_ = new BitMap(BitMapPixelFormat::ERGB, BitMapComponentFormat::EUInt8, resolution, 
                (uint8_t*)alloc.allocate_object<Spectrum>(pixelCount));
        
        mPixels_ = new Pixel[pixelCount];

        Vector2 filterRadius = mFilter_->Radius();
        int offset = 0;
        for (int i = 0; i < filterTableWidth; ++i) {
            for (int j = 0; j < filterTableWidth; ++j) {
                Point2 p;
                p.x = (i + 0.5f) * filterRadius.x / filterTableWidth;
                p.y = (j + 0.5f) * filterRadius.y / filterTableWidth;
                filterTable[offset] = mFilter_->Evaluate(p);
                offset++;
            }
        }
    }

    FilmTile* GetFilmTile(const AABB2i &sampleBounds);

    void MergeFilmTile(FilmTile* tile);

    void AddSplat(const Point2f &p, Spectrum v);

    void Write(Float lightImageScale = 0.f);

    AABB2i GetSampledBounds() const {
        Vector2 filterRadius = mFilter_->Radius();
        Point2 pMin = Point2(bounds.pMin) + Vector2(0.5f, 0.5f) - Vector2(filterRadius.x, filterRadius.y);
        Point2 pMax = Point2(bounds.pMax) - Vector2(0.5f, 0.5f) + Vector2(filterRadius.x, filterRadius.y);
        return AABB2i(Point2i(::floor(pMin.x), ::floor(pMin.y)), Point2i(::ceil(pMax.x), ::ceil(pMax.y)));
    }

public:
    const Point2i resolution;
    const AABB2i bounds;
private:
    std::string mFilename_;

    Pixel* mPixels_;
    const Filter* mFilter_;
    BitMap* mBitMap_;
    std::mutex mutex;
    static constexpr int filterTableWidth = 16;
    Float filterTable[filterTableWidth * filterTableWidth];
    
    friend FilmTile;

    Pixel& GetPixel(const Point2i& p) {
        return mPixels_[p.y * resolution.x + p.x];
    }

    const Pixel& GetPixel(const Point2i& p) const {
        return mPixels_[p.y * resolution.x + p.x];
    }

    Allocator mAlloc_;
};

class FilmTile {    
public:

    FilmTile(const AABB2i& bounds, const Vector2& filterRadius, 
             const Float* filterTable, const int filterTableWidth) :
             bounds(bounds),
             filterRadius(filterRadius),
             invFilterRadius(Vector2(1 / filterRadius.x, 1 / filterRadius.y)),
             filterTable(filterTable),
             filterTableWidth(filterTableWidth) {
        pixels.resize(bounds.Area());
    }


    void AddSample(const Point2& pFilm, const Spectrum& L, const Spectrum& importance = Spectrum(1.0f));

    FilmTilePixel& GetPixel(const Point2i&p ) {
        int width = bounds.pMax.x - bounds.pMin.x;
        int offset = (p.x - bounds.pMin.x) + (p.y - bounds.pMin.y) * width;
        return pixels[offset];
    }

    const FilmTilePixel& GetPixel(const Point2i&p ) const {
        int width = bounds.pMax.x - bounds.pMin.x;
        int offset = (p.x - bounds.pMin.x) + (p.y - bounds.pMin.y) * width;
        return pixels[offset];
    }

    const AABB2i& GetBounds() const { return bounds; }

private:
    rstd::vector<FilmTilePixel> pixels;
    const AABB2i bounds;
    const Vector2 filterRadius;
    const Vector2 invFilterRadius;
    const Float* filterTable;
    const int filterTableWidth;

    friend class Film;
};



}