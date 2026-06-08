#ifndef LZW_DECOMPRESS_HPP
#define LZW_DECOMPRESS_HPP

#include "../compressor.hpp"

class LZWDecompressor : public Decompressor {
public:
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data) override;
    std::string name() const override { return "lzw"; }
};

#endif // LZW_DECOMPRESS_HPP
