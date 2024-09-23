#pragma once
#include "Model.hpp"

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>
#include <filesystem>

// structs
struct DigimonPara
{
    char name[20];
    int32_t boneCount;
    int16_t radius;
    int16_t height;
    uint8_t type;
    uint8_t level;
    uint8_t special[3];
    uint8_t dropItem;
    uint8_t dropChance;
    int8_t moves[16];
    uint8_t padding;
};

struct DigimonParaPAL
{
    int32_t boneCount;
    int16_t radius;
    int16_t height;
    uint8_t type;
    uint8_t level;
    uint8_t special[3];
    uint8_t dropItem;
    uint8_t dropChance;
    int8_t moves[16];
    uint8_t padding;
};

static_assert(sizeof(DigimonPara) == 52);
static_assert(sizeof(DigimonParaPAL) == 32);

struct VersionData
{
    std::string_view psexePath;
    std::string_view alltimPath;
    uint32_t nameOffset;
    uint32_t paraOffset;
    uint32_t skelOffset;
    uint32_t mapEntryOffset;
    uint32_t mapNamePtrOffset;
    uint32_t toiletDataOffset;
    uint32_t doorDataOffset;
    bool isPAL;
};

struct MMDTexture
{
    uint8_t buffer[0x4800];
};

struct DigimonEntry
{
    std::string filename;
    std::vector<NodeEntry> skeleton;
    std::vector<uint8_t> texture;
};



struct ToiletData
{
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
};

struct MapEntryFlags
{
    uint8_t soundId     : 5;
    uint8_t             : 1; // padding
    bool hasNoTimeCycle : 1;
    bool hasDigimon     : 1;
};

static_assert(sizeof(MapEntryFlags) == 1);

struct MapEntryData
{
    char name[10];
    uint8_t numMapImages;
    uint8_t numMapObjects;
    MapEntryFlags flags;
    uint8_t doorsId;
    uint8_t toiletId;
    uint8_t loadingNameId;
};

struct DoorData
{
    uint8_t modelId[6];
    int16_t posX[6];
    int16_t posY[6];
    int16_t posZ[6];
    int16_t rotation[6];
};

struct MapEntry
{
    MapEntryData data;
    std::string name;
    std::optional<ToiletData> toilet;
    std::optional<DoorData> doors;
};

static_assert(sizeof(MapEntryData) == 0x10);

// TODO map entry data
// data
/*
 *                  name     para     skel mapEntry  mapName  toilets    doors
 * SLUS_010.32  0x133B44 0x12CEB4 0x11ce60 0x1292d4 0x1291bc 0x122e10 0x122e60
 * SLPS_017.97  0x13d844 0x13b344 0x123780 (1.1)
 * SLPS_017.97  0x13CE24 0x13A924 0x122E68 (1.0)
 * SLPM_804.02  0x13e874 0x13c32c 0x124728
 * SLES_029.14  0x13AC0C 0x138B5C 0x122DD4
 * SLES_034.34  0x13AE00 0x138D50 0x122DE4
 * SLES_034.35  0x13ADD8 0x138D28 0x122D6C
 * SLES_034.36  0x13B7E4 0x139734 0x122DA0
 * SLES_034.37  0x13B314 0x139264 0x122DA0
 */
constexpr uint32_t PSEXE_OFFSET(uint32_t offset) { return (offset - 0x90000) & 0x7FFFFFFF; }

constexpr VersionData SLUS_DATA = {
    .psexePath        = "SLUS_010.32",
    .alltimPath       = "CHDAT/ALLTIM.TIM",
    .nameOffset       = PSEXE_OFFSET(0x133b44),
    .paraOffset       = PSEXE_OFFSET(0x12ceb4),
    .skelOffset       = PSEXE_OFFSET(0x11ce60),
    .mapEntryOffset   = PSEXE_OFFSET(0x1292d4),
    .mapNamePtrOffset = PSEXE_OFFSET(0x1291bc),
    .toiletDataOffset = PSEXE_OFFSET(0x122e10),
    .doorDataOffset   = PSEXE_OFFSET(0x122e60),
    .isPAL            = false,
};

constexpr VersionData SLPS_11_DATA = {
    .psexePath  = "SLPS_017.97",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13d844),
    .paraOffset = PSEXE_OFFSET(0x13b344),
    .skelOffset = PSEXE_OFFSET(0x123780),
    .isPAL      = false,
};

constexpr VersionData SLPS_10_DATA = {
    .psexePath  = "SLPS_017.97",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13ce24),
    .paraOffset = PSEXE_OFFSET(0x13a924),
    .skelOffset = PSEXE_OFFSET(0x122e68),
    .isPAL      = false,
};

constexpr VersionData SLPM_DATA = {
    .psexePath  = "SLPM_804.02",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13e874),
    .paraOffset = PSEXE_OFFSET(0x13c32c),
    .skelOffset = PSEXE_OFFSET(0x124728),
    .isPAL      = false,
};

constexpr VersionData SLES02914_DATA = {
    .psexePath  = "SLES_029.14",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13ac0c),
    .paraOffset = PSEXE_OFFSET(0x138b5c),
    .skelOffset = PSEXE_OFFSET(0x122dd4),
    .isPAL      = true,
};

constexpr VersionData SLES03434_DATA = {
    .psexePath  = "SLES_034.34",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13ae00),
    .paraOffset = PSEXE_OFFSET(0x138d50),
    .skelOffset = PSEXE_OFFSET(0x122de4),
    .isPAL      = true,
};

constexpr VersionData SLES03435_DATA = {
    .psexePath  = "SLES_034.35",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13add8),
    .paraOffset = PSEXE_OFFSET(0x138d28),
    .skelOffset = PSEXE_OFFSET(0x122d6c),
    .isPAL      = true,
};

constexpr VersionData SLES03436_DATA = {
    .psexePath  = "SLES_034.36",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13b7e4),
    .paraOffset = PSEXE_OFFSET(0x139734),
    .skelOffset = PSEXE_OFFSET(0x122da0),
    .isPAL      = true,
};

constexpr VersionData SLES03437_DATA = {
    .psexePath  = "SLES_034.37",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13b314),
    .paraOffset = PSEXE_OFFSET(0x139264),
    .skelOffset = PSEXE_OFFSET(0x122da0),
    .isPAL      = true,
};

constexpr VersionData VERSION_DATA[] = { SLUS_DATA,      SLPS_11_DATA,   SLPS_10_DATA,   SLPM_DATA,     SLES02914_DATA,
                                         SLES03434_DATA, SLES03435_DATA, SLES03436_DATA, SLES03437_DATA };
// functions

VersionData getVersion(std::filesystem::path parentPath);
std::vector<DigimonEntry> loadDigimonEntries(std::filesystem::path parentPath);
std::array<MapEntry, 255> getMapEntries(std::filesystem::path parentPath);