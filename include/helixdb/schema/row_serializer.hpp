#pragma once

#include <vector>

#include "table_schema.hpp"

namespace helixdb::schema {
    std::vector<std::byte> serialize_row(const TableSchema& schema, const std::vector<std::string>& values);
    void deserialize_row(const TableSchema& schema, const std::byte* data);
}