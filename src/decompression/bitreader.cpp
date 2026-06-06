#include "decompression/bitreader.hpp"
#include <stdexcept>

BitReader::BitReader(const uint8_t* data, size_t totalBits)
    : data(data), totalBits(totalBits), currentBit(0) {
    byteCount = (totalBits + 7) / 8;
}

int BitReader::readBit() {
    if (currentBit >= totalBits) {
        throw std::out_of_range("BitReader out of bounds");
    }
    size_t byteIndex = currentBit / 8;
    int bitIndex = 7 - (currentBit % 8);
    int bit = (data[byteIndex] >> bitIndex) & 1;
    currentBit++;
    return bit;
}

uint32_t BitReader::readBits(int numBits) {
    uint32_t value = 0;
    for (int i = 0; i < numBits; ++i) {
        value = (value << 1) | readBit();
    }
    return value;
}

bool BitReader::hasMore() const {
    return currentBit < totalBits;
}
