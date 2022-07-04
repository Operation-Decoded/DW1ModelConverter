#include "Model.hpp"

#include "utils/ReadBuffer.hpp"

#include <fstream>

/*
    Files that contain TMDs:
    - .TMD files
    - .MMD files (offset 0x8)
    - VS_REL.BIN
    - STD_REL.BIN
    - BTL_REL.BIN
    - EFE.DAT
*/

template<> Primitive ReadBuffer::read()
{
    auto val = Primitive(*this);
    skip((4 - (uint64_t)bufferCurrent % 4) % 4);
    return val;
}

Face Primitive::getFace(uint32_t idx1, uint32_t idx2, uint32_t idx3)
{
    Face face;
    face.isGradated          = flag.isGradated;
    face.lightSourceDisabled = flag.isLightSourceDisabled;
    face.hasTexture          = mode.hasTexture;
    face.brightnessDisabled  = mode.hasBrightness;
    face.hasTranslucency     = mode.hasTranslucency;
    face.shadingMode         = mode.isGouraud;

    face.clutX = clutInfo.clutX << 4;
    face.clutY = clutInfo.clutY;

    face.mixtureRate = texInfo.mixtureRate;
    face.colorMode   = texInfo.colorMode;
    face.texturePage = texInfo.page;

    face.v1 = vertices[idx1];
    face.v2 = vertices[idx2];
    face.v3 = vertices[idx3];

    if (normals.size() != 0)
    {
        face.n1 = normals[normals.size() == 1 ? 0 : idx1];
        face.n2 = normals[normals.size() == 1 ? 0 : idx2];
        face.n3 = normals[normals.size() == 1 ? 0 : idx3];
    }

    if (uvs.size() != 0)
    {
        face.uv1 = uvs[idx1];
        face.uv2 = uvs[idx2];
        face.uv3 = uvs[idx3];
    }

    if (colors.size() != 0)
    {
        face.color1 = colors[colors.size() == 1 ? 0 : idx1];
        face.color2 = colors[colors.size() == 1 ? 0 : idx2];
        face.color3 = colors[colors.size() == 1 ? 0 : idx3];
    }

    if (!face.hasTexture)
        face.material_type = MaterialType::COLOR;
    else if (face.brightnessDisabled || face.lightSourceDisabled)
        face.material_type = MaterialType::NO_LIGHT;
    else
        face.material_type = MaterialType::TEXTURE;

    return face;
}

std::vector<Face> Primitive::toFaces()
{
    std::vector<Face> faces;

    faces.push_back(getFace(2, 1, 0));
    if (flag.isDoubleFaced) faces.push_back(getFace(0, 1, 2));

    if (mode.isQuad)
    {
        faces.push_back(getFace(1, 2, 3));
        if (flag.isDoubleFaced) faces.push_back(getFace(3, 2, 1));
    }

    return faces;
}

Primitive::Primitive(ReadBuffer& buffer)
{
    // TODO texture page handling -> textures with sizes > 256

    buffer.skip(2); // magic value, ilen
    flag = buffer.read<TMDFlag>();
    mode = buffer.read<TMDMode>();

    if (mode.option != TMDCode::POLYGON) throw std::exception("Not a polygon");

    uint32_t vertexCount = mode.isQuad ? 4 : 3;
    uint32_t normalCount = flag.isLightSourceDisabled || mode.hasBrightness ? 0 : mode.isGouraud ? vertexCount : 1;
    uint32_t colorCount  = 0;

    if (flag.isGradated)
        colorCount = vertexCount;
    else if (flag.isLightSourceDisabled)
        colorCount = mode.isGouraud ? vertexCount : 1;
    else if (!mode.hasTexture)
        colorCount = 1;

    if (mode.hasTexture)
    {
        uvs.push_back(buffer.read<UVCoord>());
        clutInfo = buffer.read<CLUTInfo>();
        uvs.push_back(buffer.read<UVCoord>());
        texInfo = buffer.read<TextureInfo>();
        uvs.push_back(buffer.read<UVCoord>());
        buffer.skip(2);
        if (vertexCount == 4)
        {
            uvs.push_back(buffer.read<UVCoord>());
            buffer.skip(2);
        }
    }

    for (uint32_t i = 0u; i < colorCount; i++)
        colors.push_back(buffer.read<Color>());

    for (uint32_t i = 0u; i < vertexCount; i++)
    {
        if (i < normalCount) normals.push_back(buffer.read<uint16_t>());
        vertices.push_back(buffer.read<uint16_t>());
    }
}

void Model::loadTMD(TMD& tmd)
{
    for (uint32_t i = 0; i < tmd.numObj; i++)
    {
        Mesh mesh;

        TMDObject& obj    = tmd.objects[i];
        SVector* vertices = reinterpret_cast<SVector*>(reinterpret_cast<uint8_t*>(&(tmd.objects)) + obj.vert_top);
        SVector* normals  = reinterpret_cast<SVector*>(reinterpret_cast<uint8_t*>(&(tmd.objects)) + obj.normal_top);
        char* primitives  = reinterpret_cast<char*>(&(tmd.objects)) + obj.primitive_top;

        for (uint32_t j = 0; j < obj.n_vert; j++)
            mesh.vertices.push_back(vertices[j]);

        for (uint32_t j = 0; j < obj.n_normal; j++)
            mesh.normals.push_back(normals[j].convertToFixedPoint(12));

        ReadBuffer buffer(primitives);
        for (uint32_t j = 0; j < obj.n_primitive; j++)
        {
            std::vector<Face> faces = buffer.read<Primitive>().toFaces();
            mesh.faces.insert(mesh.faces.end(), faces.begin(), faces.end());

            /*


            001H GFED  0000 0CBA

            // seem to always hold the same value
            // when true, 0 normals are stored
            A -> Light Source calc disabled
            D -> Has Brightness Calc
            // whether there are 1 or 3 normals (light) or colors (no light)
            H -> Flat/Gouraud Shading
            // when true 3 UVs are stored, otherwise 0
            F -> Has Texture

            //
            E -> Has Translucency


            // only valid when no texture and light source enabled, does not exist in DW1
            C -> Gradated Color
            // require multiple triangles per primitive
            G -> Quad
            B -> Double Faced


            Material A: Vertex Colors
            - 3 normals (might be the same each)
            - 3 colors (might be the same each)
            - 0 UVs (no texture)

            Material B: Textured
            - 3 normals (might be the same each)
            - 0 colors
            - 3 UVs

            Material C: No light
            - 0 normals
            - 3 colors (might be the same each)
            - 3 UVs

            // 3 color, 1 normals, 0 UVs
            // 3 color, 3 normals, 0 UVs
            -> flag == 4
            // 1 color, 3 normals, 0 UVs
            if (mode == 0x38 && flag == 0) continue; // Quad, Gouraud, No Texture, Light
            // 1 color, 1 normal, 0 UVs
            if (mode == 0x28 && flag == 0) continue; // Quad, Flat, No Texture, Light
            if (mode == 0x20 && flag == 0) continue; // Tri, Flat, No Texture, Light

            // 0 color, 3 normal, 3 UVs
            if (mode == 0x3C && flag == 0) continue; // Quad, Gouraud, Texture, Light
            if (mode == 0x34 && flag == 0) continue; // Tri, Gouraud, Texture, Light
            // 0 color, 1 normal, 3 UVs
            if (mode == 0x2C && flag == 0) continue; // Quad, Flat, Texture, Light
            if (mode == 0x24 && flag == 0) continue; // Tri, Flat, Texture, Light

            // 3 color, 0 normal, 3 UVs
            if (mode == 0x3D && flag == 1) continue; // Quad, Gradation, Texture, No Light
            if (mode == 0x35 && flag == 1) continue; // Tri, Gradation, Texture, No Light
            // 1 color, 0 normal, 3 UVs
            if (mode == 0x2D && flag == 1) continue; // Quad, Flat, Texture, No Light
            if (mode == 0x25 && flag == 1) continue; // Tri, Flat, Texture, No Light


            // 0 color, 3 normal, 3 UVs | semi-transparency
            if (mode == 0x3E && flag == 0) continue; // Quad, Gouraud, Texture, Light, Translucency
            if (mode == 0x36 && flag == 0) continue; // Tri, Gouraud, Texture, Light, Translucency
            // 0 color, 1 normal, 3 UVs | semi-transparency
            if (mode == 0x2E && flag == 0) continue; // Quad, Flat, Texture, Light, Translucency
            if (mode == 0x26 && flag == 0) continue; // Tri, Flat, Texture, Light, Translucency

            // 1 color, 0 normal, 3 UVs | semi-transparency
            if (mode == 0x3F && flag == 1) continue; // Quad, Gradation, Texture, No Light, Translucency
            // 3 color, 0 normal, 3 UVs | semi-transparency
            if (mode == 0x2F && flag == 1) continue; // Quad, Flat, Texture, No Light, Translucency
            if (mode == 0x27 && flag == 1) continue; // Tri, Flat, Texture, No Light, Translucency
            */
        }

        meshes.push_back(mesh);
    }
}

Model::Model(filepath mesh, std::optional<filepath> nodes, std::optional<filepath> texture)
    : texture(texture)
{
    std::streamoff length = std::filesystem::file_size(mesh);
    name                  = mesh.filename().string();

    if (nodes) loadNodes(nodes.value()); // load nodes before mesh, since animations need to know about the bone count
    loadMesh(mesh);
}

void Model::loadMesh(filepath path)
{
    if (!std::filesystem::is_regular_file(path)) throw std::exception("Expected a file, but got something else.");

    std::streamoff length = std::filesystem::file_size(path);
    std::ifstream input(path, std::ios::binary);

    std::vector<char> buffer;
    buffer.resize(length);
    input.read(buffer.data(), length);

    TMD* tmdPtr  = reinterpret_cast<TMD*>(buffer.data());
    char* mtnPtr = NULL;

    if (!path.extension().compare(".MMD"))
    {
        MMD* mmd = reinterpret_cast<MMD*>(buffer.data());

        tmdPtr = reinterpret_cast<TMD*>(buffer.data() + mmd->id);
        mtnPtr = buffer.data() + mmd->offset;
    }

    loadTMD(*tmdPtr);
    if (mtnPtr != NULL && skeleton.size() != 0)
    {
        ReadBuffer b(mtnPtr);
        anims = std::make_unique<MMDAnimations>(b, skeleton.size());
    }
    else
        anims = std::make_unique<MMDAnimations>();
}

uint32_t Model::getTexturePage() const
{
    uint32_t page    = -1;
    uint32_t maxPage = 0;

    for (auto& mesh : meshes)
        for (auto& face : mesh.faces)
        {
            if (!face.hasTexture) continue;

            if (face.texturePage < page) page = face.texturePage;

            if (face.texturePage > maxPage) maxPage = face.texturePage;
        }

    return page;
}

uint32_t Model::getClutX() const
{
    uint32_t clutX = -1;

    for (auto& mesh : meshes)
        for (auto& face : mesh.faces)
            if (face.hasTexture && face.clutX < clutX) clutX = face.clutX;

    return clutX;
}

uint32_t Model::getClutY() const
{
    uint32_t clutY = -1;

    for (auto& mesh : meshes)
        for (auto& face : mesh.faces)
            if (face.hasTexture && face.clutY < clutY) clutY = face.clutY;

    return clutY;
}

void Model::loadNodes(filepath path)
{
    if (!std::filesystem::is_regular_file(path)) throw std::exception("Expected a node file, but got something else.");

    auto length = std::filesystem::file_size(path);
    std::ifstream input(path, std::ios::binary);

    for (uint32_t i = 0u; i < length / 2; i++)
    {
        NodeEntry entry;
        input.read(reinterpret_cast<char*>(&entry), 2);
        skeleton.push_back(entry);
    }
}