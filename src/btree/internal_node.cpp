#include "helixdb/btree/internal_node.hpp"

namespace helixdb::bplushtree {
    InternalNode::InternalNode(storage::Page &page) : page_(page) {
        header_ = reinterpret_cast<NodeHeader*>(page_.data());
    }

    void InternalNode::_init_(storage::Page& page) {
        auto* header = reinterpret_cast<NodeHeader*>(page.data());
        header->type = NodeType::INTERNAL;
        header->key_count = 0;
        header->parent = 0;
    }

    uint64_t* InternalNode::keys() {
        return reinterpret_cast<uint64_t*>(page_.data() + HEADER_SIZE);
    }

    uint32_t* InternalNode::children() {
        return reinterpret_cast<uint32_t*>(page_.data() + HEADER_SIZE + sizeof(uint64_t) * MAX_KEYS);
    }

    void InternalNode::set_left_child(uint32_t child) {
        children()[0] = child;
    }

    uint32_t InternalNode::find_child(uint64_t key) {
        for (uint32_t i = 0; i < header_->key_count; ++i) {
            if (key < keys()[i]) return children()[i];
        }

        return children()[header_->key_count];
    }

    bool InternalNode::insert(uint64_t key, uint32_t right_child) {
        if (header_->key_count >= MAX_KEYS) return false;

        uint32_t pos = 0;
        while (pos < header_->key_count && keys()[pos] < key) pos++;

        for (uint32_t i = header_->key_count; i > pos; --i) {
            keys()[i] = keys()[i - 1];
            children()[i + 1] = children()[i];
        }

        keys()[pos] = key;
        children()[pos + 1] = right_child;

        header_->key_count++;

        page_.mark_dirty();

        return true;
    }

    uint64_t InternalNode::split(storage::Page &new_page) {
        _init_(new_page);

        auto* new_header = reinterpret_cast<NodeHeader*>(new_page.data());
        auto* new_keys = reinterpret_cast<uint64_t*>(new_page.data() + HEADER_SIZE);
        auto* new_children = reinterpret_cast<uint32_t*>(new_page.data() + HEADER_SIZE + sizeof(uint64_t) * MAX_KEYS);

        uint32_t mid = header_->key_count / 2;
        uint64_t seperator = keys()[mid];

        new_children[0] = children()[mid + 1];

        for (uint32_t i = mid + 1; i < header_->key_count; ++i) {
            uint32_t index = new_header->key_count++;
            new_keys[index] = keys()[i];
            new_children[index + 1] = children()[i + 1];
        }

        header_->key_count = mid;

        page_.mark_dirty();
        new_page.mark_dirty();

        return seperator;
    }
}
