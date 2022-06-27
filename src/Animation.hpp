#pragma once

#include "CLUTMap.hpp"
#include "utils/ReadBuffer.hpp"

#include <stdint.h>

#include <map>
#include <memory>
#include <vector>

// forward declaration
class Animation;

struct Position
{
    int16_t scaleX = 0x1000;
    int16_t scaleY = 0x1000;
    int16_t scaleZ = 0x1000;
    int16_t posX   = 0;
    int16_t posY   = 0;
    int16_t posZ   = 0;
    int16_t rotX   = 0;
    int16_t rotY   = 0;
    int16_t rotZ   = 0;
};

enum class Axis
{
    SCALE_X,
    SCALE_Y,
    SCALE_Z,

    ROT_X,
    ROT_Y,
    ROT_Z,

    POS_X,
    POS_Y,
    POS_Z,
};

class Instruction
{
public:
    Instruction()                               = default;
    Instruction(const Instruction&)             = delete; // can't copy
    Instruction& operator=(const Instruction&&) = delete; // can't copy

    virtual bool run(Animation& anim) = 0;
    virtual void handleTexture(CLUTMap& clutMap){};
};

struct KeyframeEntry
{
    uint16_t enabledAxis;
    uint8_t affectedNode;
    uint16_t scale;
    std::vector<std::pair<Axis, float>> values{};

    KeyframeEntry(ReadBuffer& buffer);
};

class KeyframeInstruction : public Instruction
{
    uint32_t timecode;
    std::vector<KeyframeEntry> entries;

public:
    KeyframeInstruction(uint32_t instruction, ReadBuffer& buffer);
    bool run(Animation& anim) override;
};

class LoopStartInstruction : public Instruction
{
    uint32_t loopCount;

public:
    LoopStartInstruction(uint32_t instruction) { loopCount = instruction & 0x00FF; }
    bool run(Animation& anim) override;
};

class LoopEndInstruction : public Instruction
{
    uint32_t timecode;
    uint32_t newTime;

public:
    LoopEndInstruction(uint32_t instruction, ReadBuffer& buffer);
    bool run(Animation& anim) override;
};

class PlaySoundInstruction : public Instruction
{
    uint32_t timecode;
    uint8_t vabId;
    uint8_t soundId;

public:
    PlaySoundInstruction(uint32_t instruction, ReadBuffer& buffer);
    bool run(Animation& anim) override;
};

class TextureInstruction : public Instruction
{
    uint32_t timecode;
    uint8_t srcX;
    uint8_t srcY;
    uint8_t width;
    uint8_t height;
    uint8_t destX;
    uint8_t destY;

public:
    TextureInstruction(uint32_t instruction, ReadBuffer& buffer);
    bool run(Animation& anim) override;
    void handleTexture(CLUTMap& clutMap) override;
};

class MMDAnimation
{
public:
    uint32_t frameCount;
    std::vector<Position> initialPositions;
    std::vector<std::unique_ptr<Instruction>> instructions;

    MMDAnimation(ReadBuffer& buffer, std::size_t boneCount);
    MMDAnimation();
};

class MMDAnimations
{
public:
    std::vector<MMDAnimation> anims;

    MMDAnimations(ReadBuffer& buffer, std::size_t boneCount);
    MMDAnimations();
};

struct AnimNodeData
{
    std::map<Axis, std::vector<std::pair<float, float>>> data;
    AnimNodeData(const Position& pos);
};

typedef std::map<Axis, std::pair<uint32_t, float>> MomentumData;

class Animation
{
    std::vector<AnimNodeData> nodeData;
    std::vector<MomentumData> momentumData;

    uint32_t mtnFrame = 1; // used by MTN data, can jump
    uint32_t keyFrame = 1; // used by keyframe data, is continuious

    uint32_t currentIndex  = 0;
    uint32_t jumpbackIndex = 0;
    uint32_t loopCount     = 0;

    void setMomentum(const Axis axis, uint32_t node, float value);

    friend Instruction;
    friend KeyframeInstruction;
    friend LoopStartInstruction;
    friend LoopEndInstruction;
    friend PlaySoundInstruction;
    friend TextureInstruction;

public:
    Animation(const MMDAnimation& anim);

    const std::vector<AnimNodeData>& getData() const { return nodeData; }
};