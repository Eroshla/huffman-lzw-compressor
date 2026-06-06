#ifndef BITREADER_HPP
#define BITREADER_HPP

#include <cstdint>
#include <cstddef>

class BitReader {
    const uint8_t* data;
    size_t byteCount;
    size_t totalBits;
    size_t currentBit;

public:
    BitReader(const uint8_t* data, size_t totalBits);
    int readBit();
    uint32_t readBits(int numBits);
    bool hasMore() const;
};

#endif // BITREADER_HPP
