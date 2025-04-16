#pragma once

#include "Registry.hpp"
#include "RegistryEntry.hpp"

#include <ranges>

class AnyRegDbConnection final
{
public:
    AnyRegDbConnection() = default;
    ~AnyRegDbConnection() = default;

    AnyRegDbConnection(AnyRegDbConnection&& other) noexcept = default;
    AnyRegDbConnection& operator=(AnyRegDbConnection&& other) noexcept = default;

    AnyRegDbConnection(const AnyRegDbConnection& other) = delete;
    AnyRegDbConnection& operator=(const AnyRegDbConnection& other) = delete;

    void index(HKEY root, std::wstring_view sub_path = L"");

    [[nodiscard]] auto find_key(std::wstring_view query) const
    {
        return _keys | std::views::filter([&query](const RegistryKeyEntry& key_entry) { return key_entry.name.contains(query); });
    }

private:
    std::vector<RegistryKeyEntry> _keys;
    std::vector<RegistryValueEntry> _values;
};
