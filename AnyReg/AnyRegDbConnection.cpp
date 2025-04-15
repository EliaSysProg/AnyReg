#include "AnyRegDbConnection.hpp"

#include "Registry.hpp"

#include <format>
#include <print>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

void AnyRegDbConnection::index(const HKEY root, std::wstring_view sub_path)
{
    std::vector<std::wstring> stack_keys;
    size_t keys_count = 0;
    size_t values_count = 0;
    stack_keys.reserve(0x1000);
    // TODO: Add to DB
    stack_keys.emplace_back(std::begin(sub_path), std::end(sub_path));

    RegistryKeyEntry key_entry;
    RegistryValueEntry value_entry;
    std::wstring temp_name;

    while (!stack_keys.empty())
    {
        if (keys_count % 10000 == 0)
        {
            std::println("Keys: {}", keys_count);
            std::println("Values: {}", values_count);
        }

        auto current_key = std::move(stack_keys.back());
        stack_keys.pop_back();

        if (!current_key.empty() && current_key.back() != L'\\')
            current_key.push_back(L'\\');

        RegistryKey key;

        try
        {
            key = RegistryKey(root, current_key, KEY_READ);
        }
        catch (const std::system_error& e)
        {
            const auto error_code = e.code().value();
            if (error_code & (ERROR_ACCESS_DENIED | ERROR_PATH_NOT_FOUND))
            {
                continue;
            }
        }

        ++keys_count;

        // Enumerate values
        for (DWORD i = 0; key.get_value(i, value_entry.name, value_entry.type); ++i)
        {
            value_entry.key = current_key;
            _values.push_back(value_entry);
            ++values_count;
        }

        // Enumerate subkeys
        for (DWORD i = 0; key.get_sub_key(i, temp_name, key_entry.last_write_time); ++i)
        {
            key_entry.path = std::format(L"{}{}", current_key, temp_name);
            _keys.push_back(key_entry);
            stack_keys.push_back(key_entry.path);
        }
    }
}
