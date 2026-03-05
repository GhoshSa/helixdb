#pragma once

#include "helixdb/storage/pager.hpp"

namespace helixdb::bplushtree {
    class BPlushTree {
    public:
        explicit BPlushTree(storage::Pager& pager);

        void insert(uint64_t key, const std::byte* value);

        bool find(uint64_t key, std::byte* out);

    private:
        storage::Pager& pager_;

        uint32_t find_leaf(uint64_t key);

        void insert_into_root(uint32_t left, uint64_t key, uint32_t right);
    };
}