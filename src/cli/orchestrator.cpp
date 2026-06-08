#include "cli/orchestrator.hpp"
#include "compression/huffman.hpp"
#include "compression/lzw.hpp"
#include "decompression/huffman.hpp"
#include "decompression/lzw.hpp"
#include "compressor.hpp"
#include "utils/metrics.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
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
    return original == decompressed;
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

static std::pair<std::vector<uint8_t>, double> run_compress(Compressor& comp, const std::vector<uint8_t>& data) {
    auto t0 = Metrics::start();
    auto result = comp.compress(data);
    return {result, Metrics::stop(t0)};
}

static std::pair<std::vector<uint8_t>, double> run_decompress(Decompressor& decomp, const std::vector<uint8_t>& data) {
    auto t0 = Metrics::start();
    auto result = decomp.decompress(data);
    return {result, Metrics::stop(t0)};
}

void Orchestrator::run(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <acao> <arquivo_original>\n"
                  << "acao = compress | decompress\n"
                  << "O programa ira rodar Huffman e LZW automaticamente para comparar.\n";
        return;
    }

    std::string action = argv[1];
    std::string filename = argv[2];

    try {
        std::filesystem::create_directory("output");
        std::string base = std::filesystem::path(filename).filename().string();

        if (action == "compress") {
            std::vector<uint8_t> original = readFile(filename);
            size_t originalSize = original.size();

            HuffmanCompressor huffComp;
            LZWCompressor lzwComp;

            auto [huffData, huffTime] = run_compress(huffComp, original);
            auto [lzwData,  lzwTime]  = run_compress(lzwComp,  original);

            writeFile("output/" + base + ".huff", huffData);
            writeFile("output/" + base + ".lzw",  lzwData);

            printCompressReport(filename, originalSize,
                huffData.size(),
                Metrics::calculate_ratio(huffData.size(), originalSize),
                Metrics::calculate_savings(huffData.size(), originalSize),
                huffTime,
                lzwData.size(),
                Metrics::calculate_ratio(lzwData.size(), originalSize),
                Metrics::calculate_savings(lzwData.size(), originalSize),
                lzwTime);

        } else if (action == "decompress") {
            std::string huffFile = "output/" + base + ".huff";
            std::string lzwFile  = "output/" + base + ".lzw";

            HuffmanDecompressor huffDecomp;
            LZWDecompressor lzwDecomp;

            auto [huffRestored, huffTime] = run_decompress(huffDecomp, readFile(huffFile));
            auto [lzwRestored, lzwTime] = run_decompress(lzwDecomp,  readFile(lzwFile));

            writeFile("output/" + base + ".huff.out", huffRestored);
            writeFile("output/" + base + ".lzw.out",  lzwRestored);

            bool huffValid = false, lzwValid = false;
            try {
                std::vector<uint8_t> original = readFile(filename);
                huffValid = verifyIntegrity(original, huffRestored);
                lzwValid  = verifyIntegrity(original, lzwRestored);
            } catch (...) {}

            printDecompressReport(filename,
                huffRestored.size(), huffTime, huffValid,
                lzwRestored.size(),  lzwTime,  lzwValid);

        } else {
            std::cerr << "Acao desconhecida: " << action << "\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << "\n";
    }
}
