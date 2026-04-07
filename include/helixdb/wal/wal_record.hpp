#pragma once

#include "helixdb/storage/block_constants.hpp"

namespace helixdb::wal {
    struct WalRecordHeader {
        uint32_t page_id;
        uint32_t page_size;
    };
}