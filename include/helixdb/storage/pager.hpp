#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "helixdb/storage/page.hpp"
#include "helixdb/storage/block_device.hpp"
#include "helixdb/storage/file_header.hpp"
#include "helixdb/wal/wal_manager.hpp"

namespace helixdb::storage {
    class Pager {
    public:
        explicit Pager(const std::string& path);
        ~Pager();

        Pager(const Pager&) = delete;
        auto operator=(const Pager&) -> Pager& = delete;

        Pager(Pager&&) = delete;
        auto operator=(Pager&&) -> Pager& = delete;

        auto getPage(uint32_t page_id) -> Page&;

        auto allocatePage() -> uint32_t;
        void freePage(uint32_t page_id);

        void markPageDirty(uint32_t page_id);

        void flushPage(uint32_t page_id);
        void flushAll();
        
        [[nodiscard]] auto rootPageId() const -> uint32_t;
        void setRootPage(uint32_t page_id);
    
    private:
        BlockDevice device_;
        wal::WalManager wal_;

        std::unordered_map<uint32_t, std::unique_ptr<Page>> cache_;
        uint32_t page_count_;

        auto loadPage(uint32_t page_id) -> Page&;
    };
}