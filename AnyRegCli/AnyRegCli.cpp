#include "AnyRegCore/RegistryDatabase.hpp"

#include <iostream>
#include <exception>
#include <print>

template <typename Func>
static auto timeit(Func&& func)
{
    namespace chr = std::chrono;
    const auto start = chr::high_resolution_clock::now();
    std::invoke(std::forward<Func>(func));
    const auto end = chr::high_resolution_clock::now();

    return chr::duration_cast<chr::milliseconds>(end - start);
}

int wmain(const int argc, const wchar_t* const argv[])
{
    try
    {
        RegistryDatabase db;
        db.index({HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, HKEY_USERS, HKEY_CURRENT_CONFIG});

        if (argc > 1)
        {
            for (const auto& [name, path, last_write_time] : db.find_key(argv[1]))
            {
                std::wcout << path << L'\\' << name << L'\n';
            }
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
