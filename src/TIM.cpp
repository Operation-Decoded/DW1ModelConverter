#include "TIM.hpp"

#include <fstream>
#include <iostream>

namespace cimg = cimg_library;

static uint32_t toTexturePage(uint32_t x, uint32_t y) { return (x / 64) + ((y / 256) * 16); }

void AbstractTIM::writeImage(CLUTMap& clutMap, std::filesystem::path path)
{
    cimg::CImg<uint8_t> new_image(width, height, 1, 4, 0);
    auto& clut = clutMap[toTexturePage(pixelOrgX, pixelOrgY)];

    for (uint32_t y = 0; y < height; y++)
        for (uint32_t x = 0; x < width; x++)
        {
            ClutCoordsUnion clutCoord = { .u32 = clut(x, y) };

            if (clutCoord.u32 == 0xFFFFFFFF) continue; // no CLUT found
            if (clutCoord.coords.x != clutOrgX) throw std::runtime_error("Misalinged CLUT");

            uint32_t palette = clutCoord.coords.y - clutOrgY;

            TIMColor color = palettes[palette][pixels[(y * static_cast<uint64_t>(width)) + x]];
            RGBA rgba      = color.getColor();
            new_image.draw_point(x, y, reinterpret_cast<uint8_t*>(&rgba), 1.0f);
        }

    new_image.save_png(path.string().c_str());
}

std::vector<uint8_t> AbstractTIM::getImage(CLUTMap& clutMap)
{
    std::vector<uint8_t> data;
    data.resize(width * height * 4);
    auto& clut = clutMap[toTexturePage(pixelOrgX, pixelOrgY)];

    for (uint32_t y = 0; y < height; y++)
        for (uint32_t x = 0; x < width; x++)
        {
            ClutCoordsUnion clutCoord = { .u32 = clut(x, y) };

            if (clutCoord.u32 == 0xFFFFFFFF) continue; // no CLUT found
            if (clutCoord.coords.x != clutOrgX) throw std::runtime_error("Misalinged CLUT");

            uint32_t palette = clutCoord.coords.y - clutOrgY;

            TIMColor color                  = palettes[palette][pixels[(y * static_cast<uint64_t>(width)) + x]];
            RGBA rgba                       = color.getColor();
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

    init(path.filename().string(), buffer);
}

AbstractTIM::AbstractTIM(const std::string name, const std::vector<uint8_t>& buffer) { init(name, buffer); }

void AbstractTIM::init(const std::string name, const std::vector<uint8_t>& buffer)
{
    this->name = name;

    TIM tim;
    tim.magic  = reinterpret_cast<const uint32_t*>(buffer.data())[0];
    tim.flag   = reinterpret_cast<const uint32_t*>(buffer.data())[1];
    tim.clut   = tim.flag & 8 ? reinterpret_cast<const TIMCLUT*>(buffer.data() + 8) : nullptr;
    tim.pixels = reinterpret_cast<const TIMPIXELS*>(buffer.data() + 8 + (tim.flag & 8 ? tim.clut->size : 0));

    if (tim.flag != 8 && tim.flag != 9) std::cout << "Direct color format detected..." << std::endl;

    this->clutOrgX  = tim.clut->orgX;
    this->clutOrgY  = tim.clut->orgY;
    this->pixelOrgX = tim.pixels->orgX;
    this->pixelOrgY = tim.pixels->orgY;

    this->height = tim.pixels->height;
    this->width  = tim.pixels->width * ((tim.flag & 1) ? 2 : 4);

    for (uint32_t i = 0; i < tim.clut->paletteCount; i++)
    {
        TIMPalette palette;

        for (uint32_t j = 0; j < tim.clut->colorCount; j++)
            palette.push_back(tim.clut->palettes[(i * tim.clut->colorCount) + j]);

        this->palettes.push_back(palette);
    }

    for (uint32_t i = 0; i < tim.pixels->size - 0x0C; i++)
    {
        CLUTIndex index = tim.pixels->pixels[i];

        if (tim.flag & 1)
            this->pixels.push_back(index.bpp8);
        else
        {
            this->pixels.push_back(index.bpp4.pix1);
            this->pixels.push_back(index.bpp4.pix2);
        }
    }
}