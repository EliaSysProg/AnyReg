#include "Pch.hpp"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <regex>
#include <ranges>

static std::vector<std::string> keys;
static std::unordered_map<std::string, std::vector<std::string>> values;

// ----------------------- Registry Enumeration ----------------------- //
static void enumerate_registry(const HKEY root, const std::string& path)
{
    HKEY h_key;
    if (RegOpenKeyExA(root, path.c_str(), 0, KEY_READ, &h_key) != ERROR_SUCCESS)
        return;

    // Add key to Radix Tree
    // std::cout << "Key: " << path << '\n';
    keys.emplace_back(path);

    // Enumerate values
    char value_name[256];
    DWORD value_name_size;
    DWORD index_val = 0;
    while (true)
    {
        value_name_size = sizeof(value_name);
        if (RegEnumValueA(h_key, index_val++, value_name, &value_name_size, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
            break;
        if (!values.contains(value_name))
        {
            values.emplace(value_name, std::vector<std::string>());
        }
        // std::cout << "Value: " << path << '\n';
        values[value_name].emplace_back(path);
    }

    // Enumerate subkeys
    char sub_key_name[256];
    DWORD sub_key_size;
    DWORD index_key = 0;
    while (true)
    {
        sub_key_size = sizeof(sub_key_name);
        if (RegEnumKeyExA(h_key, index_key++, sub_key_name, &sub_key_size, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
            break;
        enumerate_registry(root, path + "\\" + sub_key_name);
    }

    RegCloseKey(h_key);
}

static auto find_keys(std::string_view substring)
{
    auto range = keys | std::views::filter([&substring](const std::string& item) { return item.contains(substring); });
    return range;
}

// ----------------------- Example Usage ----------------------- //
int main()
{
    std::cout << "Indexing registry...\n";
    enumerate_registry(HKEY_LOCAL_MACHINE, "SOFTWARE\\Adobe");
    std::cout << "Keys: " << keys.size() << "\n";
    std::cout << "Values: " << values.size() << "\n";

    std::cout << "Searching for keys... sent\n";

    auto matches = find_keys("sent");
    const auto vec = std::vector(matches.begin(), matches.end());
    for (const auto& key : vec)
        std::cout << key.c_str() << "\n";
    // enumerate_registry(HKEY_USERS, "");

    // std::cout << "\nSearching for registry keys matching '.*qtproject.*':\n";
    // for (const auto& match : registry_tree.search_regex(".*qtproject.*"))
    // {
    //     std::cout << " - " << match << "\n";
    // }

    // std::cout << "\nSearching for values matching '[Tt]est.*':\n";
    // for (const auto& match : value_index.search_regex("[Tt]est.*"))
    // {
    //     std::cout << " - " << match << "\n";
    // }

    return 0;
}
