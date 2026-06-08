#pragma once

#include <vector>
#include <cstdint>
#include <string>

class Compressor {
public:
    virtual std::vector<uint8_t> compress(const std::vector<uint8_t>& data) = 0;
    virtual std::string name() const = 0;
    virtual ~Compressor() = default;
};

class Decompressor {
public:
    virtual std::vector<uint8_t> decompress(const std::vector<uint8_t>& data) = 0;
    virtual std::string name() const = 0;
    virtual ~Decompressor() = default;
};
