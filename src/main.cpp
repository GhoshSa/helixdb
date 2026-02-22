#include <iostream>
#include <vector>

#include "storage/pager.hpp"
#include "btree/btree_mvp.hpp"

/* Main Function */
int main() {
    try {
        /* Open database file */
        helixdb::storage::Pager pager("main.db");
        helixdb::btree::BTreeMVP tree(pager);

        /* Insert */
        std::cout << "Inserting keys..." << std::endl;

        for (uint64_t key = 1; key <= 200; ++key) {
            std::string message = "Value: " + std::to_string(key);
            std::vector<std::byte>value(
                reinterpret_cast<const std::byte*>(message.data()),
                reinterpret_cast<const std::byte*>(message.data() + message.size())
            );

            tree.insert(key, value);
        }
        
        pager.flush_all();
        std::cout << "Done" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}