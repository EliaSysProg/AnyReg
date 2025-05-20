#pragma once

#include <chrono>
#include <string_view>

namespace anyreg
{
    using RegistryTime = std::chrono::file_time<std::chrono::file_clock::duration>;

    struct RegistryKeyView final
    {
        std::string_view name;
        int64_t parent_id;
        RegistryTime last_write_time;
    };
}
