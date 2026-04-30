# HelixDB — A Lightweight Embedded Database Engine in C++

[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/Build-CMake-1f8dd6.svg)](https://cmake.org/)

HelixDB is a lightweight, embedded key-value database engine written in modern C++.  
It uses a custom page-based storage format, a B+ tree index, and a write-ahead log for durability.

---

## 🎯 Goal

HelixDB is built from scratch to understand how embedded databases work internally.  
It is designed for applications that need simple, fast, and reliable local storage.

---

## ✨ Features (Current)

- ✅ Embedded key-value store (no SQL layer yet)
- ✅ Persistent storage using fixed 4KB pages
- ✅ B+ tree indexing for ordered data
- ✅ Write-ahead logging for crash recovery
- ✅ Simple page cache and free-list allocator

---

## 🚀 Quick Start

### 1. Clone the repository
```bash
git clone https://github.com/GhoshSa/helixdb.git
cd helixdb
```

### 2. Build
```bash
mkdir -p build
cmake -B build
cmake --build build
```

### 3. Run
```bash
./build/helixdb_main
```

---

## 🔧 Build Requirements

- C++20 compatible compiler
- CMake
- GoogleTest for unit tests

---

## 🧠 Internal Design

- `storage/BlockDevice` performs raw file I/O using 4KB blocks.
- `storage/Page` holds an in-memory page buffer.
- `storage/Pager` manages page caching, allocation, dirty tracking, and flushing.
- `storage/FileHeader` stores DB metadata in page 0.
- `btree/BPlushTree` implements insertion, lookup, and node splitting.
- `btree/Cursor` provides sequential traversal of leaf pages.
- `wal/WalManager` logs page changes before they are written to disk.

---

## 🧪 Tests

Run the unit tests from the main directory:

```bash
./build/test
```

The tests cover durability, structural consistency, cursor behavior, and page allocation.

---

## ⚠️ Limitations

Current implementation is intentionally minimal:

- Single-threaded only
- Fixed key type: `uint64_t`
- Fixed value type: `uint32_t`
- No SQL or schema support
- No multi-operation transactions
- WAL overhead

---

## 💡 Author

Sayantan Ghosh  
GitHub: [@GhoshSa](https://github.com/GhoshSa)

Contributions, issues, and pull requests are welcome.