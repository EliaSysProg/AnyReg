#include "EmptyStatement.hpp"

#include "FindStatementBase.hpp"

namespace anyreg
{
    EmptyStatement::EmptyStatement(const sql::DatabaseConnection& db, const SortColumn sort_column, const SortOrder sort_order)
        : FindStatementBase({db, query_string(sort_column, sort_order)}),
          _db(&db)
    {
    }

    size_t EmptyStatement::get_row_count() const
    {
        sql::Statement count_statement(*_db, "SELECT COUNT(*) FROM RegistryKeys");
        if (!count_statement.step())
        {
            throw std::runtime_error("Could not query count");
        }

        return count_statement.get_int64(0);
    }


    std::string EmptyStatement::query_string(const SortColumn sort_column, const SortOrder sort_order)
    {
        return std::format("SELECT Name, Hive, Path, LastWriteTime FROM RegistryKeys ORDER BY {} {} LIMIT ?3 OFFSET ?2",
                           to_string(sort_column),
                           to_string(sort_order));
    }
}
