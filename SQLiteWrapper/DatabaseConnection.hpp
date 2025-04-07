#pragma once

#include "DatabaseError.hpp"
#include "SQLite/sqlite3.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace sql
{
class DatabaseConnection final
{
    struct DatabaseConnectionDeleter final
    {
        void operator()(sqlite3* db) const noexcept;
    };
    
public:
    // TODO: Get values and column names by random access iterators
    using ExecCallback = std::function<void(std::uint32_t /* column_count */,
                                            char** /* values */,
                                            char** /* column_names */)>;

    DatabaseConnection() = default;
    ~DatabaseConnection() = default;

    explicit DatabaseConnection(std::string_view file_name);
    DatabaseConnection(std::string_view file_name, int flags);

    DatabaseConnection(DatabaseConnection&& other) noexcept = default;
    DatabaseConnection& operator=(DatabaseConnection&& other) noexcept = default;

    DatabaseConnection(const DatabaseConnection& other) = delete;
    DatabaseConnection& operator=(const DatabaseConnection& other) = delete;

    void execute(const std::string& sql, ExecCallback callback = {}) const;

    [[nodiscard]] sqlite3* get() const;
    const char* errmsg() const; 

private:
    std::unique_ptr<sqlite3, DatabaseConnectionDeleter> _db;
};

class ConnectionError : public DatabaseError
{
public:
    using DatabaseError::DatabaseError;
};
}