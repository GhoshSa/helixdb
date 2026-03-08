#pragma once

#include <cstdint>

namespace helixdb::bplushtree{
    enum class NodeType : uint8_t {
        INTERNAL = 1,
        LEAF = 2
    };

    struct NodeHeader {
        NodeType type;
        uint16_t key_count;
        uint32_t parent;
    };
}