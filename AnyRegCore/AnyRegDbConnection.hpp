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

    void index(const std::vector<HKEY>& hives);
    void index(HKEY root, std::wstring_view sub_path = L"");

    [[nodiscard]] std::vector<RegistryKeyEntry> find_key(std::wstring_view query) const;
    [[nodiscard]] std::vector<RegistryKeyEntry> keys() const;

private:
    std::vector<RegistryKeyEntry> _keys;
    std::vector<RegistryValueEntry> _values;
};
