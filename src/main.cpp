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

        /* Prepare value */
        const char* message1 = "hello helixdb.";
        const char* message2 = "hello helixdb main.";
        std::vector<std::byte> value1(
            reinterpret_cast<const std::byte*>(message1),
            reinterpret_cast<const std::byte*>(message1) + std::strlen(message1)
        );
        std::vector<std::byte> value2(
            reinterpret_cast<const std::byte*>(message2),
            reinterpret_cast<const std::byte*>(message2) + std::strlen(message2)
        );

        /* Insert */
        if (!tree.insert(42, value1)) {
            std::cout << "Insert failed!" << std::endl;
        } else {
            std::cout << "Insert successfull." << std::endl;
        }
        if (!tree.insert(43, value2)) {
            std::cout << "Insert failed!" << std::endl;
        } else {
            std::cout << "Insert successfull." << std::endl;
        }

        /* Find */
        std::span<const std::byte> out;
        if (!tree.find(42, out)) {
            std::cout << "Key not found!" << std::endl;
        } else {
            std::string result(
                reinterpret_cast<const char*>(out.data()),
                out.size()
            );
            std::cout << "Found value: " << result << std::endl;
        }
        if (!tree.find(43, out)) {
            std::cout << "Key not found!" << std::endl;
        } else {
            std::string result(
                reinterpret_cast<const char*>(out.data()),
                out.size()
            );
            std::cout << "Found value: " << result << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}