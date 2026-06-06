#ifndef LZW_COMPRESS_HPP
#define LZW_COMPRESS_HPP

#include <vector>
#include <cstdint>

class LZWCompressor {
public:
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
};

#endif // LZW_COMPRESS_HPP
