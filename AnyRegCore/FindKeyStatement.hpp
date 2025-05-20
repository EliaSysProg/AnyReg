#pragma once

#include "RegistryRecordRange.hpp"
#include "SQLite3Wrapper/SQLite3Wrapper.hpp"

namespace anyreg
{
    class RegistryDatabase;

    class FindKeyStatement final
    {
    public:
        FindKeyStatement(const RegistryDatabase& db, SortColumn column, SortOrder order);

        void bind(std::string_view query);
        void reset();

        [[nodiscard]] RegistryRecordRange find();

    private:
        const RegistryDatabase* _db;
        sql::Statement _find_id_statement;
    };
}
