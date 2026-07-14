cmake_minimum_required(VERSION 3.20)
project(Solda)
set(CMAKE_C_STANDARD 11)

#cmake -DCMAKE_BUILD_TYPE=Debug -DASAN=ON -DTEST=ON ..
# compilation flags
#valgrind --leak-check=full --show-leak-kinds=all ./test_db_service
#
# ASAN optional -DASAN=ON
option(ASAN "Enable AddressSanitizer" ON)
if(ASAN AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-fsanitize=address,leak)
    add_link_options(-fsanitize=address,leak)
    message(STATUS "ASAN: enabled")
elseif(ASAN AND CMAKE_BUILD_TYPE STREQUAL "Release")
    message(FATAL_ERROR 
        "ASAN could not be used in Release mode! "
        "Use -DCMAKE_BUILD_TYPE=Debug"
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native)
    message(STATUS "Build: Release (-O3)")
else()
    add_compile_options(-O0 -g -DSOLDA_DEBUG)
    message(STATUS "Build: Debug (-O0)")
endif()

# include path
include_directories(include)

# core — static library
add_library(solda_core STATIC
    src/db.c
    src/db_command.c
    src/db_memory.c
    src/db_service.c
    src/db_log.c
    src/db_messages.c
)

add_compile_options(
    -Wall
    -Wextra
)


# TEST optional -DTEST=ON
option(TEST "Enable test" ON)
if(TEST)
    add_compile_options(-DDB_TEST)

    # test only compiling
    add_library(solda_asserts STATIC
        src/tests/asserts.c
    )

    enable_testing()
    add_subdirectory(tests)
endif()
