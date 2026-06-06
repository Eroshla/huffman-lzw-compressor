#ifndef BITWRITER_HPP
#define BITWRITER_HPP

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

#endif // BITWRITER_HPP
