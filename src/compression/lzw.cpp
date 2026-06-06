#include "compression/lzw.hpp"
#include <unordered_map>
#include <string>
#include <cstring>
#include <iostream>

std::vector<uint8_t> LZW::compress(const std::vector<uint8_t>& data) {
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
                // Reset dictionary
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

    // Header:
    // [4 bytes] Orig size
    // [4 bytes] Num codes
    // Codes packed into 12 bits (2 codes = 3 bytes)
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

std::vector<uint8_t> LZW::decompress(const std::vector<uint8_t>& compressed) {
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
            // Invalid dictionary state, shouldn't happen with correct encoding
            return {};
        }

        for (char c : entry) out.push_back(c);

        if (dictSize < 4096) {
            dict[dictSize++] = w + entry[0];
        } else {
            // Dictionary reset
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
