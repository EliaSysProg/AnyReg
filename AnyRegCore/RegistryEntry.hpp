#pragma once

#include <chrono>
#include <string>
#include <string_view>

namespace anyreg
{
    using RegistryTime = std::chrono::file_time<std::chrono::file_clock::duration>;

    struct RegistryKeyView final
    {
        std::string_view name;
        uint64_t hive;
        std::string_view path;
        RegistryTime last_write_time;

        [[nodiscard]] std::string full_path() const;
        [[nodiscard]] std::string hive_relative_path() const;
    };
}
