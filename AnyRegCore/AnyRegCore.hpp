#pragma once

#include <Windows.h>

#include <chrono>
#include <map>
#include <stop_token>
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

    using RegistryId = uint64_t;

    struct RegistryKeyEntry final
    {
        std::string name;
        RegistryTime last_write_time;
        RegistryId parent_id;
    };

    using RegistryDatabase = std::map<RegistryId, RegistryKeyEntry>;

    void index_hive(RegistryDatabase& db, HKEY hive, const std::stop_token& stop_token = {});
    void index_hive(RegistryDatabase& db, HKEY hive, std::string_view path, const std::stop_token& stop_token = {});

    std::string key_full_path(const RegistryDatabase& db, RegistryId id);
}
