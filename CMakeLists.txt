cmake_minimum_required(VERSION 3.20)
project(Solda)

set(CMAKE_C_STANDARD 11)

# compilation flags
add_compile_options(
    -Wall
    -Wextra
    -g
    -O0
)

# ASAN optional -DASAN=ON
option(ASAN "Enable AddressSanitizer" ON)
if(ASAN)
    add_compile_options(-fsanitize=address,leak)
    add_link_options(-fsanitize=address,leak)
endif()

# include path
include_directories(include)

# core — static library
add_library(solda_core STATIC
    src/db.c
    src/db_command.c
    src/db_memory.c
)

# tests
enable_testing()
add_subdirectory(tests)

