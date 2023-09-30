#pragma once

#include "CImg/CImg.h"
#include "CLUTMap.hpp"

#include <filesystem>
#include <map>
#include <string>
#include <utility>
#include <vector>

struct RGBA
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct TIMColor
{
    uint16_t r   : 5;
    uint16_t g   : 5;
    uint16_t b   : 5;
    uint16_t stp : 1;

    const RGBA getColor()
    {
        if (r == 0 && g == 0 && b == 0 && stp == 0) return { 0, 0, 0, 0 };   // transparent
        if (r == 0 && g == 0 && b == 0 && stp == 1) return { 0, 0, 0, 255 }; // black

        return { static_cast<uint8_t>(r << 3),
                 static_cast<uint8_t>(g << 3),
                 static_cast<uint8_t>(b << 3),
                 static_cast<uint8_t>(0xFF - stp * 0x80) };
    }
};

struct CLUT4BPP
{
    uint8_t pix1 : 4;
    uint8_t pix2 : 4;
};

union CLUTIndex
{
    uint8_t bpp8;
    CLUT4BPP bpp4;
};

struct TIMPIXELS
{
    uint32_t size;
    uint16_t orgX;
    uint16_t orgY;
    uint16_t width;
    uint16_t height;
    CLUTIndex pixels[0];
};

struct TIMCLUT
{
    uint32_t size;
    uint16_t orgX;
    uint16_t orgY;
    uint16_t colorCount;
    uint16_t paletteCount;
    TIMColor palettes[0];
};

struct TIM
{
    uint32_t magic;
    uint32_t flag;
    const TIMCLUT* clut;
    const TIMPIXELS* pixels;
};

typedef std::vector<TIMColor> TIMPalette;

class AbstractTIM
{
private:
    uint32_t width  = 0;
    uint32_t height = 0;

    uint32_t clutOrgX = 0;
    uint32_t clutOrgY = 0;

    uint32_t pixelOrgX = 0;
    uint32_t pixelOrgY = 0;

    std::vector<TIMPalette> palettes;
    std::vector<uint32_t> pixels;

public:
    AbstractTIM(const std::filesystem::path path);
    AbstractTIM(const std::vector<uint8_t>& buffer);
    void writeImage(CLUTMap& clutMap, std::filesystem::path path);
    void writeImage(int clutId, std::filesystem::path path);
    std::vector<uint8_t> getImage(CLUTMap& clutMap);

    const std::pair<uint32_t, uint32_t> getSize() { return { width, height }; }

private:
    void init(const std::vector<uint8_t>& buffer);
};