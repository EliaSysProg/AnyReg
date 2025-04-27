#pragma once

#include "DatabaseError.hpp"
#include "SQLite3/sqlite3.h"

#include <memory>
#include <string>

#include "DatabaseConnection.hpp"

namespace sql
{
    class Statement
    {
        struct SQLite3StatementDeleter final
        {
            void operator()(sqlite3_stmt* stmt) const noexcept;
        };

    public:
        Statement(const DatabaseConnection& db, const std::string& sql);

        void bind_text(int index, const std::string& value, bool take_copy = false);
        void bind_int64(int index, int64_t value);
        void bind_int(int index, int value);

        void reset();
        void clear_bindings();
        int step();

        std::string get_text(int index) const;
        uint64_t get_int64(int index) const;

    private:
        std::unique_ptr<sqlite3_stmt, SQLite3StatementDeleter> _sqlite3_stmt;
    };

    class StatementError final : public DatabaseError
    {
    public:
        using DatabaseError::DatabaseError;
    };
}
