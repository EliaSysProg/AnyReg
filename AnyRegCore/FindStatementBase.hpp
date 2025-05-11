#pragma once

#include "RegistryEntry.hpp"
#include "RegistryRecordIterator.hpp"
#include "SQLite3Wrapper/SQLite3Wrapper.hpp"

namespace anyreg
{
    class FindStatementBase
    {
    public:
        explicit FindStatementBase(sql::Statement statement);

        void bind_range(size_t offset, size_t limit);
        void reset();
        void clear_bindings();

        RegistryRecordIterator begin();
        RegistryRecordIterator end();

        [[nodiscard]] std::string get_sql() const;

    protected:
        void bind_query(const std::string& query, bool take_copy);

    private:
        sql::Statement _statement;
    };
}
