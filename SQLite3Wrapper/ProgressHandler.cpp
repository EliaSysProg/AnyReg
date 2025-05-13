#include "ProgressHandler.hpp"

namespace sql
{
    ProgressHandler::ProgressHandler(const DatabaseConnection& db, std::function<bool()> callback, const int instructions_interval)
        : _db(&db),
          _callback(std::move(callback))
    {
        sqlite3_progress_handler(db.get(), instructions_interval, [](void* arg) -> int
        {
            try
            {
                const auto func = *static_cast<std::function<bool()>*>(arg);
                return func();
            }
            catch (...)
            {
                return EXIT_FAILURE;
            }
        }, &_callback);
    }

    ProgressHandler::~ProgressHandler()
    {
        try
        {
            sqlite3_progress_handler(_db->get(), 0, nullptr, nullptr);
        }
        catch (...)
        {
        }
    }
}
