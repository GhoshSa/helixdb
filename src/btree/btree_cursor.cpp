#include "helixdb/btree/btree_cursor.hpp"
#include "helixdb/btree/bplustree.hpp"

namespace helixdb::bplustree {
    Cursor::Cursor(storage::Pager& pager, uint32_t pageid, uint32_t index) : pager_(&pager), pageid_(pageid), index_(index) { // NOLINT(bugprone-easily-swappable-parameters)
        auto& page = pager_->getPage(pageid_);
        auto* hdr = reinterpret_cast<BPlusTree::NodeHeader*>(page.data());

        valid_ = (index_ < hdr->num_keys_);
    }

    auto Cursor::valid() const -> bool {
        return valid_;
    }

    void Cursor::next() {
        if (!valid_) {
            return;
        }

        auto& page = pager_->getPage(pageid_);
        auto* hdr = reinterpret_cast<BPlusTree::NodeHeader*>(page.data());

        index_++;

        if (index_ < hdr->num_keys_) {
            return;
        }

        auto* next = reinterpret_cast<uint32_t*>(hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (*next == 0) {
            valid_ = false;
            return;
        }

        pageid_ = *next;
        index_ = 0;
    }

    auto Cursor::key() const -> uint64_t {
        auto& page = pager_->getPage(pageid_);
        auto* hdr = reinterpret_cast<BPlusTree::NodeHeader*>(page.data());

        auto* next = reinterpret_cast<uint32_t*>(hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* keys = reinterpret_cast<uint64_t*>(next + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        return keys[index_]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }

    auto Cursor::value() const -> uint32_t {
        auto& page = pager_->getPage(pageid_);
        auto* hdr = reinterpret_cast<BPlusTree::NodeHeader*>(page.data());

        auto* next = reinterpret_cast<uint32_t*>(hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* keys = reinterpret_cast<uint64_t*>(next + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        uint32_t max_keys = (storage::PAGE_SIZE - sizeof(BPlusTree::NodeHeader) - sizeof(uint32_t)) / (sizeof(uint64_t) + sizeof(uint32_t));

        auto* values = reinterpret_cast<uint32_t*>(keys + max_keys); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        return values[index_]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
}