#include "FindKeyStatement.hpp"

#include <string>

namespace anyreg
{
    FindKeyStatement::FindKeyStatement(const sql::DatabaseConnection& db)
        : _fts_statement(db,
                         R"(
SELECT k.Name, k.Path, k.LastWriteTime
FROM RegistryKeys k
         INNER JOIN RegistryKeys_fts fts ON k.ID = fts.rowid
WHERE RegistryKeys_fts MATCH ?1)"),
          _like_statement(db,
                          R"(
SELECT Name, Path, LastWriteTime
FROM RegistryKeys
WHERE Name LIKE '%' || ?1 || '%' escape '|')")
    {}

    void FindKeyStatement::bind(const std::string& user_query)
    {
        if (user_query.size() < 3)
        {
            _is_fts = false;
            _escaped_query = sql::query::like_clause_escape(user_query, '|');
        }
        else
        {
            _is_fts = true;
            _escaped_query = sql::query::fts_escape(user_query);
        }
        current_statement().bind_text(1, _escaped_query);
    }

    bool FindKeyStatement::has_value() const
    {
        return _has_value;
    }

    RegistryKeyEntry FindKeyStatement::get_value()
    {
        RegistryKeyEntry entry{
            .name = current_statement().get_text(0),
            .path = current_statement().get_text(1),
            .last_write_time = RegistryKeyTime(std::chrono::file_clock::duration(current_statement().get_int64(2)))
        };
        return entry;
    }

    void FindKeyStatement::step()
    {
        switch (const auto error_code = current_statement().step())
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
        current_statement().reset();
        current_statement().clear_bindings();
    }

    sql::Statement& FindKeyStatement::current_statement()
    {
        return _is_fts ? _fts_statement : _like_statement;
    }
}
