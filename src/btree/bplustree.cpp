#include <utility>

#include "helixdb/btree/bplustree.hpp"

namespace helixdb::bplustree {
    BPlusTree::BPlusTree(storage::Pager& pager) : pager_(&pager) {
        if (pager_->rootPageId() == 0) {
            uint32_t root = pager_->allocatePage();
            initLeaf(root);
            pager_->setRootPage(root);
        }
    }

    auto BPlusTree::leafMaxKeys() -> uint32_t {
        uint32_t header = sizeof(NodeHeader) + sizeof(uint32_t);
        uint32_t entry = sizeof(uint64_t) + sizeof(uint32_t);
        return (storage::PAGE_SIZE - header) / entry;
    }

    auto BPlusTree::internalMaxKeys() -> uint32_t {
        uint32_t header = sizeof(NodeHeader);
        uint32_t entry = sizeof(uint64_t) + sizeof(uint32_t);
        return ((storage::PAGE_SIZE - header) / entry) - 1;
    }

    void BPlusTree::initLeaf(uint32_t pageid) {
        auto& page = pager_->getPage(pageid);

        auto* hdr = reinterpret_cast<NodeHeader*>(page.data());
        hdr->type_ = NodeType::LEAF;
        hdr->num_keys_ = 0;
        hdr->parent_ = UINT32_MAX;

        auto* next = reinterpret_cast<uint32_t*>(hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *next = 0;

        pager_->markPageDirty(pageid);
    }

    void BPlusTree::initInternal(uint32_t pageid) {
        auto& page = pager_->getPage(pageid);

        auto* hdr = reinterpret_cast<NodeHeader*>(page.data());
        hdr->type_ = NodeType::INTERNAL;
        hdr->num_keys_ = 0;
        hdr->parent_ = UINT32_MAX;

        pager_->markPageDirty(pageid);
    }

    auto BPlusTree::begin() -> Cursor {
        uint32_t pageid = pager_->rootPageId();

        while (true) {
            auto& page = pager_->getPage(pageid);
            auto* hdr = reinterpret_cast<NodeHeader*>(page.data());

            if (hdr->type_ == NodeType::LEAF) {
                break;
            }

            auto* keys = reinterpret_cast<uint64_t*>(hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            auto* children = reinterpret_cast<uint32_t*>(keys + internalMaxKeys()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

            pageid = children[0]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        auto& page = pager_->getPage(pageid);
        auto* hdr = reinterpret_cast<NodeHeader*>(page.data());

        if (hdr->num_keys_ == 0) {
            return {*pager_, pageid, 0};
        }

        return {*pager_, pageid, 0};
    }

    auto BPlusTree::lowerBound(uint64_t key) -> Cursor {
        uint32_t pid = findLeaf(key);

        auto& page = pager_->getPage(pid);
        auto* hdr = reinterpret_cast<NodeHeader*>(page.data());

        auto* next = reinterpret_cast<uint32_t*>(hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* keys = reinterpret_cast<uint64_t*>(next + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        int idx = 0;
        while (std::cmp_less(idx , hdr->num_keys_) && keys[idx] < key) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            idx++;
        }

        if (std::cmp_greater_equal(idx , hdr->num_keys_)) {
            if (*next == 0) {
                return {*pager_, pid, hdr->num_keys_};
            }
            return {*pager_, *next, 0};
        }

        return {*pager_, pid, static_cast<uint32_t>(idx)};
    }

    auto BPlusTree::find(uint64_t key, uint32_t& out) -> bool {
        uint32_t leaf = findLeaf(key);

        auto& page = pager_->getPage(leaf);
        auto* hdr = reinterpret_cast<NodeHeader*>(page.data());

        auto* next = reinterpret_cast<uint32_t*>(hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* keys = reinterpret_cast<uint64_t*>(next + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* values = reinterpret_cast<uint32_t*>(keys + leafMaxKeys()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        for (int idx = 0; std::cmp_less(idx , hdr->num_keys_); idx++) {
            if (keys[idx] == key) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                out = values[idx]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return true;
            }
        }
        return false;
    }

    auto BPlusTree::findLeaf(uint64_t key) -> uint32_t {
        uint32_t pageid = pager_->rootPageId();

        while (true) {
            auto& page = pager_->getPage(pageid);
            auto* hdr = reinterpret_cast<NodeHeader*>(page.data());

            if (hdr->type_ == NodeType::LEAF) {
                return pageid;
            }

            auto* keys = reinterpret_cast<uint64_t*>(hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            auto* children = reinterpret_cast<uint32_t*>(keys + internalMaxKeys()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

            uint32_t idx = 0;
            while (idx < hdr->num_keys_ && std::cmp_greater_equal(key , keys[idx])) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                idx++;
            }

            pageid = children[idx]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
    }

    void BPlusTree::insert(uint64_t key, uint32_t value) {
        uint32_t leaf = findLeaf(key);
        insertIntoLeaf(leaf, key, value);
    }

    void BPlusTree::insertIntoLeaf(uint32_t pageid, uint64_t key, uint32_t value) { // NOLINT(bugprone-easily-swappable-parameters)
        auto& page = pager_->getPage(pageid);
        auto* hdr = reinterpret_cast<NodeHeader*>(page.data());

        auto* next = reinterpret_cast<uint32_t*>(hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* keys = reinterpret_cast<uint64_t*>(next + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* values = reinterpret_cast<uint32_t*>(keys + leafMaxKeys()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        for (uint16_t j = 0; j < hdr->num_keys_; ++j) {
            if (keys[j] == key) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                values[j] = value; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                pager_->markPageDirty(pageid);
                return;
            }
        }

        int idx = hdr->num_keys_ - 1;
        while (idx >= 0 && keys[idx] > key) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            keys[idx + 1] = keys[idx]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            values[idx + 1] = values[idx]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            idx--;
        }

        keys[idx + 1] = key; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        values[idx + 1] = value; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        hdr->num_keys_++;

        pager_->markPageDirty(pageid);

        if (hdr->num_keys_ > leafMaxKeys()) {
            splitLeaf(pageid);
        }
    }

    void BPlusTree::splitLeaf(uint32_t leaf_id) {
        auto& old_page = pager_->getPage(leaf_id);
        auto* old_hdr = reinterpret_cast<NodeHeader*>(old_page.data());

        uint32_t new_id = pager_->allocatePage();
        initLeaf(new_id);
        
        auto& new_page = pager_->getPage(new_id);
        auto* new_hdr = reinterpret_cast<NodeHeader*>(new_page.data());

        auto* old_next = reinterpret_cast<uint32_t*>(old_hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* old_keys = reinterpret_cast<uint64_t*>(old_next + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* old_vals = reinterpret_cast<uint32_t*>(old_keys + leafMaxKeys()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        auto* new_next = reinterpret_cast<uint32_t*>(new_hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* new_keys = reinterpret_cast<uint64_t*>(new_next + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* new_vals = reinterpret_cast<uint32_t*>(new_keys + leafMaxKeys()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        uint32_t mid = old_hdr->num_keys_ / 2;
        uint32_t iter = 0;
        for (uint32_t i = mid; i < old_hdr->num_keys_; i++) {
            new_keys[iter] = old_keys[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            new_vals[iter] = old_vals[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            new_hdr->num_keys_++;
            iter++;
        }
        old_hdr->num_keys_ = mid;

        *new_next = *old_next;
        *old_next = new_id;
        new_hdr->parent_ = old_hdr->parent_;

        pager_->markPageDirty(leaf_id);
        pager_->markPageDirty(new_id);

        uint64_t split_key = new_keys[0]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        insertIntoParent(leaf_id, split_key, new_id);
    }

    void BPlusTree::insertIntoParent(uint32_t left, uint64_t key, uint32_t right) { // NOLINT(bugprone-easily-swappable-parameters)
        auto& left_page = pager_->getPage(left);
        auto* left_hdr = reinterpret_cast<NodeHeader*>(left_page.data());

        if (left_hdr->parent_ == UINT32_MAX) {
            uint32_t root = pager_->allocatePage();
            initInternal(root);

            auto& root_page = pager_->getPage(root);
            auto* root_hdr = reinterpret_cast<NodeHeader*>(root_page.data());

            auto* keys = reinterpret_cast<uint64_t*>(root_hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            auto* children = reinterpret_cast<uint32_t*>(keys + internalMaxKeys()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

            keys[0] = key; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            children[0] = left; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            children[1] = right; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

            left_hdr->parent_ = root;
            auto& right_page = pager_->getPage(right);
            auto* right_hdr = reinterpret_cast<NodeHeader*>(right_page.data());
            right_hdr->parent_ = root;

            pager_->markPageDirty(root);
            pager_->markPageDirty(left);
            pager_->markPageDirty(right);
            pager_->setRootPage(root);
            return;
        }

        uint32_t parent_id = left_hdr->parent_;
        auto& parent_page = pager_->getPage(parent_id);
        auto* parent_hdr = reinterpret_cast<NodeHeader*>(parent_page.data());

        auto* p_keys = reinterpret_cast<uint64_t*>(parent_hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* p_children = reinterpret_cast<uint32_t*>(p_keys + internalMaxKeys()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        int idx = parent_hdr->num_keys_ - 1;
        while (idx >= 0 && p_keys[idx] > key) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            p_keys[idx + 1] = p_keys[idx]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            p_children[idx + 2] = p_children[idx + 1]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            idx--;
        }

        p_keys[idx + 1] = key; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        p_children[idx + 2] = right; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        parent_hdr->num_keys_++;

        auto& right_page = pager_->getPage(right);
        auto* right_hdr = reinterpret_cast<NodeHeader*>(right_page.data());
        right_hdr->parent_ = parent_id;

        pager_->markPageDirty(parent_id);
        pager_->markPageDirty(right);

        if (parent_hdr->num_keys_ > internalMaxKeys()) {
            splitInternal(parent_id);
        }
    }

    void BPlusTree::splitInternal(uint32_t pageid) {
        auto& page = pager_->getPage(pageid);
        auto* hdr = reinterpret_cast<NodeHeader*>(page.data());

        uint32_t new_id = pager_->allocatePage();
        initInternal(new_id);

        auto& new_page = pager_->getPage(new_id);
        auto* new_hdr = reinterpret_cast<NodeHeader*>(new_page.data());

        auto* keys = reinterpret_cast<uint64_t*>(hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* children = reinterpret_cast<uint32_t*>(keys + internalMaxKeys()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        auto* new_keys = reinterpret_cast<uint64_t*>(new_hdr + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* new_children = reinterpret_cast<uint32_t*>(new_keys + internalMaxKeys()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        uint32_t mid = hdr->num_keys_ / 2;
        uint64_t up_key = keys[mid]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        uint32_t iter = 0;
        for (uint32_t i = mid + 1; i < hdr->num_keys_; i++) {
            new_keys[iter] = keys[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            new_children[iter] = children[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            new_hdr->num_keys_++;
            
            auto& child_p = pager_->getPage(children[i]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            auto* child_h = reinterpret_cast<NodeHeader*>(child_p.data());
            child_h->parent_ = new_id;
            pager_->markPageDirty(children[i]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            iter++;
        }
        new_children[iter] = children[hdr->num_keys_]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto& last_child_p = pager_->getPage(children[hdr->num_keys_]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* last_child_h = reinterpret_cast<NodeHeader*>(last_child_p.data());
        last_child_h->parent_ = new_id;
        pager_->markPageDirty(children[hdr->num_keys_]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        hdr->num_keys_ = mid;
        new_hdr->parent_ = hdr->parent_;

        pager_->markPageDirty(pageid);
        pager_->markPageDirty(new_id);

        insertIntoParent(pageid, up_key, new_id);
    }
}