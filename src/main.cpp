#include "GLTF.hpp"
#include "Model.hpp"
#include "TIM.hpp"

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>

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

static_assert(sizeof(DigimonPara) == 52);

constexpr std::string_view alltimPath = "CHDAT/ALLTIM.TIM";
constexpr std::string_view psexePath  = "SLUS_010.32";

using DigimonFileName = char[8];
struct MMDTexture
{
    uint8_t buffer[0x4800];
};

struct Entry
{
    std::string filename;
    DigimonPara para;
    std::vector<NodeEntry> skeleton;
    std::vector<uint8_t> texture;
};

template<typename T> std::vector<T> readFileAsVector(std::filesystem::path path)
{
    std::vector<T> data;
    std::ifstream slus(path, std::ios::binary);
    auto size = std::filesystem::file_size(path);
    data.resize(size / sizeof(T));
    slus.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

std::vector<Entry> loadDigimonEntries(std::filesystem::path parentPath)
{
    std::vector<Entry> digimonEntries;
    std::vector<uint8_t> data           = readFileAsVector<uint8_t>(parentPath / psexePath);
    std::vector<MMDTexture> textureData = readFileAsVector<MMDTexture>(parentPath / alltimPath);
    DigimonFileName* names              = reinterpret_cast<DigimonFileName*>(data.data() + (0x133b44 - 0x90000));
    DigimonPara* para                   = reinterpret_cast<DigimonPara*>(data.data() + (0x12ceb4 - 0x90000));
    uint32_t* skelOffset                = reinterpret_cast<uint32_t*>(data.data() + (0x11ce60 - 0x90000));

    for (int i = 0; i < 180; i++)
    {
        NodeEntry* skeletonOffset = reinterpret_cast<NodeEntry*>(data.data() + skelOffset[i] - 0x80090000);

        Entry entry;
        entry.filename = std::string(names[i]);
        entry.para     = para[i];

        for (int32_t j = 0; j < entry.para.boneCount; j++)
            entry.skeleton.push_back(skeletonOffset[j]);

        std::copy(textureData[i].buffer, textureData[i].buffer + sizeof(MMDTexture), std::back_inserter(entry.texture));
        digimonEntries.push_back(entry);
    }

    return digimonEntries;
}

int main()
{
    constexpr uint32_t id = 0;
    std::filesystem::path dataPath = "../../assets/original/";
    std::vector<Entry> entries     = loadDigimonEntries(dataPath);

    std::filesystem::path modelPath  = dataPath / std::format("CHDAT/MMD{}/{}.MMD", id / 30, entries[id].filename);
    std::filesystem::path outputName = std::format("{}.gltf", entries[id].filename);
    AbstractTIM tim(entries[id].filename, entries[id].texture);
    Model model(modelPath, entries[id].skeleton);
    GLTFExporter gltf(model, tim);

    gltf.save(outputName);

    // TODO support for multiple images (that one arena)
    // TODO support for images being used by multiple models
    return 0;
}
