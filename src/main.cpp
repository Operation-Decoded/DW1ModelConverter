#include "Collada.hpp"
#include "Model.hpp"
#include "TIM.hpp"

#include <filesystem>
#include <iostream>

int main()
{
    std::filesystem::path nodePath    = "..\\..\\..\\..\\assets\\skeletons\\BOYS.node";
    std::filesystem::path texturePath = "..\\..\\..\\..\\assets\\original\\CHDAT\\ALLTIM.TIM";
    std::filesystem::path meshPath    = "..\\..\\..\\..\\assets\\original\\CHDAT\\MMD0\\BOYS.MMD";

    AbstractTIM tim(texturePath);
    Model model(meshPath, nodePath, texturePath);
    ColladaExporter exporter(model, tim);
    exporter.getDocument().SaveFile("BOYS.dae");
    CLUTMap clutMap(model);

    tim.writeImage(clutMap, "BOYS.png");

    // TODO support transparency channel for textures
    // TODO support for texture+color primitives
    // TODO support for semi-transparent primitives

    std::cout << "Done" << std::endl;
    return 0;
}
