#pragma once

#include <cstring>

#include "storage/page.hpp"
#include "btree/internal_layout.hpp"
#include "btree/btree_layout.hpp"

namespace helixdb::btree {
	/* InternalFrameMut Class
	   Mutable MVP view of an internal B-Tree node */
	class InternalFrameMut {
	public:
		explicit InternalFrameMut (storage::Page& page) : page_(page) {}

		/* Initialization */
		void init (uint32_t child0_id, uint64_t key0, uint32_t child1_id) {
			auto* data = page_.data();
			data[INTERNAL_NODE_TYPE_OFFSET] = static_cast<std::byte>(NodeType::INTERNAL);
			data[INTERNAL_IS_ROOT_OFFSET] = static_cast<std::byte>(true);
			uint16_t key_count = 1;
			std::memcpy(data + INTERNAL_KEY_COUNT_OFFSET, &key_count, sizeof(uint16_t));
			std::memcpy(data + INTERNAL_CHILD0_OFFSET, &child0_id, sizeof(uint32_t));
			std::memcpy(data + INTERNAL_KEY0_OFFSET, &key0, sizeof(uint64_t));
			std::memcpy(data + INTERNAL_CHILD1_OFFSET, &child1_id, sizeof(uint32_t));
			page_.mark_dirty();
		}
	private:
		storage::Page& page_;
	};
}