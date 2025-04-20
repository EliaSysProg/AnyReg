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

int main(const int argc, const char* const argv[])
{
    try
    {
        anyreg::RegistryDatabase db;
        db.index({HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, HKEY_USERS, HKEY_CURRENT_CONFIG});

        if (argc > 1)
        {
            UNREFERENCED_PARAMETER(argv);
            for (const auto& entry : db.find_keys(argv[1]))
            {
                std::println("{}", entry.get_full_path());
            }
            std::cout.flush();
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

    return EXIT_FAILURE;
}
