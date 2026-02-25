#pragma once
#include <cstdint>

namespace helixdb::storage {
    constexpr std::size_t BLOCK_SIZE = 4096;
    constexpr std::size_t PAGE_SIZE = BLOCK_SIZE;
}