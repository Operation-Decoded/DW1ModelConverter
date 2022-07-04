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
    Model model(meshPath, nodePath, "BOYS.PNG");
    ColladaExporter exporter(model, tim);
    exporter.getDocument().SaveFile("BOYS.dae");
    CLUTMap clutMap;
    clutMap.applyModel(model);

    tim.writeImage(clutMap, "BOYS.PNG");
    
    // TODO support for multiple images (that one arena)
    // TODO support for images being used by multiple models
    // TODO support for semi-transparent primitives

    std::cout << "Done" << std::endl;
    return 0;
}
