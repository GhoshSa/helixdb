#include "helixdb/btree/leaf_node.hpp"

#include <cstring>

namespace helixdb::bplushtree {
    LeafNode::LeafNode(storage::Page &page) : page_(page) {
        header_ = reinterpret_cast<NodeHeader*>(page_.data());
    }

    void LeafNode::_init_(storage::Page &page) {
        auto* header = reinterpret_cast<NodeHeader*>(page.data());
        header->type = NodeType::LEAF;
        header->key_count = 0;
        header->parent = 0;
    }

    uint64_t* LeafNode::keys() {
        return reinterpret_cast<uint64_t*>(page_.data() + HEADER_SIZE);
    }

    std::byte* LeafNode::values() {
        return page_.data() + HEADER_SIZE + KEY_ARRAY_SIZE;
    }

    bool LeafNode::insert(uint64_t key, const std::byte *value) {
        if (header_->key_count >= MAX_KEYS) return false;

        uint32_t pos = 0;
        while (pos < header_->key_count && keys()[pos] < key) pos++;

        for (uint32_t i = header_->key_count; i > pos; --i) {
            keys()[i] = keys()[i - 1];
            std::memcpy(values() + i * VALUE_SIZE, values() + (i - 1) * VALUE_SIZE, VALUE_SIZE);
        }

        keys()[pos] = key;
        std::memcpy(values() + pos * VALUE_SIZE, value, VALUE_SIZE);

        header_->key_count++;

        page_.mark_dirty();

        return true;
    }

    bool LeafNode::find(uint64_t key, std::byte *out) {
        int left = 0, right = header_->key_count - 1;

        while (left <= right) {
            int mid = (left + right) / 2;

            if (keys()[mid] == key) {
                std::memcpy(out, values() + mid * VALUE_SIZE, VALUE_SIZE);
                return true;
            }
            if (keys()[mid] < key) left = mid + 1;
            else right = mid - 1;
        }

        return false;
    }

    uint64_t LeafNode::split(storage::Page &new_page) {
        _init_(new_page);

        uint32_t mid = header_->key_count / 2;

        auto* new_header = reinterpret_cast<NodeHeader*>(new_page.data());
        new_header->parent = header_->parent;

        auto* new_keys = reinterpret_cast<uint64_t*>(new_page.data() + HEADER_SIZE);
        auto* new_values = new_page.data() + HEADER_SIZE + KEY_ARRAY_SIZE;

        for (uint32_t i = mid; i < header_->key_count; ++i) {
            uint32_t index = new_header->key_count++;
            new_keys[index] = keys()[i];
            std::memcpy(new_values + index * VALUE_SIZE, values() + i * VALUE_SIZE, VALUE_SIZE);
        }

        header_->key_count = mid;

        page_.mark_dirty();
        new_page.mark_dirty();

        return new_keys[0];
    }
}
