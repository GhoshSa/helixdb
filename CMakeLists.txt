cmake_minimum_required(VERSION 3.20)

project(
    helixdb
    VERSION 0.1.0
    DESCRIPTION "A simple database engine written in C++."
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

add_library(helixdb_warnings INTERFACE)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(helixdb_warnings INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversation
        -Wsign-conversation
        -Wshadow
        -Wnon-virtual-dtor
        -World-style-cast
        -Woverloaded-virtual
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
    )
endif()

add_library(helixdb_storage STATIC src/storage/block_device.cpp)

target_include_directories(helixdb_storage PUBLIC ${PROJECT_SOURCE_DIR}/include)

option(HELIXDB_ENABLE_ASAN "Enable AddressSanitizer" OFF)

if (HELIXDB_ENABLE_ASAN)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(helixdb_storage PRIVATE -fsanitize=address)
    endif()
endif()