#pragma once

#include "storage/pager.hpp"
#include "btree/leaf_frame_mut.hpp"

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
            LeafFrameMut leaf(page);
            return leaf.insert(key, value);
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
    };
}