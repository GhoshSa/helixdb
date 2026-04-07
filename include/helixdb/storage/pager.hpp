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

        Page& get_page(uint32_t page_id);

        uint32_t allocate_page();
        void free_page(uint32_t page_id);

        void mark_page_dirty(uint32_t page_id);

        void flush_page(uint32_t page_id);
        void flush_all();

        uint32_t root_page_id() const;
        void set_root_page(uint32_t id);
    
    private:
        BlockDevice device_;
        wal::WalManager wal_;

        std::unordered_map<uint32_t, std::unique_ptr<Page>> cache_;
        uint32_t page_count_;

        void load_page(uint32_t page_id);
    };
}