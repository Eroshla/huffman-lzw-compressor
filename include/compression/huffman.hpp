#ifndef HUFFMAN_COMPRESS_HPP
#define HUFFMAN_COMPRESS_HPP

#include "../compressor.hpp"

class HuffmanCompressor : public Compressor {
public:
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data) override;
    std::string name() const override { return "huffman"; }
};

#endif 
