#pragma once

#include <filesystem>

inline void remove_db(const std::string &name) {
    std::filesystem::remove(name);
    std::filesystem::remove(name + ".wal");
}