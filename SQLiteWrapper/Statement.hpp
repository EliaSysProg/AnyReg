#pragma once

#include "DatabaseConnection.hpp"
#include "SQLite/sqlite3.h"

#include <memory>

namespace sql
{
class Statement final
{
    struct StatementDeleter final
    {
        void operator()(sqlite3_stmt* statement) const noexcept;
    };

public:
    Statement() = default;
    ~Statement() = default;

    Statement(const DatabaseConnection& db, std::string_view sql);

    Statement(Statement&& other) noexcept = default;
    Statement& operator=(Statement&& other) noexcept = default;

    Statement(const Statement& other) = delete;
    Statement& operator=(const Statement& other) = delete;

    void bind_text(int index, std::string_view value, bool take_copy = true);
    void bind_int64(int index, int64_t value);
    void bind_int(int index, int value);
    void reset();
    void clear_bindings();
    int step();

private:
    std::unique_ptr<sqlite3_stmt, StatementDeleter> _statement;
};

class StatementError final : public DatabaseError
{
public:
    using DatabaseError::DatabaseError;
};
}
