#include "LikeStatement.hpp"

namespace anyreg
{
    LikeStatement::LikeStatement(const sql::DatabaseConnection& db, const SortColumn sort_column, const SortOrder sort_order)
        : FindStatementBase({db, query_string(sort_column, sort_order)}),
          _db(&db)
    {
    }

    void LikeStatement::bind_query(const std::string& query)
    {
        _query = sql::query::like_clause_escape(query, '|');
        FindStatementBase::bind_query(_query, false);
    }

    size_t LikeStatement::get_row_count() const
    {
        sql::Statement count_statement(*_db, "SELECT COUNT(*) FROM RegistryKeys WHERE Name LIKE '%' || ?1 || '%' escape '|'");
        count_statement.bind_text(1, _query);
        if (!count_statement.step())
        {
            throw std::runtime_error("Could not query count");
        }

        return count_statement.get_int64(0);
    }

    std::string LikeStatement::query_string(const SortColumn sort_column, const SortOrder sort_order)
    {
        return std::format(
            "SELECT Name, Hive, Path, LastWriteTime FROM RegistryKeys WHERE Name LIKE '%' || ?1 || '%' escape '|' ORDER BY {} {} LIMIT ?3 OFFSET ?2",
            to_string(sort_column), to_string(sort_order));
    }
}
