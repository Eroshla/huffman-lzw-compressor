#include "cli/orchestrator.hpp"
#include "compression/huffman.hpp"
#include "compression/lzw.hpp"
#include "decompression/huffman.hpp"
#include "decompression/lzw.hpp"
#include "utils/metrics.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <filesystem>

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

static void printCompressReport(const std::string& filename, size_t originalSize,
                 size_t huffSize, double huffRatio, double huffSavings, double huffCompTime,
                 size_t lzwSize, double lzwRatio, double lzwSavings, double lzwCompTime) {
    std::cout << "===========================================\n";
    std::cout << "          Relatorio de Compressao\n";
    std::cout << "===========================================\n\n";
    std::cout << "Arquivo original: " << filename << "\n";
    std::cout << "Tamanho original: " << originalSize << " bytes\n\n";
    
    std::cout << "-------------------------------------------\n";
    std::cout << std::left << std::setw(15) << " " << std::setw(15) << "Huffman" << "LZW\n";
    std::cout << "-------------------------------------------\n";
    std::cout << std::left << std::setw(15) << "Comprimido:" << std::setw(15) << std::to_string(huffSize) + " b" << std::to_string(lzwSize) + " b\n";
    std::cout << std::left << std::setw(15) << "Taxa:" << std::setw(15) << std::to_string(huffRatio * 100).substr(0, 5) + " %" << std::to_string(lzwRatio * 100).substr(0, 5) + " %\n";
    std::cout << std::left << std::setw(15) << "Economia:" << std::setw(15) << std::to_string(huffSavings).substr(0, 5) + " %" << std::to_string(lzwSavings).substr(0, 5) + " %\n";
    std::cout << std::left << std::setw(15) << "Tempo:" << std::setw(15) << std::to_string(huffCompTime).substr(0, 5) + " ms" << std::to_string(lzwCompTime).substr(0, 5) + " ms\n";
    std::cout << "-------------------------------------------\n";
}

static void printDecompressReport(const std::string& filename,
                 size_t huffRestoredSize, double huffDecompTime, bool huffValid,
                 size_t lzwRestoredSize, double lzwDecompTime, bool lzwValid) {
    std::cout << "===========================================\n";
    std::cout << "         Relatorio de Descompressao\n";
    std::cout << "===========================================\n\n";
    std::cout << "Arquivo base: " << filename << "\n\n";
    
    std::cout << "-------------------------------------------\n";
    std::cout << std::left << std::setw(15) << " " << std::setw(15) << "Huffman" << "LZW\n";
    std::cout << "-------------------------------------------\n";
    std::cout << std::left << std::setw(15) << "Tamanho Rest.:" << std::setw(15) << std::to_string(huffRestoredSize) + " b" << std::to_string(lzwRestoredSize) + " b\n";
    std::cout << std::left << std::setw(15) << "Tempo:" << std::setw(15) << std::to_string(huffDecompTime).substr(0, 5) + " ms" << std::to_string(lzwDecompTime).substr(0, 5) + " ms\n";
    
    std::string hV = huffValid ? "OK" : "N/A";
    std::string lV = lzwValid ? "OK" : "N/A";
    std::cout << std::left << std::setw(15) << "Integridade:" << std::setw(15) << hV << lV << "\n";
    
    std::cout << "-------------------------------------------\n";
}

void Orchestrator::run(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <acao> <arquivo_original>\n";
        std::cerr << "acao = compress | decompress\n";
        std::cerr << "O programa ira rodar Huffman e LZW automaticamente para comparar.\n";
        return;
    }

    std::string action = argv[1];
    std::string filename = argv[2];

    try {
        std::filesystem::create_directory("output");
        std::string baseFilename = std::filesystem::path(filename).filename().string();

        if (action == "compress") {
            std::vector<uint8_t> originalData = readFile(filename);
            size_t originalSize = originalData.size();

            HuffmanCompressor huffComp;
            auto startHuff = Metrics::start();
            std::vector<uint8_t> huffCompressed = huffComp.compress(originalData);
            double huffCompTime = Metrics::stop(startHuff);
            size_t huffSize = huffCompressed.size();
            writeFile("output/" + baseFilename + ".huff", huffCompressed);

            LZWCompressor lzwComp;
            auto startLzw = Metrics::start();
            std::vector<uint8_t> lzwCompressed = lzwComp.compress(originalData);
            double lzwCompTime = Metrics::stop(startLzw);
            size_t lzwSize = lzwCompressed.size();
            writeFile("output/" + baseFilename + ".lzw", lzwCompressed);

            double huffRatio = Metrics::calculate_ratio(huffSize, originalSize);
            double huffSavings = Metrics::calculate_savings(huffSize, originalSize);
            double lzwRatio = Metrics::calculate_ratio(lzwSize, originalSize);
            double lzwSavings = Metrics::calculate_savings(lzwSize, originalSize);

            printCompressReport(filename, originalSize,
                        huffSize, huffRatio, huffSavings, huffCompTime,
                        lzwSize, lzwRatio, lzwSavings, lzwCompTime);

        } else if (action == "decompress") {
            std::string huffFile = "output/" + baseFilename + ".huff";
            std::string lzwFile = "output/" + baseFilename + ".lzw";
            
            std::vector<uint8_t> huffData = readFile(huffFile);
            std::vector<uint8_t> lzwData = readFile(lzwFile);

            HuffmanDecompressor huffDecomp;
            auto startHuff = Metrics::start();
            std::vector<uint8_t> huffDecompressed = huffDecomp.decompress(huffData);
            double huffDecompTime = Metrics::stop(startHuff);
            writeFile("output/" + baseFilename + ".huff.out", huffDecompressed);

            LZWDecompressor lzwDecomp;
            auto startLzw = Metrics::start();
            std::vector<uint8_t> lzwDecompressed = lzwDecomp.decompress(lzwData);
            double lzwDecompTime = Metrics::stop(startLzw);
            writeFile("output/" + baseFilename + ".lzw.out", lzwDecompressed);

            bool huffValid = false;
            bool lzwValid = false;
            try {
                std::vector<uint8_t> originalData = readFile(filename);
                huffValid = verifyIntegrity(originalData, huffDecompressed);
                lzwValid = verifyIntegrity(originalData, lzwDecompressed);
            } catch(...) {
                // Ignore if original is not available
            }

            printDecompressReport(filename, huffDecompressed.size(), huffDecompTime, huffValid, lzwDecompressed.size(), lzwDecompTime, lzwValid);
            
        } else {
            std::cerr << "Acao desconhecida: " << action << "\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << "\n";
    }
}
