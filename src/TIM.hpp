#pragma once

#include "CLUTMap.hpp"

#include <CImg.h>

#include <filesystem>
#include <utility>
#include <vector>

union RGBA
{
    struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint32_t rgba;
    uint8_t data[4];
};

struct TIMColor
{
    uint16_t r   : 5;
    uint16_t g   : 5;
    uint16_t b   : 5;
    uint16_t stp : 1;

    const RGBA getColor(bool isSemiTrans = true) const
    {
        if (r == 0 && g == 0 && b == 0 && stp == 0) return { 0, 0, 0, 0 };   // transparent
        if (r == 0 && g == 0 && b == 0 && stp == 1) return { 0, 0, 0, 255 }; // black

        uint8_t alpha = isSemiTrans ? 0xFF - stp * 0x80 : 0xFF;

        return { static_cast<uint8_t>(r << 3),
                 static_cast<uint8_t>(g << 3),
                 static_cast<uint8_t>(b << 3),
                 static_cast<uint8_t>(alpha) };
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
    uint8_t pixels[0];
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

struct TIMFlag
{
    uint32_t pixelMode : 3;
    uint32_t hasClut   : 1;
};

struct TIM
{
    uint32_t magic;
    TIMFlag flag;
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

    uint32_t pixelOrgX      = 0;
    uint32_t pixelOrgY      = 0;
    uint32_t pixelOrgWidth  = 0;
    uint32_t pixelOrgHeight = 0;
    uint32_t bitPerPixel;

    std::vector<TIMPalette> palettes;
    std::vector<uint32_t> pixels;

public:
    AbstractTIM(const std::filesystem::path path);
    AbstractTIM(const std::vector<uint8_t>& buffer);
    AbstractTIM(const uint8_t* buffer);
    void writeImage(CLUTMap& clutMap, std::filesystem::path path) const;
    void writeImage(uint32_t clutId, std::filesystem::path path) const;
    RGBA getColor(uint32_t clutId, uint32_t x, uint32_t y, bool isSemiTrans = true) const;
    RGBA getColor(CLUTMap& clutMap, uint32_t x, uint32_t y) const;
    std::vector<uint8_t> getRawImage(CLUTMap& clutMap) const;
    std::vector<uint8_t> getRawImage(TIMPalette palette, bool isSemiTrans = true) const;
    cimg_library::CImg<uint8_t>
    getImage(CLUTMap& clutMap, uint32_t x, uint32_t y, uint32_t width, uint32_t height) const;
    cimg_library::CImg<uint8_t>
    getImage(uint32_t clutId, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool isSemiTrans = true) const;
    cimg_library::CImg<uint8_t> getImage(TIMPalette& palette,
                                         uint32_t x,
                                         uint32_t y,
                                         uint32_t width,
                                         uint32_t height,
                                         bool isSemiTrans = true) const;

    const std::pair<uint32_t, uint32_t> getSize() const { return { width, height }; }

    uint32_t getClutX() const { return clutOrgX; }
    uint32_t getClutY() const { return clutOrgY; }
    uint32_t getPixelX() const { return pixelOrgX; }
    uint32_t getPixelY() const { return pixelOrgY; }
    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }
    uint32_t getBitPerPixel() const { return bitPerPixel; }
    bool containsTexCoord(uint32_t x, uint32_t y) const
    {
        if (x < pixelOrgX || x >= (pixelOrgX + pixelOrgWidth)) return false;
        if (y < pixelOrgY || y >= (pixelOrgY + pixelOrgHeight)) return false;
        return true;
    }
    std::vector<TIMPalette> getPalettes() const { return palettes; }

private:
    void init(const uint8_t* buffer);
};