#ifndef HUFFMAN_COMPRESS_HPP
#define HUFFMAN_COMPRESS_HPP

#include <vector>
#include <cstdint>

class HuffmanCompressor {
public:
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
};

#endif // HUFFMAN_COMPRESS_HPP
