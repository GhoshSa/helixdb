#include "helixdb/btree/bplushtree.hpp"
#include "helixdb/btree/node_layout.hpp"
#include "helixdb/btree/internal_node.hpp"
#include "helixdb/btree/leaf_node.hpp"

namespace helixdb::bplushtree {
    BPlushTree::BPlushTree(storage::Pager& pager) : pager_(pager) {
        if (pager_.root_page_id() == 0) {
            uint32_t root = pager_.allocate_page();
            auto& page = pager_.get_page(root);
            LeafNode::_init_(page);
            pager_.set_root_page(root);
        }
    }

    uint32_t BPlushTree::find_leaf(uint64_t key) {
        uint32_t page_id = pager_.root_page_id();

        while (true) {
            auto& page = pager_.get_page(page_id);
            auto* header = reinterpret_cast<NodeHeader*>(page.data());

            if (header->type == NodeType::LEAF) return page_id;

            InternalNode node(page);
            page_id = node.find_child(key);
        }
    }

    bool BPlushTree::find(uint64_t key, std::byte* out) {
        uint32_t leaf_id = find_leaf(key);

        auto& page = pager_.get_page(leaf_id);
        LeafNode leaf(page);

        return leaf.find(key, out);
    }

    void BPlushTree::insert(uint64_t key, const std::byte* value) {

        uint32_t leaf_id = find_leaf(key);

        auto& page = pager_.get_page(leaf_id);
        LeafNode leaf(page);

        if (leaf.insert(key, value)) return;

        uint32_t new_page = pager_.allocate_page();
        auto& new_leaf = pager_.get_page(new_page);

        uint64_t seperator = leaf.split(new_leaf);

        insert_into_root(leaf_id, seperator, new_page);
    }

    void BPlushTree::insert_into_root(uint32_t left, uint64_t key, uint32_t right) {
        auto& left_page = pager_.get_page(left);
        NodeHeader* left_header = reinterpret_cast<NodeHeader*>(left_page.data());

        if (left_header->parent == 0) {
            uint32_t root_id = pager_.allocate_page();
            auto& root_page = pager_.get_page(root_id);

            InternalNode::_init_(root_page);
            InternalNode node(root_page);

            node.set_left_child(left);
            node.insert(key, right);

            auto* right_header = reinterpret_cast<NodeHeader*>(pager_.get_page((right)).data());
            left_header->parent = root_id;
            right_header->parent = root_id;

            pager_.set_root_page(root_id);

            return;
        }

        auto& parent_page = pager_.get_page(left_header->parent);
        InternalNode parent(parent_page);

        if (parent.insert(key, right)) return;

        uint32_t new_page_id = pager_.allocate_page();
        auto& new_internal = pager_.get_page(new_page_id);

        uint64_t seperator = parent.split(new_internal);

        insert_into_root(left_header->parent, seperator, new_page_id);
    }
}