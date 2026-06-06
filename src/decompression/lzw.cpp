#include "decompression/lzw.hpp"
#include <unordered_map>
#include <string>
#include <cstring>
#include <iostream>

std::vector<uint8_t> LZWDecompressor::decompress(const std::vector<uint8_t>& compressed) {
    if (compressed.empty()) return {};

    size_t offset = 0;
    if (compressed.size() < 8) return {};

    uint32_t origSize;
    std::memcpy(&origSize, &compressed[offset], 4);
    offset += 4;

    uint32_t numCodes;
    std::memcpy(&numCodes, &compressed[offset], 4);
    offset += 4;

    std::vector<uint16_t> codes;
    codes.reserve(numCodes);

    size_t i = offset;
    while (codes.size() < numCodes && i < compressed.size()) {
        if (i + 2 < compressed.size() && codes.size() + 1 < numCodes) {
            uint16_t c1 = (compressed[i] << 4) | (compressed[i+1] >> 4);
            uint16_t c2 = ((compressed[i+1] & 0x0F) << 8) | compressed[i+2];
            codes.push_back(c1);
            codes.push_back(c2);
            i += 3;
        } else {
            uint16_t c1 = (compressed[i] << 4) | (compressed[i+1] >> 4);
            codes.push_back(c1);
            i += 2;
        }
    }

    if (codes.empty()) return {};

    std::unordered_map<uint16_t, std::string> dict;
    for (int j = 0; j < 256; j++) {
        dict[j] = std::string(1, (char)j);
    }
    uint16_t dictSize = 256;

    std::vector<uint8_t> out;
    out.reserve(origSize);

    std::string w = dict[codes[0]];
    for (char c : w) out.push_back(c);

    for (size_t j = 1; j < codes.size(); j++) {
        uint16_t k = codes[j];
        std::string entry;
        
        if (dict.count(k)) {
            entry = dict[k];
        } else if (k == dictSize) {
            entry = w + w[0];
        } else {
            return {};
        }

        for (char c : entry) out.push_back(c);

        if (dictSize < 4096) {
            dict[dictSize++] = w + entry[0];
        } else {
            dict.clear();
            for (int h = 0; h < 256; h++) {
                dict[h] = std::string(1, (char)h);
            }
            dictSize = 256;
            dict[dictSize++] = w + entry[0];
        }
        w = entry;
    }

    return out;
}
