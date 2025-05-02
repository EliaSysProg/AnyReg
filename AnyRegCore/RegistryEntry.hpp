#pragma once

#include <Windows.h>

#include <string>
#include <chrono>

namespace anyreg
{
    static constexpr size_t REGISTRY_MAX_KEY_VALUE_NAME = 256;

    using RegistryKeyTime = std::chrono::file_time<std::chrono::file_clock::duration>;

    enum class RegistryValueType : DWORD
    {
        BINARY = REG_BINARY,
        DWORD = REG_DWORD,
        EXPAND_SZ = REG_EXPAND_SZ,
        LINK = REG_LINK,
        MULTI_SZ = REG_MULTI_SZ,
        NONE = REG_NONE,
        QWORD = REG_QWORD,
        SZ = REG_SZ,
    };

    struct RegistryKeyEntry final
    {
        std::string name;
        std::string path;
        HKEY hive;
        RegistryKeyTime last_write_time;

        [[nodiscard]] std::string get_full_path() const;
    };

    struct RegistryKeyView final
    {
        std::string_view name;
        std::string_view path;
        HKEY hive;
        RegistryKeyTime last_write_time;
    };

    struct RegistryValueEntry final
    {
        std::string name;
        std::string path;
        RegistryValueType type;
    };
}
