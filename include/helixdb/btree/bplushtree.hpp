#pragma once

#include "helixdb/storage/pager.hpp"
#include "helixdb/btree/btree_cursor.hpp"

namespace helixdb::bplushtree {
    class BPlushTree {
    public:
        explicit BPlushTree(storage::Pager& pager);

        void insert(uint64_t key, uint32_t value);
        bool find(uint64_t key, uint32_t& out);

        Cursor begin();
        Cursor lower_bound(uint64_t key);

    private:
        storage::Pager& pager_;

        enum class NodeType : uint8_t {
            INTERNAL = 0,
            LEAF = 1
        };

        struct NodeHeader {
            NodeType type;
            uint16_t num_keys;
            uint32_t parent;
        };
    
    private:
        uint32_t leaf_max_keys() const;
        uint32_t internal_max_keys() const;

        void init_leaf(uint32_t pageid);
        void init_internal(uint32_t pageid);

        uint32_t find_leaf(uint64_t key);

        void insert_into_leaf(uint32_t pageid, uint64_t key, uint32_t value);
        void split_leaf(uint32_t leaf_id);
        void insert_into_parent(uint32_t left, uint64_t key, uint32_t right);
        void split_internal(uint32_t pageid);

        friend class Cursor;
    };
}