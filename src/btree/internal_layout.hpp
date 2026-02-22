#pragma once

#include <cstdint>

namespace helixdb::btree {
    /* Internal Node Layout
       for now a single internal node,
       two children,
       one seperator key */
    constexpr std::size_t INTERNAL_NODE_TYPE_OFFSET = 0;
    constexpr std::size_t INTERNAL_IS_ROOT_OFFSET = 1;
    constexpr std::size_t INTERNAL_KEY_COUNT_OFFSET = 2;
    constexpr std::size_t INTERNAL_CHILD0_OFFSET = 4;
    constexpr std::size_t INTERNAL_KEY0_OFFSET = 8;
    constexpr std::size_t INTERNAL_CHILD1_OFFSET = 16;
    constexpr std::size_t INTERNAL_HEADER_SIZE = 20;

    constexpr uint16_t INTERNAL_MAX_KEYS = 1;
}