#pragma once

#include <string>
#include <cstdint>

namespace helixdb::storage {
    class BlockDevice {
    public:
        explicit BlockDevice(const std::string& path);
        ~BlockDevice();

        BlockDevice(const BlockDevice&) = delete;
        BlockDevice& operator = (const BlockDevice&) = delete;

        void read(uint64_t block_id, void* buffer);
        void write(uint64_t block_id, const void* buffer);

        void flush();

        uint64_t block_count() const;
    
    private:
        int fd_;
    };
}