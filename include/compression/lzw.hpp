#ifndef LZW_COMPRESS_HPP
#define LZW_COMPRESS_HPP

#include "../compressor.hpp"

class LZWCompressor : public Compressor {
public:
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data) override;
    std::string name() const override { return "lzw"; }
};

#endif 
