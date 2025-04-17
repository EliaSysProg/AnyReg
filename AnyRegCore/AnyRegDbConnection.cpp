#include "AnyRegDbConnection.hpp"

#include "Registry.hpp"

#include <format>
#include <print>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

void AnyRegDbConnection::index(const std::vector<HKEY>& hives)
{
    for (const auto hive : hives)
    {
        index(hive);
    }
}

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
            value_entry.path = current_key;
            _values.push_back(value_entry);
            ++values_count;
        }

        // Enumerate subkeys
        for (DWORD i = 0; key.get_sub_key(i, key_entry.name, key_entry.last_write_time); ++i)
        {
            key_entry.path = current_key;
            _keys.push_back(key_entry);

            std::wstring sub_key_path = key_entry.path;
            if (!sub_key_path.empty() && !sub_key_path.ends_with(L"\\"))
            {
                sub_key_path.push_back(L'\\');
            }
            sub_key_path.append(key_entry.name);
            stack_keys.push_back(std::move(sub_key_path));
        }
    }
}

std::vector<RegistryKeyEntry> AnyRegDbConnection::find_key(const std::wstring_view query) const
{
    std::vector<RegistryKeyEntry> result;
    for (const auto& key_entry : _keys)
    {
        if (key_entry.name.contains(query))
        {
            result.push_back(key_entry);
            // if (result.size() > 1000)
            // {
            //     break;
            // }
        }
    }

    return result;
}

std::vector<RegistryKeyEntry> AnyRegDbConnection::keys() const
{
    return _keys /* | std::views::take(1000) | std::ranges::to<std::vector>()*/;
}
