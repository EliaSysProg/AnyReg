#include "AnyRegCore/AnyRegCore.hpp"
#include "AnyRegCore/WinError.hpp"

#include <algorithm>
#include <exception>
#include <format>
#include <iostream>
#include <print>
#include <ranges>
#include <stacktrace>
#include <thread>
#include <chrono>

#define TRACE(msg, ...) OutputDebugStringW(std::format(L"[{}:{} | {}] " msg L"\n", __FILEW__, __LINE__, __FUNCTIONW__, __VA_ARGS__).c_str())

template <typename Func>
static auto timeit(Func&& func)
{
    namespace chr = std::chrono;
    const auto start = chr::high_resolution_clock::now();
    std::invoke(std::forward<Func>(func));
    const auto end = chr::high_resolution_clock::now();

    return chr::duration_cast<chr::milliseconds>(end - start);
}

// static void print_stacktrace(const std::stacktrace& stacktrace)
// {
//     for (const auto& entry : stacktrace)
//     {
//         if (entry.source_file().empty()) continue;
//         std::println("{}:{}", entry.source_file(), entry.source_line());
//     }
// }

int main(const int /*argc*/, const char* const /*argv*/[])
{
    try
    {
        TRACE(L"Application started");
        anyreg::RegistryDatabase db;

        TRACE(L"Indexing");
        for (const auto hive : {
                 // HKEY_LOCAL_MACHINE,
                 HKEY_CURRENT_USER,
                 // HKEY_USERS,
                 // HKEY_CURRENT_CONFIG,
                 // HKEY_CLASSES_ROOT,
             })
        {
            anyreg::index_hive(db, hive);
        }

        if (!SetConsoleOutputCP(GetACP()))
        {
            throw anyreg::WinError();
        }

        static constexpr auto ichar_equals = [](const char a, const char b) -> bool
        {
            return std::tolower(static_cast<unsigned char>(a)) ==
                std::tolower(static_cast<unsigned char>(b));
        };

        TRACE(L"Getting input from user");
        std::string line;
        std::print(">> ");
        while (std::getline(std::cin, line) && line != ".quit")
        {
            const auto t = timeit([&]
            {
                const auto entries = db | std::views::filter(
                    [&](const std::pair<anyreg::RegistryId, anyreg::RegistryKeyEntry>& entry) -> bool
                    {
                        return std::search(entry.second.name.begin(), entry.second.name.end(),
                                           line.begin(), line.end(),
                                           ichar_equals) != entry.second.name.end();
                    }) | std::ranges::to<std::vector>();
                for (const auto& [id, entry] : entries | std::views::take(20))
                {
                    std::println("{}", anyreg::key_full_path(db, id));
                }
                std::println("Count: {}", entries.size());
            });

            std::println("Time: {}", t);
            std::print(">> ");
        }

        TRACE(L"Application finished gracefully");

        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::println("C++ Exception: {}", e.what());
    }
    catch (...)
    {
        std::println("An unknown exception occured!");
    }

    TRACE(L"Application aborted");
    return EXIT_FAILURE;
}
