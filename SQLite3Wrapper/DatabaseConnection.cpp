#include "DatabaseConnection.hpp"

namespace sql
{
    void DatabaseConnection::SQLite3Deleter::operator()(sqlite3* const sqlite3) const noexcept
    {
        sqlite3_close(sqlite3);
    }

    DatabaseConnection::DatabaseConnection(const std::string& filename, const int flags)
    {
        sqlite3* db = nullptr;
        const auto error_code = sqlite3_open_v2(filename.c_str(), &db, flags, nullptr);
        if (error_code != SQLITE_OK)
        {
            throw ConnectionError(error_code);
        }

        _sqlite3.reset(db);
    }

    void DatabaseConnection::execute(const std::string& sql) const
    {
        char* err_msg = nullptr;
        const int error_code = sqlite3_exec(_sqlite3.get(), sql.data(), nullptr, nullptr, &err_msg);
        if (error_code != SQLITE_OK)
        {
            throw ConnectionError(err_msg);
        }
    }

    sqlite3* DatabaseConnection::get() const
    {
        return _sqlite3.get();
    }

    const char* DatabaseConnection::errmsg() const
    {
        return sqlite3_errmsg(_sqlite3.get());
    }

    const wchar_t* DatabaseConnection::errmsg16() const
    {
        return static_cast<const wchar_t*>(sqlite3_errmsg16(_sqlite3.get()));
    }

    int64_t DatabaseConnection::last_insert_rowid() const
    {
        const auto rowid = sqlite3_last_insert_rowid(_sqlite3.get());
        if (rowid == 0)
        {
            throw ConnectionError(errmsg());
        }

        return rowid;
    }
}
