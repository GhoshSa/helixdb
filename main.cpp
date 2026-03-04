#include <iostream>

#include "helixdb/storage/pager.hpp"

int main() {
    try {
        helixdb::storage::Pager pager("main.db");

        std::cout << "Page count: " << pager.page_count() << std::endl;

        uint32_t id = pager.allocate_page();
        std::cout << "Allocated page: " << id << std::endl;

        pager.flush_all();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}