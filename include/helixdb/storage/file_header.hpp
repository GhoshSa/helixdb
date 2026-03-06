#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "helixdb/storage/block_constants.hpp"

namespace helixdb::storage {
    constexpr uint32_t HELIXDB_FORMAT_VERSION = 1;

    struct FileHeader {
        char magic[8];
        uint32_t format_version;
        uint32_t page_size;
        uint32_t page_count;
        uint32_t root_page_id;
        uint32_t free_list_head;
    };

    inline void init_header(FileHeader& header) {
        std::memcpy(header.magic, "HLXDB\0\0", 8);
        header.format_version = HELIXDB_FORMAT_VERSION;
        header.page_size = PAGE_SIZE;
        header.page_count = 1;
        header.root_page_id = 0;
        header.free_list_head = 0;
    }

    inline void validate_header(const FileHeader& header) {
        if (std::memcmp(header.magic, "HLXDB\0\0", 8) != 0) {
            throw std::runtime_error("Invalid database file (bad magic)");
        }

        if (header.format_version != HELIXDB_FORMAT_VERSION) {
            throw std::runtime_error("Unsupported database format version");
        }

        if (header.page_size != PAGE_SIZE) {
            throw std::runtime_error("Page size mismatch");
        }
    }
}