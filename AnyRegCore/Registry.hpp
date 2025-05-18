#pragma once

#include "AnyRegCore.hpp"

#include <Windows.h>

#include <functional>
#include <span>

namespace anyreg
{
    bool is_predefined_hkey(HKEY hkey);

    class RegistryKey
    {
    public:
        RegistryKey() = default;
        explicit RegistryKey(HKEY root, std::string_view path, REGSAM access = KEY_READ);
        ~RegistryKey();

        RegistryKey(RegistryKey&& other) noexcept;
        RegistryKey& operator=(RegistryKey&& other) noexcept;

        RegistryKey(const RegistryKey& other) = delete;
        RegistryKey& operator=(const RegistryKey& other) = delete;

        [[nodiscard]] DWORD sub_key_count() const;

        [[nodiscard]] RegistryKey sub_key(std::string_view name, REGSAM access = KEY_READ) const;

        [[nodiscard]] bool get_sub_key(DWORD index, std::span<char>& name, RegistryTime& last_write_time) const;

        friend void swap(RegistryKey& first, RegistryKey& second) noexcept;

    private:
        static HKEY open(HKEY root, std::string_view path, REGSAM access);

        explicit RegistryKey(HKEY hkey) noexcept;
        
        HKEY _key{};
    };
}
