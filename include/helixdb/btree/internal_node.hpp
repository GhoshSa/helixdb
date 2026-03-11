#pragma once

#include "helixdb/storage/page.hpp"
#include "helixdb/btree/node_layout.hpp"

namespace helixdb::bplushtree {
    class InternalNode {
    public:
        explicit InternalNode(storage::Page& page);

        static void _init_(storage::Page& page);

        void set_left_child(uint32_t child);
        uint32_t find_child(uint64_t key);
        bool insert(uint64_t key, uint32_t right_child);
        uint64_t split(storage::Page& new_page);

        static constexpr uint32_t MAX_KEYS = 128;
    
    private:
        storage::Page& page_;
        NodeHeader* header_;

        static constexpr uint32_t HEADER_SIZE = sizeof(NodeHeader);

        uint64_t* keys();
        uint32_t* children();
    };
}