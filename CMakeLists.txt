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

execute_process(
    COMMAND pg_config --includedir-server
    OUTPUT_VARIABLE PG_SERVER_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT PG_SERVER_INCLUDE_DIR)
    message(FATAL_ERROR "pg_config --includedir-server not found! Is PostgreSQL server installed?")
endif()

message(STATUS "PostgreSQL server include: ${PG_SERVER_INCLUDE_DIR}")

include_directories(${PG_SERVER_INCLUDE_DIR})

# core — static library
add_library(solda_core STATIC
    src/db.c
    src/db_command.c
    src/db_memory.c
    src/db_service.c
    src/db_log.c
    src/db_messages.c
    src/db_time.c
)

add_compile_options(
    -Wall
    -Wextra
)


###################### C++ Qt6 Configuration ######################

option(ONLY_SOLDA_CORE "Build only solda core" ON)

if(NOT ONLY_SOLDA_CORE)
# Qt6.11.1 locally installed
set(CMAKE_PREFIX_PATH
    "$ENV{HOME}/Qt/6.11.1/gcc_64"
    ${CMAKE_PREFIX_PATH}
)

# Find modules
find_package(Qt6 REQUIRED COMPONENTS
    Widgets        # Window, buttons, layouts
    PrintSupport   # OS print
)

# Qt — moc, uic, atomatic rcc 
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt6 Executable
add_executable(SoldaApp
    src/ui/main.cpp
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
)

target_link_libraries(SoldaApp
    solda_core      # core C
    Qt6::Widgets
    Qt6::PrintSupport
    pq
)

endif()
####################################################################

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
