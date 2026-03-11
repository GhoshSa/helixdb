#pragma once

#include "helixdb/storage/page.hpp"
#include "helixdb/btree/node_layout.hpp"

namespace helixdb::bplushtree {
    class LeafNode {
    public:
        explicit LeafNode(storage::Page& page);

        static void _init_(storage::Page& page);

        bool insert(uint64_t key, const std::byte* value);
        bool find(uint64_t key, std::byte* out);
        uint64_t split(storage::Page& new_page);

        static constexpr uint32_t MAX_KEYS = 10;
        static constexpr uint32_t VALUE_SIZE = 32;
    
    private:
        storage::Page& page_;
        NodeHeader* header_;

        static constexpr uint32_t HEADER_SIZE = sizeof(NodeHeader);
        static constexpr uint32_t KEY_ARRAY_SIZE = sizeof(uint64_t) * MAX_KEYS;

        static_assert(HEADER_SIZE + KEY_ARRAY_SIZE + MAX_KEYS * VALUE_SIZE <= storage::PAGE_SIZE, "LeafNode layout exceeds PAGE_SIZE");

        uint64_t* keys();
        std::byte* values();
    };
}