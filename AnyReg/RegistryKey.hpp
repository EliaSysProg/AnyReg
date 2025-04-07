#pragma once
#include <span>

class RegistryKey
{
public:
    RegistryKey() = default;
    explicit RegistryKey(HKEY root, std::string_view path, REGSAM access);
    ~RegistryKey();

    RegistryKey(RegistryKey&& other) noexcept;
    RegistryKey& operator=(RegistryKey&& other) noexcept;

    RegistryKey(const RegistryKey& other) = delete;
    RegistryKey& operator=(const RegistryKey& other) = delete;

    HKEY get();
    bool get_value(DWORD index, std::span<char> name) const;
    bool get_sub_key(DWORD index, std::span<char> name) const;

    friend void swap(RegistryKey& first, RegistryKey& second) noexcept;

private:
    HKEY _key{};
};
