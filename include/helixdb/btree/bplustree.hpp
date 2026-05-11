#pragma once

#include "helixdb/storage/pager.hpp"
#include "helixdb/btree/btree_cursor.hpp"

namespace helixdb::bplustree {
    class BPlusTree {
    public:
        explicit BPlusTree(storage::Pager& pager);

        void insert(uint64_t key, uint32_t value);
        auto find(uint64_t key, uint32_t& out) -> bool;

        auto begin() -> Cursor;
        auto lowerBound(uint64_t key) -> Cursor;

    private:
        storage::Pager* pager_;

        enum class NodeType : uint8_t {
            INTERNAL = 0,
            LEAF = 1
        };

        struct NodeHeader {
            NodeType type_;
            uint16_t num_keys_;
            uint32_t parent_;
        };
    
    
        [[nodiscard]] static auto leafMaxKeys() -> uint32_t;
        [[nodiscard]] static auto internalMaxKeys() -> uint32_t;
        [[nodiscard]] static auto leafCapacity() -> uint32_t;
        [[nodiscard]] static auto internalCapacity() -> uint32_t;

        void initLeaf(uint32_t pageid);
        void initInternal(uint32_t pageid);

        auto findLeaf(uint64_t key) -> uint32_t;

        void insertIntoLeaf(uint32_t pageid, uint64_t key, uint32_t value);
        void splitLeaf(uint32_t leaf_id);
        void insertIntoParent(uint32_t left, uint64_t key, uint32_t right);
        void splitInternal(uint32_t pageid);

        friend class Cursor;
    };
}