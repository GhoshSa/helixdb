#pragma once

#include <cstdint>

#include "storage/constants.hpp"

namespace helixdb::btree {
    /* Node Types */
    enum class NodeType : uint8_t {
        INTERNAL = 0,
        LEAF = 1
    };

    /* Common Page Header
       Offsets are byte offsets from page start */
    constexpr std::size_t NODE_TYPE_OFFSET = 0;
    constexpr std::size_t IS_ROOT_OFFSET = 1;
    constexpr std::size_t CELL_COUNT_OFFSET = 2;
    constexpr std::size_t FREE_SPACE_OFFSET = 4;
    constexpr std::size_t RIGHT_SIBLING_OFFSET = 6;
    constexpr std::size_t COMMON_HEADER_SIZE = 10;

    /* Leaf Node Layout
       Cell pointers are uint16 offset into the page */
    constexpr std::size_t LEAF_CELL_PTR_SIZE = sizeof(std::uint16_t);
    constexpr std::size_t LEAF_CELL_PTR_ARRAY_OFFSET = COMMON_HEADER_SIZE;

    /* Leaf Cell Layout */
    constexpr std::size_t LEAF_KEY_SIZE = sizeof(std::uint64_t);
    constexpr std::size_t LEAF_VALUE_SIZE = sizeof(std::uint16_t);

    constexpr std::size_t LEAF_CELL_HEADER_SIZE = LEAF_KEY_SIZE + LEAF_VALUE_SIZE;
}