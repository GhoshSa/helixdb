#include "helixdb/schema/table.hpp"
#include "helixdb/schema/row_serializer.hpp"

namespace helixdb::schema {
    Table::Table(TableSchema schema, storage::Pager &pager, uint32_t root_page_id) : schema_(std::move(schema)), index_(pager, root_page_id) {}

    void Table::insert(uint64_t key, const std::vector<std::string> &values) {
        auto row = serialize_row(schema_, values);
        index_.insert(key, row.data());
    }

    bool Table::select(uint64_t key) {
        std::vector<std::byte> buffer(4096, std::byte{0});

        if (!index_.find(key, buffer.data())) return false;

        deserialize_row(schema_, buffer.data());

        return true;
    }
}
