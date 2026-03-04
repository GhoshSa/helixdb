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

        // Page(Page&&) = default;
        // Page& operator = (Page&&) = default;

        uint32_t id() const noexcept { return page_id_; }

        std::byte* data() noexcept { return data_.data(); }
        const std::byte* data() const noexcept { return data_.data(); }

        void mark_dirty() noexcept { dirty_ = true; }
        void clear_dirty() noexcept { dirty_ = false; }
        bool is_dirty() const noexcept { return dirty_; }

        void pin() noexcept { ++pin_count_; }
        void unpin() {
            if (pin_count_ == 0) throw std::logic_error("Page::unpin() called with pin_count_ == 0");
            --pin_count_;
        }
        bool is_pinned() const noexcept { return pin_count_ > 0; }
        // uint32_t pin_count() { return pin_count_; }
    
    private:
        uint32_t page_id_;
        std::array<std::byte, PAGE_SIZE> data_ {};
        bool dirty_ = false;
        uint32_t pin_count_ = 0;
    };
}