#pragma once

#include <cstring>

#include "btree/btree_layout.hpp"
#include "storage/page.hpp"

namespace helixdb::btree {
    /* LeafFrame Class */
    class LeafFrame {
    public:
        explicit LeafFrame (const storage::Page& page) : page_(page) {}

        /* Header accessors */
        NodeType node_type () const {
            return static_cast<NodeType>(page_.data()[NODE_TYPE_OFFSET]);
        }
        bool is_root () const {
            return static_cast<bool>(page_.data()[IS_ROOT_OFFSET]);
        }
        uint16_t cell_count () const {
            uint16_t value;
            std::memcpy(&value, page_.data() +  CELL_COUNT_OFFSET, sizeof(uint16_t));
            return value;
        }
        uint16_t free_space_offset () const {
            uint16_t value;
            std::memcpy(&value, page_.data() + FREE_SPACE_OFFSET, sizeof(uint16_t));
            return value;
        }
        uint32_t right_sibling_offset () const {
            uint32_t value;
            std::memcpy(&value, page_.data() + RIGHT_SIBLING_OFFSET, sizeof(uint32_t));
            return value;
        }

        /* Cell pointer access */
        uint16_t cell_offset (uint16_t index) const {
            const auto* ptrs = reinterpret_cast<const uint16_t*>(page_.data() + LEAF_CELL_PTR_ARRAY_OFFSET);
            return ptrs[index];
        }

        /* Cell content access */
        uint64_t key_at (uint16_t index) const {
            uint64_t key;
            std::memcpy(&key, page_.data() + cell_offset(index), sizeof(uint64_t));
            return key;
        }
        uint16_t value_size_at (uint16_t index) const {
            uint16_t value;
            std::memcpy(&value, page_.data() + cell_offset(index) + LEAF_KEY_SIZE, sizeof(uint16_t));
            return value;
        }

        /* Data access */
        const std::byte* value_at (uint16_t index) const {
            return page_.data() + cell_offset(index) + LEAF_CELL_HEADER_SIZE;
        }

        /* Binary search helper */
        std::optional<uint16_t> find_key (uint64_t key) const {
            uint16_t left = 0;
            uint16_t right = cell_count();
            while (left < right) {
                uint16_t mid = left + (right - left) / 2;
                uint64_t mid_key = key_at(mid);
                if (key == mid_key) return mid;
                else if (key < mid_key) right = mid;
                else left = mid + 1;
            }
            return std::nullopt;
        }
    private:
        const storage::Page& page_;
    };
}