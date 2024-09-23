#include "GLTF.hpp"
#include "GameData.hpp"
#include "MAP.hpp"
#include "Model.hpp"
#include "TIM.hpp"

#include <filesystem>
#include <format>
#include <iostream>

void exportMaps(std::filesystem::path dataPath, std::filesystem::path outputPath)
{
    auto entries        = getMapEntries(dataPath);
    auto digimonEntries = loadDigimonEntries(dataPath);

    for (auto i = 0; i < entries.size(); i++)
    {
        auto& entry = entries[i];
        auto name   = entry.data.name;
        // entries are allowed to be empty, skip them
        if (name[0] == 0) continue;

        std::filesystem::path mapPath = dataPath / std::format("MAP/MAP{}/{}.MAP", 1 + (i / 15), name);
        std::filesystem::path tfsPath = dataPath / std::format("MAP/MAP{}/{}.TFS", 1 + (i / 15), name);

        // entries might reference files that don't exist in the final game, skip them
        if (!std::filesystem::exists(mapPath)) continue;
        if (!std::filesystem::exists(tfsPath)) continue;

        std::filesystem::path outputDir = outputPath / "maps" / name;
        std::filesystem::create_directories(outputDir);
        MapFile map(mapPath, entry);
        TFSFile tfs(tfsPath);

        std::map<uint32_t, Model> doors;
        if (entry.doors.has_value())
        {
            auto& door = entry.doors.value();
            for (auto i = 0; i < 6; i++)
            {
                auto modelId = door.modelId[i];
                if (modelId == 0xFF || doors.contains(modelId)) continue;

                doors.emplace(modelId, dataPath / std::format("DOOR/DOOR{:02}.TMD", modelId));
            }
        }

        MAPExporter exporter(map, tfs, entry, doors, entries, digimonEntries);

        bool success = exporter.save(outputDir);
        if (success)
            std::cout << "Written " << name << std::endl;
        else
            std::cout << "Failed to write " << name << std::endl;
    }
}

void exportModels(std::filesystem::path dataPath, std::filesystem::path outputPath)
{
    std::vector<DigimonEntry> entries = loadDigimonEntries(dataPath);
    std::filesystem::create_directories(outputPath / "digimon");

    if (entries.size() == 0) std::cout << "No models found, is the path correct?" << std::endl;

    for (auto id = 0; id < entries.size(); id++)
    {
        DigimonEntry& entry             = entries[id];
        std::filesystem::path modelPath = dataPath / std::format("CHDAT/MMD{}/{}.MMD", id / 30, entry.filename);

        if (!std::filesystem::exists(modelPath))
        {
            std::cout << "File " << modelPath << " does not exist, skipping." << std::endl;
            continue;
        }

        Model model(modelPath, entry.skeleton);
        AbstractTIM tim(entry.texture);
        GLTFExporter gltf(model, tim);

        bool success = gltf.save(outputPath / std::format("digimon/{}.gltf", entry.filename));
        if (success)
            std::cout << "Written " << entry.filename << std::endl;
        else
            std::cout << "Failed to write " << entry.filename << std::endl;
    }

    // TODO support for multiple images (that one arena)
    // TODO support for images being used by multiple models
}

// TODO command line switches for:
// - what to export
// - output folder
int main(int count, char* args[])
{
    const std::filesystem::path output = "output";

    if (count < 2)
    {
        std::cout << "Usage: " << std::endl;
        std::cout << "DW1ModelConverter <pathToExtractedFolder>" << std::endl;
        std::cout << "Use tools like dumpsxiso to extract the ROM." << std::endl;
        return EXIT_SUCCESS;
    }

    std::filesystem::path dataPath = args[1];

    if (!std::filesystem::exists(output))
        if (!std::filesystem::create_directories(output))
        {
            std::cout << "Failed to create output folder. Make sure you have the necessary permissions." << std::endl;
            return EXIT_FAILURE;
        }

    // exportModels(dataPath, output);
    exportMaps(dataPath, output);

    return EXIT_SUCCESS;
}
