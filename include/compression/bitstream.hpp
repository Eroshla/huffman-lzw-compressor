#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <vector>
#include <cstdint>
#include <cstddef>

class BitWriter {
    std::vector<uint8_t> buffer;
    uint8_t currentByte;
    int bitCount;

public:
    BitWriter();
    void writeBit(int bit);
    void writeBits(uint32_t value, int numBits);
    void flush();
    std::vector<uint8_t> getData() const;
    size_t getTotalBits() const;
    size_t getByteCount() const;
};

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

#endif // BITSTREAM_H
