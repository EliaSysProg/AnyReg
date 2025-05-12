#include "RegistryEntry.hpp"

#include <Windows.h>

#include <filesystem>
#include <stdexcept>

namespace anyreg
{
    constexpr static std::string_view hive_name(const HKEY hive)
    {
        if (hive == HKEY_CLASSES_ROOT) return "HKEY_CLASSES_ROOT";
        if (hive == HKEY_CURRENT_USER) return "HKEY_CURRENT_USER";
        if (hive == HKEY_LOCAL_MACHINE) return "HKEY_LOCAL_MACHINE";
        if (hive == HKEY_USERS) return "HKEY_USERS";
        if (hive == HKEY_CURRENT_CONFIG) return "HKEY_CURRENT_CONFIG";
        return "";
    }

    std::string RegistryKeyView::full_path() const
    {
        return (std::filesystem::path(hive_name(reinterpret_cast<HKEY>(hive))) / hive_relative_path()).string();
    }

    std::string RegistryKeyView::hive_relative_path() const
    {
        return (std::filesystem::path(path) / name).string();
    }
}
