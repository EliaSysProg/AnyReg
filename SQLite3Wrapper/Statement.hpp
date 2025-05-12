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

        void bind_text(int index, std::string_view value, bool take_copy = false);
        void bind_int64(int index, int64_t value);
        void bind_int(int index, int value);

        void reset();
        void clear_bindings();
        bool step();
        void finalize();

        [[nodiscard]] std::string_view get_text(int index) const;
        [[nodiscard]] int32_t get_int32(int index) const;
        [[nodiscard]] int64_t get_int64(int index) const;

        [[nodiscard]] std::string get_sql() const;

    private:
        std::unique_ptr<sqlite3_stmt, SQLite3StatementDeleter> _sqlite3_stmt;
    };

    class StatementError final : public DatabaseError
    {
    public:
        using DatabaseError::DatabaseError;
    };
}
