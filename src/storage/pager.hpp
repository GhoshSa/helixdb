#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "storage/page.hpp"

namespace helixdb::storage {
    class Pager {
    public:
        explicit Pager (const std::string& file_path);
        ~Pager();
        Pager(const Pager&) = delete;
        Pager& operator = (const Pager&) = delete;
        Pager(Pager&&) = default;
        Pager& operator = (Pager&&) = default;
        Page& get_page (PageId page_id);
        PageId allocate_page ();
        void flush_page (PageId page_id);
        void flush_all ();
        std::size_t page_count () const;
        
    private:
        int file_descriptor_;
        std::unordered_map<PageId, std::unique_ptr<Page>> page_cache_;
        PageId next_page_id_;
    };
}