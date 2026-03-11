#include "helixdb/schema/schema_manager.hpp"

namespace helixdb::schema {
    SchemaManager::SchemaManager(storage::Pager &pager) : pager_(pager), catalog_(pager) {}

    std::string SchemaManager::schema_to_string(const TableSchema &schema) {
        std::string s;
        for (const auto& c : schema.columns) {
            s += std::string(c.name.begin(), c.name.end());
            s += (c.type == ColumnType::INT) ? " INT," : " TEXT,";
        }

        return s;
    }

    void SchemaManager::create_table(TableSchema schema) {
        tables_.emplace(schema.name, Table(schema, pager_, bplushtree::BPlushTree::NULL_PAGE));
        catalog_.register_table(schema.name, tables_.at(schema.name).root_page_id(), schema_to_string(schema));
    }

    Table &SchemaManager::get_table(const std::string &name) {
        return tables_.at(name);
    }
}
