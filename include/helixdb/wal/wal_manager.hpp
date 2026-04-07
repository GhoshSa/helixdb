#pragma once

#include "helixdb/storage/block_device.hpp"

namespace helixdb::wal {
    class WalManager {
    public:
        explicit WalManager(const std::string &db_path);
        ~WalManager();

        void append(uint32_t page_id, const std::byte* page_data);
        void flush();

        void recover(storage::BlockDevice& device);
        void truncate();

    private:
        int fd_;
        std::string path_;
    };
}