#include "pager.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <sys/stat.h>

namespace helixdb::storage {
    Pager::Pager (const std::string& file_path) : file_descriptor_(-1), next_page_id_(0) {
        file_descriptor_ = ::open (
            file_path.c_str(),
            O_RDWR | O_CREAT,
            S_IRUSR | S_IWUSR
        );
        if (file_descriptor_ == -1) {
            throw std::runtime_error("Failed to open or create file: " + std::string(std::strerror(errno)));
        }
        struct stat st {};
        if (::fstat(file_descriptor_, &st) == -1) {
            ::close(file_descriptor_);
            throw std::runtime_error("Pager: failed to stat file: " + std::string(std::strerror(errno)));
        }
        if (st.st_size % PAGE_SIZE != 0) {
            ::close(file_descriptor_);
            throw std::runtime_error("Pager: database file is corrupted.");
        }
        next_page_id_ = static_cast<PageId>(st.st_size / PAGE_SIZE);
    }

    Pager::~Pager () {
        try {
            flush_all();
        } catch (...) {}
        if (file_descriptor_ != -1) {
            ::close(file_descriptor_);
        }
    }

    Page& Pager::get_page (PageId page_id) {
        auto it = page_cache_.find(page_id);
        if (it != page_cache_.end()) {
            return *(it->second);
        }
        auto page = std::make_unique<Page>();
        off_t offset = static_cast<off_t>(page_id * PAGE_SIZE);
        ssize_t bytes_read = ::pread(file_descriptor_, page->data(), PAGE_SIZE, offset);
        if (bytes_read == -1) {
            throw std::runtime_error("Pager: failed to read page: " + std::string(std::strerror(errno)));
        }
        page_cache_[page_id] = std::move(page);
        return *(page_cache_[page_id]);
    }

    PageId Pager::allocate_page () {
        PageId new_page_id = next_page_id_++;
        auto page = std::make_unique<Page>();
        page->mark_dirty();
        page_cache_[new_page_id] = std::move(page);
        return new_page_id;
    }

    void Pager::flush_page (PageId page_id) {
        auto it = page_cache_.find(page_id);
        if (it == page_cache_.end()) return;
        Page& page = *(it->second);
        if (!page.is_dirty()) return;
        off_t offset = static_cast<off_t>(page_id * PAGE_SIZE);
        ssize_t bytes_written = ::pwrite(file_descriptor_, page.data(), PAGE_SIZE, offset);
        if (bytes_written != static_cast<ssize_t>(PAGE_SIZE)) {
            throw std::runtime_error("Pager: failed to write page: " + std::string(std::strerror(errno)));
        }
        page.clear_dirty();
    }

    void Pager::flush_all () {
        for (auto& [page_id, page] : page_cache_) {
            if (page->is_dirty()) {
                flush_page(page_id);
            }
        }
    }

    std::size_t Pager::page_count () const {
        return next_page_id_;
    }
}