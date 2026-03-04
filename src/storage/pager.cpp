#include "helixdb/storage/pager.hpp"

namespace helixdb::storage {
    Pager::Pager(const std::string& path) : device_(path), wal_(path) {
        // wal_.recover(device_);
        // wal_.flush();

        // page_count_ = static_cast<uint32_t>(device_.block_count());
        // if (page_count_ == 0) {
        //     auto header_page = std::make_unique<Page>(0);

        //     FileHeader header {};
        //     init_header(header);

        //     std::memcpy(header_page->data(), &header, sizeof(FileHeader));
        //     header_page->mark_dirty();

        //     cache_[0] = std::move(header_page);
        //     page_count_ = 1;
        //     flush_all();
        // } else {
        //     auto header_page = std::make_unique<Page>(0);
        //     device_.read(0, header_page->data());

        //     FileHeader header {};
        //     std::memcpy(&header, header_page->data(), sizeof(FileHeader));
        //     validate_header(header);

        //     page_count_ = header.page_count;
        //     cache_[0] = std::move(header_page);
        // }

        page_count_ = static_cast<uint32_t>(device_.block_count());
        if (page_count_ == 0) {
            auto header_page = std::make_unique<Page>(0);

            FileHeader header {};
            init_header(header);

            std::memcpy(header_page->data(), &header, sizeof(FileHeader));
            header_page->mark_dirty();

            cache_[0] = std::move(header_page);
            page_count_ = 1;

            flush_all();
        }

        wal_.recover(device_);
        wal_.truncate();

        auto header_page = std::make_unique<Page>(0);
        device_.read(0, header_page->data());

        FileHeader header {};
        std::memcpy(&header, header_page->data(), sizeof(FileHeader));
        validate_header(header);
        
        page_count_ = header.page_count;
        cache_[0] = std::move(header_page);
    }

    Pager::~Pager() {
        flush_all();
    }

    Page& Pager::get_page(uint32_t page_id) {
        if (page_id >= page_count_) throw std::runtime_error("invalid page id");

        if (!cache_.contains(page_id)) load_page(page_id);
        Page& page = *cache_.at(page_id);
        return page;
    }

    void Pager::load_page(uint32_t page_id) {
        auto page = std::make_unique<Page>(page_id);

        device_.read(page_id, page->data());
        cache_[page_id] = std::move(page);
    }

    uint32_t Pager::allocate_page() {
        uint32_t new_id = page_count_++;

        auto page = std::make_unique<Page>(new_id);
        page->mark_dirty();
        cache_[new_id] = std::move(page);

        auto* header = reinterpret_cast<FileHeader*>(cache_.at(0)->data());
        header->page_count = page_count_;
        cache_.at(0)->mark_dirty();

        return new_id;
    }

    void Pager::flush_page(uint32_t page_id) {
        if (!cache_.contains(page_id)) return;

        Page& page = *cache_.at(page_id);
        if (!page.is_dirty()) return;

        wal_.append(page_id, page.data());
        wal_.flush();

        device_.write(page_id, page.data());
        device_.flush();

        page.clear_dirty();
    }

    void Pager::flush_all() {
        for (auto& [id, page_ptr] : cache_) {
            if (page_ptr->is_dirty()) {
                flush_page(id);
            }
        }
    }

    uint32_t Pager::root_page_id() const {
        const auto* header = reinterpret_cast<const FileHeader*>(cache_.at(0)->data());
        return header->root_page_id;
    }

    void Pager::set_root_page(uint32_t page_id) {
        auto* header = reinterpret_cast<FileHeader*>(cache_.at(0)->data());
        header->root_page_id = page_id;
        cache_.at(0)->mark_dirty();
    }
}