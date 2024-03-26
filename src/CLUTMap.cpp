#include "CLUTMap.hpp"

#include "Model.hpp"

CLUTMap::CLUTMapImage& CLUTMap::at(uint32_t id)
{
    auto result = texturePages.try_emplace(id, 256, 256, 1, 1, 0);

    // newly placed, fill empty
    if (result.second) result.first->second.fill(0xFFFFFFFFu);

    return texturePages[id];
}

CLUTMap::CLUTMapImage& CLUTMap::operator[](uint32_t id) { return at(id); }

void CLUTMap::updateBlocks()
{
    for (auto& image : texturePages)
        for (int32_t blockX = 0; blockX < 32; blockX++)
            for (int32_t blockY = 0; blockY < 32; blockY++)
            {
                std::map<uint32_t, uint32_t> count;

                for (uint32_t x = 0; x < 8; x++)
                    for (uint32_t y = 0; y < 8; y++)
                    {
                        auto value = image.second(blockX * 8 + x, blockY * 8 + y);
                        if (value != 0xFFFFFFFF) count[value]++;
                    }

                std::pair<uint32_t, uint32_t> bestEntry(0xFFFFFFFF, 0);
                for (std::pair<uint32_t, uint32_t> entry : count)
                    if (entry.second > bestEntry.second) bestEntry = entry;

                image.second
                    .draw_rectangle(blockX * 8, blockY * 8, blockX * 8 + 7, blockY * 8 + 7, &bestEntry.first, 1.0f);
            }
}

void CLUTMap::applyModel(const Model& model)
{
    for (auto& mesh : model.meshes)
        for (const Face& face : mesh.faces)
        {
            if (!face.hasTexture) continue;

            auto& page = at(face.texturePage);

            ClutCoordsUnion clutCoords;
            clutCoords.coords.x = face.clutX;
            clutCoords.coords.y = face.clutY;

            page.draw_triangle(face.uv1.u, face.uv1.v, face.uv2.u, face.uv2.v, face.uv3.u, face.uv3.v, &clutCoords.u32);
        }

    for (auto& anim : model.anims.anims)
        for (auto& instr : anim.instructions)
            instr->handleTexture(*this);

    updateBlocks();
}