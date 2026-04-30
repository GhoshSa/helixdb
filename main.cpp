#include <iostream>
#include <string>
#include <iomanip>
#include "helixdb/storage/pager.hpp"
#include "helixdb/btree/bplushtree.hpp"

using namespace helixdb;

void print_banner() {
    std::cout << R"(
  _    _      _       ____   ____  
 | |  | |    | |     |  __ \|  _  \
 | |__| | ___| |__  _| |  | | |  ) )
 |  __  |/ _ \ |\ \/ / |  | | |  < 
 | |  | |  __/ | >  <| |__| | |_ ) )
 |_|  |_|\___|_|/_/\_\_____/|_____/ 
    )" << std::endl;
    std::cout << " HelixDB Embedded Engine | v0.1.0-alpha" << std::endl;
    std::cout << " Type 'help' for commands, 'exit' to quit.\n" << std::endl;
}

void print_help() {
    std::cout << "\nAvailable Commands:\n"
              << "  insert <key> <val>  Insert or update a key-value pair\n"
              << "  find <key>          Look up a value by its key\n"
              << "  stats               Show database metadata and engine stats\n"
              << "  help                Display this menu\n"
              << "  exit                Flush all changes and close database\n" << std::endl;
}

int main(int argc, char* argv[]) {
    print_banner();

    std::string db_file = (argc > 1) ? argv[1] : "helix.db";
    
    try {
        storage::Pager pager(db_file);
        bplushtree::BPlushTree tree(pager);

        std::cout << "Connected to: " << db_file << "\n" << std::endl;

        std::string cmd;
        while (true) {
            std::cout << "helixdb> " << std::flush;
            if (!(std::cin >> cmd)) break;

            if (cmd == "insert") {
                uint64_t key;
                uint32_t value;
                if (std::cin >> key >> value) {
                    tree.insert(key, value);
                    std::cout << "OK" << std::endl;
                } else {
                    std::cout << "Error: Usage is 'insert <uint64> <uint32>'" << std::endl;
                    std::cin.clear();
                    std::cin.ignore(1000, '\n');
                }
            } else if (cmd == "find") {
                uint64_t key;
                if (std::cin >> key) {
                    uint32_t value;
                    if (tree.find(key, value)) {
                        std::cout << value << std::endl;
                    } else {
                        std::cout << "(not found)" << std::endl;
                    }
                }
            } else if (cmd == "stats") {
                std::cout << "--- Metadata ---\n"
                          << "Root Page: " << pager.root_page_id() << "\n"
                          << "Page Size: " << storage::PAGE_SIZE << " bytes[cite: 5]\n"
                          << "----------------" << std::endl;
            } else if (cmd == "help") {
                print_help();
            } else if (cmd == "exit") {
                break; 
            } else {
                std::cout << "Unknown command. Type 'help'." << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Critical Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Closing database and flushing WAL..." << std::endl;
    return 0;
}