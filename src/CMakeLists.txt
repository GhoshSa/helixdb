add_library(helixdb STATIC
    storage/page.cpp
    storage/pager.cpp
)

target_include_directories(helixdb
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(helixdb PUBLIC cxx_std_20)

add_executable(helixdb_bin main.cpp)
target_link_libraries(helixdb_bin PRIVATE helixdb)