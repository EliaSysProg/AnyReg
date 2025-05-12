#include "DatabaseError.hpp"

#include "SQLite3/sqlite3.h"

namespace sql
{
    DatabaseError::DatabaseError(const int error_code)
        : DatabaseError(sqlite3_errstr(error_code))
    {
    }

    DatabaseError::DatabaseError(const std::string_view msg)
        : std::runtime_error(msg.data()),
          _stacktrace(std::stacktrace::current())
    {
    }

    const std::stacktrace& DatabaseError::stacktrace() const noexcept
    {
        return _stacktrace;
    }
}
