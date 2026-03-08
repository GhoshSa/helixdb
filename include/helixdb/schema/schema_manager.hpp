#pragma once

#include "catalog.hpp"

#include <unordered_map>

namespace helixdb::schema {
    class SchemaManager {
    public:
        explicit SchemaManager(storage::Pager& pager);

        void create_table(TableSchema schema);

        Table& get_table(const std::string& name);

    private:
        std::string schema_to_string(const TableSchema& schema);
        storage::Pager& pager_;
        Catalog catalog_;
        std::unordered_map<std::string,Table> tables_;
    };
}