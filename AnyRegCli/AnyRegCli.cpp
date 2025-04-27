#include "AnyRegCore/RegistryDatabase.hpp"

#include <iostream>
#include <exception>
#include <future>
#include <print>
#include <ranges>

#define TRACE(msg) OutputDebugStringW(msg L"\r\n")

template <typename Func>
static auto timeit(Func&& func)
{
    namespace chr = std::chrono;
    const auto start = chr::high_resolution_clock::now();
    std::invoke(std::forward<Func>(func));
    const auto end = chr::high_resolution_clock::now();

    return chr::duration_cast<chr::milliseconds>(end - start);
}

int main(const int argc, const char* const argv[])
{
    try
    {
        anyreg::RegistryDatabase db;
        std::jthread save_thread;

        if (argc == 2 && std::string(argv[1]) == "--index")
        {
            TRACE(L"Indexing");
            db.index({
                HKEY_LOCAL_MACHINE,
                HKEY_CURRENT_USER,
                HKEY_USERS,
                HKEY_CURRENT_CONFIG,
                HKEY_CLASSES_ROOT,
            });

            save_thread = std::jthread([&db]
            {
                TRACE(L"Backing up");
                db.save(LR"(C:\Windows\Temp\AnyReg.db)");
                TRACE(L"Backed up");
            });
        }


        TRACE(L"Getting input from user");
        std::string line;
        std::vector<anyreg::RegistryKeyEntry> entries;
        std::print(">> ");
        while (std::getline(std::cin, line))
        {
            const auto time = timeit([&] { entries = db.find_keys(line); });
            for (const auto& entry : entries | std::views::take(10))
            {
                std::println("{}", entry.get_full_path());
            }
            std::println("Results: {}. Time: {}", entries.size(), time);
            std::print(">> ");
            std::cout.flush();
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
    std::cout.flush();
    return EXIT_FAILURE;
}
