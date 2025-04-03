#include "Pch.hpp"

#include "AnyRegDB.hpp"
#include "SQLiteWrapper/DatabaseConnection.hpp"

#include <iostream>
#include <vector>
#include <regex>
#include <ranges>


// static void enumerate_registry(const HKEY root, const std::string& path, AnyRegDB& db)
// {
//     HKEY h_key;
//     if (RegOpenKeyExA(root, path.c_str(), 0, KEY_READ, &h_key) != ERROR_SUCCESS)
//         return;
//
//     // Add key to Radix Tree
//     // std::cout << "Key: " << path << '\n';
//     db.insert_key(path);
//
//     // Enumerate values
//     char value_name[256];
//     DWORD value_name_size;
//     DWORD index_val = 0;
//     while (true)
//     {
//         value_name_size = sizeof(value_name);
//         if (RegEnumValueA(h_key, index_val++, value_name, &value_name_size, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
//             break;
//         if (!values.contains(value_name))
//         {
//             values.emplace(value_name, std::vector<std::string>());
//         }
//         // std::cout << "Value: " << path << '\n';
//         values[value_name].emplace_back(path);
//     }
//
//     // Enumerate subkeys
//     char sub_key_name[256];
//     DWORD sub_key_size;
//     DWORD index_key = 0;
//     while (true)
//     {
//         sub_key_size = sizeof(sub_key_name);
//         if (RegEnumKeyExA(h_key, index_key++, sub_key_name, &sub_key_size, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
//             break;
//         enumerate_registry(root, path + "\\" + sub_key_name);
//     }
//
//     RegCloseKey(h_key);
// }


// ----------------------- Example Usage ----------------------- //
int main()
{
    try
    {
        AnyRegDB db(R"(C:\Windows\Temp\AnyReg.db)");
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (...)
    {
        std::cerr << "An unknown exception occured!\n";
    }

    return EXIT_FAILURE;
}
