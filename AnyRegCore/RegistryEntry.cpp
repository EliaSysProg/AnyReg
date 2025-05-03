#include "RegistryEntry.hpp"

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

    static std::string build_absolute_path(const HKEY hive, const std::string_view path)
    {
        std::string absolute_path;
        absolute_path.reserve(path.size() + 19); // 19 - Longest hive name

        absolute_path.append(hive_name(hive));
        if (!absolute_path.empty() && !path.empty())
        {
            absolute_path.push_back('\\');
        }

        absolute_path.append(path);
        return absolute_path;
    }

    static std::string build_full_path(const HKEY hive, const std::string_view path, const std::string_view name)
    {
        auto full_path = build_absolute_path(hive, path);
        if (!full_path.empty())
        {
            full_path.push_back('\\');
        }

        full_path.append(name);
        return full_path;
    }

    std::string RegistryKeyEntry::get_absolute_path() const
    {
        return build_absolute_path(hive, path);
    }

    std::string RegistryKeyEntry::get_full_path() const
    {
        return build_full_path(hive, path, name);
    }

    std::string RegistryKeyView::get_absolute_path() const
    {
        return build_absolute_path(hive, path);
    }

    std::string RegistryKeyView::get_full_path() const
    {
        return build_full_path(hive, path, name);
    }
}
