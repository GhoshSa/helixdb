#pragma once

#include "table_schema.hpp"
#include "helixdb/btree/bplushtree.hpp"

namespace helixdb::schema {
    class Table {
    public:
        Table(TableSchema schema, storage::Pager& pager, uint32_t root_page_id = 0);

        void insert(uint64_t key, const std::vector<std::string>& values);
        bool select(uint64_t key);

        uint32_t root_page_id() const noexcept { return index_.root_page_id(); }

    private:
        TableSchema schema_;
        bplushtree::BPlushTree index_;
    };
}