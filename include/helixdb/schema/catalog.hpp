#pragma once

#include "table.hpp"

#include <string>

namespace helixdb::schema {
    class Catalog {
    public:
        explicit Catalog(storage::Pager& pager);

        void register_table(
            const std::string& name,
            uint32_t root_page,
            const std::string& schema_str);

    private:
        uint64_t hash(const std::string& s);
        TableSchema master_schema_;
        Table master_;
    };

}