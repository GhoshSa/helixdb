#pragma once

#include <cstdint>
#include <array>
#include <stdexcept>

#include "helixdb/storage/block_constants.hpp"

namespace helixdb::storage {
    class Page {
    public:
        explicit Page(uint32_t page_id) : page_id_(page_id) {}
        ~Page() = default;
        
        Page(const Page&) = delete;
        auto operator = (const Page&) -> Page& = delete;

        Page(Page&&) = delete;
        auto operator = (Page&&) -> Page& = delete;

        [[nodiscard]] auto id() const noexcept -> uint32_t { return page_id_; }

        auto data() noexcept -> std::byte* { return data_.data(); }

        [[nodiscard]] auto isDirty() const noexcept -> bool { return dirty_; }
    
    private:
        uint32_t page_id_;
        std::array<std::byte, PAGE_SIZE> data_ {};
        bool dirty_ = false;
        
        friend class Pager;
        
        void markDirty() noexcept { dirty_ = true; }
        void clearDirty() noexcept { dirty_ = false; }
    };
}