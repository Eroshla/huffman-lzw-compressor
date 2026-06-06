#include "compression/huffman.hpp"
#include "compression/bitwriter.hpp"
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

    void buildCodes(HuffNode* node, const std::vector<int>& path, std::vector<std::vector<int>>& codes) {
        if (!node) return;
        if (!node->left && !node->right) {
            codes[node->symbol] = path;
            return;
        }
        std::vector<int> leftPath = path;
        leftPath.push_back(0);
        buildCodes(node->left, leftPath, codes);

        std::vector<int> rightPath = path;
        rightPath.push_back(1);
        buildCodes(node->right, rightPath, codes);
    }
}

std::vector<uint8_t> HuffmanCompressor::compress(const std::vector<uint8_t>& data) {
    if (data.empty()) return {};

    uint32_t frequencies[256] = {0};
    for (uint8_t byte : data) {
        frequencies[byte]++;
    }

    uint32_t nextId = 0;
    std::priority_queue<HuffNode*, std::vector<HuffNode*>, CompareNode> pq;
    uint16_t tableSize = 0;
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            pq.push(new HuffNode(i, frequencies[i], nextId++));
            tableSize++;
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
    std::vector<std::vector<int>> codes(256);
    buildCodes(root, {}, codes);

    BitWriter bw;
    for (uint8_t byte : data) {
        const auto& code = codes[byte];
        for (int bit : code) {
            bw.writeBit(bit);
        }
    }
    bw.flush();

    size_t totalBits = bw.getTotalBits();
    std::vector<uint8_t> compressedData = bw.getData();

    std::vector<uint8_t> out;
    uint32_t origSize = data.size();
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&origSize), reinterpret_cast<uint8_t*>(&origSize) + 4);
    
    uint32_t tBits = totalBits;
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&tBits), reinterpret_cast<uint8_t*>(&tBits) + 4);

    out.insert(out.end(), reinterpret_cast<uint8_t*>(&tableSize), reinterpret_cast<uint8_t*>(&tableSize) + 2);

    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            uint8_t sym = i;
            out.push_back(sym);
            uint32_t f = frequencies[i];
            out.insert(out.end(), reinterpret_cast<uint8_t*>(&f), reinterpret_cast<uint8_t*>(&f) + 4);
        }
    }

    out.insert(out.end(), compressedData.begin(), compressedData.end());

    delete root;
    return out;
}
