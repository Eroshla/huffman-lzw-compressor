#ifndef HUFFMAN_DECOMPRESS_HPP
#define HUFFMAN_DECOMPRESS_HPP

#include "../compressor.hpp"

class HuffmanDecompressor : public Decompressor {
public:
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data) override;
    std::string name() const override { return "huffman"; }
};

#endif // HUFFMAN_DECOMPRESS_HPP
