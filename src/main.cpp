#include "GLTF.hpp"
#include "Model.hpp"
#include "TIM.hpp"

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>

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

static_assert(sizeof(DigimonPara) == 52);

struct VersionData
{
    std::string_view psexePath;
    std::string_view alltimPath;
    uint32_t nameOffset;
    uint32_t paraOffset;
    uint32_t skelOffset;
};

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

// data
/*
 *                  name     para     skel
 * SLUS_010.32  0x133B44 0x12CEB4 0x11ce60
 * SLPS_017.97  0x13d844 0x13b344 0x123780 (1.1)
 * SLPS_017.97  0x13CE24 0x13A924 0x122E68 (1.0)
 * SLPM_804.02  0x13e874 0x13c32c 0x124728
 * SLES_029.14  0x13AC0C 0x138B5C 0x122DD4
 * SLES_034.34  0x13AE00 0x138D50 0x122DE4
 * SLES_034.35  0x13ADD8 0x138D28 0x122D6C
 * SLES_034.36  0x13B7E4 0x139734 0x122DA0
 * SLES_034.37  0x13B314 0x139264 0x122DA0
 */
constexpr uint32_t PSEXE_OFFSET(uint32_t offset) { return offset - 0x90000; }

constexpr VersionData SLUS_DATA = {
    .psexePath  = "SLUS_010.32",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x133b44),
    .paraOffset = PSEXE_OFFSET(0x12ceb4),
    .skelOffset = PSEXE_OFFSET(0x11ce60),
};

constexpr VersionData SLPS_11_DATA = {
    .psexePath  = "SLPS_017.97",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13d844),
    .paraOffset = PSEXE_OFFSET(0x13b344),
    .skelOffset = PSEXE_OFFSET(0x123780),
};

constexpr VersionData SLPS_10_DATA = {
    .psexePath  = "SLPS_017.97",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13ce24),
    .paraOffset = PSEXE_OFFSET(0x13a924),
    .skelOffset = PSEXE_OFFSET(0x122e68),
};

constexpr VersionData SLPM_DATA = {
    .psexePath  = "SLPM_804.02",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13e874),
    .paraOffset = PSEXE_OFFSET(0x13c32c),
    .skelOffset = PSEXE_OFFSET(0x124728),
};

constexpr VersionData SLES02914_DATA = {
    .psexePath  = "SLES_029.14",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13ac0c),
    .paraOffset = PSEXE_OFFSET(0x138b5c),
    .skelOffset = PSEXE_OFFSET(0x122dd4),
};

constexpr VersionData SLES03434_DATA = {
    .psexePath  = "SLES_034.34",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13ae00),
    .paraOffset = PSEXE_OFFSET(0x138d50),
    .skelOffset = PSEXE_OFFSET(0x122de4),
};

constexpr VersionData SLES03435_DATA = {
    .psexePath  = "SLES_034.35",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13add8),
    .paraOffset = PSEXE_OFFSET(0x138d28),
    .skelOffset = PSEXE_OFFSET(0x122d6c),
};

constexpr VersionData SLES03436_DATA = {
    .psexePath  = "SLES_034.36",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13b7e4),
    .paraOffset = PSEXE_OFFSET(0x139734),
    .skelOffset = PSEXE_OFFSET(0x122da0),
};

constexpr VersionData SLES03437_DATA = {
    .psexePath  = "SLES_034.37",
    .alltimPath = "CHDAT/ALLTIM.TIM",
    .nameOffset = PSEXE_OFFSET(0x13b314),
    .paraOffset = PSEXE_OFFSET(0x139264),
    .skelOffset = PSEXE_OFFSET(0x122da0),
};

constexpr VersionData VERSION_DATA[] = { SLUS_DATA,      SLPS_11_DATA,   SLPS_10_DATA,   SLPM_DATA,     SLES02914_DATA,
                                         SLES03434_DATA, SLES03435_DATA, SLES03436_DATA, SLES03437_DATA };
// functions

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

std::vector<Entry> loadDigimonEntries(std::filesystem::path parentPath)
{
    using DigimonFileName = char[8];

    std::vector<Entry> digimonEntries;
    VersionData version                 = getVersion(parentPath);
    std::vector<uint8_t> data           = readFileAsVector<uint8_t>(parentPath / version.psexePath);
    std::vector<MMDTexture> textureData = readFileAsVector<MMDTexture>(parentPath / version.alltimPath);
    DigimonFileName* names              = reinterpret_cast<DigimonFileName*>(data.data() + version.nameOffset);
    DigimonPara* para                   = reinterpret_cast<DigimonPara*>(data.data() + version.paraOffset);
    uint32_t* skelOffset                = reinterpret_cast<uint32_t*>(data.data() + version.skelOffset);

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

int main(int count, char* args[])
{
    if (count < 2)
    {
        std::cout << "Usage: " << std::endl;
        std::cout << "DW1ModelConverter <pathToExtractedFolder>" << std::endl;
        std::cout << "Use tools like dumpsxiso to extract the ROM." << std::endl;
        return EXIT_SUCCESS;
    }

    std::filesystem::path dataPath = args[1];
    std::vector<Entry> entries     = loadDigimonEntries(dataPath);
    uint32_t id                    = 0;

    if (entries.size() == 0)
    {
        std::cout << "No models found, is the path correct?" << std::endl;
        return EXIT_SUCCESS;
    }

    if (!std::filesystem::exists("output"))
        if (!std::filesystem::create_directories("output"))
        {
            std::cout << "Failed to create output folder. Make sure you have the necessary permissions." << std::endl;
            return EXIT_FAILURE;
        }

    for (Entry& entry : entries)
    {
        std::filesystem::path modelPath = dataPath / std::format("CHDAT/MMD{}/{}.MMD", id++ / 30, entry.filename);

        if (!std::filesystem::exists(modelPath))
        {
            std::cout << "File " << modelPath << "does not exist, skipping." << std::endl;
            continue;
        }

        Model model(modelPath, entry.skeleton);
        AbstractTIM tim(entry.texture);
        GLTFExporter gltf(model, tim);

        bool success = gltf.save(std::format("output/{}.gltf", entry.filename));
        if (success)
            std::cout << "Written " << entry.filename << std::endl;
        else
            std::cout << "Failed to write " << entry.filename << std::endl;
    }

    // TODO support for multiple images (that one arena)
    // TODO support for images being used by multiple models
    return EXIT_SUCCESS;
}
