#include "AnyRegCore/RegistryDatabase.hpp"
#include "AnyRegCore/RegistryIndexer.hpp"
#include "AnyRegCore/WinError.hpp"
#include "SQLite3Wrapper/DatabaseError.hpp"

#include <exception>
#include <format>
#include <iostream>
#include <print>
#include <ranges>
#include <stacktrace>
#include <thread>

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

static void print_stacktrace(const std::stacktrace& stacktrace)
{
    for (const auto& entry : stacktrace)
    {
        if (entry.source_file().empty()) continue;
        std::println("{}:{}", entry.source_file(), entry.source_line());
    }
}

int main(const int argc, const char* const argv[])
{
    try
    {
        TRACE(L"Application started");

        auto db = anyreg::RegistryDatabase::create();

        // Construct it here to avoid waiting for the thread to finish while backing up
        std::jthread save_thread;

        if (argc == 2 && std::string(argv[1]) == "--index")
        {
            auto db_indexing = anyreg::RegistryDatabase::open_write();
            auto indexer = anyreg::RegistryIndexer(db_indexing);
            TRACE(L"Indexing");
            for (const auto hive : {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, HKEY_USERS, HKEY_CURRENT_CONFIG, HKEY_CLASSES_ROOT})
            {
                indexer.index(reinterpret_cast<uint64_t>(hive));
            }

            save_thread = std::jthread([]
            {
                TRACE(L"Saving DB");
                anyreg::RegistryDatabase::open_read().save(L"AnyReg.db");
                TRACE(L"DB saved");
            });
        }

        if (!SetConsoleOutputCP(GetACP()))
        {
            throw anyreg::WinError();
        }

        const auto db_querying = anyreg::RegistryDatabase::open_read();
        auto find_statement = db_querying.find_keys(anyreg::SortColumn::PATH, anyreg::SortOrder::ASCENDING);
        find_statement.bind(0, 10);
        TRACE(L"Getting input from user");
        std::string line;
        std::print(">> ");
        while (std::getline(std::cin, line) && line != ".quit")
        {
            const auto t = timeit([&]
            {
                find_statement.bind(line);
                std::ranges::for_each(find_statement.find(),
                                      [](const anyreg::RegistryKeyView& entry) { std::println("{}", entry.full_path()); });
                find_statement.reset();
                std::println("Count: {}", db_querying.count_keys(line));
            });

            std::println("Time: {}", t);
            std::print(">> ");
        }

        TRACE(L"Application finished gracefully");

        return EXIT_SUCCESS;
    }
    catch (const sql::DatabaseError& e)
    {
        std::println("Database error: {}", e.what());
        print_stacktrace(e.stacktrace());
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
