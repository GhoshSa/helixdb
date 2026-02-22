#pragma once

#include <span>
#include <optional>

#include "storage/page.hpp"
#include "btree/leaf_frame.hpp"

namespace helixdb::btree {
    /* LeafFrameMut Class
       Mutable MVP view over B-Tree leaf page
       For now no splitting, no balancing */
    class LeafFrameMut {
    public:
        explicit LeafFrameMut (storage::Page& page) : page_(page) {}

        /* Initialization */
        void init (bool is_root = true) {
            auto* data = page_.data();
            data[NODE_TYPE_OFFSET] = static_cast<std::byte>(NodeType::LEAF);
            data[IS_ROOT_OFFSET] = static_cast<std::byte>(is_root);
            uint16_t zero16 = 0;
            uint32_t zero32 = 0;
            uint16_t free_space = static_cast<uint16_t>(storage::PAGE_SIZE);
            std::memcpy(data + CELL_COUNT_OFFSET, &zero16, sizeof(uint16_t));
            std::memcpy(data + FREE_SPACE_OFFSET, &free_space, sizeof(uint16_t));
            std::memcpy(data + RIGHT_SIBLING_OFFSET, &zero32, sizeof(uint32_t));
            page_.mark_dirty();
        }

        /* Find key using binary search */
        bool find (uint64_t key, std::span<const std::byte>& out_values) {
            LeafFrame view(page_);
            auto index = view.find_key(key);
            if (!index.has_value()) return false;
            out_values = {
                view.value_at(*index),
                view.value_size_at(*index)
            };
            return true;
        }

        /* Insert key-value */
        bool insert (uint64_t key, std::span<const std::byte> value) {
            LeafFrame view(page_);
            uint16_t count = view.cell_count();

            // Reject duplicates
            for (uint16_t i = 0; i < count; ++i) {
                if (view.key_at(i) == key) return false;
            }
            
            uint16_t value_size = static_cast<uint16_t>(value.size());
            uint16_t cell_size = LEAF_CELL_HEADER_SIZE + value_size;
            uint16_t free_space = view.free_space_offset();
            uint16_t next_ptr_space_end = LEAF_CELL_PTR_ARRAY_OFFSET + (count + 1) * LEAF_CELL_PTR_SIZE;

            /* Check space
               If page full, no splitting for now */
            if (free_space < next_ptr_space_end + cell_size) return false;

            uint16_t new_cell_offset = free_space - cell_size;
            auto* data = page_.data();
            std::memcpy(data + new_cell_offset, &key, sizeof(uint64_t)); // Write key
            std::memcpy(data + new_cell_offset + LEAF_KEY_SIZE, &value_size, sizeof(uint16_t)); // Write value size
            std::memcpy(data + new_cell_offset + LEAF_CELL_HEADER_SIZE, value.data(), value.size()); // Write value bytes

            // Insert the pointer
            auto* ptrs = reinterpret_cast<uint16_t*>(data + LEAF_CELL_PTR_ARRAY_OFFSET);
            uint16_t insert_pos = count;
            for (uint16_t i = 0; i < count; ++i) {
                if (key < view.key_at(i)) {
                    insert_pos = i;
                    break;
                }
            }
            for (uint16_t i = count; i > insert_pos; --i) ptrs[i] = ptrs[i - 1];
            ptrs[insert_pos] = new_cell_offset;

            // Update the header
            uint16_t new_count = count + 1;
            std::memcpy(data + CELL_COUNT_OFFSET, &new_count, sizeof(uint16_t));
            std::memcpy(data + FREE_SPACE_OFFSET, &new_cell_offset, sizeof(uint16_t));
            page_.mark_dirty();
            return true;
        }
    private:
        storage::Page& page_;
    };
}