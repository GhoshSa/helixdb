#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "storage/page.hpp"

namespace helixdb::storage {
    /* Pager Class */
    class Pager {
    public:
        explicit Pager (const std::string& file_path); // Opens or creates a database file
        ~Pager(); // Flushes all dirty pages and closes the database file
        Pager(const Pager&) = delete;
        Pager& operator = (const Pager&) = delete;
        Pager(Pager&&) = default;
        Pager& operator = (Pager&&) = default;
        Page& get_page (PageId page_id); // Returns a reference to the page with the given page ID
        PageId allocate_page (); // Allocates a new page ID
        void flush_page (PageId page_id); // Flushes a single dirty page to disk
        void flush_all (); // Flushes all dirty pages to the disk
        std::size_t page_count () const; // Returns the number of pages currently allocated on disk
        
    private:
        int file_descriptor_; // File descriptor of the database file
        std::unordered_map<PageId, std::unique_ptr<Page>> page_cache_; // In-memory page cache
        PageId next_page_id_; // ID to assign to the next allocated page
    };
}