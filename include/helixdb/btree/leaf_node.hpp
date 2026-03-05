#pragma once

#include <cstring>

#include "helixdb/storage/page.hpp"
#include "helixdb/btree/node_layout.hpp"

namespace helixdb::bplushtree {
    constexpr uint32_t VALUE_SIZE = 64;

    struct LeafEntry {
        uint64_t key;
        std::byte value[VALUE_SIZE];
    };

    class LeafNode {
    public:
        explicit LeafNode(storage::Page& page) : page_(page) {}

        void _init_() {
            auto h = header();
            h->type = NodeType::LEAF;
            h->key_count = 0;
            h->parent = 0;

            *next_leaf() = 0;

            page_.mark_dirty();
        }

        uint16_t key_count() const {
            return header()->key_count;
        }

        uint32_t capacity() const {
            return (storage::PAGE_SIZE - sizeof(NodeHeader) - sizeof(uint32_t)) / sizeof(LeafEntry);
        }

        bool find(uint64_t key, std::byte* out) {
            auto* e = entries();
            
            for (uint16_t i = 0; i < key_count(); ++i) {
                if (e[i].key == key) {
                    std::memcpy(out, e[i].value, VALUE_SIZE);
                    return true;
                }
            }

            return false;
        }

        bool insert(uint64_t key, const std::byte* value) {
            auto* e = entries();

            if (key_count() >= capacity()) return false;

            int pos = 0;
            while (pos < key_count() && e[pos].key < key) pos++;

            for (int i = key_count(); i > pos; --i) e[i] = e[i - 1];

            e[pos].key = key;
            std::memcpy(e[pos].value, value, VALUE_SIZE);

            header()->key_count++;

            page_.mark_dirty();

            return true;
        }

        uint64_t split(storage::Page& new_page) {
            LeafNode right(new_page);
            right._init_();

            auto* left_entries = entries();
            auto* right_entries = right.entries();

            uint16_t total = key_count();
            uint16_t mid = total / 2;

            for (uint16_t i = mid; i < total; ++i) right_entries[i - mid] = left_entries[i];

            right.header()->key_count = total - mid;
            header()->key_count = mid;

            *right.next_leaf() = *next_leaf();
            *next_leaf() = new_page.id();

            page_.mark_dirty();
            new_page.mark_dirty();

            return right_entries[0].key;
        }
    
    private:
        storage::Page& page_;

        NodeHeader* header () {
            return reinterpret_cast<NodeHeader*>(page_.data());
        }

        NodeHeader* header () const {
            return reinterpret_cast<NodeHeader*>(page_.data());
        }

        uint32_t* next_leaf() {
            return reinterpret_cast<uint32_t*>(page_.data() + sizeof(NodeHeader));
        }

        LeafEntry* entries() {
            return reinterpret_cast<LeafEntry*>(page_.data() + sizeof(NodeHeader) + sizeof(uint32_t));
        }
    };
}