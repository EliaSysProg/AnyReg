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

    void FindKeyStatement::reset_and_clear()
    {
        current_statement().reset();
        current_statement().clear_bindings();
    }

    sql::Statement& FindKeyStatement::current_statement()
    {
        return _is_fts ? _fts_statement : _like_statement;
    }

    FindKeyStatement::iterator::iterator(sql::Statement* statement)
        : _statement(statement)
    {
        ++*this; // Get the first element
    }

    RegistryKeyView FindKeyStatement::iterator::operator*() const
    {
        return _current;
    }

    FindKeyStatement::iterator& FindKeyStatement::iterator::operator++()
    {
        if (_statement)
        {
            switch (const auto error_code = _statement->step())
            {
            case SQLITE_ROW:
                _current = RegistryKeyView{
                    .name = _statement->get_text(0),
                    .path = _statement->get_text(1),
                    .last_write_time = RegistryKeyTime(std::chrono::file_clock::duration(_statement->get_int64(2)))
                };
                break;
            case SQLITE_DONE:
                _current = {};
                _statement = nullptr;
                break;
            default:
                throw sql::StatementError(error_code);
            }
        }

        return *this;
    }

    bool FindKeyStatement::iterator::operator!=(const iterator& other) const
    {
        return _statement != other._statement;
    }

    FindKeyStatement::iterator FindKeyStatement::begin()
    {
        return iterator(&current_statement());
    }

    FindKeyStatement::iterator FindKeyStatement::end()
    {
        return iterator();
    }
}
