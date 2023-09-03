#pragma once
#include <RayFlow/Engine/fwd.h>
#include <RayFlow/Std/vector.h>
#include <RayFlow/Util/color.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <unordered_map>
#include <chrono>

namespace rayflow
{

    class RayFlowEngine
    {
    public:
        RayFlowEngine() = default;

        bool Init(const std::string &filename);

        void Render();

        void AddCamera(Camera *camera)
        {
            mCamera_ = camera;
        }

        void AddFilm(Film *film)
        {
            mFilm_ = film;
        }

        void AddScene(Scene *scene)
        {
            mScene_ = scene;
        }

        void AddIntegrator(Integrator *integrator)
        {
            mIntegrator_ = integrator;
        }

        void AddMeshObject(const std::string &name, TriangleMeshObject *obj)
        {
            mMeshObjects_[name] = obj;
        }

        void AddBitMap(const std::string &name, BitMap *bitmap)
        {
            mImages_[name] = bitmap;
        }

        BitMap *GetBitMap(const std::string &name)
        {
            if (mImages_.find(name) == mImages_.end())
            {
                return nullptr;
            }

            return mImages_[name];
        }

    private:
        bool ReadScene(const std::string &configFileName);

        bool ReadMeshFile(const std::string &objFileName,
                          const std::unordered_map<std::string, Spectrum> &lightsPara,
                          std::unordered_map<std::string, Material *> &sceneMaterials,
                          std::vector<Primitive> &scenePrimitives,
                          std::vector<Light *> &sceneLights);

        void ProcessNode(aiNode *node, const aiScene *scene,
                         const std::unordered_map<std::string, Spectrum> &lightsPara,
                         const std::unordered_map<std::string, Material *> &sceneMaterials,
                         std::vector<Primitive> &scenePrimitives,
                         std::vector<Light *> &sceneLights);

        void ProcessMesh(aiMesh *mesh, const aiScene *scene,
                         const std::unordered_map<std::string, Spectrum> &lightsPara,
                         const std::unordered_map<std::string, Material *> &sceneMaterials,
                         std::vector<Primitive> &scenePrimitives,
                         std::vector<Light *> &sceneLights);

    private:
        std::unordered_map<std::string, BitMap *> mImages_;
        std::unordered_map<std::string, TriangleMeshObject *> mMeshObjects_;
        Allocator mAlloc_;

        Camera *mCamera_;
        Film *mFilm_;
        Scene *mScene_;
        Integrator *mIntegrator_;

        friend class ConfigFileParser;
    };

    class Timer {
    public:
        Timer() {
            start = std::chrono::high_resolution_clock::now();
        }

        ~Timer() {
            std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> duration = end - start;
            float seconds = duration.count();
            std::cout << "Time cost: " << seconds << " s" << std::endl;
        }
    private:
        std::chrono::time_point<std::chrono::steady_clock> start;
    };

}