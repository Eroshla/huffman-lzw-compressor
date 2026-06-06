# Huffman & LZW Compressor

Uma ferramenta de linha de comando desenvolvida em C++17 para realizar a compressão e descompressão de arquivos, focada no armazenamento eficiente de índices e estruturas de Árvores B. O programa implementa e analisa, de ponta a ponta, dois algoritmos clássicos de compressão: **Huffman** e **LZW**.

## 🚀 Funcionalidades

- **Compressão Comparativa:** Comprime qualquer arquivo usando simultaneamente os algoritmos Huffman e LZW, e exibe um relatório detalhado de eficiência.
- **Descompressão Comparativa:** Restaura os arquivos para o seu formato original, validando automaticamente a integridade byte a byte (caso o arquivo original esteja presente).
- **Relatórios Detalhados:** Métricas em tempo real de Tempo de Execução (ms), Tamanho Final (bytes), Taxa de Compressão (%) e Economia de Espaço (%).
- **Tratamento Binário Universal:** O sistema trata a entrada estritamente como *raw bytes*, tornando-o capaz de comprimir perfeitamente não só textos de Árvore B, mas qualquer tipo de formato de arquivo, mantendo espaços, formatações e quebras intactas.
- **Sem Dependências Externas:** Escrito puramente em C++ moderno, dependendo unicamente da Standard Template Library (STL).

## 🏗️ Arquitetura Modular

O projeto adota uma arquitetura rigorosa de separação de responsabilidades e *Clean Code*:
- **`include/compression/` e `src/compression/`**: Lógicas de codificação dedicadas à redução de dados (classes `HuffmanCompressor`, `LZWCompressor` e `BitWriter`).
- **`include/decompression/` e `src/decompression/`**: Lógicas de decodificação e restauração de dados (classes `HuffmanDecompressor`, `LZWDecompressor` e `BitReader`).
- **`cli/orchestrator`**: Módulo que atua como *Controller*, responsável por coordenar a interação do usuário, carregar os arquivos, cronometrar a execução com a biblioteca genérica `utils/metrics` e exibir o resultado visualmente.

## 🛠️ Como Compilar

Certifique-se de que o `g++` e o `make` estão instalados em seu ambiente (Linux ou WSL com `build-essential`).
Na raiz do diretório, basta utilizar o `make` para compilar todo o código:

```bash
make
```

Isso irá gerar o binário de nome `compressor`.

> **Limpeza:** Se desejar apagar os arquivos compilados (`.o`) e recomeçar, basta rodar `make clean`.

## 💻 Como Usar

O `compressor` opera por meio de duas **ações principais**. A ferramenta é inteligente e gerenciará as extensões de arquivo e os outputs automaticamente. Tudo o que for produzido pelo sistema será salvo dentro de uma pasta segura chamada `output/`.

### 1. Comprimir um arquivo
Rode a ação `compress` passando o nome do arquivo alvo (ex: sua Árvore B):

```bash
./compressor compress txt_exaple_10K.txt
```

O programa construirá as árvores de compressão e gerará os arquivos otimizados:
- `output/txt_exaple_10K.txt.huff`
- `output/txt_exaple_10K.txt.lzw`

### 2. Descomprimir um arquivo
Rode a ação `decompress`, passando o nome base do arquivo que foi processado anteriormente:

```bash
./compressor decompress txt_exaple_10K.txt
```

O programa irá buscar os arquivos compactados gerados anteriormente, efetuará a descompressão (gerando arquivos paralelos finalizados em `.out`), e aplicará um **Teste de Integridade** se o arquivo original ainda estiver disponível para comparação.

---
*Este projeto foi arquitetado como solução para avaliação, implementação e benchmark da eficiência de estruturas de compressão do mundo real.*