#include "TIM.hpp"

#include "utils/ReadBuffer.hpp"

#include <fstream>

namespace cimg = cimg_library;

struct uint24_t
{
    uint32_t v : 24;
};

static uint32_t toTexturePage(uint32_t x, uint32_t y) { return (x / 64) + ((y / 256) * 16); }

RGBA AbstractTIM::getColor(CLUTMap& clutMap, uint32_t x, uint32_t y) const
{
    uint32_t clutId = 255;

    if (palettes.size() != 0)
    {
        auto& clut                = clutMap[toTexturePage(pixelOrgX, pixelOrgY)];
        ClutCoordsUnion clutCoord = { .u32 = clut(x, y) };

        if (clutCoord.u32 == 0xFFFFFFFF) return { 0 }; // no CLUT found
        if (clutCoord.coords.x != clutOrgX) throw std::runtime_error("Misalinged CLUT");

        clutId = clutCoord.coords.y - clutOrgY;
    }

    return getColor(clutId, x, y);
}

RGBA AbstractTIM::getColor(uint32_t clutId, uint32_t x, uint32_t y, bool isSemiTrans) const
{
    if (palettes.size() == 0)
        return { .rgba = pixels[(y * static_cast<uint64_t>(width)) + x] };
    else
        return palettes[clutId][pixels[(y * static_cast<uint64_t>(width)) + x]].getColor(isSemiTrans);
}

void AbstractTIM::writeImage(CLUTMap& clutMap, std::filesystem::path path) const
{
    getImage(clutMap, 0, 0, width, height).save_png(path.string().c_str());
}

void AbstractTIM::writeImage(uint32_t clutId, std::filesystem::path path) const
{
    getImage(clutId, 0, 0, width, height).save_png(path.string().c_str());
}

cimg::CImg<uint8_t> AbstractTIM::getImage(TIMPalette& palette,
                                          uint32_t x,
                                          uint32_t y,
                                          uint32_t width,
                                          uint32_t height,
                                          bool isSemiTrans) const
{
    cimg::CImg<uint8_t> new_image(width, height, 1, 4, 0);

    for (auto ly = 0; ly < height; ly++)
        for (auto lx = 0; lx < width; lx++)
        {
            auto localX = (x % this->width) + lx;
            auto localY = (y % this->height) + ly;
            auto pixel  = pixels[(localY * static_cast<uint64_t>(this->width)) + localX];
            auto color  = palette.size() >= pixel ? palette[pixel].getColor(isSemiTrans) : RGBA();
            new_image.draw_point(lx, ly, color.data, 1.0f);
        }

    return new_image;
}

cimg::CImg<uint8_t>
AbstractTIM::getImage(CLUTMap& clutMap, uint32_t x, uint32_t y, uint32_t width, uint32_t height) const
{
    cimg::CImg<uint8_t> new_image(width, height, 1, 4, 0);

    for (auto ly = 0; ly < height; ly++)
        for (auto lx = 0; lx < width; lx++)
        {
            RGBA rgba = getColor(clutMap, x + lx, y + ly);
            new_image.draw_point(lx, ly, rgba.data, 1.0f);
        }

    return new_image;
}

cimg::CImg<uint8_t>
AbstractTIM::getImage(uint32_t clutId, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool isSemiTrans) const
{
    cimg::CImg<uint8_t> new_image(width, height, 1, 4, 0);

    for (auto ly = 0; ly < height; ly++)
        for (auto lx = 0; lx < width; lx++)
        {
            RGBA rgba = getColor(clutId, x + lx, y + ly, isSemiTrans);
            new_image.draw_point(lx, ly, rgba.data, 1.0f);
        }

    return new_image;
}

std::vector<uint8_t> AbstractTIM::getRawImage(CLUTMap& clutMap) const
{
    std::vector<uint8_t> data;
    data.resize(width * height * 4);

    for (uint32_t y = 0; y < height; y++)
        for (uint32_t x = 0; x < width; x++)
        {
            RGBA rgba                       = getColor(clutMap, x, y);
            data[((y * width) + x) * 4 + 0] = rgba.r;
            data[((y * width) + x) * 4 + 1] = rgba.g;
            data[((y * width) + x) * 4 + 2] = rgba.b;
            data[((y * width) + x) * 4 + 3] = rgba.a;
        }

    return data;
}

std::vector<uint8_t> AbstractTIM::getRawImage(TIMPalette palette, bool isSemiTrans) const
{
    std::vector<uint8_t> data;
    data.resize(width * height * 4);

    for (uint32_t y = 0; y < height; y++)
        for (uint32_t x = 0; x < width; x++)
        {
            auto pixel                      = pixels[(y * static_cast<uint64_t>(width)) + x];
            auto rgba                       = palette.size() >= pixel ? palette[pixel].getColor(isSemiTrans) : RGBA();
            data[((y * width) + x) * 4 + 0] = rgba.r;
            data[((y * width) + x) * 4 + 1] = rgba.g;
            data[((y * width) + x) * 4 + 2] = rgba.b;
            data[((y * width) + x) * 4 + 3] = rgba.a;
        }

    return data;
}

AbstractTIM::AbstractTIM(const std::filesystem::path path)
{
    if (!std::filesystem::is_regular_file(path)) return;

    auto length = std::filesystem::file_size(path);

    if (length == 0) return;

    std::ifstream input(path, std::ios::binary);
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

    init(buffer.data());
}

AbstractTIM::AbstractTIM(const std::vector<uint8_t>& buffer) { init(buffer.data()); }
AbstractTIM::AbstractTIM(const uint8_t* buffer) { init(buffer); }

void AbstractTIM::init(const uint8_t* buffer)
{
    TIM tim;
    tim.magic  = reinterpret_cast<const uint32_t*>(buffer)[0];
    tim.flag   = reinterpret_cast<const TIMFlag*>(buffer)[1];
    tim.clut   = tim.flag.hasClut ? reinterpret_cast<const TIMCLUT*>(buffer + 8) : nullptr;
    tim.pixels = reinterpret_cast<const TIMPIXELS*>(buffer + 8 + (tim.flag.hasClut ? tim.clut->size : 0));

    this->clutOrgX       = tim.clut != nullptr ? tim.clut->orgX : 0;
    this->clutOrgY       = tim.clut != nullptr ? tim.clut->orgY : 0;
    this->pixelOrgX      = tim.pixels->orgX;
    this->pixelOrgY      = tim.pixels->orgY;
    this->pixelOrgWidth  = tim.pixels->width;
    this->pixelOrgHeight = tim.pixels->height;
    this->bitPerPixel    = tim.flag.pixelMode;

    this->height = tim.pixels->height;

    switch (tim.flag.pixelMode)
    {
        case 0: this->bitPerPixel = 4; break;
        case 1: this->bitPerPixel = 8; break;
        case 2: this->bitPerPixel = 16; break;
        case 3: this->bitPerPixel = 24; break;
    }

    switch (tim.flag.pixelMode)
    {
        case 0: this->width = tim.pixels->width * 4; break;
        case 1: this->width = tim.pixels->width * 2; break;
        case 2: this->width = tim.pixels->width; break;
        case 3: this->width = (tim.pixels->width * 16) / 24; break;
    }

    for (uint32_t i = 0; i < tim.clut->paletteCount; i++)
    {
        TIMPalette palette;

        for (uint32_t j = 0; j < tim.clut->colorCount; j++)
            palette.push_back(tim.clut->palettes[(i * tim.clut->colorCount) + j]);

        this->palettes.push_back(palette);
    }

    ReadBuffer pixelBuffer(const_cast<uint8_t*>(tim.pixels->pixels));

    for (uint32_t i = 0; i < tim.pixels->size - 0x0C; i++)
    {
        switch (tim.flag.pixelMode)
        {
            case 0:
            {
                CLUTIndex index = pixelBuffer.read<CLUTIndex>();
                this->pixels.push_back(index.bpp4.pix1);
                this->pixels.push_back(index.bpp4.pix2);
                break;
            }
            case 1: this->pixels.push_back(pixelBuffer.read<uint8_t>()); break;
            case 2: this->pixels.push_back(pixelBuffer.read<TIMColor>().getColor().rgba); break;
            case 3: this->pixels.push_back(pixelBuffer.read<uint24_t>().v); break;
        }
    }
}