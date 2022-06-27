#include "Animation.hpp"

static float axisFactor(Axis axis)
{
    switch (axis)
    {
        case Axis::SCALE_X:
        case Axis::SCALE_Y:
        case Axis::SCALE_Z: return 1.0f / 4096.0f;
        case Axis::ROT_X:
        case Axis::ROT_Y:
        case Axis::ROT_Z: return 360.0f / 4096.0f;
        default: return 1.0f;
    }
}

AnimNodeData::AnimNodeData(const Position& pos)
{
    data[Axis::SCALE_X].push_back(std::make_pair(0.0f, pos.scaleX * axisFactor(Axis::SCALE_X)));
    data[Axis::SCALE_Y].push_back(std::make_pair(0.0f, pos.scaleY * axisFactor(Axis::SCALE_Y)));
    data[Axis::SCALE_Z].push_back(std::make_pair(0.0f, pos.scaleZ * axisFactor(Axis::SCALE_Z)));
    data[Axis::ROT_X].push_back(std::make_pair(0.0f, pos.rotX * axisFactor(Axis::ROT_X)));
    data[Axis::ROT_Y].push_back(std::make_pair(0.0f, pos.rotY * axisFactor(Axis::ROT_Y)));
    data[Axis::ROT_Z].push_back(std::make_pair(0.0f, pos.rotZ * axisFactor(Axis::ROT_Z)));
    data[Axis::POS_X].push_back(std::make_pair(0.0f, pos.posX));
    data[Axis::POS_Y].push_back(std::make_pair(0.0f, pos.posY));
    data[Axis::POS_Z].push_back(std::make_pair(0.0f, pos.posZ));
}

Animation::Animation(const MMDAnimation& anim)
{
    for (auto pos : anim.initialPositions)
    {
        nodeData.emplace_back(pos);
        momentumData.emplace_back();
    }

    while (mtnFrame <= anim.frameCount)
    {
        while (anim.instructions.size() > currentIndex && anim.instructions[currentIndex]->run(*this))
            currentIndex++;

        mtnFrame++;
        keyFrame++;
    }

    for (uint32_t node = 0; node < momentumData.size(); node++)
    {
        setMomentum(Axis::SCALE_X, node, 0.0f);
        setMomentum(Axis::SCALE_Y, node, 0.0f);
        setMomentum(Axis::SCALE_Z, node, 0.0f);
        setMomentum(Axis::ROT_X, node, 0.0f);
        setMomentum(Axis::ROT_Y, node, 0.0f);
        setMomentum(Axis::ROT_Z, node, 0.0f);
        setMomentum(Axis::POS_X, node, 0.0f);
        setMomentum(Axis::POS_Y, node, 0.0f);
        setMomentum(Axis::POS_Z, node, 0.0f);
    }
}

void Animation::setMomentum(const Axis axis, uint32_t node, float value)
{
    auto oldMomentum         = momentumData[node][axis];
    momentumData[node][axis] = { keyFrame, value };

    auto oldPos     = nodeData[node].data[axis].back();
    auto frameCount = keyFrame - oldMomentum.first;

    nodeData[node].data[axis].push_back({ (keyFrame - 1) / 20.0f, oldPos.second + (oldMomentum.second * frameCount) });
}

bool KeyframeInstruction::run(Animation& anim)
{
    if (anim.mtnFrame != timecode) return false;

    for (auto entry : entries)
        for (auto value : entry.values)
            anim.setMomentum(value.first, entry.affectedNode, value.second * axisFactor(value.first));

    return true;
}

bool LoopStartInstruction::run(Animation& anim)
{
    anim.jumpbackIndex = anim.currentIndex;
    anim.loopCount     = loopCount;

    return true;
}

bool LoopEndInstruction::run(Animation& anim)
{
    if (anim.mtnFrame != timecode) return false; // not yet active
    if (anim.loopCount == 0xFF) return true;     // endless loop, we skip those

    anim.mtnFrame = newTime;
    anim.loopCount--;

    if (anim.loopCount != 0) anim.currentIndex = anim.jumpbackIndex;

    return true;
}

bool PlaySoundInstruction::run(Animation& anim)
{
    if (anim.mtnFrame != timecode) return false;

    return true;
}

bool TextureInstruction::run(Animation& anim)
{
    if (anim.mtnFrame != timecode) return false;

    return true;
}

KeyframeEntry::KeyframeEntry(ReadBuffer& buffer)
{
    uint16_t instruction = buffer.read<uint16_t>();
    enabledAxis          = (instruction & 0x7FC0) >> 6;
    affectedNode         = instruction & 0x3F;
    scale                = buffer.read<uint16_t>();

    for (int32_t i = 8; i >= 0; i--)
    {
        if ((enabledAxis & (1 << i)) == 0) continue;
        values.push_back(std::make_pair(static_cast<Axis>(8 - i), buffer.read<int16_t>() / (float)scale));
    }
}

KeyframeInstruction::KeyframeInstruction(uint32_t instruction, ReadBuffer& buffer)
{
    timecode = instruction & 0x0FFF;

    while (buffer.peek<uint16_t>() & 0x8000)
        entries.emplace_back(buffer);
}

LoopEndInstruction::LoopEndInstruction(uint32_t instruction, ReadBuffer& buffer)
{
    timecode = instruction & 0x0FFF;
    newTime  = buffer.read<uint16_t>();
}

PlaySoundInstruction::PlaySoundInstruction(uint32_t instruction, ReadBuffer& buffer)
{
    timecode = instruction & 0x0FFF;
    soundId  = buffer.read<uint8_t>();
    vabId    = buffer.read<uint8_t>();
}

TextureInstruction::TextureInstruction(uint32_t instruction, ReadBuffer& buffer)
{
    timecode = instruction & 0x0FFF;
    srcY     = buffer.read<uint8_t>();
    srcX     = buffer.read<uint8_t>();
    height   = buffer.read<uint8_t>();
    width    = buffer.read<uint8_t>();
    destY    = buffer.read<uint8_t>();
    destX    = buffer.read<uint8_t>();
}

void TextureInstruction::handleTexture(CLUTMap& clutMap)
{
    for (auto& entry : clutMap.texturePages)
    {
        auto subImage = entry.second.get_crop(destX * 4, destY, (destX + width) * 4, destY + height);
        entry.second.draw_image(srcX * 4, srcY, subImage, 1.0f);
    }
}

MMDAnimation::MMDAnimation(ReadBuffer& buffer, std::size_t boneCount)
{
    frameCount    = buffer.read<uint16_t>();
    bool hasScale = frameCount & 0x8000;
    frameCount &= 0x7FFF;

    initialPositions.emplace_back();

    for (auto i = 1u; i < boneCount; i++)
    {
        Position pos;
        if (hasScale)
        {
            pos.scaleX = buffer.read<int16_t>();
            pos.scaleY = buffer.read<int16_t>();
            pos.scaleZ = buffer.read<int16_t>();
        }
        pos.rotX = buffer.read<int16_t>();
        pos.rotY = buffer.read<int16_t>();
        pos.rotZ = buffer.read<int16_t>();
        pos.posX = buffer.read<int16_t>();
        pos.posY = buffer.read<int16_t>();
        pos.posZ = buffer.read<int16_t>();

        initialPositions.push_back(pos);
    }

    while (true)
    {
        uint16_t instruction = buffer.read<uint16_t>();

        if (instruction == 0x0000) break; // end of animation data

        switch (instruction & 0xF000)
        {
            case 0x0000: // keyframe
            {
                uint16_t timeCode = instruction & 0xFFF;
                instructions.emplace_back(new KeyframeInstruction(instruction, buffer));
                break;
            }
            case 0x1000: // loop start
            {
                instructions.emplace_back(new LoopStartInstruction(instruction));
                break;
            }
            case 0x2000: // loop end
            {
                instructions.emplace_back(new LoopEndInstruction(instruction, buffer));
                break;
            }
            case 0x3000: // change texture
            {
                instructions.emplace_back(new TextureInstruction(instruction, buffer));
                break;
            }
            case 0x4000: // play sound
            {
                instructions.emplace_back(new PlaySoundInstruction(instruction, buffer));
                break;
            }
        }
    }
}

MMDAnimations::MMDAnimations(ReadBuffer& buffer, std::size_t boneCount)
{
    uint32_t animCount = buffer.peek<uint32_t>() / 4;

    std::vector<uint32_t> animOffsets;
    for (uint32_t i = 0; i < animCount; i++)
        animOffsets.push_back(buffer.read<uint32_t>());

    for (auto a : animOffsets)
    {
        if (a == 0)
            anims.emplace_back();
        else
        {
            buffer.setPosition(a);
            anims.emplace_back(buffer, boneCount);
        }
    }
}

MMDAnimations::MMDAnimations() {}
MMDAnimation::MMDAnimation()
    : frameCount(0)
{
}