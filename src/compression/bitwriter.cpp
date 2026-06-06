#include "compression/bitwriter.hpp"
#include <stdexcept>

BitWriter::BitWriter() : currentByte(0), bitCount(0) {}

void BitWriter::writeBit(int bit) {
    if (bit) {
        currentByte |= (1 << (7 - bitCount));
    }
    bitCount++;
    if (bitCount == 8) {
        buffer.push_back(currentByte);
        currentByte = 0;
        bitCount = 0;
    }
}

void BitWriter::writeBits(uint32_t value, int numBits) {
    for (int i = numBits - 1; i >= 0; --i) {
        writeBit((value >> i) & 1);
    }
}

void BitWriter::flush() {
    if (bitCount > 0) {
        buffer.push_back(currentByte);
        currentByte = 0;
        bitCount = 0;
    }
}

std::vector<uint8_t> BitWriter::getData() const {
    std::vector<uint8_t> res = buffer;
    if (bitCount > 0) {
        res.push_back(currentByte);
    }
    return res;
}

size_t BitWriter::getTotalBits() const {
    return buffer.size() * 8 + bitCount;
}

size_t BitWriter::getByteCount() const {
    return buffer.size() + (bitCount > 0 ? 1 : 0);
}
