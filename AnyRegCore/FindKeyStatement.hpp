#pragma once

#include "RegistryEntry.hpp"
#include "SQLite3Wrapper/SQLite3Wrapper.hpp"

namespace anyreg
{
    class FindKeyStatement final
    {
    public:
        explicit FindKeyStatement(const sql::DatabaseConnection& db);

        void bind(const std::string& user_query);
        [[nodiscard]] bool has_value() const;
        RegistryKeyEntry get_value();
        void step();
        void reset_and_clear();

    private:
        sql::Statement& current_statement();
        
        sql::Statement _fts_statement;
        sql::Statement _like_statement;
        std::string _escaped_query;
        bool _is_fts = false;
        bool _has_value = false;
    };
}
