#pragma once

#include <chrono>
#include <string_view>

namespace anyreg
{
    enum class SortColumn : uint8_t
    {
        NAME,
        PATH,
        LAST_WRITE_TIME,
    };

    enum class SortOrder : uint8_t
    {
        ASCENDING,
        DESCENDING,
    };

    using RegistryTime = std::chrono::file_time<std::chrono::file_clock::duration>;

    struct RegistryKeyView final
    {
        std::string_view name;
        int64_t parent_id;
        RegistryTime last_write_time;
    };
}
