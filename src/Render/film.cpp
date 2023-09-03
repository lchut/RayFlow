#include <RayFlow/Render/film.h>

namespace rayflow {

FilmTile* Film::GetFilmTile(const AABB2i &sampleBounds) {
    Vector2 halfPixel(0.5f, 0.5f);
    Vector2 filterRadius = mFilter_->Radius();
    // discrete to continus pixel coordinate
    Point2 p0f = Point2f(sampleBounds.pMin) - halfPixel - filterRadius;
    Point2 p1f = Point2f(sampleBounds.pMax) - halfPixel + filterRadius;
    Point2i p0(std::ceil(p0f.x), std::ceil(p0f.y));
    Point2i p1(std::floor(p1f.x) + 1, std::floor(p1f.y) + 1);
    
    AABB2i tileBounds = Intersect(bounds, AABB2i(p0, p1));

    return mAlloc_.new_object<FilmTile>(tileBounds, filterRadius, filterTable, filterTableWidth);
}

void Film::MergeFilmTile(FilmTile* tile) {
    const AABB2i& bounds = tile->GetBounds();
    const Point2i& pMin = bounds.pMin;
    const Point2i& pMax = bounds.pMax;
    
    for (int y = pMin.y; y < pMax.y; ++y) {
        for (int x = pMin.x; x < pMax.x; ++x) {
            const Point2i pixelPos(x, y);
            Pixel& pixel = GetPixel(pixelPos);
            const FilmTilePixel& ftPixel = tile->GetPixel(pixelPos);

            pixel.rgb[0] += ftPixel.L[0];
            pixel.rgb[1] += ftPixel.L[1];
            pixel.rgb[2] += ftPixel.L[2];

            pixel.weightSum += ftPixel.weightSum;
        }
    }
}

void Film::AddSplat(const Point2f &p, Spectrum v) {
    Point2i pixlePos(::floor(p.x), ::floor(p.y));

    if (!InsideExclusive(pixlePos, bounds)) { return; }
    Pixel& pixel = GetPixel(pixlePos);

    pixel.splatRGB[0].Add(v[0]);
    pixel.splatRGB[1].Add(v[1]);
    pixel.splatRGB[2].Add(v[2]);
}

void Film::Write(Float lightImageScale)  {
    const int width = resolution.x;
    for (int y = 0; y < resolution.y; ++y) {
        for (int x = 0; x < resolution.x; ++x) {
            const Point2i pixelPos(x, y);
            Pixel& pixel = GetPixel(pixelPos);
            Float* rgb = pixel.rgb;
            const AtomicFloat* splatRGB = pixel.splatRGB;
            const Float& ws = pixel.weightSum;
            if (ws != 0) {
                rgb[0] /= ws;
                rgb[1] /= ws;
                rgb[2] /= ws;
            }

            rgb[0] += lightImageScale * splatRGB[0];
            rgb[1] += lightImageScale * splatRGB[1];
            rgb[2] += lightImageScale * splatRGB[2];

            mBitMap_->SetPixel(Point2i(x, y), Spectrum(rgb));
        }
    }

    mBitMap_->Write(mFilename_);
}   

void FilmTile::AddSample(const Point2& pFilm, const Spectrum& L, const Spectrum& importance) {
    Vector2 halfPixel(0.5f, 0.5f);
    Point2 pFilmDiscrete = pFilm - halfPixel;
    Point2 p0f = pFilm - filterRadius;
    Point2 p1f = pFilm + filterRadius;

    Point2i p0(std::ceil(p0f.x), std::ceil(p0f.y));
    Point2i p1(std::floor(p1f.x) + 1, std::floor(p1f.y) + 1);

    p0 = Max(p0, bounds.pMin);
    p1 = Min(p1, bounds.pMax);

    rstd::vector<int> ftX(p1.x - p0.x);
    for (int i = p0.x; i < p1.x; ++i) {
        float offset = std::abs((p0.x - pFilmDiscrete.x) * invFilterRadius.x * filterTableWidth);
        ftX[i - p0.x] = std::min((int)std::floor(offset), filterTableWidth - 1);
    }

    rstd::vector<int> ftY(p1.y - p0.y);
    for (int i = p0.y; i < p1.y; ++i) {
        float offset = std::abs((p0.y - pFilmDiscrete.y) * invFilterRadius.y * filterTableWidth);
        ftY[i - p0.y] = std::min((int)std::floor(offset), filterTableWidth - 1);
    }

    for (int y = p0.y; y < p1.y; ++y) {
        for (int x = p0.x; x < p1.x; ++x) {
            int offset = ftY[y - p0.y] * filterTableWidth + ftX[x - p0.x];
            Float weight = filterTable[offset];
            
            FilmTilePixel& pixel = GetPixel(Point2i(x, y));
            pixel.L += weight * importance * L;
            pixel.weightSum += weight;
        }
    }
}

}