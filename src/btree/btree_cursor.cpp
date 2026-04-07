#include "helixdb/btree/btree_cursor.hpp"
#include "helixdb/btree/bplushtree.hpp"

namespace helixdb::bplushtree {
    Cursor::Cursor(storage::Pager& pager, uint32_t pageid, uint32_t index) : pager_(pager), pageid_(pageid), index_(index) {
        auto& page = pager_.get_page(pageid_);
        auto* hdr = reinterpret_cast<BPlushTree::NodeHeader*>(page.data());

        valid_ = (index_ < hdr->num_keys);
    }

    bool Cursor::valid() const {
        return valid_;
    }

    void Cursor::next() {
        if (!valid_) return;

        auto& page = pager_.get_page(pageid_);
        auto* hdr = reinterpret_cast<BPlushTree::NodeHeader*>(page.data());

        index_++;

        if (index_ < hdr->num_keys)return;

        uint32_t* next = reinterpret_cast<uint32_t*>(hdr + 1);
        if (*next == 0) {
            valid_ = false;
            return;
        }

        pageid_ = *next;
        index_ = 0;
    }

    uint64_t Cursor::key() const {
        auto& page = pager_.get_page(pageid_);
        auto* hdr = reinterpret_cast<BPlushTree::NodeHeader*>(page.data());

        uint32_t* next = reinterpret_cast<uint32_t*>(hdr + 1);
        uint64_t* keys = reinterpret_cast<uint64_t*>(next + 1);

        return keys[index_];
    }

    uint32_t Cursor::value() const {
        auto& page = pager_.get_page(pageid_);
        auto* hdr = reinterpret_cast<BPlushTree::NodeHeader*>(page.data());

        uint32_t* next = reinterpret_cast<uint32_t*>(hdr + 1);
        uint64_t* keys = reinterpret_cast<uint64_t*>(next + 1);

        uint32_t max_keys = (storage::PAGE_SIZE - sizeof(BPlushTree::NodeHeader) - sizeof(uint32_t)) / (sizeof(uint64_t) + sizeof(uint32_t));

        uint32_t* values = reinterpret_cast<uint32_t*>(keys + max_keys);

        return values[index_];
    }
}