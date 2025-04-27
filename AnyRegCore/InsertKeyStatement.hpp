#pragma once

#include "RegistryEntry.hpp"
#include "SQLite3Wrapper/SQLite3Wrapper.hpp"

namespace anyreg
{
    class InsertKeyStatement final
    {
    public:
        explicit InsertKeyStatement(const sql::DatabaseConnection& db);

        void bind(const RegistryKeyEntry& entry);
        void execute();
        void reset_and_clear();

    private:
        sql::Statement _statement;
    };
}
