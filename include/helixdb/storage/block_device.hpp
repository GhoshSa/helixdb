#pragma once

#include <string>
#include <cstdint>

namespace helixdb::storage {
    class BlockDevice {
    public:
        explicit BlockDevice(const std::string& path);
        ~BlockDevice();

        BlockDevice(const BlockDevice&) = delete;
        auto operator = (const BlockDevice&) -> BlockDevice& = delete;

        BlockDevice(BlockDevice&& other) noexcept;
        auto operator = (BlockDevice&& other) noexcept -> BlockDevice&;

        void read(uint64_t block_id, void* buffer) const;
        void write(uint64_t block_id, const void* buffer) const;

        void flush() const;

        [[nodiscard]] auto blockCount() const -> uint64_t;
    
    private:
    #ifdef _WIN32
        void* handle_;
    #else
        int fd_;
    #endif
    };
}