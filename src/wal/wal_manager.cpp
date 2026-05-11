#include "helixdb/wal/wal_manager.hpp"
#include "helixdb/wal/wal_record.hpp"
#include "helixdb/storage/block_constants.hpp"
#include "helixdb/storage/block_device.hpp"

#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>
#include <array>
#include <iostream>
#include <utility>

namespace helixdb::wal {
    WalManager::WalManager(const std::string &db_path)
        : fd_(::open((db_path + ".wal").c_str(), O_CREAT | O_RDWR, 0644)), // NOLINT(cppcoreguidelines-pro-type-vararg)
          path_(db_path + ".wal")
    {
        if (fd_ < 0) {
            throw std::runtime_error("Failed to open WAL file");
        }
    }

    WalManager::~WalManager() noexcept {
        try {
            flush();
        } catch (const std::exception& e) {
            std::cerr << "Error during WalManager destruction: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "Unknown error during WalManager destruction." << "\n";
        }
        ::close(fd_);
    }

    WalManager::WalManager(WalManager&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }

    void WalManager::append(uint32_t page_id, const std::byte* page_data) const {
        WalRecordHeader header {};
        header.page_id_ = page_id;
        header.page_size_ = storage::PAGE_SIZE;

        if (const ssize_t written_header = write(fd_, &header, sizeof(header)); written_header != sizeof(header)) {
            throw std::runtime_error("WAL write failed");
        }
        if (const ssize_t written_data = write(fd_, page_data, header.page_size_); std::cmp_not_equal(written_data , header.page_size_)) {
            throw std::runtime_error("WAL write failed");
        }
    }

    void WalManager::flush() const {
        if (fsync(fd_) != 0) {
            throw std::runtime_error("WAL fsync failed");
        }
    }

    void WalManager::recover(storage::BlockDevice &device) const {
        lseek(fd_, 0, SEEK_SET);
        
        while (true) {
            WalRecordHeader header {};

            const ssize_t read_header = read(fd_, &header, sizeof(header));
            if (read_header != sizeof(header)) {
                break;
            }

            std::array<std::byte, storage::PAGE_SIZE> buffer_array{};
            if (const ssize_t read_data = read(fd_, buffer_array.data(), header.page_size_); std::cmp_not_equal(read_data , header.page_size_)) {
                break;
            }

            device.write(header.page_id_, buffer_array.data());
        }

        device.flush();
    }

    void WalManager::truncate() const {
        if (ftruncate(fd_, 0) != 0) {
            throw std::runtime_error("WAL truncate failed");
        }
        lseek(fd_, 0, SEEK_SET);
    }
}