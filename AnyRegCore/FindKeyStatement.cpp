#include "FindKeyStatement.hpp"

#include <string>


namespace anyreg
{
    sql::Statement format_query(const sql::DatabaseConnection& db,
                                const std::string& query,
                                const FindKeyStatement::SortColumn sort_column,
                                const FindKeyStatement::SortOrder sort_order)
    {
        std::string column;
        switch (sort_column)
        {
        case FindKeyStatement::SortColumn::NAME:
            column = "Name";
            break;
        case FindKeyStatement::SortColumn::PATH:
            column = "Path";
            break;
        case FindKeyStatement::SortColumn::LAST_WRITE_TIME:
            column = "LastWriteTime";
            break;
        default:
            column = "Name";
            break;
        }

        const std::string order = sort_order == FindKeyStatement::SortOrder::ASCENDING ? "ASC" : "DESC";
        const std::string order_by = std::format("{} {}", column, order);

        std::string sql_string;
        std::string escaped_query;
        bool should_bind;
        if (query.empty())
        {
            sql_string = std::format("SELECT Name, Path, LastWriteTime FROM RegistryKeys ORDER BY {}", order_by);
            escaped_query = query;
            should_bind = false;
        }
        else if (query.size() < 3)
        {
            sql_string = std::format(
                "SELECT Name, Path, LastWriteTime FROM RegistryKeys WHERE Name LIKE '%' || ?1 || '%' escape '|' ORDER BY {}",
                order_by);
            escaped_query = sql::query::like_clause_escape(query, '|');
            should_bind = true;
        }
        else
        {
            sql_string = std::format(
                "SELECT k.Name, k.Path, k.LastWriteTime FROM RegistryKeys k INNER JOIN RegistryKeys_fts fts ON k.ID = fts.rowid WHERE RegistryKeys_fts MATCH ?1 ORDER BY k.{}",
                order_by);
            escaped_query = sql::query::fts_escape(query);
            should_bind = true;
        }

        sql::Statement statement(db, sql_string);
        if (should_bind)
        {
            statement.bind_text(1, escaped_query, true /* take_copy */);
        }

        return statement;
    }

    FindKeyStatement::FindKeyStatement(const sql::DatabaseConnection& db)
        : _db(&db),
          _statement(format_query(db, "", SortColumn::PATH, SortOrder::ASCENDING))
    {
    }

    void FindKeyStatement::bind(const std::string& user_query, const SortColumn sort_column, const SortOrder order)
    {
        _statement.finalize();
        _statement = format_query(*_db, user_query, sort_column, order);
    }

    void FindKeyStatement::reset_and_clear()
    {
        current_statement().reset();
        current_statement().clear_bindings();
    }

    sql::Statement& FindKeyStatement::current_statement()
    {
        return _statement;
    }

    FindKeyStatement::iterator::iterator(sql::Statement* statement)
        : _statement(statement)
    {
        ++*this; // Get the first element
    }

    const RegistryKeyView& FindKeyStatement::iterator::operator*() const
    {
        return _current;
    }

    const RegistryKeyView* FindKeyStatement::iterator::operator->() const
    {
        return &_current;
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
