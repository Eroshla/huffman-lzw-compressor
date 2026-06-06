#include "decompression/huffman.hpp"
#include "decompression/bitreader.hpp"
#include <queue>
#include <cstring>
#include <iostream>
#include <vector>

namespace {
    struct HuffNode {
        uint8_t symbol;
        uint32_t freq;
        uint32_t id;
        HuffNode *left, *right;

        HuffNode(uint8_t s, uint32_t f, uint32_t id) : symbol(s), freq(f), id(id), left(nullptr), right(nullptr) {}
        HuffNode(HuffNode* l, HuffNode* r, uint32_t id) : symbol(0), freq(l->freq + r->freq), id(id), left(l), right(r) {}
        ~HuffNode() {
            delete left;
            delete right;
        }
    };

    struct CompareNode {
        bool operator()(HuffNode* const& n1, HuffNode* const& n2) {
            if (n1->freq == n2->freq) {
                return n1->id > n2->id; // tie-breaker deterministico
            }
            return n1->freq > n2->freq;
        }
    };
}

std::vector<uint8_t> HuffmanDecompressor::decompress(const std::vector<uint8_t>& compressed) {
    if (compressed.empty()) return {};

    size_t offset = 0;
    if (compressed.size() < 10) return {};

    uint32_t origSize;
    std::memcpy(&origSize, &compressed[offset], 4);
    offset += 4;

    uint32_t totalBits;
    std::memcpy(&totalBits, &compressed[offset], 4);
    offset += 4;

    uint16_t tableSize;
    std::memcpy(&tableSize, &compressed[offset], 2);
    offset += 2;

    if (compressed.size() < offset + tableSize * 5) return {};

    uint32_t frequencies[256] = {0};
    for (int i = 0; i < tableSize; ++i) {
        uint8_t sym = compressed[offset];
        offset += 1;
        uint32_t freq;
        std::memcpy(&freq, &compressed[offset], 4);
        offset += 4;
        frequencies[sym] = freq;
    }

    uint32_t nextId = 0;
    std::priority_queue<HuffNode*, std::vector<HuffNode*>, CompareNode> pq;
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            pq.push(new HuffNode(i, frequencies[i], nextId++));
        }
    }

    if (pq.size() == 1) {
        HuffNode* only = pq.top();
        pq.pop();
        HuffNode* dummy = new HuffNode(only->symbol == 0 ? 1 : 0, 0, nextId++);
        pq.push(new HuffNode(only, dummy, nextId++));
    }

    while (pq.size() > 1) {
        HuffNode* left = pq.top(); pq.pop();
        HuffNode* right = pq.top(); pq.pop();
        pq.push(new HuffNode(left, right, nextId++));
    }

    HuffNode* root = pq.top();

    const uint8_t* bitData = &compressed[offset];
    BitReader br(bitData, totalBits);

    std::vector<uint8_t> out;
    out.reserve(origSize);

    HuffNode* curr = root;
    while (br.hasMore() && out.size() < origSize) {
        int bit = br.readBit();
        if (bit == 0) curr = curr->left;
        else curr = curr->right;

        if (!curr->left && !curr->right) {
            out.push_back(curr->symbol);
            curr = root;
        }
    }

    delete root;
    return out;
}
