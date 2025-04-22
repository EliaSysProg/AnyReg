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
        sql::Statement _statement;
        bool _has_value = false;
    };
}
