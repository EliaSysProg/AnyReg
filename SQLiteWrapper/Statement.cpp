#include "Statement.hpp"

namespace sql
{
void Statement::StatementDeleter::operator()(sqlite3_stmt* const statement) const noexcept
{
    sqlite3_finalize(statement);
}

Statement::Statement(const DatabaseConnection& db, const std::string_view sql)
{
    sqlite3_stmt* stmt = nullptr;
    const char* tail = nullptr;
    const auto error_code = sqlite3_prepare_v2(db.get(), sql.data(), static_cast<int>(sql.size() * sizeof(sql[0])), &stmt, &tail);
    if (error_code != SQLITE_OK)
    {
        throw StatementError(db.errmsg());
    }
    if (tail != sql.data() + sql.size())
    {
        throw StatementError("Tail remained on statement - not supported");
    }

    _statement.reset(stmt);
}

void Statement::bind_text(const int index, const std::string_view text, const bool take_copy)
{
    const auto error_code = sqlite3_bind_text(_statement.get(),
                                              index,
                                              text.data(), static_cast<int>(text.size() * sizeof(text[0])),
                                              take_copy ? SQLITE_TRANSIENT : SQLITE_STATIC);
    if (error_code != SQLITE_OK)
    {
        throw StatementError(error_code);
    }
}

void Statement::reset()
{
    const auto error_code = sqlite3_reset(_statement.get());
    if (error_code != SQLITE_OK)
    {
        throw StatementError(error_code);
    }
}

void Statement::step()
{
    sqlite3_step(_statement.get());
    // if (error_code != SQLITE_ROW)
    // {
    //     throw StatementError(error_code);
    // }
}
}
