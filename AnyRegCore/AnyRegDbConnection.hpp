#pragma once

#include "RegistryEntry.hpp"

#include <ranges>
#include <vector>

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
        std::vector<RegistryKeyEntry> result;
        for (const auto& key_entry : _keys)
        {
            if (key_entry.name.contains(query))
            {
                result.push_back(key_entry);
            }
        }

        return result;
    }

private:
    std::vector<RegistryKeyEntry> _keys;
    std::vector<RegistryValueEntry> _values;
};
