#include "MAP.hpp"

#include "GLTF.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>

MapFile::MapFile(const std::filesystem::path path, const MapEntry entry)
    : entry(entry)
{
    if (!std::filesystem::is_regular_file(path)) return;

    auto length = std::filesystem::file_size(path);

    if (length == 0) return;

    std::ifstream input(path, std::ios::binary);
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

    init(buffer.data());
}

MapFile::MapFile(std::vector<uint8_t>& buffer, const MapEntry entry)
    : entry(entry)
{
    init(buffer.data());
}

template<> auto ReadBuffer::read() -> MapObjects
{
    MapObjects obj;
    obj.objCount = read<decltype(obj.objCount)>();
    for (auto i = 0; i < obj.objCount; i++)
        obj.objects.push_back(read<decltype(obj.objects)::value_type>());

    obj.instanceCount = read<decltype(obj.instanceCount)>();
    for (auto i = 0; i < obj.instanceCount; i++)
        obj.instances.push_back(read<decltype(obj.instances)::value_type>());

    return obj;
}

template<> auto ReadBuffer::read() -> MapDigimon
{
    MapDigimon digimon;

    digimon.type          = read<decltype(digimon.type)>();
    digimon.aiType        = read<decltype(digimon.aiType)>();
    digimon.pos           = read<decltype(digimon.pos)>();
    digimon.rotX          = read<decltype(digimon.rotX)>();
    digimon.rotY          = read<decltype(digimon.rotY)>();
    digimon.rotZ          = read<decltype(digimon.rotZ)>();
    digimon.trackingRange = read<decltype(digimon.trackingRange)>();
    digimon.unk2          = read<decltype(digimon.unk2)>();
    digimon.scriptId      = read<decltype(digimon.scriptId)>();
    digimon.unk3          = read<decltype(digimon.unk3)>();
    digimon.hp            = read<decltype(digimon.hp)>();
    digimon.mp            = read<decltype(digimon.mp)>();
    digimon.maxHP         = read<decltype(digimon.maxHP)>();
    digimon.maxMP         = read<decltype(digimon.maxMP)>();
    digimon.offense       = read<decltype(digimon.offense)>();
    digimon.defense       = read<decltype(digimon.defense)>();
    digimon.speed         = read<decltype(digimon.speed)>();
    digimon.brains        = read<decltype(digimon.brains)>();
    digimon.bits          = read<decltype(digimon.bits)>();
    digimon.chargeMode    = read<decltype(digimon.chargeMode)>();
    digimon.unk5          = read<decltype(digimon.unk5)>();
    digimon.moves         = read<decltype(digimon.moves)>();
    digimon.moveWeights   = read<decltype(digimon.moveWeights)>();
    digimon.fleePos       = read<decltype(digimon.fleePos)>();
    digimon.waypointCount = read<decltype(digimon.waypointCount)>();
    digimon.waypointSpeed = read<decltype(digimon.waypointSpeed)>();

    for (auto i = 0; i < digimon.waypointCount; i++)
        digimon.waypoints.push_back(read<decltype(digimon.waypoints)::value_type>());

    return digimon;
}

template<> auto ReadBuffer::read() -> MapSetup
{
    MapSetup setup;

    setup.cameraOrigin       = read<decltype(setup.cameraOrigin)>();
    setup.cameraPosition     = read<decltype(setup.cameraPosition)>();
    setup.lights             = read<decltype(setup.lights)>();
    setup.unusedAmbientRed   = read<decltype(setup.unusedAmbientRed)>();
    setup.unusedAmbientGreen = read<decltype(setup.unusedAmbientGreen)>();
    setup.unusedAmbientBlue  = read<decltype(setup.unusedAmbientBlue)>();
    setup.viewerDistance     = read<decltype(setup.viewerDistance)>();
    setup.likedArea          = read<decltype(setup.likedArea)>();
    setup.dislikedArea       = read<decltype(setup.dislikedArea)>();
    setup.width              = read<decltype(setup.width)>();
    setup.height             = read<decltype(setup.height)>();

    for (auto i = 0; i < setup.width * setup.height; i++)
        setup.tiles.push_back(read<decltype(setup.tiles)::value_type>());

    return setup;
}

template<> auto ReadBuffer::read() -> MapElements
{
    MapElements elem;
    elem.spawnX          = read<decltype(elem.spawnX)>();
    elem.spawnY          = read<decltype(elem.spawnY)>();
    elem.spawnZ          = read<decltype(elem.spawnZ)>();
    elem.spawnRotation   = read<decltype(elem.spawnRotation)>();
    elem.warpTargetMap   = read<decltype(elem.warpTargetMap)>();
    elem.warpTargetSpawn = read<decltype(elem.warpTargetSpawn)>();
    elem.digimonCount    = read<decltype(elem.digimonCount)>();

    for (auto i = 0; i < elem.digimonCount; i++)
        elem.digimon.push_back(read<decltype(elem.digimon)::value_type>());

    return elem;
}

template<> auto ReadBuffer::peek(std::ptrdiff_t offset) const -> AbstractTIM
{
    return AbstractTIM(bufferStart + offset);
}

void MapFile::init(ReadBuffer buffer)
{
    // get header
    uint32_t setupOffset = buffer.read<uint32_t>();
    std::vector<uint32_t> imageOffsets1;

    for (auto i = 0; i < entry.data.numMapImages; i++)
        imageOffsets1.push_back(buffer.read<uint32_t>());
    std::vector<uint32_t> imageOffsets2;
    for (auto i = 0; i < entry.data.numMapObjects; i++)
        imageOffsets2.push_back(buffer.read<uint32_t>());

    uint32_t objectOffset = 0;
    if (entry.data.numMapImages > 0 || entry.data.numMapObjects > 0) objectOffset = buffer.read<uint32_t>();

    uint32_t elementsOffset = buffer.read<uint32_t>();
    uint32_t tileMapOffset  = buffer.read<uint32_t>();

    // read data
    setup = buffer.read<MapSetup>(setupOffset);
    for (auto offset : imageOffsets1)
        images8bpp.push_back(buffer.peek<AbstractTIM>(offset));
    for (auto offset : imageOffsets2)
        images4bpp.push_back(buffer.peek<AbstractTIM>(offset));

    if (entry.data.numMapImages > 0 || entry.data.numMapObjects > 0)
        objects = buffer.read<decltype(objects)>(objectOffset);

    elements = buffer.read<decltype(elements)>(elementsOffset);
    tileMap  = buffer.read<decltype(tileMap)>(tileMapOffset);

    if (!entry.data.flags.hasDigimon) elements.digimon.clear();
}

template<typename T> void to_json(nlohmann::ordered_json& json, const Position3D<T>& pos)
{
    json["x"] = pos.x;
    json["y"] = pos.y;
    json["z"] = pos.z;
}

void to_json(nlohmann::ordered_json& json, const MapLight& light)
{
    json["position"] = light.position;
    json["red"]      = light.red;
    json["green"]    = light.green;
    json["blue"]     = light.blue;
}

void to_json(nlohmann::ordered_json& json, const MapSetup& setup)
{
    json["camera"]["origin"]           = setup.cameraOrigin;
    json["camera"]["target"]           = setup.cameraPosition;
    json["camera"]["distance"]         = setup.viewerDistance;
    json["lights"]                     = setup.lights;
    json["preference"]["dislike"]      = setup.dislikedArea;
    json["preference"]["like"]         = setup.likedArea;
    json["unused_ambient"]["red"]      = setup.unusedAmbientRed;
    json["unused_ambient"]["green"]    = setup.unusedAmbientGreen;
    json["unused_ambient"]["blue"]     = setup.unusedAmbientBlue;
    json["background_tiles"]["width"]  = setup.width;
    json["background_tiles"]["height"] = setup.height;
    json["background_tiles"]["tiles"]  = setup.tiles;
}

void to_json(nlohmann::ordered_json& json, const MapDigimon& elements)
{
    json["type"]                      = elements.type;
    json["ai"]["ai_type"]             = elements.aiType;
    json["ai"]["tracking_range"]      = elements.trackingRange;
    json["ai"]["ai_mod"]              = elements.chargeMode;
    json["location"]["position"]      = elements.pos;
    json["location"]["rotation"]["X"] = elements.rotX;
    json["location"]["rotation"]["Y"] = elements.rotY;
    json["location"]["rotation"]["Z"] = elements.rotZ;
    json["stats"]["hp"]               = elements.hp;
    json["stats"]["mp"]               = elements.mp;
    json["stats"]["hp_max"]           = elements.maxHP;
    json["stats"]["mp_max"]           = elements.maxMP;
    json["stats"]["offense"]          = elements.offense;
    json["stats"]["defense"]          = elements.defense;
    json["stats"]["speed"]            = elements.speed;
    json["stats"]["brains"]           = elements.brains;
    json["bits"]                      = elements.bits;
    json["unk5"]                      = elements.unk5;
    json["unk2"]                      = elements.unk2;
    json["scriptId"]                  = elements.scriptId;
    json["unk3"]                      = elements.unk3;

    for (auto i = 0; i < 4; i++)
    {
        json["moves"][std::to_string(i)]["id"]     = elements.moves[i];
        json["moves"][std::to_string(i)]["weight"] = elements.moveWeights[i];
    }

    json["ai"]["flee_pos"]       = elements.fleePos;
    json["ai"]["waypoint_speed"] = elements.waypointSpeed;
    json["ai"]["waypoints"]      = elements.waypoints;
}

void to_json(nlohmann::ordered_json& json, const MapElements& elements)
{
    for (int i = 0; i < 10; i++)
    {
        json["spawn"][std::to_string(i)]["x"]        = elements.spawnX[i];
        json["spawn"][std::to_string(i)]["y"]        = elements.spawnY[i];
        json["spawn"][std::to_string(i)]["z"]        = elements.spawnZ[i];
        json["spawn"][std::to_string(i)]["rotation"] = elements.spawnRotation[i];

        json["warp"][std::to_string(i)]["map"]  = elements.warpTargetMap[i];
        json["warp"][std::to_string(i)]["spot"] = elements.warpTargetSpawn[i];
    }

    json["digimon"] = elements.digimon;
}

void to_json(nlohmann::ordered_json& json, const MapObject& obj)
{
    json["clut"]         = obj.clut;
    json["width"]        = obj.width;
    json["height"]       = obj.height;
    json["uv_x"]         = obj.uvX;
    json["uv_y"]         = obj.uvY;
    json["transparency"] = obj.transparency;
    json["3d_position"]  = obj.pos3d;
}

void to_json(nlohmann::ordered_json& json, const MapObjectInstance& obj)
{
    for (auto i = 0; i < 8; i++)
    {
        json["animation"][std::to_string(i)]["sprite"]   = obj.animState[i];
        json["animation"][std::to_string(i)]["duration"] = obj.animDuration[i];
    }

    json["position"]["x"] = obj.posX;
    json["position"]["y"] = obj.posY;
    json["flag"]          = obj.flag;
}

void to_json(nlohmann::ordered_json& json, const MapObjects& objs)
{
    json["objects"]  = objs.objects;
    json["instance"] = objs.instances;
}

nlohmann::ordered_json MapFile::to_json()
{
    nlohmann::ordered_json json;
    json["file_name"]            = entry.data.name;
    json["display_name"]         = entry.name;
    json["generator"]["name"]    = PROJECT_NAME;
    json["generator"]["version"] = PROJECT_VERSION;

    json["setup"]                 = setup;
    json["setup"]["has_daynight"] = !entry.data.flags.hasNoTimeCycle;
    json["setup"]["sound_id"]     = static_cast<uint8_t>(entry.data.flags.soundId);
    json["elements"]              = elements;
    json["objects"]               = objects;
    json["tile_map"]              = tileMap;

    if (entry.toilet.has_value())
    {
        json["toilet"]["pos1"]["x"] = entry.toilet->x1;
        json["toilet"]["pos1"]["y"] = entry.toilet->y1;
        json["toilet"]["pos2"]["x"] = entry.toilet->x2;
        json["toilet"]["pos2"]["y"] = entry.toilet->y2;
    }

    if (entry.doors.has_value())
    {
        for (auto i = 0; i < 6; i++)
        {
            json["door"][i]["model_id"] = entry.doors->modelId[i];
            json["door"][i]["x"]        = entry.doors->posX[i];
            json["door"][i]["y"]        = entry.doors->posY[i];
            json["door"][i]["z"]        = entry.doors->posZ[i];
            json["door"][i]["rotation"] = entry.doors->rotation[i];
        }
    }

    return json;
}

auto readFile(const std::filesystem::path path) -> std::vector<uint8_t>
{
    if (!std::filesystem::is_regular_file(path)) return {};

    auto length = std::filesystem::file_size(path);

    if (length == 0) return {};

    std::ifstream input(path, std::ios::binary);

    return std::vector<uint8_t>(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

TFSFile::TFSFile(const std::filesystem::path path)
{
    auto buffer = readFile(path);
    init(readFile(path).data(), buffer.size());
}

TFSFile::TFSFile(std::vector<uint8_t>& buffer) { init(buffer.data(), buffer.size()); }

void TFSFile::init(ReadBuffer buff, std::size_t fileSize)
{
    width             = buff.read<decltype(width)>();
    height            = buff.read<decltype(height)>();
    auto paletteCount = buff.read<uint32_t>();
    auto imageCount   = (fileSize - 8 - paletteCount * 512) / sizeof(TFSImage);

    for (auto i = 0; i < paletteCount; i++)
        palettes.push_back(buff.read<decltype(palettes)::value_type>());

    for (auto i = 0; i < imageCount; i++)
        images.push_back(buff.read<decltype(images)::value_type>());
}

cimg_library::CImg<uint8_t> TFSImage::getImage(std::array<TIMColor, 256> palette)
{
    cimg_library::CImg<uint8_t> new_image(128, 128, 1, 4, 0);

    for (auto y = 0; y < 128; y++)
        for (auto x = 0; x < 128; x++)
        {
            RGBA color = palette[data[y][x]].getColor(false);
            new_image.draw_point(x, y, color.data, 1.0f);
        }

    return new_image;
}

cimg_library::CImg<uint8_t> TFSFile::getImage(uint32_t paletteId, MapFile& map)
{
    if (palettes.size() <= paletteId) return {};

    auto& pal    = palettes[paletteId];
    auto width   = map.setup.width;
    auto height  = map.setup.height;
    auto imageId = 0;

    cimg_library::CImg<uint8_t> new_image(width * 128, height * 128, 1, 4, 0);

    for (auto h = 0; h < height; h++)
        for (auto w = 0; w < width; w++)
        {
            auto tileId = map.setup.tiles[w + h * width];
            if (tileId == -1) continue;

            new_image.draw_image(w * 128, h * 128, images[imageId++].getImage(pal));
        }

    return new_image;
}
auto TFSFile::getImages(MapFile& map) -> std::vector<cimg_library::CImg<uint8_t>>
{
    std::vector<cimg_library::CImg<uint8_t>> images;

    for (auto i = 0; i < palettes.size(); i++)
        images.push_back(getImage(i, map));

    return images;
}

void MAPExporter::saveObject(MapObject& obj, TIMPalette& pal, std::filesystem::path path, bool is4bpp)
{
    auto pixelX = 384 + obj.uvX / (is4bpp ? 4 : 2);

    auto image = map.getImageByTexCoord(pixelX, obj.uvY);
    if (image.has_value() && obj.width > 0 && obj.height > 0)
    {
        auto png = image.value()->getImage(pal, obj.uvX, obj.uvY, obj.width, obj.height, obj.transparency != 4);
        png.save_png(path.c_str());
    }
    else
    {
        std::cout << "Object with invalid image data detected.\n";
        cimg_library::CImg<uint8_t> png(std::max<uint16_t>(obj.width, 1), std::max<uint16_t>(obj.height, 1), 1, 4, 0);
        png.save_png(path.c_str());
    }
}

bool MAPExporter::save(std::filesystem::path outputDir)
{
    // write JSON
    auto json = map.to_json();
    for (int i = 0; i < 10; i++)
    {
        auto& val = json["elements"]["warp"][std::to_string(i)]["map"];
        if (val < mapEntries.size())
            val = mapEntries[val].data.name;
        else
            val = "";
    }
    std::ofstream(outputDir / "map.json") << json.dump(2);

    // write background images
    auto images = tfs.getImages(map);
    for (auto i = 0; i < images.size(); i++)
        images[i].save_png((outputDir / std::format("background_{}.png", i)).c_str());

    // write object images
    std::map<uint32_t, TIMPalette> clutMapping = getCLUTMap();

    for (auto objId = 0; objId < map.objects.objects.size(); objId++)
    {
        auto& obj = map.objects.objects[objId];

        if (obj.clut == 0xFFFF)
        {
            for (int i = 0; i < tfs.palettes.size(); i++)
            {
                auto path = outputDir / std::format("object_{}_{}.png", objId, i);
                saveObject(obj, clutMapping[481 + i], path);
            }
        }
        else if (obj.clut < 16)
        {
            auto path    = outputDir / std::format("object_{}.png", objId);
            auto begin   = clutMapping[486].begin() + obj.clut * 16;
            auto end     = begin + 16;
            auto palette = TIMPalette(begin, end);
            saveObject(obj, palette, path, true);
        }
        else
        {
            auto path = outputDir / std::format("object_{}.png", objId);
            saveObject(obj, clutMapping[468 + obj.clut], path);
        }
    }

    // export doors
    for (auto& [id, model] : doors)
    {
        auto image = map.getImageByTexCoord(model.getTexturePage() * 64, 0);
        auto pal   = clutMapping[model.getClutY()];
        pal        = TIMPalette(pal.begin() + model.getClutX(), pal.end());

        GLTFExporter exporter(model, **image, pal);
        exporter.save(outputDir / std::format("door_{}.gltf", id));
    }

    return true;
}

std::map<uint32_t, TIMPalette> MAPExporter::getCLUTMap()
{
    std::map<uint32_t, TIMPalette> clutMap;

    // make sure 486 is always filled, since it is always assumed to exist
    clutMap[486] = TIMPalette(256);

    // TFS palettes can be used by map images, they fill 481-483, 480 is used for the "active" CLUT
    for (int i = 0; i < tfs.palettes.size(); i++)
        clutMap[481 + i] = TIMPalette(tfs.palettes[i].begin(), tfs.palettes[i].end());
    clutMap[480] = clutMap[481];

    // each image can put it's CLUT anywhere, or they may reuse the TFS cluts
    for (auto& img : map.images8bpp)
    {
        auto clutY = img.getClutY();
        if (clutY == 480) continue;

        auto palettes = img.getPalettes();

        for (int i = 0; i < palettes.size(); i++)
            clutMap[clutY + i] = palettes[i];
    }

    // CLUT 486 is used for up to 16 4bpp images. It's taken from the first 4bpp image, everything else is ignored
    if (!map.images4bpp.empty())
    {
        TIMPalette finalPalette;
        for (auto palette : map.images4bpp[0].getPalettes())
            finalPalette.insert(finalPalette.end(), palette.begin(), palette.begin() + 16);

        clutMap[486] = finalPalette;
    }

    return clutMap;
}