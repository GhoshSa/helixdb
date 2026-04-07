cmake_minimum_required(VERSION 3.20)

project(
    helixdb
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(STORAGE_SOURCES src/storage/block_device.cpp src/storage/pager.cpp)

set(WAL_SOURCES src/wal/wal_manager.cpp)

set(BTREE_SOURCES src/btree/bplushtree.cpp src/btree/btree_cursor.cpp)

add_library(helixdb ${STORAGE_SOURCES} ${WAL_SOURCES} ${BTREE_SOURCES})

target_include_directories(helixdb PUBLIC include)

add_executable(helixdb_main main.cpp)

target_link_libraries(helixdb_main PRIVATE helixdb)

find_package(GTest REQUIRED)

add_executable(test tests/test.cpp)

target_link_libraries(test PRIVATE helixdb GTest::GTest GTest::Main pthread)

include(GoogleTest)
gtest_discover_tests(test)