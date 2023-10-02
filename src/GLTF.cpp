
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "GLTF.hpp"

#include <format>
#include <numbers>

struct Quaternion
{
    float x;
    float y;
    float z;
    float w;

    Quaternion() = default;
    Quaternion(FVector angles)
    {
        double c1 = std::cos((angles.x * std::numbers::pi / 180.0f) * 0.5);
        double s1 = std::sin((angles.x * std::numbers::pi / 180.0f) * 0.5);
        double c2 = std::cos((angles.y * std::numbers::pi / 180.0f) * 0.5);
        double s2 = std::sin((angles.y * std::numbers::pi / 180.0f) * 0.5);
        double c3 = std::cos((angles.z * std::numbers::pi / 180.0f) * 0.5);
        double s3 = std::sin((angles.z * std::numbers::pi / 180.0f) * 0.5);

        this->w = c1 * c2 * c3 - s1 * s2 * s3;
        this->x = s1 * c2 * c3 + c1 * s2 * s3;
        this->y = -s1 * c2 * s3 + c1 * s2 * c3;
        this->z = c1 * c2 * s3 + s1 * s2 * c3;
    }
};

template<typename T> std::size_t push(std::vector<T>& vec, T& val)
{
    auto id = vec.size();
    vec.push_back(val);
    return id;
}

template<typename T> T myMin(T& a, T& b) { return std::min(a, b); }

template<typename T> T myMax(T& a, T& b) { return std::max(a, b); }

template<typename T> void push_to_vector(std::vector<double>& vec, T val) { vec.push_back(val); }

template<> void push_to_vector(std::vector<double>& vec, FVector val)
{
    vec.push_back(val.x);
    vec.push_back(val.y);
    vec.push_back(val.z);
}

template<> void push_to_vector(std::vector<double>& vec, Quaternion val)
{
    vec.push_back(val.x);
    vec.push_back(val.y);
    vec.push_back(val.z);
    vec.push_back(val.w);
}

template<> void push_to_vector(std::vector<double>& vec, ColorRGB val)
{
    vec.push_back(val.r);
    vec.push_back(val.g);
    vec.push_back(val.b);
}

template<> void push_to_vector(std::vector<double>& vec, TexCoord val)
{
    vec.push_back(val.u);
    vec.push_back(val.v);
}

template<> FVector myMin(FVector& a, FVector& b)
{
    FVector out;
    out.x = std::min(a.x, b.x);
    out.y = std::min(a.y, b.y);
    out.z = std::min(a.z, b.z);
    return out;
}

template<> FVector myMax(FVector& a, FVector& b)
{
    FVector out;
    out.x = std::max(a.x, b.x);
    out.y = std::max(a.y, b.y);
    out.z = std::max(a.z, b.z);
    return out;
}

template<> Quaternion myMin(Quaternion& a, Quaternion& b)
{
    Quaternion out;
    out.w = std::min(a.w, b.w);
    out.x = std::min(a.x, b.x);
    out.y = std::min(a.y, b.y);
    out.z = std::min(a.z, b.z);
    return out;
}

template<> Quaternion myMax(Quaternion& a, Quaternion& b)
{
    Quaternion out;
    out.w = std::max(a.w, b.w);
    out.x = std::max(a.x, b.x);
    out.y = std::max(a.y, b.y);
    out.z = std::max(a.z, b.z);
    return out;
}

template<> ColorRGB myMin(ColorRGB& a, ColorRGB& b)
{
    ColorRGB out;
    out.r = std::min(a.r, b.r);
    out.g = std::min(a.g, b.g);
    out.b = std::min(a.b, b.b);
    return out;
}
template<> ColorRGB myMax(ColorRGB& a, ColorRGB& b)
{
    ColorRGB out;
    out.r = std::max(a.r, b.r);
    out.g = std::max(a.g, b.g);
    out.b = std::max(a.b, b.b);
    return out;
}

template<> TexCoord myMin(TexCoord& a, TexCoord& b)
{
    TexCoord out;
    out.u = std::min(a.u, b.u);
    out.v = std::min(a.v, b.v);
    return out;
}

template<> TexCoord myMax(TexCoord& a, TexCoord& b)
{
    TexCoord out;
    out.u = std::max(a.u, b.u);
    out.v = std::max(a.v, b.v);
    return out;
}

void GLTFExporter::buildAssetEntry()
{
    model.asset.version   = "2.0";
    model.asset.generator = "DW1ModelConverter v1.0";
}

std::size_t GLTFExporter::buildPrimitiveVertex(Mesh& mesh, std::vector<Face> faces)
{
    std::vector<FVector> data;

    for (auto& face : faces)
    {
        data.push_back(mesh.vertices[face.v1].convertToFixedPoint(0));
        data.push_back(mesh.vertices[face.v2].convertToFixedPoint(0));
        data.push_back(mesh.vertices[face.v3].convertToFixedPoint(0));
    }

    return buildAccessor(data, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3, TINYGLTF_TARGET_ARRAY_BUFFER);
}

std::size_t GLTFExporter::buildPrimitiveNormal(Mesh& mesh, std::vector<Face> faces)
{
    std::vector<FVector> data;

    for (auto& face : faces)
    {
        data.push_back(mesh.normals[face.n1]);
        data.push_back(mesh.normals[face.n2]);
        data.push_back(mesh.normals[face.n3]);
    }

    return buildAccessor(data, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3, TINYGLTF_TARGET_ARRAY_BUFFER);
}

std::size_t GLTFExporter::buildPrimitiveColor(std::vector<Face> faces)
{
    std::vector<ColorRGB> data;

    for (auto& face : faces)
    {
        data.push_back(face.color1);
        data.push_back(face.color2);
        data.push_back(face.color3);
    }

    return buildAccessor(data, TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE, TINYGLTF_TYPE_VEC3, TINYGLTF_TARGET_ARRAY_BUFFER);
}

std::size_t GLTFExporter::buildPrimitiveTexcoord(std::vector<Face> faces)
{
    std::vector<TexCoord> data;

    for (auto& face : faces)
    {
        data.push_back(TexCoord(face.uv1, tim.getSize()));
        data.push_back(TexCoord(face.uv2, tim.getSize()));
        data.push_back(TexCoord(face.uv3, tim.getSize()));
    }

    return buildAccessor(data, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC2, TINYGLTF_TARGET_ARRAY_BUFFER);
}

template<typename T>
std::size_t GLTFExporter::buildAccessor(std::vector<T> data, int componentType, int type, int target)
{
    tinygltf::Buffer buffer;
    T min = data[0];
    T max = data[0];

    for (T& val : data)
    {
        min = myMin(min, val);
        max = myMax(max, val);

        std::copy_n(reinterpret_cast<unsigned char*>(&val), sizeof(val), std::back_inserter(buffer.data));
    }

    auto bufferId = push(model.buffers, buffer);

    tinygltf::BufferView view;
    view.buffer     = bufferId;
    view.byteLength = buffer.data.size();
    view.byteOffset = 0;
    view.target     = target;

    auto viewId = push(model.bufferViews, view);

    tinygltf::Accessor accessor;
    accessor.bufferView    = viewId;
    accessor.byteOffset    = 0;
    accessor.componentType = componentType;
    accessor.count         = data.size();
    accessor.type          = type;
    push_to_vector(accessor.maxValues, max);
    push_to_vector(accessor.minValues, min);

    return push(model.accessors, accessor);
}

tinygltf::Primitive GLTFExporter::buildPrimitive(Mesh& mesh, MaterialMode material, std::vector<Face> faces)
{
    tinygltf::Primitive prim;

    prim.mode     = TINYGLTF_MODE_TRIANGLES;
    prim.material = buildMaterial(material);

    prim.attributes["POSITION"] = buildPrimitiveVertex(mesh, faces);
    if (material.type != MaterialType::NO_LIGHT) prim.attributes["NORMAL"] = buildPrimitiveNormal(mesh, faces);
    if (material.type != MaterialType::TEXTURE) prim.attributes["COLOR_0"] = buildPrimitiveColor(faces);
    if (material.type != MaterialType::COLOR) prim.attributes["TEXCOORD_0"] = buildPrimitiveTexcoord(faces);

    return prim;
}

void GLTFExporter::buildSkeletonScene()
{
    tinygltf::Scene scene;
    tinygltf::Skin skin;
    skin.skeleton = 1;
    auto skinId   = push(model.skins, skin);

    for (auto& mmdNode : mmd.skeleton)
    {
        tinygltf::Node node;
        node.name = std::format("node-{}", model.nodes.size());

        if (mmdNode.object != 255)
        {
            auto mesh = mmd.meshes[mmdNode.object];
            tinygltf::Mesh lMesh;
            std::map<MaterialMode, std::vector<Face>> faceMap;

            for (const Face& face : mesh.faces)
            {
                MaterialMode mode(face);
                faceMap[mode].push_back(face);
            }

            for (auto& entry : faceMap)
                lMesh.primitives.push_back(buildPrimitive(mesh, entry.first, entry.second));

            node.mesh = push(model.meshes, lMesh);
        }

        auto id = push(model.nodes, node);

        if (mmdNode.parent != 255)
            model.nodes[mmdNode.parent].children.push_back(id);
        else
            scene.nodes.push_back(id);

        if (id > 0) model.skins[skinId].joints.push_back(id);
    }

    model.defaultScene = push(model.scenes, scene);
}

void GLTFExporter::buildStaticScene()
{
    tinygltf::Scene scene;

    for (Mesh& mesh : mmd.meshes)
    {
        tinygltf::Mesh lMesh;
        std::map<MaterialMode, std::vector<Face>> faceMap;

        for (const Face& face : mesh.faces)
        {
            MaterialMode mode(face);
            faceMap[mode].push_back(face);
        }

        for (auto& entry : faceMap)
            lMesh.primitives.push_back(buildPrimitive(mesh, entry.first, entry.second));

        auto meshId = push(model.meshes, lMesh);

        tinygltf::Node node;
        node.mesh = meshId;

        auto nodeId = push(model.nodes, node);
        scene.nodes.push_back(nodeId);
    }

    model.defaultScene = push(model.scenes, scene);
}

void GLTFExporter::buildMeshEntries()
{
    if (mmd.skeleton.size() == 0)
        buildStaticScene();
    else
        buildSkeletonScene();
}

int32_t GLTFExporter::buildMaterial(MaterialMode mode)
{
    auto existing = materialMapping.find(mode);

    if (existing != materialMapping.end()) return existing->second;

    tinygltf::Material mat;

    mat.doubleSided = mode.isDoubleSided;
    if (!mode.hasTranslucency)
        mat.alphaMode = "OPAQUE";
    else
    {
        tinygltf::Value::Object extras;
        extras.emplace("blendMode", std::to_string(mode.mixtureRate));
        mat.extras    = tinygltf::Value(extras);
        mat.alphaMode = "BLEND";
    }

    mat.pbrMetallicRoughness.baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
    if (mode.type != MaterialType::COLOR) mat.pbrMetallicRoughness.baseColorTexture.index = 0;
    auto id               = push(model.materials, mat);
    materialMapping[mode] = id;
    return id;
}

#include <iostream>

void GLTFExporter::buildAnimations()
{
    for (auto& raw : mmd.anims->anims)
    {
        Animation data(raw);
        tinygltf::Animation anim;
        int nodeId = 0;

        for (auto a : data.getData())
        {
            std::vector<float> posTime;
            std::vector<FVector> pos;
            std::vector<float> rotTime;
            std::vector<Quaternion> rot;
            std::vector<float> scaleTime;
            std::vector<FVector> scale;

            float time = -1;
            for (int i = 0; i < a.data[Axis::POS_X].size(); i++)
            {
                auto posX = a.data[Axis::POS_X][i];
                auto posY = a.data[Axis::POS_Y][i];
                auto posZ = a.data[Axis::POS_Z][i];

                if (time >= posX.first) continue;

                posTime.push_back(posX.first);
                pos.emplace_back(posX.second, posY.second, posZ.second);
                time = posX.first;
            }

            time = -1;
            for (int i = 0; i < a.data[Axis::ROT_X].size(); i++)
            {
                auto rotX = a.data[Axis::ROT_X][i];
                auto rotY = a.data[Axis::ROT_Y][i];
                auto rotZ = a.data[Axis::ROT_Z][i];

                if (time >= rotX.first) continue;

                rotTime.push_back(rotX.first);
                rot.emplace_back(FVector{ rotX.second, rotY.second, rotZ.second });
                time = rotX.first;
            }

            time = -1;
            for (int i = 0; i < a.data[Axis::SCALE_X].size(); i++)
            {
                auto scaleX = a.data[Axis::SCALE_X][i];
                auto scaleY = a.data[Axis::SCALE_Y][i];
                auto scaleZ = a.data[Axis::SCALE_Z][i];

                if (time >= scaleX.first) continue;

                scaleTime.push_back(scaleX.first);
                scale.emplace_back(scaleX.second, scaleY.second, scaleZ.second);
                time = scaleX.first;
            }

            tinygltf::AnimationChannel posChannel;
            tinygltf::AnimationChannel rotChannel;
            tinygltf::AnimationChannel scaleChannel;
            tinygltf::AnimationSampler posSampler;
            tinygltf::AnimationSampler rotSampler;
            tinygltf::AnimationSampler scaleSampler;

            posSampler.input         = buildAccessor(posTime, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_SCALAR, 0);
            posSampler.interpolation = "LINEAR";
            posSampler.output        = buildAccessor(pos, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3, 0);

            rotSampler.input         = buildAccessor(rotTime, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_SCALAR, 0);
            rotSampler.interpolation = "LINEAR";
            rotSampler.output        = buildAccessor(rot, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC4, 0);

            scaleSampler.input = buildAccessor(scaleTime, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_SCALAR, 0);
            scaleSampler.interpolation = "LINEAR";
            scaleSampler.output        = buildAccessor(scale, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3, 0);

            posChannel.target_node   = nodeId;
            posChannel.target_path   = "translation";
            posChannel.sampler       = push(anim.samplers, posSampler);
            rotChannel.target_node   = nodeId;
            rotChannel.target_path   = "rotation";
            rotChannel.sampler       = push(anim.samplers, rotSampler);
            scaleChannel.target_node = nodeId;
            scaleChannel.target_path = "scale";
            scaleChannel.sampler     = push(anim.samplers, scaleSampler);

            push(anim.channels, posChannel);
            push(anim.channels, rotChannel);
            push(anim.channels, scaleChannel);
            nodeId++;
        }

        tinygltf::Value::Object extras;
        tinygltf::Value::Array soundArray;
        tinygltf::Value::Array textureArray;

        for (auto s : data.sound)
        {
            tinygltf::Value::Object sound;
            sound.emplace("time", s.time);
            sound.emplace("vabId", s.vabId);
            sound.emplace("soundId", s.soundId);
            soundArray.emplace_back(sound);
        }

        for (auto t : data.texture)
        {
            tinygltf::Value::Object texture;
            texture.emplace("time", t.time);
            texture.emplace("srxX", t.srcX);
            texture.emplace("srcY", t.srcY);
            texture.emplace("destX", t.destX);
            texture.emplace("destY", t.destY);
            texture.emplace("width", t.width);
            texture.emplace("height", t.height);
            textureArray.emplace_back(texture);
        }

        if (data.endlessStart != -1) extras.emplace("endlessStart", std::to_string(data.endlessStart));
        if (!data.sound.empty()) extras.emplace("sounds", soundArray);
        if (!data.texture.empty()) extras.emplace("textures", textureArray);
        anim.extras = tinygltf::Value(extras);
        push(model.animations, anim);
    }
}

void GLTFExporter::buildTexture()
{
    CLUTMap map;
    map.applyModel(mmd);

    tinygltf::Image image;

    image.mimeType   = "image/png";
    image.bits       = 8;
    image.component  = 4;
    image.pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    image.image      = tim.getImage(map);
    image.width      = tim.getSize().first;
    image.height     = tim.getSize().second;
    image.name       = "texture";

    tinygltf::Sampler sampler;
    sampler.magFilter = TINYGLTF_TEXTURE_FILTER_NEAREST;
    sampler.minFilter = TINYGLTF_TEXTURE_FILTER_NEAREST;

    tinygltf::Texture tex;
    tex.source  = push(model.images, image);
    tex.sampler = push(model.samplers, sampler);

    push(model.textures, tex);
}

GLTFExporter::GLTFExporter(Model& mmd, AbstractTIM& tim)
    : mmd(mmd)
    , tim(tim)
{
    buildAssetEntry();
    buildMeshEntries();
    buildAnimations();
    buildTexture();
}

bool GLTFExporter::save(const std::string& filename)
{
    tinygltf::TinyGLTF gltf;
    return gltf.WriteGltfSceneToFile(&model,
                                     filename,
                                     true,   // embedImages
                                     true,   // embedBuffers
                                     true,   // pretty print
                                     false); // write binary
}