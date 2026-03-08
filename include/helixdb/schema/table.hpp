#pragma once

#include "table_schema.hpp"
#include "helixdb/btree/bplushtree.hpp"

namespace helixdb::schema {
    class Table {
    public:
        Table(TableSchema schema, storage::Pager& pager);

        void insert(uint64_t key, const std::vector<std::string>& values);

        bool select(uint64_t key);

    private:
        TableSchema schema_;
        bplushtree::BPlushTree index_;
    };
}