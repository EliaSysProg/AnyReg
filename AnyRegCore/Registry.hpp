#pragma once

#include "RegistryEntry.hpp"

#include <string>

class RegistryKey
{
public:
    RegistryKey() = default;
    explicit RegistryKey(HKEY root, const std::string& path, REGSAM access = KEY_READ);
    ~RegistryKey();

    RegistryKey(RegistryKey&& other) noexcept;
    RegistryKey& operator=(RegistryKey&& other) noexcept;

    RegistryKey(const RegistryKey& other) = delete;
    RegistryKey& operator=(const RegistryKey& other) = delete;

    [[nodiscard]] bool get_value(DWORD index, std::string& name, RegistryValueType& type) const;
    [[nodiscard]] bool get_sub_key(DWORD index, std::string& name, RegistryKeyTime& last_write_time) const;

    friend void swap(RegistryKey& first, RegistryKey& second) noexcept;

private:
    HKEY _key{};
};
