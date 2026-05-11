#include <iostream>
#include <string>
#include <iomanip>
#include <limits>
#include "helixdb/storage/pager.hpp"
#include "helixdb/btree/bplustree.hpp"

using namespace helixdb;

void printBanner() {
    std::cout << R"(
  _    _      _         ____   ____  
 | |  | |    | |()     |  __ \|  _  \
 | |__| | ___| |_ __  _| |  | | |  ) )
 |  __  |/ _ \ | |\ \/ / |  | | |  < 
 | |  | |  __/ | | >  <| |__| | |_ ) )
 |_|  |_|\___|_|_|/_/\_\_____/|_____/ 
    )" << "\n";
    std::cout << " HelixDB Embedded Engine | v0.1.0-alpha" << "\n";
    std::cout << " Type 'help' for commands, 'exit' to quit.\n" << "\n";
}

void printHelp() {
    std::cout << "\nAvailable Commands:\n"
              << "  insert <key> <val>  Insert or update a key-value pair\n"
              << "  find <key>          Look up a value by its key\n"
              << "  stats               Show database metadata and engine stats\n"
              << "  help                Display this menu\n"
              << "  exit                Flush all changes and close database\n" << "\n";
}

auto main(int argc, char* argv[]) -> int {
    printBanner();

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::string db_file = (argc > 1) ? argv[1] : "helix.db";
    
    try {
        storage::Pager pager(db_file);
        bplustree::BPlusTree tree(pager);

        std::cout << "Connected to: " << db_file << "\n" << "\n";

        std::string cmd;
        while (true) {
            std::cout << "helixdb> " << std::flush;
            if (!(std::cin >> cmd)) {
                break;
            }

            if (cmd == "insert") {
                uint64_t key = 0;
                uint32_t value = 0;
                if (std::cin >> key >> value) {
                    tree.insert(key, value);
                    std::cout << "OK" << "\n";
                } else {
                    std::cout << "Error: Usage is 'insert <uint64> <uint32>'" << "\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
            } else if (cmd == "find") {
                uint64_t key = 0;
                if (std::cin >> key) {
                    uint32_t value = 0;
                    if (tree.find(key, value)) {
                        std::cout << value << "\n";
                    } else {
                        std::cout << "(not found)" << "\n";
                    }
                }
            } else if (cmd == "stats") {
                std::cout << "--- Metadata ---\n"
                          << "Root Page: " << pager.rootPageId() << "\n"
                          << "Page Size: " << storage::PAGE_SIZE << " bytes\n"
                          << "----------------" << "\n";
            } else if (cmd == "help") {
                printHelp();
            } else if (cmd == "exit") {
                break; 
            } else {
                std::cout << "Unknown command. Type 'help'." << "\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Critical Error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "Closing database and flushing WAL..." << "\n";
    return 0;
}