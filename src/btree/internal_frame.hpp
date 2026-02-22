#pragma once

#include <cstring>

#include "storage/page.hpp"
#include "btree/btree_layout.hpp"
#include "btree/internal_layout.hpp"

namespace helixdb::btree {
	/* Internal Frame Class
	   Read-only view over an internal B-tree node */
	class InternalFrame {
	public:
		explicit InternalFrame (const storage::Page& page) : page_(page) {}

		/* Headers Accessors */
		NodeType node_type () const {
			return static_cast<NodeType>(page_.data()[INTERNAL_NODE_TYPE_OFFSET]);
		}
		bool is_root () const {
			return static_cast<bool>(page_.data()[INTERNAL_IS_ROOT_OFFSET]);
		}
		uint16_t key_count () const {
			uint16_t count;
			std::memcpy(&count, page_.data() + INTERNAL_KEY_COUNT_OFFSET, sizeof(uint16_t));
			return count;
		}

		/* Child and Key Accessors */
		uint32_t child0 () const {
			uint32_t id;
			std::memcpy(&id, page_.data() + INTERNAL_CHILD0_OFFSET, sizeof(uint32_t));
			return id;
		}
		uint64_t key0 () const {
			uint64_t key;
			std::memcpy(&key, page_.data() + INTERNAL_KEY0_OFFSET, sizeof(uint64_t));
			return key;
		}
		uint32_t child1 () const {
			uint32_t id;
			std::memcpy(&id, page_.data() + INTERNAL_CHILD1_OFFSET, sizeof(uint32_t));
			return id;
		}
	private:
		const storage::Page& page_;
	};
}