#pragma once

#include <vector>

#include "column.hpp"

namespace helixdb::schema {
    struct TableSchema {
        std::string name;
        std::vector<Column> columns;
    };
}