#pragma once

#include <cstdint>
#include <array>
#include <stdexcept>

#include "helixdb/storage/block_constants.hpp"

namespace helixdb::storage {
    class Page {
    public:
        explicit Page(uint32_t page_id) : page_id_(page_id) {}

        Page(const Page&) = delete;
        Page& operator = (const Page&) = delete;

        uint32_t id() const noexcept { return page_id_; }

        std::byte* data() noexcept { return data_.data(); }

        bool is_dirty() const noexcept { return dirty_; }
    
    private:
        uint32_t page_id_;
        std::array<std::byte, PAGE_SIZE> data_ {};
        bool dirty_ = false;
    
    private:
        friend class Pager;
        
        void mark_dirty() noexcept { dirty_ = true; }
        void clear_dirty() noexcept { dirty_ = false; }
    };
}