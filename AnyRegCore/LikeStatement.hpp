#pragma once

#include "FindStatementBase.hpp"
#include "SQLite3Wrapper/SQLite3Wrapper.hpp"

namespace anyreg
{
    class LikeStatement : public FindStatementBase
    {
    public:
        LikeStatement(const sql::DatabaseConnection& db, SortColumn sort_column, SortOrder sort_order);

        void bind_query(const std::string& query);

        [[nodiscard]] size_t get_row_count() const;

    private:
        static std::string query_string(SortColumn sort_column, SortOrder sort_order);
        
        const sql::DatabaseConnection* _db;
        std::string _query;
    };
}
