#include "FindKeyStatement.hpp"

#include <string>


namespace anyreg
{
    static constexpr std::string to_string(const FindKeyStatement::SortColumn sort_column)
    {
        switch (sort_column)
        {
        case FindKeyStatement::SortColumn::NAME:
            return "Name";
        case FindKeyStatement::SortColumn::PATH:
            return "Path";
        case FindKeyStatement::SortColumn::LAST_WRITE_TIME:
            return "LastWriteTime";
        default:
            throw std::invalid_argument("Invalid SortColumn value");
        }
    }

    static constexpr std::string to_string(const FindKeyStatement::SortOrder sort_order)
    {
        switch (sort_order)
        {
        case FindKeyStatement::SortOrder::ASCENDING:
            return "ASC";
        case FindKeyStatement::SortOrder::DESCENDING:
            return "DESC";
        default:
            throw std::invalid_argument("Invalid SortOrder value");
        }
    }

    static std::string format_empty_query(const FindKeyStatement::SortColumn sort_column,
                                                    const FindKeyStatement::SortOrder sort_order,
                                                    const bool count_only)
    {
        const auto selection = count_only ? "COUNT(*)" : "Name, Hive, Path, LastWriteTime";
        return std::format("SELECT {} FROM RegistryKeys ORDER BY {} {}", selection, to_string(sort_column), to_string(sort_order));
    }

    static std::string format_like_query(const std::string& query,
                                                   const FindKeyStatement::SortColumn sort_column,
                                                   const FindKeyStatement::SortOrder sort_order,
                                                   const bool count_only)
    {
        const auto selection = count_only ? "COUNT(*)" : "Name, Hive, Path, LastWriteTime";
        const auto escaped_query = sql::query::like_clause_escape(query, '|');
        return std::format("SELECT {} FROM RegistryKeys WHERE Name LIKE '%{}%' escape '|' ORDER BY {} {}",
                           selection, escaped_query, to_string(sort_column), to_string(sort_order));
    }

    static std::string format_fts_query(const std::string& query,
                                                  const FindKeyStatement::SortColumn sort_column,
                                                  const FindKeyStatement::SortOrder sort_order,
                                                  const bool count_only)
    {
        const auto selection = count_only ? "COUNT(*)" : "k.Name, k.Hive, k.Path, k.LastWriteTime";
        const auto escaped_query = sql::query::fts_escape(query);
        return std::format(
            "SELECT {} FROM RegistryKeys k INNER JOIN RegistryKeys_fts fts ON k.ID = fts.rowid WHERE RegistryKeys_fts MATCH '{}' ORDER BY k.{} {}",
            selection, escaped_query, to_string(sort_column), to_string(sort_order));
    }

    static std::string format_query(const std::string& query,
                                    const FindKeyStatement::SortColumn sort_column,
                                    const FindKeyStatement::SortOrder sort_order,
                                    size_t offset = 0,
                                    size_t limit = std::numeric_limits<uint32_t>::max(),
                                    const bool count_only = false)
    {
        std::string sql;
        if (query.empty())
        {
            sql = format_empty_query(sort_column, sort_order, count_only);
        }
        else if (query.size() < 3)
        {
            sql = format_like_query(query, sort_column, sort_order, count_only);
        }
        else
        {
            sql = format_fts_query(query, sort_column, sort_order, count_only);
        }

        if (count_only)
        {
            return sql;
        }

        return std::format("{} LIMIT {} OFFSET {}", sql, limit, offset);
    }


    static sql::Statement format_query(const sql::DatabaseConnection& db,
                                       const std::string& query,
                                       const FindKeyStatement::SortColumn sort_column,
                                       const FindKeyStatement::SortOrder sort_order,
                                       const size_t offset = 0,
                                       const size_t limit = std::numeric_limits<uint32_t>::max(),
                                       const bool count_only = false)
    {
        const auto sql_string = format_query(query, sort_column, sort_order, offset, limit, count_only);
        return {db, sql_string};
    }

    FindKeyStatement::FindKeyStatement(const sql::DatabaseConnection& db,
                                       const std::string& user_query,
                                       const SortColumn sort_column,
                                       const SortOrder order)
        : _statement(format_query(db, user_query, sort_column, order))
    {
    }

    FindKeyStatement::FindKeyStatement(const sql::DatabaseConnection& db,
                                       const std::string& user_query,
                                       const SortColumn sort_column,
                                       const SortOrder order,
                                       const size_t offset,
                                       const size_t limit)
        : _statement(format_query(db, user_query, sort_column, order, offset, limit))
    {
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
        if (!_statement)
        {
            return *this;
        }

        if (_statement->step())
        {
            _current = RegistryKeyView{
                .name = _statement->get_text(0),
                .hive = reinterpret_cast<HKEY>(static_cast<ULONG_PTR>(_statement->get_int64(1))),
                .path = _statement->get_text(2),
                .last_write_time = RegistryKeyTime(std::chrono::file_clock::duration(_statement->get_int64(3)))
            };
        }
        else
        {
            _current = {};
            _statement = nullptr;
        }

        return *this;
    }

    bool FindKeyStatement::iterator::operator!=(const iterator& other) const
    {
        return _statement != other._statement;
    }

    FindKeyStatement::iterator FindKeyStatement::begin()
    {
        return iterator(&_statement);
    }

    FindKeyStatement::iterator FindKeyStatement::end()
    {
        return iterator();
    }

    size_t FindKeyStatement::get_count(const sql::DatabaseConnection& db,
                                       const std::string& user_query,
                                       const SortColumn sort_column,
                                       const SortOrder order)
    {
        auto statement = format_query(db, user_query, sort_column, order, 0, std::numeric_limits<uint32_t>::max(), true);
        if (!statement.step())
        {
            throw std::runtime_error("Failed to execute count query");
        }

        const auto count = statement.get_int64(0);
        statement.reset();
        statement.clear_bindings();

        return static_cast<size_t>(count);
    }
}
