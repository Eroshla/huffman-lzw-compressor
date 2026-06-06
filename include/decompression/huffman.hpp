#ifndef HUFFMAN_DECOMPRESS_HPP
#define HUFFMAN_DECOMPRESS_HPP

#include <vector>
#include <cstdint>

class HuffmanDecompressor {
public:
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressed);
};

#endif // HUFFMAN_DECOMPRESS_HPP
