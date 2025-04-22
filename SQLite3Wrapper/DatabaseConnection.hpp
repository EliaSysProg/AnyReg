#pragma once

#include "DatabaseError.hpp"
#include "SQLite3/sqlite3.h"

#include <memory>
#include <string>

namespace sql
{
    class DatabaseConnection final
    {
        struct SQLite3Deleter final
        {
            void operator()(sqlite3* sqlite3) const noexcept;
        };

    public:
        DatabaseConnection() = default;
        explicit DatabaseConnection(const std::string& filename,
                                    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);

        void execute(const std::string& sql) const;

        [[nodiscard]] sqlite3* get() const; 
        [[nodiscard]] const char* errmsg() const; 

    private:
        std::unique_ptr<sqlite3, SQLite3Deleter> _sqlite3;
    };

    class ConnectionError final : public DatabaseError
    {
    public:
        using DatabaseError::DatabaseError;
    };
}
