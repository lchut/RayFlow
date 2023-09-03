#include <string>
#include <vector>

#include <tinyxml2/tinyxml2.h>
#include <tinyobjloader/tiny_obj_loader.h>

#include <RayFlow/Engine/fwd.h>
#include <RayFlow/Engine/engine.h>

namespace rayflow {

class RayFlowEngine;

class ConfigFileParser {
public:
    ConfigFileParser(RayFlowEngine* engine) :
        engine(engine) {

    }
    
    bool Parse(const std::string& filename);

private:

    template <typename T>
    T ParseNumber(const std::string& text) const;

    template<>
    int ParseNumber(const std::string& text) const {
        return std::stoi(text);
    }

    template<>
    float ParseNumber(const std::string& text) const {
        return std::stof(text);
    }

    template<>
    double ParseNumber(const std::string& text) const {
        return std::stod(text);
    }

    template<typename T>
    TVector2<T> ParseVector2(const std::string& text) const {
        T values[2];

        ParseArray<T>(text, values, 2);

        return TVector2<T>(values[0], values[1]);
    }

    template<typename T>
    TVector3<T> ParseVector3(const std::string& text) const {
        T values[3];

        ParseArray<T>(text, values, 3);

        return TVector3<T>(values[0], values[1], values[2]);
    }

    Quaternion ParseQuaternion(const std::string& text) const {
        Float values[4] = {0.0};

        ParseArray<Float>(text, values, 4);

        return Quaternion(values[0], values[1], values[2], values[3]);
    }

    Spectrum ParseSpectrum(const std::string& text) const {
        Float values[3] = {0.0};

        ParseArray<Float>(text, values, 3);

        return Spectrum(values);
    }

    Transform ParseTransform(const std::string& text) const {
        Float m[4][4] = {0.0f};

        ParseArray<Float>(text, &m[0][0], 16);

        return Transform(m);
    }

    std::string::const_iterator FindNext(std::string::const_iterator& begin, const std::string::const_iterator& end, const std::function<bool(char)>& pred) const {
        auto it = begin;
        while (it != end && !pred(*it)) {
            ++it;
        }
        return it;
    }

    template <typename T>
    void ParseArray(const std::string& text, T* arr, int n) const {
        std::string::const_iterator begin = text.cbegin();
        std::string::const_iterator end = text.cend();
        
        auto isNumberComponent = [](char c)->bool {
            return c == '-' || c == 'e' || c == '.' || isdigit(c);
        };

        auto isNotNumberComponent = [](char c)->bool {
            return !(c == '-' || c == 'e' || c == '.' || isdigit(c));
        };

        std::string::const_iterator it = begin;

        for (int i = 0; i < n; ++i) {
            std::string::const_iterator numEnd = FindNext(it, end, isNotNumberComponent);
            arr[i] = ParseNumber<T>(text.substr(it - begin, numEnd - it));
            it = FindNext(numEnd, end, isNumberComponent);
        }
    }

    Texture<Spectrum>* ParseRGBTexture(tinyxml2::XMLElement* node) const {
        Texture<Spectrum>* tex = nullptr;

        if (strcmp(node->Name(), "rgb") == 0) {
            Spectrum rgb = ParseSpectrum(node->Attribute("value"));
            tex = engine->mAlloc_.new_object<ConstantTexture<Spectrum>>(rgb);
        }
        else {
            std::string texname = node->FirstChildElement("string")->Attribute("value");
            BitMap* bitmap = engine->GetBitMap(texname);

            if (bitmap == nullptr) {
                bitmap = engine->mAlloc_.new_object<BitMap>(assetPath + texname, false);
                engine->AddBitMap(texname, bitmap);
            }

            TextureMapping* mapping = engine->mAlloc_.new_object<UVMapping>();
            tex = engine->mAlloc_.new_object<ImageTexture<Spectrum>>(mapping, bitmap);
        }
        
        return tex;
    }

    Texture<Float>* ParseFloatTexture(tinyxml2::XMLElement* node) const {
        Texture<Float>* tex = nullptr;

        if (strcmp(node->Name(), "float") == 0) {
            Float parm = ParseNumber<Float>(node->Attribute("value"));
            tex = engine->mAlloc_.new_object<ConstantTexture<Float>>(parm);
        }

        return tex;
    }

private:
    RayFlowEngine* engine;
    std::string assetPath;
};

}