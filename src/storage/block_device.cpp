#include "helixdb/storage/block_device.hpp"
#include "helixdb/storage/block_constants.hpp"

#ifdef _WIN32
    #ifdef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/stat.h>
    #include <cstring>
    #include <cerrno>
    #include <stdexcept>
#include <utility>
#endif

namespace helixdb::storage {
    static void throwSysError(const char* msg) {
        #ifdef _WIN32
            throw std::runtime_error(std::string(msg) + " (WinError: " + std::to_string(GetLastError()) + ")");
        #else
            throw std::runtime_error(std::string(msg) + ": " + std::strerror(errno));
        #endif
    }

    BlockDevice::BlockDevice(const std::string& path)
    #ifdef _WIN32
        : handle_(CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL))
    #else
        : fd_(::open(path.c_str(), O_RDWR | O_CREAT, 0644)) // NOLINT(cppcoreguidelines-pro-type-vararg)
    #endif
    {
        #ifdef _WIN32
            if (handle_ == INVALID_HANDLE_VALUE) throwSysError("Failed to open file");
        #else
            if (fd_ < 0) {
                throwSysError("Failed to open block device");
            }
        #endif
    }
    
    BlockDevice::~BlockDevice() {
        #ifdef _WIN32
            if (handle_ != INVALID_HANDLE_VALUE) {
                FlushFileBuffers(handle_);
                CloseHandle(handle_);
            }
        #else
            if (fd_ >= 0) {
                ::fsync(fd_);
                ::close(fd_);
            }
        #endif
    }

    BlockDevice::BlockDevice(BlockDevice&& other) noexcept
        #ifdef _WIN32
            : handle_(other.handle_)
        #else
            : fd_(other.fd_)
        #endif
    {
        #ifdef _WIN32
            other.handle_ = INVALID_HANDLE_VALUE;
        #else
            other.fd_ = -1;
        #endif
    }

    auto BlockDevice::operator=(BlockDevice&& other) noexcept -> BlockDevice& {
        if (this != &other) {
            #ifdef _WIN32
                if (handle_ != INVALID_HANDLE_VALUE) {
                    FlushFileBuffers(handle_);
                    CloseHandle(handle_);
                }
                handle_ = other.handle_;
                other.handle_ = INVALID_HANDLE_VALUE;
            #else
                if (fd_ >= 0) {
                    ::fsync(fd_);
                    ::close(fd_);
                }
                fd_ = other.fd_;
                other.fd_ = -1;
            #endif
        }
        return *this;
    }

    void BlockDevice::read(uint64_t block_id, void* buffer) const {
        auto offset = static_cast<off_t>(block_id * BLOCK_SIZE);
        #ifdef _WIN32
            OVERLAPPED ov = {0};
            ov.Offset = static_cast<DWORD>(offset);
            ov.OffsetHigh = static_cast<DWORD>(offset >> 32);
            DWORD read_bytes;
            if (!ReadFile(handle_, buffer, BLOCK_SIZE, &read_bytes, &ov) && GetLastError() != ERROR_HANDLE_EOF) {
                throwSysError("read failed");
            }
        #else
            ssize_t bytes = ::pread(fd_, buffer, BLOCK_SIZE, offset);
            if (bytes < 0) {
                throwSysError("read failed");
            }
            if (std::cmp_not_equal(bytes ,BLOCK_SIZE)) {
                throwSysError("short read (corrupt or truncated file)");
            }
        #endif
    }

    void BlockDevice::write(uint64_t block_id, const void* buffer) const {
        auto offset = static_cast<off_t>(block_id * BLOCK_SIZE);
        #ifdef _WIN32
            OVERLAPPED ov = {0};
            ov.Offset = static_cast<DWORD>(offset);
            ov.OffsetHigh = static_cast<DWORD>(offset >> 32);
            DWORD written;
            if (!WriteFile(handle_, buffer, BLOCK_SIZE, &written, &ov)) throwSysError("write failed");
        #else
            ssize_t bytes = ::pwrite(fd_, buffer, BLOCK_SIZE, offset);
            if (bytes < 0) {
                throwSysError("write failed");
            }
            if (std::cmp_not_equal(bytes ,BLOCK_SIZE)) {
                throwSysError("short write (disk may be full)");
            }
        #endif
    }

    void BlockDevice::flush() const {
        #ifdef _WIN32
            if (!FlushFileBuffers(handle_)) throwSysError("flush failed");
        #else
            if (::fsync(fd_) != 0) {
                throwSysError("fsync failed");
            }
        #endif
    }

    auto BlockDevice::blockCount() const -> uint64_t {
        #ifdef _WIN32
            LARGE_INTEGER size;
            if (!GetFileSizeEx(handle_, &size)) throwSysError("fstat failed");
            return static_cast<uint64_t>(size.QuadPart / BLOCK_SIZE);
        #else
            struct stat file_stats {};
            if (::fstat(fd_, &file_stats) != 0) {
                throwSysError("fstat failed");
            }
            return static_cast<uint64_t>(file_stats.st_size / BLOCK_SIZE);
        #endif
    }
}