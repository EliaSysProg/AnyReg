#include "AnyRegCore/RegistryDatabase.hpp"
#include "AnyRegCore/WinError.hpp"

#include <array>
#include <exception>
#include <format>
#include <future>
#include <iostream>
#include <print>
#include <ranges>

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

        std::jthread save_thread;

        if (argc == 2 && std::string(argv[1]) == "--index")
        {
            anyreg::RegistryDatabase db;
            TRACE(L"Indexing");
            db.index(std::array{
                HKEY_LOCAL_MACHINE,
                HKEY_CURRENT_USER,
                HKEY_USERS,
                HKEY_CURRENT_CONFIG,
                HKEY_CLASSES_ROOT,
            });

            save_thread = std::jthread([]
            {
                const anyreg::RegistryDatabase db(SQLITE_OPEN_READONLY);
                TRACE(L"Backing up");
                db.save(L"AnyReg.db");
                TRACE(L"Backed up");
            });
        }

        if (!SetConsoleOutputCP(GetACP()))
        {
            throw anyreg::WinError();
        }

        const anyreg::RegistryDatabase db;
        auto empty_query = db.get_empty_query(anyreg::SortColumn::PATH, anyreg::SortOrder::ASCENDING);
        auto like_query = db.get_like_query(anyreg::SortColumn::PATH, anyreg::SortOrder::ASCENDING);
        auto fts_query = db.get_fts_query(anyreg::SortColumn::PATH, anyreg::SortOrder::ASCENDING);
        empty_query.bind_range(0, 20);
        like_query.bind_range(0, 20);
        fts_query.bind_range(0, 20);
        TRACE(L"Getting input from user");
        std::string line;
        std::print(">> ");
        while (std::getline(std::cin, line) && line != ".quit")
        {
            const auto t = timeit([&]
            {
                anyreg::RegistryRecordIterator it;

                if (line.empty())
                {
                    empty_query.reset();
                    std::println("Executing: \"{}\"", empty_query.get_sql());
                    it = empty_query.begin();
                }
                else if (line.size() < 3)
                {
                    like_query.reset();
                    like_query.bind_query(line);
                    std::println("Executing: \"{}\"", like_query.get_sql());
                    it = like_query.begin();
                }
                else
                {
                    fts_query.reset();
                    fts_query.bind_query(line);
                    std::println("Executing: \"{}\"", fts_query.get_sql());
                    it = fts_query.begin();
                }

                std::for_each(it, {}, [](const anyreg::RegistryKeyView& entry) { std::println("{}", entry.get_full_path()); });
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
