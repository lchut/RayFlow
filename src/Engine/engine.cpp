#include <RayFlow/Engine/engine.h>
#include <RayFlow/Engine/parse.h>


namespace rayflow
{

	inline void parseIntNum(const std::string& text, int* e)
	{
		*e = std::stoi(text);
	}

	inline void parseFloatNum(const std::string& text, float* e)
	{
		*e = std::stof(text);
	}

	inline void parseMultiFloatNums(const std::string& text, float* e)
	{
		int idx = 0;
		int anchor = 0;
		int eid = 0;
		while (idx < text.size())
		{
			if (std::isdigit(text[idx]) || text[idx] == '-')
			{
				anchor = idx;
				idx += 1;
				while (idx < text.size() && (std::isdigit(text[idx]) || text[idx] == '.'))
				{
					idx++;
				}
				parseFloatNum(text.substr(anchor, idx - anchor), &e[eid++]);
			}
			else
			{
				idx++;
			}
		}
	}

	bool RayFlowEngine::ReadScene(const std::string& configFileName)
	{
		std::string filepath = configFileName.substr(0, configFileName.find_last_of('/'));

		tinyxml2::XMLDocument configFileDoc;
		tinyxml2::XMLError errcode = configFileDoc.LoadFile(configFileName.c_str());
		if (errcode != 0)
		{
			std::cout << configFileDoc.ErrorStr() << std::endl;
			std::cout << "File to load scene config file: " << configFileName << std::endl;
			return false;
		}
		float ele[32];
		// camera config parameter
		int filmWidth, filmHeight;
		float fovy;
		tinyxml2::XMLElement* cameraNode = configFileDoc.FirstChildElement("camera");
		tinyxml2::XMLElement* cameraAttribNode = cameraNode->FirstChildElement();
		// eye pos
		parseMultiFloatNums(cameraAttribNode->Attribute("value"), ele);
		cameraAttribNode = cameraAttribNode->NextSiblingElement();
		// target pos
		parseMultiFloatNums(cameraAttribNode->Attribute("value"), ele + 3);
		cameraAttribNode = cameraAttribNode->NextSiblingElement();
		// up dir
		parseMultiFloatNums(cameraAttribNode->Attribute("value"), ele + 6);
		cameraAttribNode = cameraAttribNode->NextSiblingElement();
		// fovy
		parseFloatNum(cameraAttribNode->Attribute("value"), &fovy);
		cameraAttribNode = cameraAttribNode->NextSiblingElement();
		// width
		parseIntNum(cameraAttribNode->Attribute("value"), &filmWidth);
		cameraAttribNode = cameraAttribNode->NextSiblingElement();
		// height
		parseIntNum(cameraAttribNode->Attribute("value"), &filmHeight);
		cameraAttribNode = cameraAttribNode->NextSiblingElement();

		Point2i resolution = Point2i(filmWidth, filmHeight);
		AABB2i filmBounds = AABB2i(Point2i(0, 0), Point2i(filmWidth, filmHeight));
		Filter* filter = mAlloc_.new_object<BoxFilter>();
		mFilm_ = mAlloc_.new_object<Film>(filmBounds, resolution, filter, "test.png", mAlloc_);

		AABB2 screenWindow;
		if (filmWidth > filmHeight)
		{
			screenWindow = AABB2(Point2f(-float(filmWidth) / float(filmHeight), -1.0f),
				Point2(float(filmWidth) / float(filmHeight), 1.0f));
		}
		else
		{
			screenWindow = AABB2(Point2f(-1.0f, -float(filmHeight) / float(filmWidth)),
				Point2(1.0f, float(filmHeight) / float(filmWidth)));
		}
		Transform* worldToCamera = mAlloc_.new_object<Transform>(LookAt(Point3(ele[0], ele[1], ele[2]),
			Point3(ele[3], ele[4], ele[5]),
			Vector3(ele[6], ele[7], ele[8])));
		Transform* cameraToWorld = mAlloc_.new_object<Transform>(Inverse(*worldToCamera));
		mCamera_ = new PerspectiveCamera(cameraToWorld, resolution, filmBounds, screenWindow, 0.0f, 1.0f, fovy, mFilm_);

		// light config parameter
		std::unordered_map<std::string, Spectrum> lightsParms;
		tinyxml2::XMLElement* lightNode = configFileDoc.FirstChildElement("light");

		while (lightNode)
		{
			if (strcmp(lightNode->Attribute("type"), "Area Light") == 0)
			{
				tinyxml2::XMLElement* lightAttribNode = lightNode->FirstChildElement();
				std::string lightName = lightAttribNode->Attribute("value");
				lightAttribNode = lightAttribNode->NextSiblingElement();
				parseMultiFloatNums(lightAttribNode->Attribute("value"), ele);
				lightsParms[lightName] = Spectrum(ele);
			}
			lightNode = lightNode->NextSiblingElement();
		}
		// todo: bsdf config parameter

		// primitive
		std::vector<Light*> sceneLights;
		std::vector<Primitive> scenePrimitives;
		std::unordered_map<std::string, Material*> sceneMaterials;

		tinyxml2::XMLElement* primitiveNode = configFileDoc.FirstChildElement("primitive");
		while (primitiveNode)
		{
			if (strcmp(primitiveNode->Attribute("type"), "Mesh") == 0)
			{
				tinyxml2::XMLElement* primitiveAttribNode = primitiveNode->FirstChildElement();
				std::string filename = filepath + "/" + primitiveAttribNode->Attribute("value");
				if (!ReadMeshFile(filename, lightsParms, sceneMaterials, scenePrimitives, sceneLights))
				{
					break;
				}
			}
			else if (strcmp(primitiveNode->Attribute("type"), "Sphere") == 0)
			{
				tinyxml2::XMLElement* primitiveAttribNode = primitiveNode->FirstChildElement();
				parseMultiFloatNums(primitiveAttribNode->Attribute("value"), ele);
				Point3f pos(ele[0], ele[1], ele[2]);
				primitiveAttribNode = primitiveAttribNode->NextSiblingElement();

				float radius = 0;
				parseFloatNum(primitiveAttribNode->Attribute("value"), &radius);

				primitiveAttribNode = primitiveAttribNode->NextSiblingElement();
				parseMultiFloatNums(primitiveAttribNode->Attribute("value"), ele);

				Spectrum kd(ele);
				primitiveAttribNode = primitiveAttribNode->NextSiblingElement();
				parseMultiFloatNums(primitiveAttribNode->Attribute("value"), ele);

				Spectrum ks(ele);
				primitiveAttribNode = primitiveAttribNode->NextSiblingElement();
				parseMultiFloatNums(primitiveAttribNode->Attribute("value"), ele);

				Spectrum kt(ele);
				primitiveAttribNode = primitiveAttribNode->NextSiblingElement();

				float ior;
				parseFloatNum(primitiveAttribNode->Attribute("value"), &ior);

				Transform* localToWorld = mAlloc_.new_object<Transform>(Translate(Vector3f(pos.x, pos.y, pos.z)));
				Transform* worldToLocal = mAlloc_.new_object<Transform>(Translate(Vector3f(-pos.x, -pos.y, -pos.z)));

				bool kdIsblack = kd.IsBlack();
				bool ksIsblack = ks.IsBlack();

				Material* material = nullptr;
				if (ior != 1)
				{
					Texture<Spectrum>* ksTex = mAlloc_.new_object<ConstantTexture<Spectrum>>(ks);
					Texture<Spectrum>* ktTex = mAlloc_.new_object<ConstantTexture<Spectrum>>(kt);
					material = mAlloc_.new_object<DielectricMaterial>(ksTex, ktTex, ior);
				}
				else if (!kdIsblack && !ksIsblack) 
				{
					Texture<Spectrum>* kdTex = mAlloc_.new_object<ConstantTexture<Spectrum>>(kd);
					Texture<Spectrum>* ksTex = mAlloc_.new_object<ConstantTexture<Spectrum>>(ks);
					Texture<Float>* exponent = mAlloc_.new_object<ConstantTexture<Float>>(32);
					material = mAlloc_.new_object<PhongMaterial>(kdTex, ksTex, exponent);
				}
				else if (kdIsblack) 
				{
					material = mAlloc_.new_object<MirrorMaterial>(mAlloc_.new_object<ConstantTexture<Spectrum>>(ks));
				}
				else if (ksIsblack) 
				{
					material = mAlloc_.new_object<DiffuseMaterial>(mAlloc_.new_object<ConstantTexture<Spectrum>>(kd));
				}

				Shape* shape = mAlloc_.new_object<Sphere>(localToWorld, radius);

				AreaLight* areaLight = nullptr;
				scenePrimitives.push_back({ shape, material, areaLight });
			}
			primitiveNode = primitiveNode->NextSiblingElement();
		}

		mScene_ = mAlloc_.new_object<Scene>(scenePrimitives, sceneLights);
		return true;
	}

	bool RayFlowEngine::ReadMeshFile(const std::string& objFileName,
		const std::unordered_map<std::string, Spectrum>& lightsParms,
		std::unordered_map<std::string, Material*>& sceneMaterials,
		std::vector<Primitive>& scenePrimitives,
		std::vector<Light*>& sceneLights)
	{
		Assimp::Importer importer;
		std::string objPath = objFileName.substr(0, objFileName.find_last_of('/'));

		const aiScene* assimpScene = importer.ReadFile(
			objFileName.c_str(),
			aiProcess_Triangulate |
			aiProcess_FlipUVs);
		if (!assimpScene || assimpScene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return false;
		}

		aiMaterial** mMaterials = assimpScene->mMaterials;
		for (unsigned int i = 0; i < assimpScene->mNumMaterials; ++i)
		{
			Texture<Spectrum>* Kd = nullptr;
			Texture<Spectrum>* Ks = nullptr;
			Texture<Spectrum>* Kt = nullptr;
			bool kdIsBlack = false;
			bool ksIsBlack = false;
			aiColor3D color;
			// diffuse
			if (mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE) != 0)
			{
				aiString texName;
				mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texName);
				std::string imgPath = objPath + "/" + texName.C_Str();

				if (mImages_.find(imgPath) == mImages_.end())
				{
					mImages_[imgPath] = mAlloc_.new_object<BitMap>(imgPath, false);
				}
				TextureMapping* mapping = mAlloc_.new_object<UVMapping>();
				Kd = mAlloc_.new_object<ImageTexture<Spectrum>>(mapping, mImages_[imgPath],
					ImageTextureWrapMode::REPEAT,
					ImageTextureFilterType::BILINEAR);
			}
			else
			{
				mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, color);
				if (!color.IsBlack())
				{
					Kd = mAlloc_.new_object<ConstantTexture<Spectrum>>(Spectrum(color.r, color.g, color.b));
				}
				else
				{
					kdIsBlack = true;
				}
			}
			// specular
			if (mMaterials[i]->GetTextureCount(aiTextureType_SPECULAR) != 0)
			{
				aiString texName;
				mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texName);
				std::string imgPath = objPath + "/" + texName.C_Str();

				if (mImages_.find(imgPath) == mImages_.end())
				{
					mImages_[imgPath] = mAlloc_.new_object<BitMap>(imgPath, false);
				}
				TextureMapping* mapping = mAlloc_.new_object<UVMapping>();
				Ks = mAlloc_.new_object<ImageTexture<Spectrum>>(mapping, mImages_[imgPath],
					ImageTextureWrapMode::REPEAT,
					ImageTextureFilterType::BILINEAR);
			}
			else
			{
				mMaterials[i]->Get(AI_MATKEY_COLOR_SPECULAR, color);
				if (!color.IsBlack())
				{
					Ks = mAlloc_.new_object<ConstantTexture<Spectrum>>(Spectrum(color.r, color.g, color.b));
				}
				else
				{
					ksIsBlack = true;
				}
			}

			Float ior = 1;
			mMaterials[i]->Get(AI_MATKEY_REFRACTI, ior);

			if (ior > 1) {
				Texture<Spectrum>* ksTex = mAlloc_.new_object<ConstantTexture<Spectrum>>(Spectrum(0.f));
				Texture<Spectrum>* ktTex = mAlloc_.new_object<ConstantTexture<Spectrum>>(Spectrum(1.f));
				sceneMaterials[mMaterials[i]->GetName().C_Str()] = mAlloc_.new_object<DielectricMaterial>(ksTex, ktTex, ior);
			}
			else if (kdIsBlack && ksIsBlack)
			{
				Kd = mAlloc_.new_object<ConstantTexture<Spectrum>>(Spectrum(0.0, 0.0, 0.0));
				sceneMaterials[mMaterials[i]->GetName().C_Str()] = mAlloc_.new_object<DiffuseMaterial>(Kd);
			}
			else if (ksIsBlack)
			{
				sceneMaterials[mMaterials[i]->GetName().C_Str()] = mAlloc_.new_object<DiffuseMaterial>(Kd);
			}
			else if (kdIsBlack)
			{
				sceneMaterials[mMaterials[i]->GetName().C_Str()] = mAlloc_.new_object<MirrorMaterial>(Ks);
			}
			else {
				Float exponent = 0;
				mMaterials[i]->Get(AI_MATKEY_SHININESS, exponent);
				Texture<Float>* expTex = mAlloc_.new_object<ConstantTexture<Float>>(exponent);
				sceneMaterials[mMaterials[i]->GetName().C_Str()] = mAlloc_.new_object<PhongMaterial>(Kd, Ks, expTex);
			}
		}

		ProcessNode(assimpScene->mRootNode, assimpScene, lightsParms,
			sceneMaterials, scenePrimitives, sceneLights);

		return true;
	}

	void RayFlowEngine::ProcessNode(aiNode* node, const aiScene* scene,
		const std::unordered_map<std::string, Spectrum>& lightsParms,
		const std::unordered_map<std::string, Material*>& sceneMaterials,
		std::vector<Primitive>& scenePrimitives,
		std::vector<Light*>& sceneLights)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(mesh, scene, lightsParms, sceneMaterials,
				scenePrimitives, sceneLights);
		}
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, lightsParms, sceneMaterials,
				scenePrimitives, sceneLights);
		}
	}

	void RayFlowEngine::ProcessMesh(aiMesh* mesh, const aiScene* scene,
		const std::unordered_map<std::string, Spectrum>& lightsParms,
		const std::unordered_map<std::string, Material*>& sceneMaterials,
		std::vector<Primitive>& scenePrimitives,
		std::vector<Light*>& sceneLights)
	{

		std::vector<Point3> vertices;
		std::vector<Normal3> normals;
		std::vector<Point2> texCoords;
		std::vector<int> faceIndices;

		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			vertices.push_back({ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });
			if (mesh->HasNormals())
			{
				normals.push_back({ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z });
			}
			if (mesh->mTextureCoords[0])
			{
				texCoords.push_back({ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
			}
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			faceIndices.push_back(mesh->mFaces[i].mIndices[0]);
			faceIndices.push_back(mesh->mFaces[i].mIndices[1]);
			faceIndices.push_back(mesh->mFaces[i].mIndices[2]);
		}

		Transform* localToWorld = mAlloc_.new_object<Transform>();
		Transform* worldToLocal = mAlloc_.new_object<Transform>();
		TriangleMeshObject* trimesh = mAlloc_.new_object<TriangleMeshObject>(*localToWorld, vertices, normals,
			texCoords, faceIndices, mAlloc_);

		aiMaterial* faceMaterial = scene->mMaterials[mesh->mMaterialIndex];
		std::string materialName(faceMaterial->GetName().C_Str());
		Material* meshMaterial = sceneMaterials.find(materialName)->second;

		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			Shape* triangle = mAlloc_.new_object<Triangle>(trimesh, i * 3);

			AreaLight* meshLight = nullptr;

			if (lightsParms.find(materialName) != lightsParms.end())
			{
				meshLight = mAlloc_.new_object<AreaLight>(worldToLocal, localToWorld, triangle, lightsParms.find(materialName)->second);
				sceneLights.push_back(meshLight);
			}

			scenePrimitives.push_back({ triangle, meshMaterial, meshLight });
		}
	}

	bool RayFlowEngine::Init(const std::string& filename)
	{
		ConfigFileParser parser(this);
		return parser.Parse(filename);
	}

	void RayFlowEngine::Render()
	{
		Timer timer;
		mIntegrator_->Render(*mScene_);
		//ReadScene("C:/FlowSource/code/FlowLab/RayFlow/resources/cornell-box-monster/cornell-box.xml");
		/*
		BitMap testMap("C:/FlowSource/code/FlowLab/RayFlow/resources/cornell-box/cornell-box.png", false);
		UVMapping uvm;
		ImageTexture<Spectrum> testTex(&uvm, &testMap, ImageTextureWrapMode::REPEAT, ImageTextureFilterType::BILINEAR);
		Point2i resolution = testMap.GetResolution();
		AABB2i bounds(Point2i(0, 0), resolution);
		Filter* filter = new BoxFilter();
		mFilm_ = new Film(bounds, resolution, filter, "sample.png", mAlloc_);

		for (int y = 0; y < resolution.y; ++y) {
			for (int x = 0; x < resolution.x; ++x) {
				Point2 uv(Float(x) / resolution.x, Float(y) / resolution.y);
				SurfaceIntersection si;
				si.uv = uv;
				mFilm_->AddSplat(Point2(x + 0.5, y + 0.5), testTex.Evaluate(si));
			}
		}
		mFilm_->Write();
		*/
		//Sampler* mSampler_ = mAlloc_.new_object<StratifiedSampler>(8, 8, 12, true);

		// mIntegrator_ = mAlloc_.new_object<DirectIntegrator>(mCamera_, mSampler_);
		// mIntegrator_->Render(*mScene_);

		//mIntegrator_ = mAlloc_.new_object<PathTracerIntegrator>(mCamera_, mSampler_, 8);
		//mIntegrator_->Render(*mScene_);

		// mIntegrator_ = mAlloc_.new_object<BDPTIntegrator>(mCamera_, mSampler_, 8);
		// mIntegrator_->Render(*mScene_);

		// Film* film = mCamera_->mFilm_;
		// Point2i resolution = film->resolution;
		// AABB2i sampledBounds = mCamera_->mSampleBounds_;
		// int filmTileWidth = 16;
		//
		// int tileXCount = ((resolution.x + filmTileWidth - 1) / filmTileWidth);
		// int tileYCount = ((resolution.y + filmTileWidth - 1) / filmTileWidth);
		// Point2i tileCount(tileXCount, tileYCount);
		// Scheduler::Parallel2D(tileCount,
		//     [&](Point2i tile) {
		//         ResetGMalloc();
		//
		//         int seed = tile.y * tileCount.x + tile.x;
		//         Sampler* sampler = mSampler_->Clone(seed);
		//
		//         int x0 = sampledBounds.pMin.x + tile.x * filmTileWidth;
		//         int x1 = std::min<int>(x0 + filmTileWidth, sampledBounds.pMax.x);
		//         int y0 = sampledBounds.pMin.y + tile.y * filmTileWidth;
		//         int y1 = std::min<int>(y0 + filmTileWidth, sampledBounds.pMax.y);
		//         FilmTile* filmTile = film->GetFilmTile(AABB2i(Point2i(x0, y0), Point2i(x1, y1)));
		//         for (int y = y0; y < y1; ++y) {
		//             for (int x = x0; x < x1; ++x) {
		//                 Point2i pRaster(x, y);
		//                 Spectrum Ltotal(0.f);
		//                 size_t sampleCount = sampler->GetSampleCount();
		//
		//                 sampler->StartPixel(pRaster);
		//                 for (int i = 0; i < sampleCount; ++i) {
		//                     if (x == 0 && 512 == y) {
		//                         std::cout << 1 << std::endl;
		//                     }
		//                     Point2 pFilm = Point2(pRaster) + sampler->Get2D();
		//                     CameraRaySample raySample = mCamera_->GenerateRay(pFilm, sampler->Get2D());
		//                     /*
		//                     Float rgb[3] = { std::abs(raySample.ray.d.x),
		//                                      std::abs(raySample.ray.d.y),
		//                                      std::abs(raySample.ray.d.z)};
		//                     Spectrum L(rgb);
		//                     L = raySample.weight * L;
		//                     */
		//                     Spectrum L(0.4f, 0.8f, 0.2f);
		//                     rstd::optional<ShapeIntersection> si = mScene_->Intersect(raySample.ray);
		//                     const SurfaceIntersection& re = si->isect;
		//                     if (si) {
		//                         //L = Spectrum(0.5f, 0.5f, 0.5f);
		//                         rstd::optional<BSDF> bsdf = si->isect.EvaluateBSDF();
		//                         if (bsdf) {
		//                             L = bsdf->f(si->isect.wo, si->isect.wo);
		//                         }
		//                         else {
		//                             L = Spectrum(0.0f, 0.0f, 0.0f);
		//                         }
		//                     }
		//
		//                     L = raySample.weight * L;
		//
		//                     if (L.HasNaN()) {
		//                         /*
		//                         LOG(ERROR) << "Not-a-number radiance value returned "
		//                             "for pixel (" << x << "," << y << "), sample" <<
		//                              (int)sampler->GetCurrentSampleNumber() << ". Setting to black.";
		//                              */
		//                         L = Spectrum(0.f);
		//                     }
		//                     else if (L.HasINF()) {
		//                         /*
		//                         LOG(ERROR) <<
		//                             "Infinite luminance value returned "
		//                             "for pixel (" << x << "," << y << "), sample" <<
		//                              (int)sampler->GetCurrentSampleNumber() << ". Setting to black.";
		//                             */
		//                         L = Spectrum(0.f);
		//                     }
		//
		//                     if (!L.IsBlack()) {
		//                         filmTile->AddSample(pFilm, L);
		//                     }
		//
		//                     sampler->Advance();
		//                 }
		//             }
		//         }
		//
		//         film->MergeFilmTile(filmTile);
		//     }
		//);
		//
		// film->Write();
	};

}