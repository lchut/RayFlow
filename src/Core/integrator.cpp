#include <RayFlow/Core/integrator.h>
#include <algorithm>

namespace rayflow {

void SamplingIntegrator::Render(const Scene& scene) {
    Preprocess(scene, *mSampler_);

    Film* film = mCamera_->mFilm_;
    Point2i resolution = film->resolution;
    AABB2i sampledBounds = mCamera_->mSampleBounds_;
    int filmTileWidth = 16;

    int tileXCount = ((resolution.x + filmTileWidth - 1) / filmTileWidth);
    int tileYCount = ((resolution.y + filmTileWidth - 1) / filmTileWidth);
    Point2i tileCount(tileXCount, tileYCount);

    Scheduler::Parallel2D(tileCount,
        [&](Point2i tile) {
            ResetGMalloc();

            int seed = tile.y * tileCount.x + tile.x;
            Sampler* sampler = mSampler_->Clone(seed);

            int x0 = sampledBounds.pMin.x + tile.x * filmTileWidth;
            int x1 = std::min<int>(x0 + filmTileWidth, sampledBounds.pMax.x);
            int y0 = sampledBounds.pMin.y + tile.y * filmTileWidth;
            int y1 = std::min<int>(y0 + filmTileWidth, sampledBounds.pMax.y);
            FilmTile* filmTile = film->GetFilmTile(AABB2i(Point2i(x0, y0), Point2i(x1, y1)));

            for (int y = y0; y < y1; ++y) {
                for (int x = x0; x < x1; ++x) {
                    Point2i pRaster(x, y);
                    size_t sampleCount = sampler->GetSampleCount();
                    sampler->StartPixel(pRaster);
                    //if (x == 409 && y == 477) {
                    //    std::cout << 1 << std::endl;
                    //}
                    //else {
                    //    continue;
                    //}
                    for (int i = 0; i < sampleCount; ++i) {
                        Point2 pFilm = Point2(pRaster) + sampler->Get2D();
                        CameraRaySample raySample = mCamera_->GenerateRay(pFilm, sampler->Get2D());
                        Spectrum L = raySample.weight * Li(raySample.ray, scene, *sampler);

                        if (L.HasNaN()) {

                            std::cout << "Not-a-number radiance value returned for pixel ("
                                << x << "," << y << "), sample" << (int)sampler->GetCurrentSampleNumber()
                                << ".Setting to black.\n";

                            L = Spectrum(0.f);
                        }
                        else if (L.HasINF()) {
                            std::cout << "Not-a-number radiance value returned for pixel ("
                                << x << "," << y << "), sample" << (int)sampler->GetCurrentSampleNumber()
                                << ".Setting to black.\n";
                            L = Spectrum(0.f);
                        }

                        filmTile->AddSample(pFilm, L);

                        sampler->Advance();
                        //ResetGMalloc();
                    }
                }
            }

            film->MergeFilmTile(filmTile);
        }
    );

    film->Write();
}

}