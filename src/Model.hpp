#pragma once

#include "Animation.hpp"

#include <stdint.h>

#include <filesystem>
#include <vector>

struct TMDObject
{
    uint32_t vert_top;
    uint32_t n_vert;
    uint32_t normal_top;
    uint32_t n_normal;
    uint32_t primitive_top;
    uint32_t n_primitive;
    int32_t scale;
};

struct TMD
{
    uint32_t id;
    uint32_t flag;
    uint32_t numObj;
    TMDObject objects[0];
};

struct MMD
{
    uint32_t id;
    uint32_t offset;
    TMD tmd;
};

enum class TMDCode : uint8_t
{
    UNKNOWN = 0,
    POLYGON = 1,
    LINE    = 2,
    SPRITE  = 3,
};

struct TMDFlag
{
    uint8_t isLightSourceDisabled : 1;
    uint8_t isDoubleFaced         : 1;
    uint8_t isGradated            : 1;
    uint8_t unused                : 5;
};

struct TMDMode
{
    uint8_t hasBrightness   : 1;
    uint8_t hasTranslucency : 1;
    uint8_t hasTexture      : 1;
    uint8_t isQuad          : 1;
    uint8_t isGouraud       : 1;
    TMDCode option          : 3;
};

struct FVector
{
    float x;
    float y;
    float z;

    friend std::ostream& operator<<(std::ostream& stream, const FVector& vec)
    {
        stream << vec.x << " " << vec.y << " " << vec.z;
        return stream;
    }
};

struct SVector
{
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t pad; // can also be boneId

    FVector convertToFixedPoint(uint32_t decimalBits) const
    {
        float scale = 1.0f / (1 << decimalBits);
        return { x * scale, y * scale, z * scale };
    }

    friend std::ostream& operator<<(std::ostream& stream, const SVector& vec)
    {
        stream << vec.x << " " << vec.y << " " << vec.z;
        return stream;
    }
};

struct RectSize
{
    uint32_t width;
    uint32_t height;
};

struct Vector
{
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t pad; // can also be boneId
};

struct UVCoord
{
    uint8_t u;
    uint8_t v;

    friend std::ostream& operator<<(std::ostream& stream, const UVCoord& tex)
    {
        stream << std::to_string(tex.u) << " " << std::to_string(tex.v);
        return stream;
    }
};

struct TexCoord
{
    float u;
    float v;

    TexCoord()
    {
        u = 0;
        v = 0;
    }

    TexCoord(uint32_t u, uint32_t v, const std::pair<uint32_t, uint32_t> size)
    {
        this->u = ((float)u / size.first) + 0.0001f;
        this->v = ((float)v / size.second) + 0.0001f;
    }

    TexCoord(UVCoord uv, const std::pair<uint32_t, uint32_t> size)
    {
        u = ((float)uv.u / size.first) + 0.0001f;
        v = ((float)uv.v / size.second) + 0.0001f;
    }

    friend std::ostream& operator<<(std::ostream& stream, const TexCoord& tex)
    {
        stream << (tex.u) << " " << (1.0f - tex.v);
        return stream;
    }
};

struct TextureInfo
{
    uint8_t page        : 5 = 0;
    uint8_t mixtureRate : 2 = 0;
    uint8_t colorMode   : 2 = 0;
};

struct CLUTInfo
{
    uint16_t clutX : 6 = 0;
    uint16_t clutY : 9 = 0;
};

struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t unused;

    friend std::ostream& operator<<(std::ostream& stream, const Color& color)
    {
        stream << (color.r / 255.0f) << " " << (color.g / 255.0f) << " " << (color.b / 255.0f);
        return stream;
    }
};

enum class MaterialType
{
    COLOR,
    TEXTURE,
    NO_LIGHT,
};

struct Face
{
    // vertex index, mandatory
    uint32_t v1;
    uint32_t v2;
    uint32_t v3;

    // normal index, optional
    uint32_t n1;
    uint32_t n2;
    uint32_t n3;

    // uv, optional
    UVCoord uv1;
    UVCoord uv2;
    UVCoord uv3;

    // color, optional
    Color color1;
    Color color2;
    Color color3;

    // flags
    bool isGradated;
    bool lightSourceDisabled;
    bool isDoubleSided;

    // mode
    bool shadingMode; // true -> gouraud, false -> flag
    bool hasTexture;
    bool hasTranslucency;
    bool brightnessDisabled;

    // texture information
    uint8_t mixtureRate;
    uint8_t texturePage;
    uint8_t colorMode;

    // clut information
    uint16_t clutX;
    uint16_t clutY;

    MaterialType material_type;
};

struct Mesh
{
    std::vector<SVector> vertices;
    std::vector<FVector> normals;
    std::vector<Face> faces;
};

struct NodeEntry
{
    uint8_t object;
    uint8_t parent;
};

class Primitive
{
private:
    // texture data
    std::vector<UVCoord> uvs;
    TextureInfo texInfo;
    CLUTInfo clutInfo;
    // color data
    std::vector<Color> colors;
    // vertex + normals
    std::vector<uint16_t> vertices;
    std::vector<uint16_t> normals;

    TMDFlag flag;
    TMDMode mode;

private:
    Face getFace(uint32_t idx1, uint32_t idx2, uint32_t idx3);

public:
    Primitive(ReadBuffer& buffer);

    std::vector<Face> toFaces();
};

class Model
{
    using filepath = std::filesystem::path;

private:
    void loadTMD(TMD& tmd);
    void loadMesh(filepath path);
    void loadNodes(filepath path);

public:
    std::string name;
    std::vector<Mesh> meshes;
    MMDAnimations anims;
    std::vector<NodeEntry> skeleton;

public:
    uint32_t getTexturePage() const;
    uint32_t getClutX() const;
    uint32_t getClutY() const;

    Model(filepath mesh, std::vector<NodeEntry> nodes = {});


};

/*
Files containing TIM:

CARD/CARD.ALL       -> 0x7000 per image, 66 in total
CHDAT/ALLTIM.TIM    -> 0x4800 per image, 180 in total
ETCDAT/BI_IN.TIM    -> single
ETCDAT/BTLCOMND.TIM -> single
ETCDAT/DEFEFF.TIM   -> single
ETCDAT/ETCTIM.BIN   -> 0x000000 0x002220 0x004440 0x005D00 0x007F20 0x00A140 0x012360 0x016680 0x01A8A0
ETCDAT/EX.TIM       -> single (heh)
ETCDAT/FEEL_EF.TIM  -> single
ETCDAT/FI_INFO.TIM  -> single
ETCDAT/FISHING.TIM  -> single
ETCDAT/FISHING2.TIM -> single
ETCDAT/ITEM.TIM     -> single
ETCDAT/SBOY.TIM     -> single
ETCDAT/SLOT.TIM     -> single
ETCDAT/SPOT256.TIM  -> single
ETCDAT/SYSTEM_W.TIM -> single
ETCDAT/TAMA.TIM     -> single
ETCHI/EFEDATA.EFE   -> 122 sub-images, one being none, see EFE format
ETCHI/LIFE.TIM      -> single
ETCHI/OP.TIM        -> single
ETCNA/IWA1.TIM      -> single
ETCNA/TAKE.TIM      -> single
ETCNA/TITLE2.TIM    -> single
ETCNA/TITLE256.TIM  -> single
MAP/*.TFS           -> see TFS format
MAP/*.MAP           -> see MAP format
STDDAT/GRADE/*.TIM  -> single
STDDAT/TIME.TIM     -> single
STDDAT/TAISEN1.TIM  -> single
STDDAT/TAISEN1.TIM  -> single
STDDAT/TAISEN_F.TIM -> single
STDDAT/T_TOGI.TIM   -> 0x000000 0x00F0E0 0x012F00 0x016D20
STDDAT/E_TOGI.TIM   -> single
STDDAT/B_TOGI.TIM   -> single
STDDAT/STDDAT.BIN   -> 0x000000 0x008220 0x010440 0x014120 0x01C180 0x01C9C0

BTL_REL.BIN         -> 0x000000
VS_REL.BIN          -> 0x000000
STD_REL.BIN         -> 0x000000


Files containing TMD and the texture they use:
CHDAT/MMDX/*.MMD    -> ALLTIM.TIM, entry based on their Digimon ID
DOOR/*.TMD          -> MAP image 0 and 1
ETCDAT/DEFEFF.TMD   -> ETCDAT/DEFEFF.TIM
ETCDAT/EX.TMD       -> ETCDAT/EX.TIM
ETCDAT/FEEL_EF.TMD  -> none, only vertex color
ETCDAT/KARRING.TMD  -> FRZL17 image 1
ETCDAT/SAO.TMD      -> FISHING.TIM?
ETCDAT/SEA.TMD      -> FISHING2.TIM?
ETCDAT/TAMA.TMD     -> TAMA.TIM

ETCHI/BOSS_EFE.TMD  -> MGEN99 MAP image 3
ETCHI/EFEDAT.EFE    -> self contained per EFE entry
ETCHI/MAHI.TMD      ->

ETCNA/ABOX.TMD      -> OMOC08 MAP Image 2
ETCNA/BIGBOX.TMD    -> OMOC08 MAP Image 2

BTL_REL.BIN@0x0D20  ->
VS_REL.BIN@0x0D20   ->
STD_REL.BIN@0x0D20  ->

BTL_REL.BIN@0x1D58  ->
VS_REL.BIN@0x1D58   ->
STD_REL.BIN@0x1D58  ->

BTL_REL.BIN@0x2220  ->
VS_REL.BIN@0x2220   ->
STD_REL.BIN@0x2220  ->

BTL_REL.BIN@0x2848  ->
VS_REL.BIN@0x2848   ->
STD_REL.BIN@0x2848  ->
*/