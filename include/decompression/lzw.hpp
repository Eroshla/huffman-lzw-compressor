#ifndef LZW_DECOMPRESS_HPP
#define LZW_DECOMPRESS_HPP

#include <vector>
#include <cstdint>

class LZWDecompressor {
public:
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressed);
};

#endif // LZW_DECOMPRESS_HPP
