#pragma once

#include "helixdb/storage/block_constants.hpp"

namespace helixdb::wal {
    constexpr std::size_t WAL_HEADER_SIZE = sizeof(uint32_t) + sizeof(uint32_t);
    constexpr std::size_t WAL_RECORD_SIZE = WAL_HEADER_SIZE + storage::PAGE_SIZE;
}