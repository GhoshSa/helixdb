#include "helixdb/btree/bplushtree.hpp"

namespace helixdb::bplushtree {
    BPlushTree::BPlushTree(storage::Pager& pager) : pager_(pager) {
        if (pager_.root_page_id() == 0) {
            uint32_t root = pager_.allocate_page();
            init_leaf(root);
            pager_.set_root_page(root);
        }
    }

    uint32_t BPlushTree::leaf_max_keys() const {
        uint32_t header = sizeof(NodeHeader) + sizeof(uint32_t);
        uint32_t entry = sizeof(uint64_t) + sizeof(uint32_t);
        return (storage::PAGE_SIZE - header) / entry;
    }

    uint32_t BPlushTree::internal_max_keys() const {
        uint32_t header = sizeof(NodeHeader);
        uint32_t entry = sizeof(key_t) + sizeof(uint32_t);
        return (storage::PAGE_SIZE - header) / entry - 1;
    }

    void BPlushTree::init_leaf(uint32_t pageid) {
        auto& page = pager_.get_page(pageid);

        auto* hdr = reinterpret_cast<NodeHeader*>(page.data());
        hdr->type = NodeType::LEAF;
        hdr->num_keys = 0;
        hdr->parent = UINT32_MAX;

        uint32_t* next = reinterpret_cast<uint32_t*>(hdr + 1);
        *next = 0;

        pager_.mark_page_dirty(pageid);
    }

    void BPlushTree::init_internal(uint32_t pageid) {
        auto& page = pager_.get_page(pageid);

        auto* hdr = reinterpret_cast<NodeHeader*>(page.data());
        hdr->type = NodeType::INTERNAL;
        hdr->num_keys = 0;
        hdr->parent = UINT32_MAX;

        pager_.mark_page_dirty(pageid);
    }

    Cursor BPlushTree::begin() {
        uint32_t pageid = pager_.root_page_id();

        while (true) {
            auto& page = pager_.get_page(pageid);
            auto* hdr = reinterpret_cast<NodeHeader*>(page.data());

            if (hdr->type == NodeType::LEAF) break;

            uint64_t* keys = reinterpret_cast<uint64_t*>(hdr + 1);
            uint32_t* children = reinterpret_cast<uint32_t*>(keys + internal_max_keys());

            pageid = children[0];
        }

        auto& page = pager_.get_page(pageid);
        auto* hdr = reinterpret_cast<NodeHeader*>(page.data());

        if (hdr->num_keys == 0) return Cursor(pager_, pageid, 0);

        return Cursor(pager_, pageid, 0);
    }

    Cursor BPlushTree::lower_bound(uint64_t key) {
        uint32_t pid = find_leaf(key);

        auto& page = pager_.get_page(pid);
        auto* hdr = reinterpret_cast<NodeHeader*>(page.data());

        uint32_t* next = reinterpret_cast<uint32_t*>(hdr + 1);
        uint64_t* keys = reinterpret_cast<uint64_t*>(next + 1);

        int i = 0;
        while (i < hdr->num_keys && keys[i] < key) i++;

        if (i >= hdr->num_keys) {
            if (*next == 0) return Cursor(pager_, pid, hdr->num_keys);
            return Cursor(pager_, *next, 0);
        }

        return Cursor(pager_, pid, i);
    }

    bool BPlushTree::find(uint64_t key, uint32_t& out) {
        uint32_t leaf = find_leaf(key);

        auto& page = pager_.get_page(leaf);
        auto* hdr = reinterpret_cast<NodeHeader*>(page.data());

        uint32_t* next = reinterpret_cast<uint32_t*>(hdr + 1);
        uint64_t* keys = reinterpret_cast<uint64_t*>(next + 1);
        uint32_t* values = reinterpret_cast<uint32_t*>(keys + leaf_max_keys());

        for (int i = 0; i < hdr->num_keys; i++) {
            if (keys[i] == key) {
                out = values[i];
                return true;
            }
        }
        return false;
    }

    uint32_t BPlushTree::find_leaf(uint64_t key) {
        uint32_t pageid = pager_.root_page_id();

        while (true) {
            auto& page = pager_.get_page(pageid);
            auto* hdr = reinterpret_cast<NodeHeader*>(page.data());

            if (hdr->type == NodeType::LEAF) return pageid;

            key_t* keys = reinterpret_cast<key_t*>(hdr + 1);
            uint32_t* children = reinterpret_cast<uint32_t*>(keys + internal_max_keys());

            uint32_t i = 0;
            while (i < hdr->num_keys && key >= keys[i]) i++;

            pageid = children[i];
        }
    }

    void BPlushTree::insert(uint64_t key, uint32_t value) {
        uint32_t leaf = find_leaf(key);
        insert_into_leaf(leaf, key, value);
    }

    void BPlushTree::insert_into_leaf(uint32_t pageid, uint64_t key, uint32_t value) {
        auto& page = pager_.get_page(pageid);
        auto* hdr = reinterpret_cast<NodeHeader*>(page.data());

        uint32_t* next = reinterpret_cast<uint32_t*>(hdr + 1);
        uint64_t* keys = reinterpret_cast<uint64_t*>(next + 1);
        uint32_t* values = reinterpret_cast<uint32_t*>(keys + leaf_max_keys());

        for (uint16_t j = 0; j < hdr->num_keys; ++j) {
            if (keys[j] == key) {
                values[j] = value;
                pager_.mark_page_dirty(pageid);
                return;
            }
        }

        int i = hdr->num_keys - 1;
        while (i >= 0 && keys[i] > key) {
            keys[i + 1] = keys[i];
            values[i + 1] = values[i];
            i--;
        }

        keys[i + 1] = key;
        values[i + 1] = value;
        hdr->num_keys++;

        pager_.mark_page_dirty(pageid);

        if (hdr->num_keys > leaf_max_keys()) {
            split_leaf(pageid);
        }
    }

    void BPlushTree::split_leaf(uint32_t leaf_id) {
        auto& old_page = pager_.get_page(leaf_id);
        auto* old_hdr = reinterpret_cast<NodeHeader*>(old_page.data());

        uint32_t new_id = pager_.allocate_page();
        init_leaf(new_id);
        
        auto& new_page = pager_.get_page(new_id);
        auto* new_hdr = reinterpret_cast<NodeHeader*>(new_page.data());

        uint32_t* old_next = reinterpret_cast<uint32_t*>(old_hdr + 1);
        uint64_t* old_keys = reinterpret_cast<uint64_t*>(old_next + 1);
        uint32_t* old_vals = reinterpret_cast<uint32_t*>(old_keys + leaf_max_keys());

        uint32_t* new_next = reinterpret_cast<uint32_t*>(new_hdr + 1);
        uint64_t* new_keys = reinterpret_cast<uint64_t*>(new_next + 1);
        uint32_t* new_vals = reinterpret_cast<uint32_t*>(new_keys + leaf_max_keys());

        // Redistribute keys
        uint32_t mid = old_hdr->num_keys / 2;
        uint32_t j = 0;
        for (uint32_t i = mid; i < old_hdr->num_keys; i++) {
            new_keys[j] = old_keys[i];
            new_vals[j] = old_vals[i];
            new_hdr->num_keys++;
            j++;
        }
        old_hdr->num_keys = mid;

        // Link siblings
        *new_next = *old_next;
        *old_next = new_id;
        new_hdr->parent = old_hdr->parent;

        pager_.mark_page_dirty(leaf_id);
        pager_.mark_page_dirty(new_id);

        uint64_t split_key = new_keys[0];
        insert_into_parent(leaf_id, split_key, new_id);
    }

    void BPlushTree::insert_into_parent(uint32_t left, uint64_t key, uint32_t right) {
        auto& left_page = pager_.get_page(left);
        auto* left_hdr = reinterpret_cast<NodeHeader*>(left_page.data());

        if (left_hdr->parent == UINT32_MAX) {
            uint32_t root = pager_.allocate_page();
            init_internal(root);

            auto& root_page = pager_.get_page(root);
            auto* root_hdr = reinterpret_cast<NodeHeader*>(root_page.data());

            uint64_t* keys = reinterpret_cast<uint64_t*>(root_hdr + 1);
            uint32_t* children = reinterpret_cast<uint32_t*>(keys + internal_max_keys());

            keys[0] = key;
            children[0] = left;
            children[1] = right;

            left_hdr->parent = root;
            auto& right_page = pager_.get_page(right);
            auto* right_hdr = reinterpret_cast<NodeHeader*>(right_page.data());
            right_hdr->parent = root;

            pager_.mark_page_dirty(root);
            pager_.mark_page_dirty(left);
            pager_.mark_page_dirty(right);
            pager_.set_root_page(root);
            return;
        }

        uint32_t parent_id = left_hdr->parent;
        auto& parent_page = pager_.get_page(parent_id);
        auto* parent_hdr = reinterpret_cast<NodeHeader*>(parent_page.data());

        uint64_t* p_keys = reinterpret_cast<uint64_t*>(parent_hdr + 1);
        uint32_t* p_children = reinterpret_cast<uint32_t*>(p_keys + internal_max_keys());

        int i = parent_hdr->num_keys - 1;
        while (i >= 0 && p_keys[i] > key) {
            p_keys[i + 1] = p_keys[i];
            p_children[i + 2] = p_children[i + 1];
            i--;
        }

        p_keys[i + 1] = key;
        p_children[i + 2] = right;
        parent_hdr->num_keys++;

        auto& right_page = pager_.get_page(right);
        auto* right_hdr = reinterpret_cast<NodeHeader*>(right_page.data());
        right_hdr->parent = parent_id;

        pager_.mark_page_dirty(parent_id);
        pager_.mark_page_dirty(right);

        if (parent_hdr->num_keys > internal_max_keys()) {
            split_internal(parent_id);
        }
    }

    void BPlushTree::split_internal(uint32_t pageid) {
        auto& page = pager_.get_page(pageid);
        auto* hdr = reinterpret_cast<NodeHeader*>(page.data());

        uint32_t new_id = pager_.allocate_page();
        init_internal(new_id);

        auto& new_page = pager_.get_page(new_id);
        auto* new_hdr = reinterpret_cast<NodeHeader*>(new_page.data());

        uint64_t* keys = reinterpret_cast<uint64_t*>(hdr + 1);
        uint32_t* children = reinterpret_cast<uint32_t*>(keys + internal_max_keys());

        uint64_t* new_keys = reinterpret_cast<uint64_t*>(new_hdr + 1);
        uint32_t* new_children = reinterpret_cast<uint32_t*>(new_keys + internal_max_keys());

        uint32_t mid = hdr->num_keys / 2;
        uint64_t up_key = keys[mid];

        // Move keys and children to new node
        uint32_t j = 0;
        for (uint32_t i = mid + 1; i < hdr->num_keys; i++) {
            new_keys[j] = keys[i];
            new_children[j] = children[i];
            new_hdr->num_keys++;
            
            // Update parent pointer of moved children
            auto& child_p = pager_.get_page(children[i]);
            auto* child_h = reinterpret_cast<NodeHeader*>(child_p.data());
            child_h->parent = new_id;
            pager_.mark_page_dirty(children[i]);
            j++;
        }
        // Move the last child pointer
        new_children[j] = children[hdr->num_keys];
        auto& last_child_p = pager_.get_page(children[hdr->num_keys]);
        auto* last_child_h = reinterpret_cast<NodeHeader*>(last_child_p.data());
        last_child_h->parent = new_id;
        pager_.mark_page_dirty(children[hdr->num_keys]);

        hdr->num_keys = mid;
        new_hdr->parent = hdr->parent;

        pager_.mark_page_dirty(pageid);
        pager_.mark_page_dirty(new_id);

        insert_into_parent(pageid, up_key, new_id);
    }
}