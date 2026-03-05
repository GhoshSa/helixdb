#include "helixdb/btree/bplushtree.hpp"
#include "helixdb/btree/node_layout.hpp"
#include "helixdb/btree/internal_node.hpp"
#include "helixdb/btree/leaf_node.hpp"

namespace helixdb::bplushtree {
    BPlushTree::BPlushTree(storage::Pager& pager) : pager_(pager) {}

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

        uint32_t new_page_id = pager_.allocate_page();
        auto& new_page = pager_.get_page(new_page_id);

        uint64_t seperator = leaf.split(new_page);

        insert_into_root(leaf_id, seperator, new_page_id);
    }

    void BPlushTree::insert_into_root(uint32_t left, uint64_t key, uint32_t right) {
        auto& left_page = pager_.get_page(left);
        NodeHeader* left_header = reinterpret_cast<NodeHeader*>(left_page.data());

        uint32_t parent_id = left_header->parent;

        if (parent_id == 0) {
            uint32_t root_id = pager_.allocate_page();
            auto& root_page = pager_.get_page(root_id);

            InternalNode root(root_page);
            root._init_();

            *reinterpret_cast<uint32_t*>(root_page.data() + sizeof(NodeHeader)) = left;

            root.insert(key, right);

            pager_.set_root_page(root_id);

            left_header->parent = root_id;

            auto& right_page = pager_.get_page(right);
            auto* right_header = reinterpret_cast<NodeHeader*>(right_page.data());

            right_header->parent = root_id;

            return;
        }

        auto& parent_page = pager_.get_page(parent_id);
        InternalNode parent(parent_page);

        if (parent.insert(key, right)) return;

        uint32_t new_page_id = pager_.allocate_page();
        auto& new_page = pager_.get_page(new_page_id);

        uint64_t seperator = parent.split(new_page);

        insert_into_root(parent_id, seperator, new_page_id);
    }
}