#include "RayFlow/Engine/parse.h"

namespace rayflow
{
    bool ConfigFileParser::Parse(const std::string &filename)
    {
        Allocator &allocator = engine->mAlloc_;
        assetPath = filename.substr(0, filename.find_last_of('/')) + "/";
        tinyxml2::XMLDocument xmlDoc;
        tinyxml2::XMLError error = xmlDoc.LoadFile(filename.c_str());

        if (error != 0)
        {
            std::cout << xmlDoc.ErrorStr() << std::endl;
            std::cout << "File to load scene config file: " << filename << std::endl;
            return false;
        }

        tinyxml2::XMLElement *sceneNode = xmlDoc.FirstChildElement("scene");

        // default
        tinyxml2::XMLElement *defaultNode = sceneNode->FirstChildElement("default");
        std::string integratorType = defaultNode->Attribute("value");
        defaultNode = defaultNode->NextSiblingElement();
        int spp = ParseNumber<int>(defaultNode->Attribute("value"));
        defaultNode = defaultNode->NextSiblingElement();
        int resy = ParseNumber<int>(defaultNode->Attribute("value"));
        defaultNode = defaultNode->NextSiblingElement();
        int resx = ParseNumber<int>(defaultNode->Attribute("value"));
        defaultNode = defaultNode->NextSiblingElement();
        int maxDepth = ParseNumber<int>(defaultNode->Attribute("value"));

        printf("Integrator: %s\nspp: %d\nresx: %d\nresy: %d\nmax depth:%d\n", integratorType.c_str(), spp, resx, resy, maxDepth);

        // camera
        tinyxml2::XMLElement *cameraNode = sceneNode->FirstChildElement("sensor");
        Float fov = ParseNumber<Float>(cameraNode->FirstChildElement("float")->Attribute("value"));
        Transform *cameraToWorld = allocator.new_object<Transform>(ParseTransform(cameraNode->FirstChildElement("transform")->FirstChildElement("matrix")->Attribute("value")));
        Transform *worldToCamera = allocator.new_object<Transform>(Inverse(*cameraToWorld));
        Point2i resolution(resx, resy);
        AABB2i filmBounds = AABB2i(Point2i(0, 0), Point2i(resx, resy));
        Filter *filter = allocator.new_object<BoxFilter>();
        Film *film = allocator.new_object<Film>(filmBounds, resolution, filter, "a.png", allocator);
        engine->AddFilm(film);
        AABB2 screenWindow;

        if (resx > resy)
        {
            screenWindow = AABB2(Point2f(-float(resx) / float(resy), -1.0f),
                                 Point2(float(resx) / float(resy), 1.0f));
        }
        else
        {
            screenWindow = AABB2(Point2f(-1.0f, -float(resy) / float(resx)),
                                 Point2(1.0f, float(resy) / float(resx)));
        }

        Camera *camera = allocator.new_object<PerspectiveCamera>(cameraToWorld, resolution, filmBounds, screenWindow, 0.0f, 1.0f, fov, film);
        engine->AddCamera(camera);
        // integrator
        int xSamples = static_cast<int>(std::sqrt(spp));
        Sampler *sampler = allocator.new_object<StratifiedSampler>(xSamples, xSamples, 12, true);

        Integrator *integrator = nullptr;
        if (integratorType == "path")
        {
            integrator = allocator.new_object<PathTracerIntegrator>(camera, sampler, maxDepth);
        }
        else if (integratorType == "bdpt")
        {
            integrator = allocator.new_object<BDPTIntegrator>(camera, sampler, maxDepth);
        }

        engine->AddIntegrator(integrator);
        // material
        std::unordered_map<std::string, Material *> sceneMaterials;

        for (tinyxml2::XMLElement *bsdfNode = sceneNode->FirstChildElement("bsdf"); bsdfNode; bsdfNode = bsdfNode->NextSiblingElement("bsdf"))
        {
            std::string type = bsdfNode->Attribute("type");
            std::string id = bsdfNode->Attribute("id");
            tinyxml2::XMLElement *attrNode = bsdfNode->FirstChildElement("bsdf");
            std::string subtype = attrNode->Attribute("type");

            if (type == "twosided")
            {
                type = subtype;
            }

            if (type == "diffuse")
            {
                tinyxml2::XMLElement* rgbNode = attrNode->FirstChildElement("rgb");
                Texture<Spectrum> *kd = ParseRGBTexture(rgbNode);
                sceneMaterials[id] = allocator.new_object<DiffuseMaterial>(kd);
            }
            else if (type == "mirror") {
                tinyxml2::XMLElement* rgbNode = attrNode->FirstChildElement("rgb");
                Texture<Spectrum> *ks = ParseRGBTexture(rgbNode);
                sceneMaterials[id] = allocator.new_object<MirrorMaterial>(ks);
            }
            else if (type == "phong")
            {
                tinyxml2::XMLElement *parmNode = attrNode->FirstChildElement();
                Texture<Spectrum> *kdTex = ParseRGBTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Spectrum> *ksTex = ParseRGBTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float>* exponent = ParseFloatTexture(parmNode);
                sceneMaterials[id] = allocator.new_object<PhongMaterial>(kdTex, ksTex, exponent);
            }
            else if (type == "conductor")
            {
                tinyxml2::XMLElement *rgbNode = attrNode->FirstChildElement("rgb");
                Spectrum eta = ParseSpectrum(rgbNode->Attribute("value"));
                rgbNode = rgbNode->NextSiblingElement();
                Spectrum etaK = ParseSpectrum(rgbNode->Attribute("value"));
                Texture<Spectrum> *etaTex = allocator.new_object<ConstantTexture<Spectrum>>(eta);
                Texture<Spectrum> *etaKTex = allocator.new_object<ConstantTexture<Spectrum>>(etaK);
                sceneMaterials[id] = allocator.new_object<ConductorMaterial>(etaTex, etaKTex);
            }
            else if (type == "dielectric")
            {
                Texture<Spectrum> *ksTex = allocator.new_object<ConstantTexture<Spectrum>>(Spectrum(1.0f));
                Texture<Spectrum> *ktTex = allocator.new_object<ConstantTexture<Spectrum>>(Spectrum(1.0f));
                Float eta = ParseNumber<Float>(attrNode->FirstChildElement("float")->Attribute("value"));
                sceneMaterials[id] = allocator.new_object<DielectricMaterial>(ksTex, ktTex, eta);
            }
            else if (type == "roughconductor")
            {
                Texture<Spectrum> *ks = allocator.new_object<ConstantTexture<Spectrum>>(Spectrum(1.0f));
                tinyxml2::XMLElement *parmNode = attrNode->FirstChildElement();
                Texture<Float> *roughnessx = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *roughnessy = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Spectrum> *eta = ParseRGBTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Spectrum> *etaK = ParseRGBTexture(parmNode);
                sceneMaterials[id] = allocator.new_object<RoughMetalMaterial>(ks, roughnessx, roughnessy, eta, etaK);
            }
            else if (type == "roughdielectric")
            {
                Texture<Spectrum> *ks = allocator.new_object<ConstantTexture<Spectrum>>(Spectrum(1.0f));
                Texture<Spectrum> *kd = allocator.new_object<ConstantTexture<Spectrum>>(Spectrum(1.0f));
                tinyxml2::XMLElement *parmNode = attrNode->FirstChildElement();
                Texture<Float> *roughnessx = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *roughnessy = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *eta = ParseFloatTexture(parmNode);
                sceneMaterials[id] = allocator.new_object<RoughDielectricMaterial>(ks, kd, roughnessx, roughnessy, eta);
            }
            else if (type == "roughplastic")
            {
                Texture<Spectrum> *ks = allocator.new_object<ConstantTexture<Spectrum>>(Spectrum(1.0f));
                tinyxml2::XMLElement *parmNode = attrNode->FirstChildElement();
                Texture<Spectrum>* kd = ParseRGBTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *roughness = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float>* eta = ParseFloatTexture(parmNode);
                sceneMaterials[id] = allocator.new_object<RoughPlasticMaterial>(ks, kd, roughness, eta);
            }
            else if (type == "disneymaterial")
            {
                tinyxml2::XMLElement *parmNode = attrNode->FirstChildElement();
                Float eta = ParseNumber<Float>(parmNode->Attribute("value"));
                parmNode = parmNode->NextSiblingElement();
                Texture<Spectrum> *baseColorTex = ParseRGBTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *subsurface = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *metallic = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *specular = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *specularTint = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *roughness = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *anisotropic = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *sheen = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *sheenTint = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *clearcoat = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *clearcoatGloss = ParseFloatTexture(parmNode);
                sceneMaterials[id] = allocator.new_object<DisneyMaterial>(baseColorTex, subsurface, metallic, specular,
                                                                          specularTint, roughness, anisotropic, sheen,
                                                                          sheenTint, clearcoat, clearcoatGloss, eta);
            }
            else if (type == "disneydiffuse")
            {
                tinyxml2::XMLElement *parmNode = attrNode->FirstChildElement();
                Texture<Spectrum> *baseColor = ParseRGBTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *roughness = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *subsurface = ParseFloatTexture(parmNode);
                sceneMaterials[id] = allocator.new_object<DisneyDiffuseMaterial>(baseColor, roughness, subsurface);
            }
            else if (type == "disneymetal")
            {
                tinyxml2::XMLElement *parmNode = attrNode->FirstChildElement();
                Texture<Spectrum> *baseColor = ParseRGBTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *roughness = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *anisotropic = ParseFloatTexture(parmNode);
                sceneMaterials[id] = allocator.new_object<DisneyMetalMaterial>(baseColor, roughness, anisotropic);
            }
            else if (type == "disneyclearcoat")
            {
                tinyxml2::XMLElement *parmNode = attrNode->FirstChildElement();
                Texture<Float> *clearcoatGloss = ParseFloatTexture(parmNode);
                sceneMaterials[id] = allocator.new_object<DisneyClearCoatMaterial>(clearcoatGloss);
            }
            else if (type == "disneyglass")
            {
                tinyxml2::XMLElement *parmNode = attrNode->FirstChildElement();
                Float eta = ParseNumber<Float>(parmNode->Attribute("value"));
                parmNode = parmNode->NextSiblingElement();
                Texture<Spectrum> *baseColor = ParseRGBTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *roughness = ParseFloatTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *anisotropic = ParseFloatTexture(parmNode);
                sceneMaterials[id] = allocator.new_object<DisneyGlassMaterial>(baseColor, roughness, anisotropic, eta);
            }
            else if (type == "disneysheen")
            {
                tinyxml2::XMLElement *parmNode = attrNode->FirstChildElement();
                Texture<Spectrum> *baseColor = ParseRGBTexture(parmNode);
                parmNode = parmNode->NextSiblingElement();
                Texture<Float> *sheenTint = ParseFloatTexture(parmNode);
                sceneMaterials[id] = allocator.new_object<DisneySheenMaterial>(baseColor, sheenTint);
            }
            else
            {
                std::cout << "ERROR::Unspported Material [ " << type << "]\n";
            }
        }
        // object
        struct Model
        {
            std::vector<Point3> positions;
            std::vector<Normal3> normals;
            std::vector<Point2> texCoords;
            std::vector<int> fid;
        };

        std::vector<Primitive> scenePrimitives;
        std::vector<Light *> sceneLights;
        std::unordered_map<std::string, std::vector<Model *>> sceneModels;

        for (tinyxml2::XMLElement *shapeNode = sceneNode->FirstChildElement("shape"); shapeNode; shapeNode = shapeNode->NextSiblingElement("shape"))
        {
            std::string type = shapeNode->Attribute("type");

            if (type == "sphere")
            {
                tinyxml2::XMLElement *parmNode = shapeNode->FirstChildElement();
                Transform *localToWorld = allocator.new_object<Transform>(ParseTransform(parmNode->FirstChildElement("matrix")->Attribute("value")));
                Transform *worldToLocal = allocator.new_object<Transform>(Inverse(*localToWorld));
                parmNode = parmNode->NextSiblingElement();
                Float radius = ParseNumber<Float>(parmNode->Attribute("value"));
                parmNode = parmNode->NextSiblingElement();
                Material *material = sceneMaterials[parmNode->Attribute("id")];
                parmNode = parmNode->NextSiblingElement();
                Shape *sphere = allocator.new_object<Sphere>(localToWorld, radius);
                AreaLight *areaLight = nullptr;

                if (parmNode)
                {
                    Spectrum L = ParseSpectrum(parmNode->FirstChildElement("rgb")->Attribute("value"));
                    areaLight = allocator.new_object<AreaLight>(worldToLocal, localToWorld, sphere, L);
                }

                scenePrimitives.push_back(Primitive(sphere, material, areaLight));
            }
            else if (type == "obj")
            {
                tinyxml2::XMLElement *parmNode = shapeNode->FirstChildElement();
                std::string filepath = assetPath + parmNode->Attribute("value");
                // load obj file
                tinyobj::attrib_t attrib;
                std::vector<tinyobj::shape_t> shapes;
                std::vector<tinyobj::material_t> materials;
                std::string warning;
                std::string error;

                bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, filepath.c_str());

                if (!warning.empty())
                {
                    std::cout << warning << std::endl;
                }

                if (!error.empty())
                {
                    std::cerr << error << std::endl;
                }

                if (!ret)
                {
                    return false;
                }

                for (size_t s = 0; s < shapes.size(); ++s)
                {
                    const auto& vData = attrib.vertices;
                    const auto& nData = attrib.normals;
                    const auto& texData = attrib.texcoords;
                    Model *model = allocator.new_object<Model>();
                    auto &positions = model->positions;
                    auto &normals = model->normals;
                    auto &texCoords = model->texCoords;
                    auto &fid = model->fid;

                    for (size_t i = 0; i < shapes[s].mesh.indices.size(); ++i) {
                        const auto& idx = shapes[s].mesh.indices[i];
                        fid.push_back(idx.vertex_index);
                        fid.push_back(idx.normal_index);
                        fid.push_back(idx.texcoord_index);
                    }

                    for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
                        positions.push_back(Point3(Float(vData[i]), Float(vData[i + 1]), Float(vData[i + 2])));
                    }

                    for (size_t i = 0; i < attrib.normals.size(); i += 3) {
                        normals.push_back(Normal3(Float(nData[i]), Float(nData[i + 1]), Float(nData[i + 2])));
                    }

                    for (size_t i = 0; i < attrib.texcoords.size(); i += 2) {
                        texCoords.push_back(Point2(Float(texData[i]), Float(texData[i + 1])));
                    }

                    sceneModels[filepath].push_back(model);
                }

                parmNode = parmNode->NextSiblingElement();
                Transform *localToWorld = allocator.new_object<Transform>(ParseTransform(parmNode->FirstChildElement("matrix")->Attribute("value")));
                Transform *worldToLocal = allocator.new_object<Transform>(Inverse(*localToWorld));
                parmNode = parmNode->NextSiblingElement();
                Material *material = sceneMaterials[parmNode->Attribute("id")];
                parmNode = parmNode->NextSiblingElement();
                const std::vector<Model*>& models = sceneModels[filepath];

                if (parmNode)
                {
                    Spectrum L = ParseSpectrum(parmNode->FirstChildElement("rgb")->Attribute("value"));

                    for (Model* model : models) 
                    {
                        TriangleMeshObject* meshobj = allocator.new_object<TriangleMeshObject>(*localToWorld, model->positions, 
                                                                                                model->normals, model->texCoords, 
                                                                                                model->fid, allocator);

                        for (int f = 0; f < model->fid.size(); f += 9) {
                            Shape* tri = allocator.new_object<Triangle>(meshobj, f);
                            AreaLight* areaLight = allocator.new_object<AreaLight>(worldToLocal, localToWorld, tri, L);
                            scenePrimitives.push_back(Primitive(tri, material, areaLight));
                            sceneLights.push_back(areaLight);
                        }
                    }
                }
                else
                {
                    for (Model* model : models) 
                    {
                        TriangleMeshObject* meshobj = allocator.new_object<TriangleMeshObject>(*localToWorld, model->positions, 
                                                                                                model->normals, model->texCoords, 
                                                                                                model->fid, allocator);

                        for (int f = 0; f < model->fid.size(); f += 9) {
                            Shape* tri = allocator.new_object<Triangle>(meshobj, f);
                            scenePrimitives.push_back(Primitive(tri, material));
                        }
                    }
                }
            }
        }

        for (tinyxml2::XMLElement *lightNode = xmlDoc.FirstChildElement("light"); lightNode; lightNode = lightNode->NextSiblingElement("light"))
        {
            std::string type = lightNode->Attribute("type");

            if (type == "PointLight")
            {
                tinyxml2::XMLElement *parmNode = lightNode->FirstChildElement();
                Point3 pos = Point3(ParseVector3<Float>(parmNode->Attribute("value")));
                parmNode = parmNode->NextSiblingElement();
                Spectrum I = ParseSpectrum(parmNode->Attribute("value"));
                Transform *localToWorld = allocator.new_object<Transform>(Translate(Vector3(pos.x, pos.y, pos.z)));
                Transform *worldToLocal = allocator.new_object<Transform>(Inverse(*localToWorld));
                sceneLights.push_back(allocator.new_object<PointLight>(worldToLocal, localToWorld, I));
            }
            else if (type == "SpotLight")
            {
                tinyxml2::XMLElement *parmNode = lightNode->FirstChildElement();
                Point3 pos = Point3(ParseVector3<Float>(parmNode->Attribute("value")));
                parmNode = parmNode->NextSiblingElement();
                Spectrum I = ParseSpectrum(parmNode->Attribute("value"));
                parmNode = parmNode->NextSiblingElement();
                Float maxTheta = ParseNumber<Float>(parmNode->Attribute("value"));
                parmNode = parmNode->NextSiblingElement();
                Float falloff = ParseNumber<Float>(parmNode->Attribute("value"));
                Transform *localToWorld = allocator.new_object<Transform>(Translate(Vector3(pos.x, pos.y, pos.z)));
                Transform *worldToLocal = allocator.new_object<Transform>(Inverse(*localToWorld));
                sceneLights.push_back(allocator.new_object<SpotLight>(worldToLocal, localToWorld, I, maxTheta, falloff));
            }
        }

        engine->mScene_ = allocator.new_object<Scene>(scenePrimitives, sceneLights);

        return true;
    }

}