#pragma once

#include <cstddef>
#include <iterator>

class ReadBuffer
{
private:
    char* bufferStart;
    char* bufferCurrent;

public:
    ReadBuffer(ReadBuffer&) = delete;
    ReadBuffer(char* buffer)
        : bufferStart(buffer)
        , bufferCurrent(buffer)
    {
    }

    void reset() { bufferCurrent = bufferStart; }
    void skip(std::size_t count) { bufferCurrent += count; }
    void setPosition(std::size_t position) { bufferCurrent = bufferStart + position; }
    std::size_t getPosition() { return std::distance(bufferStart, bufferCurrent); }

    template<typename T> T read();
    template<typename T> T peek();
};

template<typename T> T ReadBuffer::read()
{
    T val = *reinterpret_cast<T*>(bufferCurrent);
    bufferCurrent += sizeof(T);
    return val;
}

template<typename T> T ReadBuffer::peek() { return *reinterpret_cast<T*>(bufferCurrent); }
