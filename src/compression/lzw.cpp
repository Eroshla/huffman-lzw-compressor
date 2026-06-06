#include "compression/lzw.hpp"
#include <unordered_map>
#include <string>
#include <cstring>
#include <iostream>

std::vector<uint8_t> LZWCompressor::compress(const std::vector<uint8_t>& data) {
    if (data.empty()) return {};

    std::unordered_map<std::string, uint16_t> dict;
    for (int i = 0; i < 256; i++) {
        dict[std::string(1, (char)i)] = i;
    }

    std::vector<uint16_t> outCodes;
    std::string w = "";
    uint16_t dictSize = 256;

    for (uint8_t byte : data) {
        std::string wc = w + (char)byte;
        if (dict.count(wc)) {
            w = wc;
        } else {
            outCodes.push_back(dict[w]);
            if (dictSize < 4096) {
                dict[wc] = dictSize++;
            } else {
                dict.clear();
                for (int i = 0; i < 256; i++) {
                    dict[std::string(1, (char)i)] = i;
                }
                dictSize = 256;
                dict[wc] = dictSize++;
            }
            w = std::string(1, (char)byte);
        }
    }
    if (!w.empty()) {
        outCodes.push_back(dict[w]);
    }

    std::vector<uint8_t> out;
    uint32_t origSize = data.size();
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&origSize), reinterpret_cast<uint8_t*>(&origSize) + 4);
    
    uint32_t numCodes = outCodes.size();
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&numCodes), reinterpret_cast<uint8_t*>(&numCodes) + 4);

    for (size_t i = 0; i < outCodes.size(); i += 2) {
        uint16_t c1 = outCodes[i];
        if (i + 1 < outCodes.size()) {
            uint16_t c2 = outCodes[i+1];
            out.push_back(c1 >> 4);
            out.push_back(((c1 & 0x0F) << 4) | (c2 >> 8));
            out.push_back(c2 & 0xFF);
        } else {
            out.push_back(c1 >> 4);
            out.push_back((c1 & 0x0F) << 4);
        }
    }

    return out;
}
