cmake_minimum_required(VERSION 4.3)
project(huffman)

set(CMAKE_CXX_STANDARD 11)

add_executable(huffman
        main.cpp
        tree.h
        tree.cpp
        compress.h
        compress.cpp
        decompress.h
        decompress.cpp
)
