#include "helixdb/storage/pager.hpp"
#include "helixdb/btree/bplushtree.hpp"
#include <iostream>

using namespace helixdb;

int main() {
    storage::Pager pager("test.db");
    bplushtree::BPlushTree tree(pager);

    std::string cmd;

    while (std::cin >> cmd) {
        if (cmd == "insert") {
            uint64_t key;
            uint32_t value;
            std::cin >> key >> value;
            tree.insert(key, value);
            std::cout << "Inserted\n";
        } else if (cmd == "find") {
            uint64_t key;
            std::cin >> key;
            uint32_t value;
            if (tree.find(key, value)) std::cout << "Found: " << value << "\n";
            else std::cout << "Not Found\n";
        } else if (cmd == "exit") break;
    }
}