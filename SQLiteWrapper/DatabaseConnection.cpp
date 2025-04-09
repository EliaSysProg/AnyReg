#include "DatabaseConnection.hpp"

namespace sql
{
DatabaseConnection::DatabaseConnection(const std::string_view file_name)
{
    sqlite3* db = nullptr;
    const auto error_code = sqlite3_open(file_name.data(), &db);
    if (error_code != SQLITE_OK)
    {
        throw ConnectionError(error_code);
    }

    _db.reset(db);
}

DatabaseConnection::DatabaseConnection(const std::string_view file_name, const int flags)
{
    sqlite3* db = nullptr;
    const auto error_code = sqlite3_open_v2(file_name.data(), &db, flags, nullptr);
    if (error_code != SQLITE_OK)
    {
        throw ConnectionError(error_code);
    }

    _db.reset(db);
}

void DatabaseConnection::DatabaseConnectionDeleter::operator()(sqlite3* db) const noexcept
{
    sqlite3_close(db);
}

void DatabaseConnection::execute(const std::string& sql, ExecCallback callback) const
{
    char* err_msg = nullptr;
    const int error_code = sqlite3_exec(_db.get(), sql.data(),
                                        [](void* context, const int column_count, char** values, char** column_names) -> int
                                        {
                                            if (context == nullptr)
                                                return -1;
                                            const auto function = static_cast<ExecCallback*>(context);
                                            try
                                            {
                                                (*function)(static_cast<uint32_t>(column_count), values, column_names);
                                                return SQLITE_OK;
                                            }
                                            catch (...)
                                            {
                                                return -2;
                                            }
                                        }, &callback, &err_msg);
    if (error_code != SQLITE_OK)
    {
        throw ConnectionError(err_msg);
    }
}

sqlite3* DatabaseConnection::get() const
{
    return _db.get();
}

const char* DatabaseConnection::errmsg() const
{
    return sqlite3_errmsg(_db.get());
}
}
