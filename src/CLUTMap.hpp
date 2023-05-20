#pragma once

#include "CImg/CImg.h"

#include <cstdint>
#include <map>

class Model;

struct ClutCoords
{
    uint16_t x;
    uint16_t y;
};

union ClutCoordsUnion
{
    ClutCoords coords;
    uint32_t u32;
};

class CLUTMap
{
public:
    using CLUTMapImage = cimg_library::CImg<uint32_t>;

    std::map<uint32_t, CLUTMapImage> texturePages;

    CLUTMapImage& at(uint32_t id);
    CLUTMapImage& operator[](uint32_t id);
    void updateBlocks();
    void applyModel(Model& model);
};