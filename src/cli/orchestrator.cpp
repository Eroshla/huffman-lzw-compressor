#include "cli/orchestrator.hpp"
#include "compression/huffman.hpp"
#include "compression/lzw.hpp"
#include "utils/metrics.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

static std::vector<uint8_t> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return buffer;
    }
    throw std::runtime_error("Error reading file: " + filename);
}

static void writeFile(const std::string& filename, const std::vector<uint8_t>& data) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not write to file: " + filename);
    }
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

static bool verifyIntegrity(const std::vector<uint8_t>& original, const std::vector<uint8_t>& decompressed) {
    if (original.size() != decompressed.size()) return false;
    for (size_t i = 0; i < original.size(); ++i) {
        if (original[i] != decompressed[i]) return false;
    }
    return true;
}

static void printReport(const std::string& filename, size_t originalSize,
                 size_t huffSize, double huffRatio, double huffSavings, double huffCompTime, double huffDecompTime, bool huffValid,
                 size_t lzwSize, double lzwRatio, double lzwSavings, double lzwCompTime, double lzwDecompTime, bool lzwValid) {
    std::cout << "===========================================\n";
    std::cout << "  Relatorio de Compressao/Descompressao\n";
    std::cout << "===========================================\n\n";
    std::cout << "Arquivo: " << filename << "\n";
    std::cout << "Tamanho original: " << originalSize << " bytes\n\n";
    
    std::cout << "-------------------------------------------\n";
    std::cout << std::left << std::setw(15) << " " << std::setw(15) << "Huffman" << "LZW\n";
    std::cout << "-------------------------------------------\n";
    
    std::cout << std::left << std::setw(15) << "Comprimido:" << std::setw(15) << std::to_string(huffSize) + " b" << std::to_string(lzwSize) + " b\n";
    
    std::cout << std::left << std::setw(15) << "Taxa:" << std::setw(15) << std::to_string(huffRatio * 100).substr(0, 5) + " %" << std::to_string(lzwRatio * 100).substr(0, 5) + " %\n";
    std::cout << std::left << std::setw(15) << "Economia:" << std::setw(15) << std::to_string(huffSavings).substr(0, 5) + " %" << std::to_string(lzwSavings).substr(0, 5) + " %\n";
    std::cout << std::left << std::setw(15) << "T. Compress:" << std::setw(15) << std::to_string(huffCompTime).substr(0, 5) + " ms" << std::to_string(lzwCompTime).substr(0, 5) + " ms\n";
    std::cout << std::left << std::setw(15) << "T. Decompress:" << std::setw(15) << std::to_string(huffDecompTime).substr(0, 5) + " ms" << std::to_string(lzwDecompTime).substr(0, 5) + " ms\n";
    
    std::string hValid = huffValid ? "OK" : "FALHOU";
    std::string lValid = lzwValid ? "OK" : "FALHOU";
    std::cout << std::left << std::setw(15) << "Integridade:" << std::setw(15) << hValid << lValid << "\n";
    
    std::cout << "-------------------------------------------\n";
}

void Orchestrator::run(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <arquivo_entrada> <algoritmo>\n";
        std::cerr << "algoritmo = huffman | lzw | both\n";
        return;
    }

    std::string filename = argv[1];
    std::string algo = argv[2];

    try {
        std::vector<uint8_t> originalData = readFile(filename);
        size_t originalSize = originalData.size();

        size_t huffSize = 0, lzwSize = 0;
        double huffCompTime = 0, huffDecompTime = 0, huffRatio = 0, huffSavings = 0;
        double lzwCompTime = 0, lzwDecompTime = 0, lzwRatio = 0, lzwSavings = 0;
        bool huffValid = false, lzwValid = false;

        bool runHuffman = (algo == "huffman" || algo == "both");
        bool runLZW = (algo == "lzw" || algo == "both");

        if (runHuffman) {
            Huffman huff;
            
            auto start = Metrics::start();
            std::vector<uint8_t> compressed = huff.compress(originalData);
            huffCompTime = Metrics::stop(start);
            
            huffSize = compressed.size();
            huffRatio = Metrics::calculate_ratio(huffSize, originalSize);
            huffSavings = Metrics::calculate_savings(huffSize, originalSize);
            
            writeFile(filename + ".huff", compressed);

            start = Metrics::start();
            std::vector<uint8_t> decompressed = huff.decompress(compressed);
            huffDecompTime = Metrics::stop(start);

            huffValid = verifyIntegrity(originalData, decompressed);
        }

        if (runLZW) {
            LZW lzw;
            
            auto start = Metrics::start();
            std::vector<uint8_t> compressed = lzw.compress(originalData);
            lzwCompTime = Metrics::stop(start);
            
            lzwSize = compressed.size();
            lzwRatio = Metrics::calculate_ratio(lzwSize, originalSize);
            lzwSavings = Metrics::calculate_savings(lzwSize, originalSize);
            
            writeFile(filename + ".lzw", compressed);

            start = Metrics::start();
            std::vector<uint8_t> decompressed = lzw.decompress(compressed);
            lzwDecompTime = Metrics::stop(start);

            lzwValid = verifyIntegrity(originalData, decompressed);
        }

        printReport(filename, originalSize,
                    huffSize, huffRatio, huffSavings, huffCompTime, huffDecompTime, huffValid,
                    lzwSize, lzwRatio, lzwSavings, lzwCompTime, lzwDecompTime, lzwValid);

    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << "\n";
    }
}
