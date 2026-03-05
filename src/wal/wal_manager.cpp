#include "helixdb/wal/wal_manager.hpp"
#include "helixdb/storage/block_constants.hpp"
#include "helixdb/storage/block_device.hpp"

#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

namespace helixdb::wal {
    WalManager::WalManager(const std::string &db_path) {
        path_ = db_path + ".wal";
        fd_ = open(path_.c_str(), O_CREAT | O_RDWR, 0644);

        if (fd_ < 0) throw std::runtime_error("Failed to open WAL file");
    }

    WalManager::~WalManager() {
        flush();
        close(fd_);
    }

    void WalManager::append(uint32_t page_id, const std::byte *page_data) {
        uint32_t size = storage::PAGE_SIZE;

        ssize_t w1 = write(fd_, &page_id, sizeof(page_id));
        if (w1 != sizeof(page_id)) throw std::runtime_error("WAL write failed");
        
        ssize_t w2 = write(fd_, &size, sizeof(size));
        if (w2 != sizeof(size)) throw std::runtime_error("WAL write failed");

        ssize_t w3 = write(fd_, page_data, size);
        if (w3 != size) throw std::runtime_error("WAL write failed");
    }

    void WalManager::flush() {
        if (fsync(fd_) != 0) throw std::runtime_error("WAL fsync failed");
    }

    void WalManager::recover(storage::BlockDevice &device) {
        lseek(fd_, 0, SEEK_SET);
        
        while (true) {
            uint32_t page_id;
            uint32_t size;

            ssize_t r1 = read(fd_, &page_id, sizeof(page_id));
            if (r1 == 0) break;
            if (r1 != sizeof(page_id)) break;

            ssize_t r2 = read(fd_, &size, sizeof(size));
            if (r2 != sizeof(size)) break;
            if (r2 != storage::PAGE_SIZE) break;

            if (page_id >= device.block_count()) break;

            std::byte buffer[storage::PAGE_SIZE];

            ssize_t r3 = read(fd_, buffer, size);
            if (r3 != size) break;

            device.write(page_id, buffer);
        }

        device.flush();
    }

    void WalManager::truncate() {
        if (ftruncate(fd_, 0) != 0) throw std::runtime_error("WAL truncate failed");
        lseek(fd_, 0, SEEK_SET);
    }
}