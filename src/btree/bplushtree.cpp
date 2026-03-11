#include "helixdb/btree/bplushtree.hpp"
#include "helixdb/btree/node_layout.hpp"
#include "helixdb/btree/internal_node.hpp"
#include "helixdb/btree/leaf_node.hpp"

namespace helixdb::bplushtree {
    BPlushTree::BPlushTree(storage::Pager& pager, uint32_t root_page_id) : pager_(pager), root_page_id_(root_page_id) {
        if (root_page_id_ == NULL_PAGE) {
            root_page_id_ = pager_.allocate_page();
            auto& page = pager_.get_page(root_page_id_);
            LeafNode::_init_(page);
            page.mark_dirty();
        }
    }

    uint32_t BPlushTree::find_leaf(uint64_t key) {
        uint32_t page_id = root_page_id_;

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

        uint32_t target_id = (key >= seperator) ? new_page : leaf_id;
        auto& target_page = pager_.get_page(target_id);
        LeafNode target(target_page);
        target.insert(key, value);
    }

    void BPlushTree::insert_into_root(uint32_t left, uint64_t key, uint32_t right) {
        auto& left_page = pager_.get_page(left);
        NodeHeader* left_header = reinterpret_cast<NodeHeader*>(left_page.data());

        if (left_header->parent == NULL_PAGE) {
            uint32_t root_id = pager_.allocate_page();
            auto& root_page = pager_.get_page(root_id);

            InternalNode::_init_(root_page);
            InternalNode root(root_page);
            root.set_left_child(left);
            root.insert(key, right);

            reinterpret_cast<NodeHeader*>(pager_.get_page(left).data())->parent = root_id;
            reinterpret_cast<NodeHeader*>(pager_.get_page(right).data())->parent = root_id;
            
            root_page.mark_dirty();
            root_page_id_ = root_id;

            return;
        }

        uint32_t parent_id = left_header->parent;
        auto& parent_page = pager_.get_page(parent_id);
        InternalNode parent(parent_page);

        if (parent.insert(key, right)) return;

        uint32_t new_internal_id = pager_.allocate_page();
        auto& new_internal = pager_.get_page(new_internal_id);

        uint64_t seperator = parent.split(new_internal);

        auto* new_header = reinterpret_cast<NodeHeader*>(new_internal.data());
        auto* new_children = reinterpret_cast<uint32_t*>(new_internal.data() + sizeof(NodeHeader) + sizeof(uint64_t) * InternalNode::MAX_KEYS);

        for (uint32_t i = 0; i <= new_header->key_count; ++i) {
            uint32_t child_id = new_children[i];
            if (child_id == NULL_PAGE) continue;
            reinterpret_cast<NodeHeader*>(pager_.get_page(child_id).data())->parent = new_internal_id;
        }

        new_internal.mark_dirty();
        parent_page.mark_dirty();

        insert_into_root(parent_id, seperator, new_internal_id);
    }
}