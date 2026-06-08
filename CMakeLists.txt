cmake_minimum_required(VERSION 3.16)
project(compressor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(compressor_lib STATIC
    src/compression/huffman.cpp
    src/compression/lzw.cpp
    src/compression/bitwriter.cpp
    src/decompression/huffman.cpp
    src/decompression/lzw.cpp
    src/decompression/bitreader.cpp
)

target_include_directories(compressor_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(compressor_lib PRIVATE -Wall -Wextra -O3)

add_executable(compressor
    src/main.cpp
    src/cli/orchestrator.cpp
)

target_include_directories(compressor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(compressor PRIVATE compressor_lib)
target_compile_options(compressor PRIVATE -Wall -Wextra -O3)
