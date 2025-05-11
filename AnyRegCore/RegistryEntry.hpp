#pragma once

#include <Windows.h>

#include <string>
#include <chrono>

namespace anyreg
{
    static constexpr size_t REGISTRY_MAX_KEY_VALUE_NAME = 256;

    using RegistryKeyTime = std::chrono::file_time<std::chrono::file_clock::duration>;

    enum class SortColumn : uint8_t
    {
        NAME,
        PATH,
        LAST_WRITE_TIME,
    };

    enum class SortOrder : uint8_t
    {
        ASCENDING,
        DESCENDING,
    };

    constexpr std::string to_string(const SortColumn sort_column)
    {
        switch (sort_column)
        {
        case SortColumn::NAME:
            return "Name";
        case SortColumn::PATH:
            return "Path";
        case SortColumn::LAST_WRITE_TIME:
            return "LastWriteTime";
        default:
            throw std::invalid_argument("Invalid SortColumn value");
        }
    }

    constexpr std::string to_string(const SortOrder sort_order)
    {
        switch (sort_order)
        {
        case SortOrder::ASCENDING:
            return "ASC";
        case SortOrder::DESCENDING:
            return "DESC";
        default:
            throw std::invalid_argument("Invalid SortOrder value");
        }
    }

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
        HKEY hive;
        std::string path;
        RegistryKeyTime last_write_time;

        [[nodiscard]] std::string get_absolute_path() const;
        [[nodiscard]] std::string get_full_path() const;
    };

    struct RegistryKeyView final
    {
        std::string_view name;
        HKEY hive;
        std::string_view path;
        RegistryKeyTime last_write_time;

        [[nodiscard]] std::string get_absolute_path() const;
        [[nodiscard]] std::string get_full_path() const;
    };

    struct RegistryValueEntry final
    {
        std::string name;
        std::string path;
        RegistryValueType type;
    };
}
