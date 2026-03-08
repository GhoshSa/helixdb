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

set(BTREE_SOURCES src/btree/bplushtree.cpp src/btree/leaf_node.cpp src/btree/internal_node.cpp)

set(SCHEMA_SOURCES src/schema/row_serializer.cpp src/schema/table.cpp src/schema/catalog.cpp src/schema/schema_manager.cpp)

add_library(helixdb ${STORAGE_SOURCES} ${WAL_SOURCES} ${BTREE_SOURCES} ${SCHEMA_SOURCES})

target_include_directories(helixdb PUBLIC include)

add_executable(helixdb_main main.cpp)

target_link_libraries(helixdb_main PRIVATE helixdb)

# add_executable(test_wal_basic tests/test_wal_basic.cpp)

# target_link_libraries(test_wal_basic PRIVATE helixdb_storage)

# target_include_directories(test_wal_basic PRIVATE ${PROJECT_SOURCE_DIR}/tests)

# enable_testing()
# add_test(NAME wal_basic COMMAND test_wal_basic)

# add_executable(test_wal_recovery tests/test_wal_recovery.cpp)

# target_link_libraries(test_wal_recovery PRIVATE helixdb_storage)

# target_include_directories(test_wal_recovery PRIVATE ${PROJECT_SOURCE_DIR}/tests)

# enable_testing()
# add_test(NAME wal_recovery COMMAND test_wal_recovery)