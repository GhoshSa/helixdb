#include "helixdb/schema/row_serializer.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>

namespace helixdb::schema {
    std::vector<std::byte> serialize_row(const TableSchema &schema, const std::vector<std::string>& values) {
        std::vector<std::byte> buffer;

        buffer.resize(sizeof(uint32_t));

        for (size_t i = 0; i < schema.columns.size(); ++i) {
            const Column& col = schema.columns[i];
            const std::string& val = values[i];

            if (col.type == ColumnType::INT) {
                int v = std::stoi(val);

                std::byte* ptr = reinterpret_cast<std::byte*>(&v);
                buffer.insert(buffer.end(), ptr, ptr + sizeof(int));
            } else {
                uint32_t len = val.size();

                std::byte* lptr = reinterpret_cast<std::byte*>(&len);
                buffer.insert(buffer.end(), lptr, lptr + sizeof(len));

                const std::byte* sptr = reinterpret_cast<const std::byte*>(val.data());
                buffer.insert(buffer.end(), sptr, sptr + len);
            }
        }

        uint32_t row_size = buffer.size() - sizeof(uint32_t);
        std::memcpy(buffer.data(), &row_size, sizeof(uint32_t));

        return buffer;
    }

    void deserialize_row(const TableSchema &schema, const std::byte* data) {
        const std::byte* ptr = data;

        uint32_t row_size;
        std::memcpy(&row_size, ptr, sizeof(uint32_t));
        ptr += sizeof(uint32_t);

        for (const auto& col : schema.columns) {
            if (col.type == ColumnType::INT) {
                int v;
                std::memcpy(&v, ptr, sizeof(int));

                std::cout << col.name << ":" << v << " ";

                ptr += sizeof(int);
            } else {
                uint32_t len;
                std::memcpy(&len, ptr, sizeof(uint32_t));

                ptr += sizeof(uint32_t);

                std::string s(reinterpret_cast<const char*>(ptr), len);

                std::cout << col.name << ":" << s << " ";

                ptr += len;
            }
        }

        std::cout << std::endl;
    }
}
