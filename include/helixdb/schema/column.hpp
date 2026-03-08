#pragma once

#include <string>

namespace  helixdb::schema {
    enum  class ColumnType {
        INT,
        TEXT
    };

    struct Column {
        std::pmr::string name;
        ColumnType type;
    };
}