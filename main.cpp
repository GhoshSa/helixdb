#include "helixdb/storage/pager.hpp"
#include "helixdb/schema/schema_manager.hpp"

#include <iostream>

using namespace helixdb;

int main() {
    storage::Pager pager("test.db");

    schema::SchemaManager manager(pager);

    schema::TableSchema users {
        "users",
        {
            {"id", schema::ColumnType::INT},
            {"name", schema::ColumnType::TEXT}
        }
    };

    manager.create_table(users);

    auto& table = manager.get_table("users");

    table.insert(1, {"1", "Alice"});
    table.insert(2, {"2", "Bob"});
    table.insert(3, {"3", "Charlie"});

    std::cout << "Query:\n";

    table.select(1);
    table.select(2);
    table.select(3);

    return 0;
}