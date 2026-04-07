#pragma once

#include "helixdb/storage/pager.hpp"

namespace helixdb::bplushtree {
    class Cursor {
    public:
        Cursor(storage::Pager& pager, uint32_t pageid, uint32_t index);

        bool valid() const;
        void next();
        
        uint64_t key() const;
        uint32_t value() const;
    
    private:
        storage::Pager& pager_;
        uint32_t pageid_;
        uint32_t index_;
        bool valid_;
    };
}