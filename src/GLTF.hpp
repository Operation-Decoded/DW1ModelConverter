#pragma once
#include "Model.hpp"
#include "TIM.hpp"

#include <tiny_gltf.h>

struct ColorRGB
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

    ColorRGB() = default;
    ColorRGB(Color color)
        : r(color.r)
        , g(color.g)
        , b(color.b)
    {
    }
};

class MaterialMode
{
public:
    bool isDoubleSided;
    bool hasTranslucency;
    uint32_t mixtureRate;
    MaterialType type;

public:
    MaterialMode(const Face& face)
        : isDoubleSided(face.isDoubleSided)
        , mixtureRate(face.mixtureRate)
        , type(face.material_type)
        , hasTranslucency(face.hasTranslucency)
    {
    }

    friend int32_t operator<(const MaterialMode& l, const MaterialMode& r)
    {
        return std::tie(l.isDoubleSided, l.mixtureRate, l.type, l.hasTranslucency) <
               std::tie(r.isDoubleSided, r.mixtureRate, r.type, r.hasTranslucency);
    }
};

class GLTFExporter
{
private:
    tinygltf::Model model;

    Model& mmd;
    AbstractTIM& tim;
    std::map<MaterialMode, int32_t> materialMapping;

private:
    void buildAssetEntry();
    void buildMeshEntries();
    void buildStaticScene();
    void buildSkeletonScene();
    void buildAnimations();
    void buildTexture();

    template<typename T> std::size_t buildAccessor(std::vector<T> data, int componentType, int type);

    int32_t buildMaterial(MaterialMode mode);
    tinygltf::Primitive buildPrimitive(Mesh& mesh, MaterialMode material, std::vector<Face> faces);
    std::size_t buildPrimitiveVertex(Mesh& mesh, std::vector<Face> faces);
    std::size_t buildPrimitiveNormal(Mesh& mesh, std::vector<Face> faces);
    std::size_t buildPrimitiveColor(std::vector<Face> faces);
    std::size_t buildPrimitiveTexcoord(std::vector<Face> faces);

public:
    GLTFExporter(Model& model, AbstractTIM& tim);

    void save(const std::string& filename);
};