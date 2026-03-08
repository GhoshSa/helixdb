#include "helixdb/schema/schema_manager.hpp"

namespace helixdb::schema {
    SchemaManager::SchemaManager(storage::Pager &pager) : pager_(pager), catalog_(pager) {}

    std::string SchemaManager::schema_to_string(const TableSchema &schema) {
        std::string s;

        for (const auto& c : schema.columns) {
            s += c.name;

            if (c.type == ColumnType::INT) s += " INT";
            else s += " TEXT";

            s += ",";
        }

        return s;
    }

    void SchemaManager::create_table(TableSchema schema) {
        tables_.emplace(schema.name, Table(schema,pager_));
        catalog_.register_table(schema.name, pager_.root_page_id(), schema_to_string(schema));
    }

    Table &SchemaManager::get_table(const std::string &name) {
        return tables_.at(name);
    }
}
