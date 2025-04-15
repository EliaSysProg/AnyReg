#include "AnyRegDbConnection.hpp"
#include "Registry.hpp"

#include <iostream>
#include <exception>
#include <print>
#include <vector>
#include <ranges>

template <typename Func, typename... Args>
static auto timeit(Func&& func, Args&&... args)
{
    namespace chr = std::chrono;
    const auto start = chr::high_resolution_clock::now();
    std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
    const auto end = chr::high_resolution_clock::now();

    return chr::duration_cast<chr::milliseconds>(end - start);
}


int wmain(const int argc, const wchar_t* const argv[])
{
    try
    {
        AnyRegDbConnection db;

        const auto index_time = timeit([&]
        {
            for (const auto hive : {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, HKEY_USERS, HKEY_CURRENT_CONFIG})
            {
                db.index(hive);
            }
        });

        std::println("Index time: {}", index_time);

        if (argc > 1)
        {
            std::vector<RegistryKeyEntry> matches;
            const auto time = timeit([&] { matches = db.find_key(argv[1]) | std::ranges::to<std::vector>(); });
            std::println("Matches: {}. Took: {}", matches.size(), time);
        }

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
