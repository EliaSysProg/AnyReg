#include "DatabaseConnection.hpp"

namespace sql
{
DatabaseConnection::DatabaseConnection(const std::string_view file_name)
{
    const auto error_code = sqlite3_open(file_name.data(), &_db);
    if (error_code != SQLITE_OK)
    {
        throw ConnectionError(error_code);
    }
}

DatabaseConnection::~DatabaseConnection()
{
    // From docs [https://www.sqlite.org/c3ref/close.html]:
    // Calling sqlite3_close() or sqlite3_close_v2() with a NULL pointer argument is a harmless no-op.
    sqlite3_close(_db);
}

DatabaseConnection::DatabaseConnection(DatabaseConnection&& other) noexcept
    : _db(std::exchange(other._db, nullptr))
{
}

DatabaseConnection& DatabaseConnection::operator=(DatabaseConnection&& other) noexcept
{
    std::swap(_db, other._db);
    return *this;
}

void DatabaseConnection::execute(std::string sql, ExecCallback callback) const
{
    char* err_msg = nullptr;
    const int error_code = sqlite3_exec(_db, sql.data(),
                                        [](void* context, const int column_count, char** values, char** column_names) -> int
                                        {
                                            const auto function = static_cast<ExecCallback*>(context);
                                            try
                                            {
                                                (*function)(static_cast<uint32_t>(column_count), values, column_names);
                                                return SQLITE_OK;
                                            }
                                            catch (...)
                                            {
                                                return -1;
                                            }
                                        }, &callback, &err_msg);
    if (error_code != SQLITE_OK)
    {
        throw ConnectionError(err_msg);
    }
}

ConnectionError::ConnectionError(const int error_code)
    : std::runtime_error(sqlite3_errstr(error_code))
{
}

ConnectionError::ConnectionError(const std::string_view msg)
    : std::runtime_error({msg.data(), msg.size()})
{
}
}
