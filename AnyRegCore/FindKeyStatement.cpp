#include "FindKeyStatement.hpp"

#include "RegistryDatabase.hpp"

namespace anyreg
{
    constexpr std::string column_name(const SortColumn sort_column)
    {
        switch (sort_column)
        {
        case SortColumn::NAME:
            return "Name";
        case SortColumn::PATH:
            return "ParentId";
        case SortColumn::LAST_WRITE_TIME:
            return "LastWriteTime";
        default:
            throw std::invalid_argument("Invalid SortColumn value");
        }
    }

    constexpr std::string order_name(const SortOrder sort_order)
    {
        switch (sort_order)
        {
        case SortOrder::ASCENDING:
            return "ASC";
        case SortOrder::DESCENDING:
            return "DESC";
        default:
            throw std::invalid_argument("Invalid SortOrder value");
        }
    }

    FindKeyStatement::FindKeyStatement(const RegistryDatabase& db,
                                       const SortColumn column,
                                       const SortOrder order,
                                       const std::string_view query)
        : _db(&db),
          _find_id_statement(db.get(), std::format(R"(
SELECT k.Id
    FROM RegistryKeys k
INNER JOIN RegistryKeys_fts fts ON k.Id = fts.rowid
    WHERE RegistryKeys_fts MATCH ?1
ORDER BY k.{} {};)", column_name(column), order_name(order)))

    {
        bind(query);
    }

    void FindKeyStatement::bind(const std::string_view query)
    {
        _find_id_statement.reset();
        _find_id_statement.bind_text(1, sql::query::fts_escape(query), true);
    }

    RegistryRecordRange FindKeyStatement::find()
    {
        std::vector<int64_t> ids;
        _find_id_statement.reset();
        while (_find_id_statement.step())
        {
            ids.push_back(_find_id_statement.get_int64(0));
        }

        return RegistryRecordRange{*_db, std::move(ids)};
    }
}
