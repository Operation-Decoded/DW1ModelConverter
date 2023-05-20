#include "Collada.hpp"
#include "GLTF.hpp"
#include "Model.hpp"
#include "TIM.hpp"

#include <filesystem>
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

void dumpSkeletons()
{
    std::vector<char> data;
    std::ifstream slus("SLUS_010.32", std::ios::binary);

    auto size = std::filesystem::file_size("SLUS_010.32");
    data.resize(size);

    slus.read(data.data(), size);

    for (int i = 0; i < 180; i++)
    {
        char name[8];
        DigimonPara para;
        uint32_t skelOffset;
        memcpy(&para, (data.data() + (0x12ceb4 - 0x90000 + i * sizeof(DigimonPara))), sizeof(DigimonPara));
        memcpy(name, (data.data() + (0x133b44 - 0x90000 + i * 8)), 8);
        memcpy(&skelOffset, (data.data() + (0x11ce60 - 0x90000 + i * 4)), 4);
        skelOffset -= 0x80090000;

        std::string fileName = std::string(name) + ".node";
        std::ofstream nodeFile("nodes/" + fileName);

        nodeFile.write(data.data() + skelOffset, para.boneCount * 2);
    }
}

int main()
{
    std::filesystem::path nodePath    = "./nodes/MUGE.node";
    std::filesystem::path texturePath = "../../assets/alltim/DIGI116.TIM";
    std::filesystem::path meshPath    = "../../assets/original/CHDAT/MMD3/MUGE.MMD";

    std::filesystem::recursive_directory_iterator itr("../../assets/");

    /* iterate over all MMD files, for verification
    for (auto file : itr)
    {
        if (!file.is_regular_file()) continue;
        if (file.path().extension().compare(".MMD")) continue;
        if (std::filesystem::file_size(file.path()) == 0) continue;

        std::cout << file.path().filename() << std::endl;

        std::string nodePath = "nodes/" + file.path().stem().string() + ".node";

        Model m(file.path(), nodePath, {});
        int i = 0;
        for(auto& a : m.anims->anims)
        {
            std::cout << "Anim " << i++ << std::endl;
            Animation anim(a);
        }
    }
    */

    AbstractTIM tim(texturePath);
    Model model(meshPath, nodePath, "MUGE.PNG");
    GLTFExporter gltf(model, tim);
    gltf.save("MUGE.gltf");

    // TODO support for multiple images (that one arena)
    // TODO support for images being used by multiple models
    // TODO playSound animation data into glTF
    // TODO changeTexture animation data into glTF

    std::cout << "Done" << std::endl;
    return 0;
}
