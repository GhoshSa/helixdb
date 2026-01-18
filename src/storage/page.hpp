#pragma once

#include <array>

#include "storage/constants.hpp"

namespace helixdb::storage {
	/* Page Class 
	   Represents a fixed-size database page */
	class Page {
	public:
		/* Newly created pages must be fully zeroed */
		Page();
		std::byte *data(); // Mutable access to raw page data 
		const std::byte *data() const; // Immutable access to raw page data
		bool is_dirty() const; // Returns true if the page has been modified since last flush
		void mark_dirty(); // Marks the page as dirty after modification
		void clear_dirty(); // Clears dirty flag

	private:
		std::array<std::byte, PAGE_SIZE> buffer_; // Fixed-size page buffer
		bool dirty_{false}; // Dirty flag
	};
}