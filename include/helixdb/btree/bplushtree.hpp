#pragma once

#include "helixdb/storage/pager.hpp"

namespace helixdb::bplushtree {
    class BPlushTree {
    public:
        explicit BPlushTree(storage::Pager& pager, uint32_t root_page_id = 0);

        void insert(uint64_t key, const std::byte* value);
        bool find(uint64_t key, std::byte* out);

        uint32_t root_page_id() const noexcept { return root_page_id_; }

        static constexpr uint32_t NULL_PAGE = 0;

    private:
        storage::Pager& pager_;
        uint32_t root_page_id_;

        uint32_t find_leaf(uint64_t key);
        void insert_into_root(uint32_t left, uint64_t key, uint32_t right);
    };
}