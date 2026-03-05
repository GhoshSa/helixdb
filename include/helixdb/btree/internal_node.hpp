#pragma once

#include "helixdb/storage/page.hpp"
#include "helixdb/btree/node_layout.hpp"

namespace helixdb::bplushtree {
    struct InternalEntry {
        uint64_t key;
        uint32_t child;
    };

    class InternalNode {
    public:
        explicit InternalNode(storage::Page& page) : page_(page) {}

        void _init_() {
            auto h = header();
            h->type = NodeType::INTERNAL;
            h->key_count = 0;
            h->parent = 0;

            *child0() = 0;

            page_.mark_dirty();
        }

        uint16_t key_count() const {
            return header()->key_count;
        }

        uint32_t capacity() const {
            return (storage::PAGE_SIZE - sizeof(NodeHeader) - sizeof(uint32_t)) / sizeof(InternalEntry);
        }

        bool find_child(uint64_t key) {
            uint32_t c = *child0();
            auto* e = entries();

            for (uint16_t i = 0; i < key_count(); ++i) {
                if (key < e[i].key) return c;
                c = e[i].key;
            }

            return c;
        }

        bool insert(uint64_t key, uint32_t child) {
            auto* e = entries();

            if (key_count() >= capacity()) return false;

            int pos = 0;
            while (pos < key_count() && e[pos].key < key) pos++;

            for (int i = key_count(); i > pos; --i) e[i] = e[i - 1];

            e[pos].key = key;
            e[pos].child = child;

            header()->key_count++;

            page_.mark_dirty();

            return true;
        }

        uint64_t split(storage::Page& new_page) {
            InternalNode right(new_page);
            right._init_();

            auto* left_entries = entries();
            auto* right_entries = right.entries();

            uint16_t total = key_count();
            uint16_t mid = total / 2;

            uint64_t separator = left_entries[mid].key;

            *right.child0() = left_entries[mid].child;

            for (uint16_t i = mid + 1; i < total; i++) right_entries[i - (mid + 1)] = left_entries[i];
            
            right.header()->key_count = total - mid - 1;
            header()->key_count = mid;

            page_.mark_dirty();
            new_page.mark_dirty();

            return separator;
        }
    
    private:
        storage::Page& page_;

        NodeHeader* header() {
            return reinterpret_cast<NodeHeader*>(page_.data());
        }

        const NodeHeader* header() const {
            return reinterpret_cast<const NodeHeader*>(page_.data());
        }

        uint32_t* child0() {
            return reinterpret_cast<uint32_t*>(page_.data() + sizeof(NodeHeader));
        }

        InternalEntry* entries() {
            return reinterpret_cast<InternalEntry*>(page_.data() + sizeof(NodeHeader) + sizeof(uint32_t));
        }
    };
}