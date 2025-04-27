#include "Statement.hpp"

#include <Windows.h>

namespace sql
{
    void Statement::SQLite3StatementDeleter::operator()(sqlite3_stmt* stmt) const noexcept
    {
        sqlite3_finalize(stmt);
    }

    Statement::Statement(const DatabaseConnection& db, const std::string& sql)
    {
        sqlite3_stmt* stmt = nullptr;
        const char* tail = nullptr;
        const auto error_code = sqlite3_prepare_v2(db.get(),
                                                   sql.data(),
                                                   static_cast<int>(sql.size() * sizeof(sql[0])),
                                                   &stmt,
                                                   &tail);
        if (error_code != SQLITE_OK)
        {
            throw StatementError(db.errmsg());
        }
        if (tail != sql.data() + sql.size())
        {
            throw StatementError("Tail remained on statement - not supported");
        }

        _sqlite3_stmt.reset(stmt);
    }

    void Statement::bind_text(const int index, const std::string& value, const bool take_copy)
    {
        const auto error_code = sqlite3_bind_text(_sqlite3_stmt.get(),
                                                  index,
                                                  value.c_str(),
                                                  static_cast<int>(value.size()),
                                                  take_copy ? SQLITE_TRANSIENT : SQLITE_STATIC);
        if (error_code != SQLITE_OK)
        {
            OutputDebugStringW((L"StatementError: " + std::to_wstring(error_code) + L"\n").c_str());
            throw StatementError(error_code);
        }
    }

    void Statement::bind_int64(const int index, const int64_t value)
    {
        const auto error_code = sqlite3_bind_int64(_sqlite3_stmt.get(), index, value);
        if (error_code != SQLITE_OK)
        {
            throw StatementError(error_code);
        }
    }

    void Statement::bind_int(const int index, const int value)
    {
        const auto error_code = sqlite3_bind_int(_sqlite3_stmt.get(), index, value);
        if (error_code != SQLITE_OK)
        {
            throw StatementError(error_code);
        }
    }

    void Statement::reset()
    {
        const auto error_code = sqlite3_reset(_sqlite3_stmt.get());
        if (error_code != SQLITE_OK)
        {
            throw StatementError(error_code);
        }
    }

    void Statement::clear_bindings()
    {
        const auto error_code = sqlite3_clear_bindings(_sqlite3_stmt.get());
        if (error_code != SQLITE_OK)
        {
            throw StatementError(error_code);
        }
    }

    int Statement::step()
    {
        // TODO: Handle error codes
        return sqlite3_step(_sqlite3_stmt.get());
    }

    std::string Statement::get_text(const int index) const
    {
        const auto text = sqlite3_column_text(_sqlite3_stmt.get(), index);
        const auto size = sqlite3_column_bytes(_sqlite3_stmt.get(), index);
        return {reinterpret_cast<const char*>(text), static_cast<size_t>(size)};
    }

    uint64_t Statement::get_int64(const int index) const
    {
        return sqlite3_column_int64(_sqlite3_stmt.get(), index);
    }
}
