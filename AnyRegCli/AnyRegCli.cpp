#include "AnyRegCore/RegistryDatabase.hpp"

#include <iostream>
#include <exception>
#include <print>
#include <ranges>

template <typename Func>
static auto timeit(Func&& func)
{
    namespace chr = std::chrono;
    const auto start = chr::high_resolution_clock::now();
    std::invoke(std::forward<Func>(func));
    const auto end = chr::high_resolution_clock::now();

    return chr::duration_cast<chr::microseconds>(end - start);
}

int main(const int argc, const char* const argv[])
{
    try
    {
        anyreg::RegistryDatabase db;
        if (argc == 2 && std::string(argv[1]) == "--index")
        {
            db.index({HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, HKEY_USERS, HKEY_CURRENT_CONFIG});
        }

        else
        {
            std::string line;
            std::vector<RegistryKeyEntry> entries;
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
        }

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

    std::cout.flush();
    return EXIT_FAILURE;
}
