CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -std=c++17
INCLUDES = -I./include

SRCS = src/main.cpp \
       src/compression/huffman.cpp \
       src/compression/lzw.cpp \
       src/compression/bitwriter.cpp \
       src/decompression/huffman.cpp \
       src/decompression/lzw.cpp \
       src/decompression/bitreader.cpp \
       src/cli/orchestrator.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = compressor

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f src/*.o src/compression/*.o src/decompression/*.o src/cli/*.o $(TARGET)

.PHONY: all clean
