#include "FindKeyStatement.hpp"

namespace anyreg
{
    FindKeyStatement::FindKeyStatement(const sql::DatabaseConnection& db)
    : _statement(db, R"(
WITH search_results AS (
    -- For queries with 3+ characters, use FTS5 for speed
    SELECT k.Name, k.Path, k.LastWriteTime
    FROM RegistryKeys k
    INNER JOIN RegistryKeys_fts fts ON k.ID = fts.rowid
    WHERE LENGTH(?1) >= 3 AND RegistryKeys_fts MATCH ?1
    
    UNION ALL
    
    -- For short queries (1-2 chars), fall back to LIKE search
    -- Only run this part when search term is short
    SELECT Name, Path, LastWriteTime
    FROM RegistryKeys
    WHERE LENGTH(?1) < 3 AND (Name LIKE '%' || ?1 || '%')
    -- Limit results for short searches to avoid performance issues
    LIMIT 100
)
SELECT Name, Path, LastWriteTime
FROM search_results
ORDER BY Name;)")
{}

    void FindKeyStatement::bind(const std::string& query)
    {
        _statement.bind_text(1, query);
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