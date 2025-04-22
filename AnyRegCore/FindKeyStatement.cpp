#include "FindKeyStatement.hpp"

#include <string>

namespace anyreg
{
    FindKeyStatement::FindKeyStatement(const sql::DatabaseConnection& db)
    : _statement(db, R"(
SELECT k.Name, k.Path, k.LastWriteTime
FROM RegistryKeys k
         INNER JOIN RegistryKeys_fts fts ON k.ID = fts.rowid
WHERE RegistryKeys_fts MATCH ?)")
{}

    void FindKeyStatement::bind(const std::string& user_query)
    {
        // The escaped query is temporary, so take a copy
        _statement.bind_text(1, sql::query::fts_escape(user_query), true);
    }

    bool FindKeyStatement::has_value() const
    {
        return _has_value;
    }

    RegistryKeyEntry FindKeyStatement::get_value()
    {
        RegistryKeyEntry entry{
            .name = _statement.get_text(0),
            .path = _statement.get_text(1),
            .last_write_time = RegistryKeyTime(std::chrono::file_clock::duration(_statement.get_int64(2)))
        };
        return entry;
    }

    void FindKeyStatement::step()
    {
        switch (const auto error_code = _statement.step())
        {
        case SQLITE_ROW:
            _has_value = true;
            break;
        case SQLITE_DONE:
            _has_value = false;
            break;
        default:
            throw sql::StatementError(error_code);
        }
    }

    void FindKeyStatement::reset_and_clear()
    {
        _statement.reset();
        _statement.clear_bindings();
    }
}
