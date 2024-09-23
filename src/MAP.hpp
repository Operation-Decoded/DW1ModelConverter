#pragma once
#include "GameData.hpp"
#include "TIM.hpp"
#include "utils/ReadBuffer.hpp"

#include <nlohmann/json.hpp>

#include <array>
#include <cstdint>
#include <optional>
#include <vector>


template<typename T> struct Position3D
{
    T x;
    T y;
    T z;
};

struct MapLight
{
    Position3D<int32_t> position;
    uint32_t red;
    uint32_t green;
    uint32_t blue;
};

struct MapSetup
{
    Position3D<int32_t> cameraOrigin;
    Position3D<int32_t> cameraPosition;
    std::array<MapLight, 3> lights;
    uint32_t unusedAmbientRed;
    uint32_t unusedAmbientGreen;
    uint32_t unusedAmbientBlue;
    uint32_t viewerDistance;
    std::array<int32_t, 4> likedArea;
    std::array<int32_t, 4> dislikedArea;
    uint32_t width;
    uint32_t height;
    std::vector<uint32_t> tiles;
};

struct MapObject
{
    uint16_t uvX;
    uint16_t uvY;
    uint16_t width;
    uint16_t height;
    Position3D<int16_t> pos3d;
    uint16_t clut;
    uint16_t transparency;
};

static_assert(sizeof(MapObject) == 0x12);

struct MapObjectInstance
{
    std::array<int16_t, 8> animState;
    std::array<int16_t, 8> animDuration;
    uint16_t posX;
    uint16_t posY;
    uint16_t flag;
};

static_assert(sizeof(MapObjectInstance) == 0x26);

struct MapObjects
{
    uint16_t objCount{ 0 };
    std::vector<MapObject> objects{};
    uint16_t instanceCount{ 0 };
    std::vector<MapObjectInstance> instances{};
};

struct MapDigimon
{
    uint16_t type;   // TODO: enum?
    uint16_t aiType; // ?

    Position3D<int16_t> pos;
    int16_t rotX;
    int16_t rotY;
    int16_t rotZ;
    uint16_t trackingRange;
    uint16_t unk2;
    uint8_t scriptId;
    uint8_t unk3;

    uint16_t hp;
    uint16_t mp;
    uint16_t maxHP;
    uint16_t maxMP;
    uint16_t offense;
    uint16_t defense;
    uint16_t speed;
    uint16_t brains;

    uint16_t bits;

    uint16_t chargeMode; // TODO: enum?
    uint16_t unk5;

    std::array<uint16_t, 4> moves;
    std::array<uint16_t, 4> moveWeights;

    Position3D<int16_t> fleePos;

    uint16_t waypointCount;
    std::array<int16_t, 8> waypointSpeed;

    std::vector<Position3D<int16_t>> waypoints;
};

struct MapElements
{
    std::array<int16_t, 10> spawnX;
    std::array<int16_t, 10> spawnY;
    std::array<int16_t, 10> spawnZ;
    std::array<int16_t, 10> spawnRotation;

    std::array<uint16_t, 10> warpTargetMap;
    std::array<uint16_t, 10> warpTargetSpawn;

    uint16_t digimonCount;
    std::vector<MapDigimon> digimon;
};

class MapFile
{
public:
    MapEntry entry;
    MapSetup setup;
    std::vector<AbstractTIM> images8bpp;
    std::vector<AbstractTIM> images4bpp;
    MapObjects objects;
    MapElements elements;
    std::array<std::array<uint8_t, 100>, 100> tileMap;

public:
    MapFile(const std::filesystem::path path, const MapEntry entry);
    MapFile(std::vector<uint8_t>& buffer, const MapEntry entry);
    nlohmann::ordered_json to_json();

    std::optional<const AbstractTIM*> getImageByTexCoord(uint32_t x, uint32_t y)
    {
        std::optional<const AbstractTIM*> tim = {};
        for (auto& i : images8bpp)
            if (i.containsTexCoord(x, y)) tim = &i;

        for (auto& i : images4bpp)
            if (i.containsTexCoord(x, y)) tim = &i;

        return tim;
    }

private:
    void init(ReadBuffer buffer);
};

using TFSData = std::array<std::array<uint8_t, 128>, 128>;

struct TFSImage
{
    uint16_t posX; // unused?
    uint16_t posY; // unused?
    TFSData data;

    cimg_library::CImg<uint8_t> getImage(std::array<TIMColor, 256> palette);
};

struct TFSFile
{
    uint16_t width;  // unused?
    uint16_t height; // unused?
    std::vector<std::array<TIMColor, 256>> palettes;
    std::vector<TFSImage> images;

    TFSFile(const std::filesystem::path path);
    TFSFile(std::vector<uint8_t>& buffer);
    void init(ReadBuffer buff, std::size_t fileSize);

    auto getImages(MapFile& map) -> std::vector<cimg_library::CImg<uint8_t>>;
    auto getImage(uint32_t paletteId, MapFile& map) -> cimg_library::CImg<uint8_t>;
};

class MAPExporter
{
private:
    MapFile map;
    TFSFile tfs;
    MapEntry mapEntry;
    std::map<uint32_t, Model> doors;
    std::array<MapEntry, 255> mapEntries;
    std::vector<DigimonEntry> digimonEntries;

public:
    MAPExporter(MapFile map,
                TFSFile tfs,
                MapEntry mapEntry,
                std::map<uint32_t, Model> doors,
                std::array<MapEntry, 255> mapEntries,
                std::vector<DigimonEntry> digimonEntries)
        : map(map)
        , tfs(tfs)
        , mapEntry(mapEntry)
        , doors(doors)
        , mapEntries(mapEntries)
        , digimonEntries(digimonEntries)
    {
    }

    bool save(std::filesystem::path outputDir);

private:
    void saveObject(MapObject& obj, TIMPalette& pal, std::filesystem::path path, bool is4bpp = false);
    std::map<uint32_t, TIMPalette> getCLUTMap();
};
