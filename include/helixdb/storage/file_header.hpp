#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "helixdb/storage/block_constants.hpp"

namespace helixdb::storage {
    constexpr uint32_t HELIXDB_FORMAT_VERSION = 1;

    struct FileHeader {
        std::array<char, 8> magic_;
        uint32_t format_version_;
        uint32_t page_size_;
        uint32_t page_count_;
        uint32_t root_page_id_;
        uint32_t free_list_head_;
    };

    inline void initHeader(FileHeader& header) {
        std::memcpy(header.magic_.data(), "HLXDB\0\0", 8);
        header.format_version_ = HELIXDB_FORMAT_VERSION;
        header.page_size_ = PAGE_SIZE;
        header.page_count_ = 1;
        header.root_page_id_ = 0;
        header.free_list_head_ = 0;
    }

    inline void validateHeader(const FileHeader& header) {
        if (std::memcmp(header.magic_.data(), "HLXDB\0\0", 8) != 0) {
            throw std::runtime_error("Invalid database file (bad magic)");
        }

        if (header.format_version_ != HELIXDB_FORMAT_VERSION) {
            throw std::runtime_error("Unsupported database format version");
        }

        if (header.page_size_ != PAGE_SIZE) {
            throw std::runtime_error("Page size mismatch");
        }
    }
}