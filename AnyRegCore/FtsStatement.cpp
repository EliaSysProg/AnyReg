#include "FtsStatement.hpp"

namespace anyreg
{
    FtsStatement::FtsStatement(const sql::DatabaseConnection& db, const SortColumn sort_column, const SortOrder sort_order)
        : FindStatementBase({db, query_string(sort_column, sort_order)}),
          _db(&db)
    {
    }

    void FtsStatement::bind_query(const std::string& query)
    {
        _query = sql::query::fts_escape(query);
        FindStatementBase::bind_query(_query, false);
    }

    size_t FtsStatement::get_row_count() const
    {
        sql::Statement count_statement(
            *_db,
            "SELECT COUNT(*) FROM RegistryKeys k INNER JOIN RegistryKeys_fts fts ON k.ID = fts.rowid WHERE RegistryKeys_fts MATCH ?1");
        count_statement.bind_text(1, _query);
        if (!count_statement.step())
        {
            throw std::runtime_error("Could not query count");
        }

        return count_statement.get_int64(0);
    }

    std::string FtsStatement::query_string(const SortColumn sort_column, const SortOrder sort_order)
    {
        return std::format(
            "SELECT k.Name, k.Hive, k.Path, k.LastWriteTime FROM RegistryKeys k INNER JOIN RegistryKeys_fts fts ON k.ID = fts.rowid WHERE RegistryKeys_fts MATCH ?1 ORDER BY k.{} {} LIMIT ?3 OFFSET ?2",
            to_string(sort_column), to_string(sort_order));
    }
}
