#include <iostream>
#include <cstdint>
#include <cassert>

#include "test_utils.hpp"
#include "helixdb/storage/pager.hpp"

int main () {
    remove_db("recovery.db");

    uint32_t test_page_id;
    {
        helixdb::storage::Pager pager("recovery.db");

        test_page_id = pager.allocate_page();
        auto& page = pager.get_page(test_page_id);

        uint32_t value = 42;
        std::memcpy(page.data(), &value, sizeof(value));

        page.mark_dirty();

        pager.wal().append(test_page_id, page.data());
        pager.wal().flush();
    }

    {
        helixdb::storage::Pager pager("recovery.db");

        auto& page = pager.get_page(test_page_id);

        uint32_t recovered;
        std::memcpy(&recovered, page.data(), sizeof(recovered));

        assert(recovered == 42);
    }
}