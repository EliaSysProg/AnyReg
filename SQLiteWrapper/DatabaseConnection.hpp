#pragma once

#include "SQLite/sqlite3.h"

#include <functional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace sql
{
class DatabaseConnection final
{
public:
    // TODO: Get values and column names by random access iterators
    using ExecCallback = std::function<void(std::uint32_t /* column_count */,
                                            char** /* values */,
                                            char** /* column_names */)>;

    DatabaseConnection() = default;
    explicit DatabaseConnection(std::string_view file_name);
    ~DatabaseConnection();

    DatabaseConnection(DatabaseConnection&& other) noexcept;
    DatabaseConnection& operator=(DatabaseConnection&& other) noexcept;

    DatabaseConnection(const DatabaseConnection& other) = delete;
    DatabaseConnection& operator=(const DatabaseConnection& other) = delete;

    void execute(std::string sql, ExecCallback callback = {}) const;

private:
    sqlite3* _db = nullptr;
};

class ConnectionError final : public std::runtime_error
{
public:
    explicit ConnectionError(int error_code);
    explicit ConnectionError(std::string_view msg);
};
}
