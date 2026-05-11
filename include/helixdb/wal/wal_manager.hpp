#pragma once

#include "helixdb/storage/block_device.hpp"

namespace helixdb::wal {
    class WalManager {
    public:
        explicit WalManager(const std::string &db_path);
        ~WalManager() noexcept;

        WalManager(const WalManager&) = delete;
        auto operator = (const WalManager&) -> WalManager& = delete;

        WalManager(WalManager&& other) noexcept;
        auto operator = (WalManager&& other) noexcept -> WalManager&;

        void append(uint32_t page_id, const std::byte* page_data) const;
        void flush() const;

        void recover(storage::BlockDevice& device) const;
        void truncate() const;

    private:
        int fd_;
        std::string path_;
    };
}