#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>

class ReadBuffer
{
private:
    uint8_t* bufferStart;
    uint8_t* bufferCurrent;

public:
    ReadBuffer(ReadBuffer&) = delete;
    ReadBuffer(uint8_t* buffer)
        : bufferStart(buffer)
        , bufferCurrent(buffer)
    {
    }

    void reset() { bufferCurrent = bufferStart; }
    void skip(std::ptrdiff_t count) { bufferCurrent += count; }
    void setPosition(std::ptrdiff_t position) { bufferCurrent = bufferStart + position; }
    auto getPosition() const -> auto { return std::distance(bufferStart, bufferCurrent); }

    template<typename T> T read();
    template<typename T> T read(std::ptrdiff_t offset);
    template<typename T> T peek() const;
    template<typename T> T peek(std::ptrdiff_t offset) const;
};

template<typename T> auto ReadBuffer::read() -> T
{
    T val = *reinterpret_cast<T*>(bufferCurrent);
    bufferCurrent += sizeof(T);
    return val;
}

template<typename T> auto ReadBuffer::read(std::ptrdiff_t offset) -> T
{
    setPosition(offset);
    return read<T>();
}

template<typename T> auto ReadBuffer::peek() const -> T { return *reinterpret_cast<T*>(bufferCurrent); }

template<typename T> auto ReadBuffer::peek(std::ptrdiff_t offset) const -> T
{
    return *reinterpret_cast<T*>(bufferStart + offset);
}
