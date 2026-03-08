#include "helixdb/schema/catalog.hpp"

namespace helixdb::schema {
    Catalog::Catalog(storage::Pager &pager) : master_schema_({
        "helixdb_master", {
            {"table_name", ColumnType::TEXT},
            {"root_page", ColumnType::INT},
            {"schema", ColumnType::TEXT}
        }
    }), master_(master_schema_, pager) {}

    uint64_t Catalog::hash(const std::string &s) {
        return std::hash<std::string>{}(s);
    }

    void Catalog::register_table(const std::string &name, uint32_t root_page, const std::string &schema_str) {
        std::vector<std::string> values = {
            name,
            std::to_string(root_page),
            schema_str
        };

        master_.insert(hash(name), values);
    }
}
