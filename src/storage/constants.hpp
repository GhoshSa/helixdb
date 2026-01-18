#pragma once

#include <cstddef>
#include <cstdint>

namespace helixdb::storage {
    /* Constants */
    constexpr std::size_t PAGE_SIZE = 4096;
    using PageId = std::uint32_t;
}