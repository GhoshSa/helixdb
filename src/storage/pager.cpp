#include "helixdb/storage/pager.hpp"

namespace helixdb::storage {
    Pager::Pager(const std::string& path) : device_(path), wal_(path), page_count_(static_cast<uint32_t>(device_.blockCount())) {
        
        if (page_count_ == 0) {
            auto header_page = std::make_unique<Page>(0);

            FileHeader header {};
            initHeader(header);

            std::memcpy(header_page->data(), &header, sizeof(FileHeader));

            cache_[0] = std::move(header_page);
            page_count_ = 1;

            markPageDirty(0);

            flushAll();
        }

        wal_.recover(device_);

        auto header_page = std::make_unique<Page>(0);
        device_.read(0, header_page->data());

        FileHeader header {};
        std::memcpy(&header, header_page->data(), sizeof(FileHeader));
        validateHeader(header);
        
        page_count_ = header.page_count_;
        
        cache_[0] = std::move(header_page);
    }

    Pager::~Pager() {
        flushAll();
    }

    auto Pager::getPage(uint32_t page_id) -> Page& {
        if (page_id >= page_count_) {
            throw std::runtime_error("invalid page id");
        }

        auto iter = cache_.find(page_id);
        if (iter == cache_.end()) {
            return loadPage(page_id);
        }
        return *iter->second;
    }

    auto Pager::loadPage(uint32_t page_id) -> Page& {
        auto page = std::make_unique<Page>(page_id);

        device_.read(page_id, page->data());
        auto [iter, inserted] = cache_.try_emplace(page_id, std::move(page));
        return *iter->second;
    }

    auto Pager::allocatePage() -> uint32_t {
        auto* header = reinterpret_cast<FileHeader*>(getPage(0).data());
        uint32_t new_id = 0;

        if (header->free_list_head_ != 0) {
            uint32_t page_id = header->free_list_head_;
            Page& page = getPage(page_id);
            header->free_list_head_ = *reinterpret_cast<uint32_t*>(page.data());
            new_id = page_id;
        } else {
            new_id = page_count_++;
            auto page = std::make_unique<Page>(new_id);
            cache_[new_id] = std::move(page);
        }

        header->page_count_ = page_count_;
        markPageDirty(0);

        return new_id;
    }

    void Pager::freePage(uint32_t page_id) {
        if (page_id == 0) {
            throw std::runtime_error("cannot free header page");
        }

        auto* header = reinterpret_cast<FileHeader*>(getPage(0).data());
        Page& page = getPage(page_id);

        auto* next = reinterpret_cast<uint32_t*>(page.data());
        *next = header->free_list_head_;
        header->free_list_head_ = page_id;

        markPageDirty(page_id);
        markPageDirty(0);
    }

    void Pager::markPageDirty(uint32_t page_id) {
        auto page_iter = cache_.find(page_id);
        if (page_iter == cache_.end()) {
            return;
        }

        Page& page = *page_iter->second;

        wal_.append(page_id, page.data());

        page.markDirty();
    }

    void Pager::flushPage(uint32_t page_id) {
        if (!cache_.contains(page_id)) {
            return;
        }

        Page& page = *cache_.at(page_id);
        if (!page.isDirty()) {
            return;
        }

        device_.write(page_id, page.data());
        device_.flush();

        page.clearDirty();
    }

    void Pager::flushAll() {
        for (auto& [page_id, page_ptr] : cache_) {
            if (page_ptr->isDirty()) {
                flushPage(page_id);
            }
        }

        wal_.truncate();
    }

    auto Pager::rootPageId() const -> uint32_t {
        const auto* header = reinterpret_cast<const FileHeader*>(cache_.at(0)->data());
        return header->root_page_id_;
    }

    void Pager::setRootPage(uint32_t page_id) {
        auto* header = reinterpret_cast<FileHeader*>(cache_.at(0)->data());
        header->root_page_id_ = page_id;
        markPageDirty(0);
    }
}