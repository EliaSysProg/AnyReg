#include "RegistryDatabase.hpp"

#include "Registry.hpp"

#include <format>
#include <string>
#include <system_error>

RegistryDatabase::RegistryDatabase()
{
    _keys.reserve(0x10'000);
    _values.reserve(0x100'000);
}

void RegistryDatabase::index(const std::vector<HKEY>& hives)
{
    for (const auto hive : hives)
    {
        index(hive);
    }
}

void RegistryDatabase::index(const HKEY hive, std::wstring_view sub_path)
{
    std::vector<std::wstring> stack_keys;
    stack_keys.reserve(0x1000);
    // TODO: Add to DB
    stack_keys.emplace_back(std::begin(sub_path), std::end(sub_path));

    RegistryKeyEntry key_entry;
    RegistryValueEntry value_entry;
    std::wstring temp_name;

    while (!stack_keys.empty())
    {
        auto current_key = std::move(stack_keys.back());
        stack_keys.pop_back();

        RegistryKey key;

        try
        {
            key = RegistryKey(hive, current_key, KEY_READ);
        }
        catch (const std::system_error& e)
        {
            const auto error_code = e.code().value();
            if (error_code & (ERROR_ACCESS_DENIED | ERROR_PATH_NOT_FOUND))
            {
                continue;
            }
        }

        // Enumerate values
        for (DWORD i = 0; key.get_value(i, value_entry.name, value_entry.type); ++i)
        {
            value_entry.path = current_key;
            _values.push_back(value_entry);
        }

        // Enumerate subkeys
        for (DWORD i = 0; key.get_sub_key(i, key_entry.name, key_entry.last_write_time); ++i)
        {
            key_entry.path = current_key;
            _keys.push_back(key_entry);
            stack_keys.push_back(key_entry.get_full_path());
        }
    }
}
