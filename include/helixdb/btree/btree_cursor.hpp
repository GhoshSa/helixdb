#pragma once

#include "helixdb/storage/pager.hpp"

namespace helixdb::bplustree {
    class Cursor {
    public:
        Cursor(storage::Pager& pager, uint32_t pageid, uint32_t index);

        [[nodiscard]] auto valid() const -> bool;
        void next();
        
        [[nodiscard]] auto key() const -> uint64_t;
        [[nodiscard]] auto value() const -> uint32_t;
    
    private:
        storage::Pager* pager_;
        uint32_t pageid_;
        uint32_t index_;
        bool valid_;
    };
}