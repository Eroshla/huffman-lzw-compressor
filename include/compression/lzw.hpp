#ifndef LZW_H
#define LZW_H

#include <vector>
#include <cstdint>

class LZW {
public:
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressed);
};

#endif // LZW_H
