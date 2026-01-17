#pragma once

#include <array>

#include "storage/constants.hpp"

namespace helixdb::storage {
	class Page {
	public:
		Page();
		std::byte *data();
		const std::byte *data() const;
		bool is_dirty() const;
		void mark_dirty();
		void clear_dirty();

	private:
		std::array<std::byte, PAGE_SIZE> buffer_;
		bool dirty_{false};
	};
}