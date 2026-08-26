cmake_minimum_required(VERSION 3.10)
project(steganography C)

set(CMAKE_C_STANDARD 99)

include_directories(include)

add_executable(steganography
    src/main.c
    src/encode.c
    src/decode.c
)
