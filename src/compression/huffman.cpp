#include "compression/huffman.hpp"
#include "compression/bitstream.hpp"
#include <queue>
#include <map>
#include <unordered_map>
#include <cstring>
#include <iostream>

struct HuffNode {
    uint8_t symbol;
    uint32_t freq;
    HuffNode *left, *right;

    HuffNode(uint8_t s, uint32_t f) : symbol(s), freq(f), left(nullptr), right(nullptr) {}
    HuffNode(HuffNode* l, HuffNode* r) : symbol(0), freq(l->freq + r->freq), left(l), right(r) {}
    ~HuffNode() {
        delete left;
        delete right;
    }
};

struct CompareNode {
    bool operator()(HuffNode* const& n1, HuffNode* const& n2) {
        return n1->freq > n2->freq;
    }
};

static void buildCodes(HuffNode* node, const std::vector<int>& path, std::unordered_map<uint8_t, std::vector<int>>& codes) {
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

std::vector<uint8_t> Huffman::compress(const std::vector<uint8_t>& data) {
    if (data.empty()) return {};

    std::unordered_map<uint8_t, uint32_t> frequencies;
    for (uint8_t byte : data) {
        frequencies[byte]++;
    }

    std::priority_queue<HuffNode*, std::vector<HuffNode*>, CompareNode> pq;
    for (auto const& [sym, freq] : frequencies) {
        pq.push(new HuffNode(sym, freq));
    }

    if (pq.size() == 1) {
        // Handle single symbol edge case by adding a dummy node
        HuffNode* only = pq.top();
        pq.pop();
        HuffNode* dummy = new HuffNode(only->symbol == 0 ? 1 : 0, 0);
        pq.push(new HuffNode(only, dummy));
    }

    while (pq.size() > 1) {
        HuffNode* left = pq.top(); pq.pop();
        HuffNode* right = pq.top(); pq.pop();
        pq.push(new HuffNode(left, right));
    }

    HuffNode* root = pq.top();
    std::unordered_map<uint8_t, std::vector<int>> codes;
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

    // Header Format:
    // [4 bytes] Original Size
    // [4 bytes] Total Bits
    // [2 bytes] Freq Table Size (N)
    // [N * 5 bytes] Freq Table: [1 byte sym][4 bytes freq]
    std::vector<uint8_t> out;
    uint32_t origSize = data.size();
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&origSize), reinterpret_cast<uint8_t*>(&origSize) + 4);
    
    uint32_t tBits = totalBits;
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&tBits), reinterpret_cast<uint8_t*>(&tBits) + 4);

    uint16_t tableSize = frequencies.size();
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&tableSize), reinterpret_cast<uint8_t*>(&tableSize) + 2);

    for (auto const& [sym, freq] : frequencies) {
        out.push_back(sym);
        uint32_t f = freq;
        out.insert(out.end(), reinterpret_cast<uint8_t*>(&f), reinterpret_cast<uint8_t*>(&f) + 4);
    }

    out.insert(out.end(), compressedData.begin(), compressedData.end());

    delete root;
    return out;
}

std::vector<uint8_t> Huffman::decompress(const std::vector<uint8_t>& compressed) {
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

    std::unordered_map<uint8_t, uint32_t> frequencies;
    for (int i = 0; i < tableSize; ++i) {
        uint8_t sym = compressed[offset];
        offset += 1;
        uint32_t freq;
        std::memcpy(&freq, &compressed[offset], 4);
        offset += 4;
        frequencies[sym] = freq;
    }

    std::priority_queue<HuffNode*, std::vector<HuffNode*>, CompareNode> pq;
    for (auto const& [sym, freq] : frequencies) {
        pq.push(new HuffNode(sym, freq));
    }

    if (pq.size() == 1) {
        HuffNode* only = pq.top();
        pq.pop();
        HuffNode* dummy = new HuffNode(only->symbol == 0 ? 1 : 0, 0);
        pq.push(new HuffNode(only, dummy));
    }

    while (pq.size() > 1) {
        HuffNode* left = pq.top(); pq.pop();
        HuffNode* right = pq.top(); pq.pop();
        pq.push(new HuffNode(left, right));
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
