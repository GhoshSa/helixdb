#include "storage/page.hpp"

namespace helixdb::storage {
    Page::Page () : dirty_ (false) {
        buffer_.fill(std::byte{0});
    }

    std::byte* Page::data () {
        return buffer_.data();
    }

    const std::byte* Page::data () const {
        return buffer_.data();
    }

    bool Page::is_dirty () const {
        return dirty_;
    }

    void Page::mark_dirty () {
        dirty_ = true;
    }

    void Page::clear_dirty () {
        dirty_ = false;
    }
}