#pragma once

#include "RegistryEntry.hpp"

#include <Windows.h>

#include <algorithm>
#include <span>

namespace anyreg
{
    inline static const auto HIVES = {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, HKEY_CLASSES_ROOT, HKEY_USERS, HKEY_CURRENT_CONFIG};

    constexpr static std::string_view hive_name(const HKEY hive)
    {
        if (hive == HKEY_CLASSES_ROOT) return "HKEY_CLASSES_ROOT";
        if (hive == HKEY_CURRENT_USER) return "HKEY_CURRENT_USER";
        if (hive == HKEY_LOCAL_MACHINE) return "HKEY_LOCAL_MACHINE";
        if (hive == HKEY_USERS) return "HKEY_USERS";
        if (hive == HKEY_CURRENT_CONFIG) return "HKEY_CURRENT_CONFIG";
        return "";
    }

    constexpr static bool is_predefined_hkey(const HKEY hkey)
    {
        return std::ranges::any_of(HIVES, [hkey](const HKEY hive) { return hkey == hive; });
    }

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

        [[nodiscard]] bool get_sub_key(DWORD index, std::span<char>& name, RegistryTime& last_write_time) const;
        [[nodiscard]] RegistryKey open_sub_key(std::string_view path, REGSAM access = KEY_READ) const;

        friend void swap(RegistryKey& first, RegistryKey& second) noexcept;

    private:
        HKEY _key{};
    };
}
