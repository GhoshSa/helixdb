#pragma once

#include <iostream>
#include <vector>
#include <algorithm>

#include "storage/pager.hpp"
#include "btree/leaf_frame_mut.hpp"
#include "btree/internal_frame_mut.hpp"

namespace helixdb::btree {
    /* BTreeMVP
       Currently single leaf 
       insert and find
       persistance via Pager */
    class BTreeMVP {
    public:
        explicit BTreeMVP (storage::Pager& pager) : pager_(pager) {
            ensure_root_intialized();
        }

        /* Insert key-value */
        bool insert (uint64_t key, std::span<const std::byte> value) {
            storage::Page& page = pager_.get_page(0);
            LeafFrame view(page);
            if (view.node_type() == NodeType::LEAF) {
                LeafFrameMut leaf(page);
                if (leaf.insert(key, value)) {
                    return true;
                }
                split(key, value);
                return true;
            }
            // For now no deeper tree
            return false;
        }

        /* Find key */
        bool find (uint64_t key, std::span<const std::byte>& out_value) {
            storage::Page& page = pager_.get_page(0);
            LeafFrameMut leaf(page);
            return leaf.find(key, out_value);
        }
    
    private:
        /* Initialization */
        storage::Pager& pager_;

        void ensure_root_intialized () {
            storage::Page& page = pager_.get_page(0);
            
            /* If page is empty, intialize */
            LeafFrame view(page);
            if (view.cell_count() == 0 && view.node_type() != NodeType::LEAF) {
                LeafFrameMut mut(page);
                mut.init(true);
            }
        }

        /* Split Node */
        void split (uint64_t key, std::span<const std::byte> value) {
            storage::Page& old_page = pager_.get_page(0);
            LeafFrame old_leaf(old_page);

            uint32_t left_page_id = pager_.allocate_page();
            uint32_t right_page_id = pager_.allocate_page();

            storage::Page& left_page = pager_.get_page(left_page_id);
            storage::Page& right_page = pager_.get_page(right_page_id);

            LeafFrameMut left_leaf(left_page);
            left_leaf.init(false);
            LeafFrameMut right_leaf(right_page);
            right_leaf.init(false);

            // All cells
            struct Cell {
                uint64_t key;
                std::vector<std::byte> value;
            };
            std::vector<Cell> cells;
            uint16_t count = old_leaf.cell_count();

            for (uint16_t i = 0; i < count; ++i) {
                auto span = std::span(
                    old_leaf.value_at(i),
                    old_leaf.value_size_at(i)
                );

                cells.push_back({
                    old_leaf.key_at(i),
                    std::vector<std::byte>(span.begin(), span.end())
                });
            }

            // New cell
            cells.push_back({
                key,
                std::vector<std::byte>(value.begin(), value.end())
            });

            std::sort(cells.begin(), cells.end(), [](const Cell& a, const Cell& b) { return a.key < b.key; });

            // Split
            size_t mid = cells.size() / 2;
            uint64_t seperator = cells[mid].key;

            // Left
            for (size_t i = 0; i < mid; ++i) {
                left_leaf.insert(
                    cells[i].key,
                    std::span(cells[i].value.data(), cells[i].value.size())
                );
            }

            //Right
            for (size_t i = mid; i < cells.size(); ++i) {
                right_leaf.insert(
                    cells[i].key,
                    std::span(cells[i].value.data(), cells[i].value.size())
                );
            }

            // Internal root node
            InternalFrameMut root(old_page);
            root.init(left_page_id, seperator, right_page_id);
        }
    };
}