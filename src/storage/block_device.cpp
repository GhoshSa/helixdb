#include "helixdb/storage/block_device.hpp"
#include "helixdb/storage/block_constants.hpp"

#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>

namespace helixdb::storage {
    static void throw_sys_error(const char* msg) {
        throw std::runtime_error(std::string(msg) + ": " + std::strerror(errno));
    }

    BlockDevice::BlockDevice(const std::string& path) {
        fd_ = ::open(path.c_str(), O_RDWR | O_CREAT, 0644);
        if (fd_ < 0) throw_sys_error("Failed to open block device");
    }

    BlockDevice::~BlockDevice() {
        if (fd_ >= 0) {
            ::fsync(fd_);
            ::close(fd_);
        }
    }

    void BlockDevice::read(uint64_t block_id, void* buffer) {
        off_t offset = static_cast<off_t>(block_id * BLOCK_SIZE);
        ssize_t bytes = ::pread(fd_, buffer, BLOCK_SIZE, offset);
        if (bytes < 0) throw_sys_error("read failed");
        if (bytes != static_cast<ssize_t>(BLOCK_SIZE)) throw_sys_error("short read (corrupt or truncated file)");
    }

    void BlockDevice::write(uint64_t block_id, const void* buffer) {
        off_t offset = static_cast<off_t>(block_id * BLOCK_SIZE);
        ssize_t bytes = ::pwrite(fd_, buffer, BLOCK_SIZE, offset);
        if (bytes < 0) throw_sys_error("write failed");
        if (bytes != static_cast<ssize_t>(BLOCK_SIZE)) throw_sys_error("short write (disk may be full)");
    }

    void BlockDevice::flush() {
        if (::fsync(fd_) != 0) throw_sys_error("fsync failed");
    }

    uint64_t BlockDevice::block_count() const {
        struct stat st {};
        if (::fstat(fd_, &st) != 0) throw_sys_error("fstat failed");
        return static_cast<uint64_t>(st.st_size / BLOCK_SIZE);
    }
}