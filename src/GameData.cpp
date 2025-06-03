
#include "GameData.hpp"

#include <array>
#include <fstream>

template<typename T> std::vector<T> readFileAsVector(std::filesystem::path path)
{
    std::vector<T> data;
    std::ifstream slus(path, std::ios::binary);
    auto size = std::filesystem::file_size(path);
    data.resize(size / sizeof(T));
    slus.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

VersionData getVersion(std::filesystem::path parentPath)
{
    VersionData version;

    if (std::filesystem::exists(parentPath / SLPS_10_DATA.psexePath))
    {
        auto size = std::filesystem::file_size(parentPath / SLPS_10_DATA.psexePath);

        if (size == 0xAF000)
            return SLPS_11_DATA;
        else if (size == 0xAE000)
            return SLPS_10_DATA;
    }

    for (VersionData v : VERSION_DATA)
        if (std::filesystem::exists(parentPath / v.psexePath)) return v;

    return SLUS_DATA;
}

std::vector<DigimonEntry> loadDigimonEntries(std::filesystem::path parentPath)
{
    using DigimonFileName = char[8];

    std::vector<DigimonEntry> digimonEntries;
    VersionData version                 = getVersion(parentPath);
    std::vector<uint8_t> data           = readFileAsVector<uint8_t>(parentPath / version.psexePath);
    std::vector<MMDTexture> textureData = readFileAsVector<MMDTexture>(parentPath / version.alltimPath);
    DigimonFileName* names              = reinterpret_cast<DigimonFileName*>(data.data() + version.nameOffset);
    DigimonPara* para                   = reinterpret_cast<DigimonPara*>(data.data() + version.paraOffset);
    DigimonParaPAL* paraPAL             = reinterpret_cast<DigimonParaPAL*>(data.data() + version.paraOffset);
    uint32_t* skelOffset                = reinterpret_cast<uint32_t*>(data.data() + version.skelOffset);

    for (int i = 0; i < 180; i++)
    {
        NodeEntry* skeletonOffset = reinterpret_cast<NodeEntry*>(data.data() + skelOffset[i] - 0x80090000);
        int32_t boneCount         = version.isPAL ? paraPAL[i].boneCount : para[i].boneCount;

        DigimonEntry entry;
        entry.filename = std::string(names[i]);

        for (int32_t j = 0; j < boneCount; j++)
            entry.skeleton.push_back(skeletonOffset[j]);

        std::copy(textureData[i].buffer, textureData[i].buffer + sizeof(MMDTexture), std::back_inserter(entry.texture));
        digimonEntries.push_back(entry);
    }

    return digimonEntries;
}

std::array<MapEntry, 255> getMapEntries(std::filesystem::path parentPath)
{
    using MapName = char[28];

    std::array<MapEntry, 255> entries;
    VersionData version       = getVersion(parentPath);
    std::vector<uint8_t> data = readFileAsVector<uint8_t>(parentPath / version.psexePath);
    MapEntryData* mapData     = reinterpret_cast<MapEntryData*>(data.data() + version.mapEntryOffset);
    ToiletData* toiletData    = reinterpret_cast<ToiletData*>(data.data() + version.toiletDataOffset);
    DoorData* doorData        = reinterpret_cast<DoorData*>(data.data() + version.doorDataOffset);
    uint32_t* mapNamePtr      = reinterpret_cast<uint32_t*>(data.data() + version.mapNamePtrOffset);

    for (auto i = 0; i < 255; i++)
    {
        auto entryData = mapData[i];

        entries[i].data = entryData;
        if (entryData.toiletId != 0) entries[i].toilet = toiletData[entryData.toiletId - 1];
        if (entryData.doorsId != 0) entries[i].doors = doorData[entryData.doorsId - 1];

        std::string_view view =
            *reinterpret_cast<MapName*>(data.data() + PSEXE_OFFSET(mapNamePtr[entryData.loadingNameId]));
        view.remove_suffix(view.size() - view.find_last_not_of(' ') - 1);
        view.remove_prefix(view.find_first_not_of(' '));
        entries[i].name = view;
    }

    return entries;
}