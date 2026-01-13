add_library(helixdb STATIC
    common/placeholder.cpp
)

target_include_directories(helixdb
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(helixdb_bin main.cpp)
target_link_libraries(helixdb_bin helixdb)

