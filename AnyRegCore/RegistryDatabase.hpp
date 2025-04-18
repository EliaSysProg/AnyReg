#pragma once

#include "RegistryEntry.hpp"

#include <ranges>
#include <string_view>
#include <vector>

class RegistryDatabase final
{
public:
    RegistryDatabase();

    ~RegistryDatabase() = default;

    RegistryDatabase(RegistryDatabase&& other) noexcept = default;
    RegistryDatabase& operator=(RegistryDatabase&& other) noexcept = default;

    RegistryDatabase(const RegistryDatabase& other) = delete;
    RegistryDatabase& operator=(const RegistryDatabase& other) = delete;

    void index(const std::vector<HKEY>& hives);
    void index(HKEY hive, std::wstring_view sub_path = L"");

    [[nodiscard]] auto find_key(const std::wstring_view query) const
    {
        return _keys | std::views::filter([&](const auto& key_entry) { return key_entry.name.contains(query); });
    }

private:
    std::vector<RegistryKeyEntry> _keys;
    std::vector<RegistryValueEntry> _values;
};
