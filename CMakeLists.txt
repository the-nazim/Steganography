cmake_minimum_required(VERSION 3.14)
project(steganography C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

include_directories(include)

# ============================================
# Fetch FTXUI library
# ============================================
include(FetchContent)
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG        v7.0.3
)
FetchContent_MakeAvailable(ftxui)

# ============================================
# Static library for C steganography code
# ============================================
add_library(stegano_core STATIC
    src/encode.c
    src/decode.c
    src/stegano_wrapper.c
)

target_include_directories(stegano_core PUBLIC include)

# ============================================
# CLI binary (original C interface)
# ============================================
add_executable(steganography
    src/main.c
)
target_link_libraries(steganography stegano_core)

# ============================================
# TUI binary (C++ with FTXUI)
# ============================================
add_executable(stegano_tui
    src/tui.cpp
)
target_link_libraries(stegano_tui
    stegano_core
    ftxui::screen
    ftxui::dom
    ftxui::component
)
