#include "helixdb/wal/wal_manager.hpp"
#include "helixdb/wal/wal_record.hpp"
#include "helixdb/storage/block_constants.hpp"
#include "helixdb/storage/block_device.hpp"

#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>
#include <iostream>

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

    void WalManager::append(uint32_t page_id, const std::byte* page_data) {
        WalRecordHeader header {};
        header.page_id = page_id;
        header.page_size = storage::PAGE_SIZE;

        if (const ssize_t w1 = write(fd_, &header, sizeof(header)); w1 != sizeof(header)) throw std::runtime_error("WAL write failed");
        if (const ssize_t w2 = write(fd_, page_data, header.page_size); w2 != header.page_size) throw std::runtime_error("WAL write failed");
    }

    void WalManager::flush() {
        if (fsync(fd_) != 0) throw std::runtime_error("WAL fsync failed");
    }

    void WalManager::recover(storage::BlockDevice &device) {
        lseek(fd_, 0, SEEK_SET);
        
        while (true) {
            WalRecordHeader header {};

            const ssize_t r1 = read(fd_, &header, sizeof(header));
            if (r1 != sizeof(header)) break;

            std::byte buffer[storage::PAGE_SIZE];
            if (const ssize_t r2 = read(fd_, buffer, header.page_size); r2 != header.page_size) break;

            device.write(header.page_id, buffer);
        }

        device.flush();
    }

    void WalManager::truncate() {
        if (ftruncate(fd_, 0) != 0) throw std::runtime_error("WAL truncate failed");
        lseek(fd_, 0, SEEK_SET);
    }
}