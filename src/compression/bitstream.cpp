#include "compression/bitstream.hpp"
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
