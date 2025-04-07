#include "Pch.hpp"

#include "AnyRegDB.hpp"
#include "RegistryKey.hpp"
#include "SQLiteWrapper/DatabaseConnection.hpp"

#include <filesystem>
#include <print>
#include <array>
#include <algorithm>
#include <ranges>

static constexpr size_t MAX_REGISTRY_KEY_NAME = 255 + 1;

static size_t keys_count = 0;
static size_t values_count = 0;

static void enumerate_registry(const HKEY root, const std::string& root_path, AnyRegDB& db)
{
    std::vector<std::string> stack;
    stack.reserve(0x5000);
    stack.push_back(root_path);

    std::array<char, MAX_REGISTRY_KEY_NAME> string_buffer{};

    while (!stack.empty())
    {
        if (stack.size() % 0x1000 == 0)
        {
            std::println("Stack size: {}", stack.size());
        }

        const auto current_path = std::move(stack.back());
        stack.pop_back();
        RegistryKey key;

        try
        {
            key = RegistryKey(root, current_path, KEY_READ);
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
        db.insert_key(current_path);

        // Enumerate values
        for (DWORD i = 0; key.get_value(i, string_buffer); ++i)
        {
            if (strlen(string_buffer.data()) == 0)
                std::ranges::copy("(Default)", string_buffer.begin());

            ++values_count;
            db.insert_value(current_path, string_buffer.data());
        }

        // Enumerate subkeys
        for (DWORD i = 0; key.get_sub_key(i, string_buffer); ++i)
        {
            auto sub_path = std::format("{}{}\\", current_path, string_buffer.data());
            stack.push_back(std::move(sub_path));
        }
    }
}


// ----------------------- Example Usage ----------------------- //
int main()
{
    try
    {
        AnyRegDB db(R"(C:\Windows\Temp\AnyReg.db)", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
        enumerate_registry(HKEY_LOCAL_MACHINE, "SYSTEM\\", db);
        std::println("Keys: {}", keys_count);
        std::println("Values: {}", values_count);
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
